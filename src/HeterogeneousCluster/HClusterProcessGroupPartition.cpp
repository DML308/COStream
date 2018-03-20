#include "HClusterProcessGroupPartition.h"

HClusterProcessGroupPartition::HClusterProcessGroupPartition(SchedulerSSG * _sssg, int _nClusters):m_sssg(_sssg),m_nClusters(_nClusters)
{
	std::vector<FlatNode *> flatNodes = _sssg->GetFlatNodes();
	m_avgWorkload = computeWorkload(flatNodes) / m_nClusters;//计算理论上的平均负载

	constructGroups();
	//为邻接矩阵m_groupAdjMatrix开辟空间
	m_groupAdjMatrix = new int *[m_groups.size()];
	for (int i = 0; i != m_groups.size(); i++)
		m_groupAdjMatrix[i] = new int[m_groups.size()];
	for (int i = 0; i != m_groups.size(); i++)
		for (int j = 0; j != m_groups.size(); j++)
			m_groupAdjMatrix[i][j] = 0;
	m_groupAdjMatrix_copy = new int *[m_groups.size()];
	for (int i = 0; i != m_groups.size(); i++)
		m_groupAdjMatrix_copy[i] = new int[m_groups.size()];
	for (int i = 0; i != m_groups.size(); i++)
		for (int j = 0; j != m_groups.size(); j++)
			m_groupAdjMatrix_copy[i][j] = 0;
}
HClusterProcessGroupPartition::~HClusterProcessGroupPartition()
{
	for (int i = 0; i != m_groups.size(); i++)
	{
		delete[]m_groupAdjMatrix[i];
	}
	delete[]m_groupAdjMatrix;

	for (int i = 0; i != m_groups.size(); i++)
	{
		delete[]m_groupAdjMatrix_copy[i];
	}
	delete[]m_groupAdjMatrix_copy;
}

map<int, vector<FlatNode*>> HClusterProcessGroupPartition::processPartition()
{
	//std::cout << "异构集群后端--组划分---预处理阶段：" << endl;
	preProcess();
	//std::cout << "异构集群后端---组划分---粗粒度阶段：" << endl;
	coarseGrainedPartition();
	//std::cout << "异构集群后端---组划分---初始划分阶段：" << endl;
	initialPartition();
	//cout << "异构集群后端---组划分---细化调整阶段：" << endl;
	fineGrainedPartition();


	map<int, vector<FlatNode*>> hCluster2FlatNodes;
	for (int i = 0; i < m_nClusters; i++)
	{
		std::pair<std::multimap<int, FlatNode *>::iterator, std::multimap<int, FlatNode *>::iterator> ret = m_cluster2flatNode.equal_range(i);
		std::vector<FlatNode *> flatNodes;
		while (ret.first != ret.second)
		{
			flatNodes.push_back(ret.first->second);
			ret.first++;
		}
		hCluster2FlatNodes.insert(make_pair(i, flatNodes));
	}
	return hCluster2FlatNodes;
}


void HClusterProcessGroupPartition::constructGroups()
{
	//本方法的主要功能：组建初始的group图（一个group只有一个节点）—确定_groups和_flatNode2group的值
	std::set<FlatNodeGroup *> basicGroup;//临时保存初始的group
	std::vector<FlatNode *> flatNodes = m_sssg->GetFlatNodes();
	std::map<FlatNode *, FlatNodeGroup*> tmpFlatNode2group;
	//构成初始的flatNodeGroup（在此时的group组中一个group只有一个flatNode）
	for (int i = 0; i != flatNodes.size(); i++)
	{
		std::set<FlatNode *> tmpFlatNodeSet;
		tmpFlatNodeSet.insert(flatNodes[i]);
		FlatNodeGroup *tmpGroup = new FlatNodeGroup(tmpFlatNodeSet, m_sssg);
		basicGroup.insert(tmpGroup);
		tmpFlatNode2group.insert(std::make_pair(flatNodes[i], tmpGroup));
	}
	//确定group的依赖关系（确定flatNodeGroup中的上下端group）
	for (std::set<FlatNodeGroup *>::iterator iter = basicGroup.begin(); iter != basicGroup.end(); iter++)
	{
		std::set<FlatNode *> curFlatNodeSet = (*iter)->getFlatNodes();
		std::set<FlatNodeGroup *> upGroup;
		std::set<FlatNodeGroup *> downGroup;
		for (std::set<FlatNode *>::iterator node_iter = curFlatNodeSet.begin(); node_iter != curFlatNodeSet.end(); node_iter++)
		{
			std::vector<FlatNode *> upFlatNodes = (*node_iter)->inFlatNodes;
			std::vector<FlatNode *> downFlatNodes = (*node_iter)->outFlatNodes;
			for (int up_index = 0; up_index != upFlatNodes.size(); up_index++)
			{
				std::map<FlatNode *, FlatNodeGroup*>::iterator iter = tmpFlatNode2group.find(upFlatNodes[up_index]);
				if (iter != tmpFlatNode2group.end())upGroup.insert(iter->second);
			}
			for (int down_index = 0; down_index != downFlatNodes.size(); down_index++)
			{
				std::map<FlatNode *, FlatNodeGroup*>::iterator iter = tmpFlatNode2group.find(downFlatNodes[down_index]);
				if (iter != tmpFlatNode2group.end())downGroup.insert(iter->second);
			}
		}
		for (std::set<FlatNodeGroup *>::iterator up_iter = upGroup.begin(); up_iter != upGroup.end(); up_iter++)
		{
			(*iter)->insertupGroup(*up_iter);
		}
		for (std::set<FlatNodeGroup *>::iterator down_iter = downGroup.begin(); down_iter != downGroup.end(); down_iter++)
		{
			(*iter)->insertdownGroup(*down_iter);
		}
	}
	m_groups = basicGroup;
}

void HClusterProcessGroupPartition::preProcess()
{
	assert(m_groupAdjMatrix != NULL);
	if (m_groups.size() <= m_nClusters)
		return;

	std::set<FlatNodeGroup *> IsProcessed;//用于已经被预处理过的group的集合（防止被重复处理）
	std::map<FlatNodeGroup *, std::set<FlatNodeGroup *> > mainGroup2slaveGroups;//记录能够被粗化在一起的group(后面的set中包含mainGroup)
	std::vector<FlatNodeGroup *> groupOrginal(m_groups.begin(), m_groups.end());
	std::vector<FlatNodeGroup *> groupTopo = topoSortGroup(groupOrginal);
	assert(m_groups.size() == groupTopo.size());

	for (std::vector<FlatNodeGroup *>::iterator gIter = groupTopo.begin(); gIter != groupTopo.end(); gIter++)
	{//考虑当前节点（向上预处理还是向下预处理）
		FlatNodeGroup* cur_group = *gIter;//取当前的group
		if (IsProcessed.count(cur_group)) continue;//当前节点被处理过了——抛弃已经被处理过的group
		if (cur_group->getDownGroup().size() <= 1 && cur_group->getUpGroup().size() <= 1) continue;//抛弃不要预处理的group
		long groupSetWorkload = cur_group->getWorkload();
		if (groupSetWorkload > m_avgWorkload) continue;//当前group的负载已经大于均值
		std::set<FlatNodeGroup*> potential_Group;//取能够被组在一起的group的集合（上端与下端的总和）
		potential_Group.clear();
		//对可能需要被预处理的节点进行预处理——减少通行边的数目
		std::vector<FlatNodeGroup*> all_upGroup = cur_group->getUpGroup();
		//收集当前节点的上端节点的输入流的数目为1的节点，放在一个set中，
		//如果所有的节点的上端节点都被放在一个set中,再判断上端的上端节点是不是同一个，如果是也加进来
		if (all_upGroup.size() > 1)
		{
			//上端节点数目大于1
			std::multimap<long, FlatNodeGroup*> potential_upGroup;//(负载与group之间的map)取能够被组在一起的上端group的集合
			for (int up_iter = 0; up_iter != all_upGroup.size(); up_iter++)
			{
				//如果节点是单入单出的加到potential_upGroup中
				if ((IsProcessed.count(all_upGroup[up_iter]) == 0) && all_upGroup[up_iter]->getDownGroup().size() <= 1 && all_upGroup[up_iter]->getUpGroup().size() <= 1)
				{
					potential_upGroup.insert(std::make_pair(all_upGroup[up_iter]->getWorkload(), all_upGroup[up_iter]));
					groupSetWorkload += all_upGroup[up_iter]->getWorkload();
				}
			}
			if (groupSetWorkload <= m_avgWorkload)
			{
				Bool FG_up = FALSE;
				if (potential_upGroup.size() == all_upGroup.size())
				{
					//判断其上端节点能不能被放进来—根据上端的特性，potential_upGroup中的节点的都是其孩子节点，并且他没有其他的孩子时将其放入，否则胡烈
					std::vector<FlatNodeGroup *>upupGroupVec = potential_upGroup.begin()->second->getUpGroup();
					FlatNodeGroup *upupGroup = NULL;
					if (!upupGroupVec.empty()) upupGroup = upupGroupVec[0];
					std::vector<FlatNodeGroup *>upupdownGroupVec = upupGroup->getDownGroup();
					//判断upupdownGroupVec与potential_upGroup中的节点是不是一致
					std::set<FlatNodeGroup *> upupdownGroupSet(upupdownGroupVec.begin(), upupdownGroupVec.end());
					for (std::multimap<long, FlatNodeGroup*>::iterator pg_iter = potential_upGroup.begin(); pg_iter != potential_upGroup.end(); pg_iter++)
					{
						if (!upupdownGroupSet.count(pg_iter->second)) { FG_up = TRUE; break; }
					}
					if (!FG_up) potential_upGroup.insert(std::make_pair(upupGroup->getWorkload(), upupGroup));
				}
			}
			else
			{
				//将负载大的节点移走，留下的节点可以直接组与当前节点组
				//反向遍历potential_upGroup，移走负载大的group
				std::multimap<long, FlatNodeGroup *>::reverse_iterator pg_riter = potential_upGroup.rbegin();
				while (!potential_upGroup.empty())
				{
					groupSetWorkload -= pg_riter->first;
					potential_upGroup.erase(--pg_riter.base());
					if (groupSetWorkload <= m_avgWorkload)break;
					pg_riter = potential_upGroup.rbegin();
				}
			}
			for (std::multimap<long, FlatNodeGroup *>::iterator pp_iter = potential_upGroup.begin(); pp_iter != potential_upGroup.end(); pp_iter++)
				potential_Group.insert(pp_iter->second);
		}
		std::vector<FlatNodeGroup*> all_downGroup = (cur_group)->getDownGroup();
		if (all_downGroup.size() > 1)
		{//下端节点的数目大于1——当前节点与下端节点进行处理（上端的节点与下端节点可以看做是独立的）
			std::multimap<long, FlatNodeGroup*> potential_downGroup;//(负载与group之间的map)取能够被组在一起的上端group的集合
			for (int down_iter = 0; down_iter != all_downGroup.size(); down_iter++)
			{//如果节点是单入单出的并且没有被预处理过，加到potential_upGroup中
				if ((IsProcessed.count(all_downGroup[down_iter]) == 0) && all_downGroup[down_iter]->getDownGroup().size() <= 1 && all_downGroup[down_iter]->getUpGroup().size() <= 1)
				{
					potential_downGroup.insert(std::make_pair(all_downGroup[down_iter]->getWorkload(), all_downGroup[down_iter]));
					groupSetWorkload += all_downGroup[down_iter]->getWorkload();
				}
			}
			if (groupSetWorkload <= m_avgWorkload)
			{
				Bool FG_up = FALSE;
				if (potential_downGroup.size() == all_downGroup.size())
				{//判断其上端节点能不能被放进来—根据上端的特性，potential_upGroup中的节点的都是其孩子节点，并且他没有其他的孩子时将其放入，否则胡烈
					std::vector<FlatNodeGroup *>downdownGroupVec = potential_downGroup.begin()->second->getDownGroup();
					FlatNodeGroup *downdownGroup = NULL;
					if (!downdownGroupVec.empty()) downdownGroup = downdownGroupVec[0];
					std::vector<FlatNodeGroup *>downdownupGroupVec = downdownGroup->getUpGroup();
					//判断upupdownGroupVec与potential_upGroup中的节点是不是一致
					std::set<FlatNodeGroup *> downdownupGroupSet(downdownupGroupVec.begin(), downdownupGroupVec.end());
					for (std::multimap<long, FlatNodeGroup*>::iterator pg_iter = potential_downGroup.begin(); pg_iter != potential_downGroup.end(); pg_iter++)
					{
						if (!downdownupGroupSet.count(pg_iter->second)) { FG_up = TRUE; break; }
					}
					if (!FG_up) potential_downGroup.insert(std::make_pair(downdownGroup->getWorkload(), downdownGroup));
				}
			}
			else
			{//将负载大的节点移走，留下的节点可以直接组与当前节点组
			 //反向遍历potential_upGroup，移走负载大的group
				std::multimap<long, FlatNodeGroup *>::reverse_iterator pg_riter = potential_downGroup.rbegin();
				while (!potential_downGroup.empty())
				{
					groupSetWorkload -= pg_riter->first;
					potential_downGroup.erase(--pg_riter.base());
					if (groupSetWorkload <= m_avgWorkload)break;
					pg_riter = potential_downGroup.rbegin();
				}
			}
			for (std::multimap<long, FlatNodeGroup *>::iterator pp_iter = potential_downGroup.begin(); pp_iter != potential_downGroup.end(); pp_iter++)
				potential_Group.insert(pp_iter->second);
		}
		potential_Group.insert(cur_group);
		//根据potential_Group修改IsProcessed
		IsProcessed.insert(potential_Group.begin(), potential_Group.end());
		mainGroup2slaveGroups.insert(std::make_pair(cur_group, potential_Group));
	}
	if (!mainGroup2slaveGroups.empty()) 
		updateGroupGraph(mainGroup2slaveGroups);//根据预处理的情况更新groupGraph
}

void HClusterProcessGroupPartition::coarseGrainedPartition()
{
	int boundaryGroupNum = m_nClusters * COARSEN_FACTOR;//粗化时group数量的下限
	int originalGroupNum = m_groups.size();//预处理后图中group的数目
	if (originalGroupNum <= boundaryGroupNum)return;
	constructAdjMatrixInfo();//根据预处理后的图构造邻接矩阵(预处理的处理过程是可以选择的)
							 //本过程涉及到的数据结构有_groupAdjMatrix，_groupWorkload，_groupNo2actualNo，以及新定义的结构(map<pair<int,int>,float>)来保存相邻节点粗化产生的收益
	std::map<std::pair<int, int>, float > grouptogroup2Gain;//保存一对group被组在一起产生的收益
	//连接关系还是用邻接矩阵保存，用上面的map来保存产生的收益情况，其他数据结构保持不变
	int curGroupNum = originalGroupNum;//记录当前节点的数目
	//根据初始的group图计算粗化一对想临节点的初始边收益
	for (int i = 0; i < originalGroupNum; i++)
	{
		for (int j = i; j < originalGroupNum; j++)
			if (m_groupAdjMatrix[i][j])
				grouptogroup2Gain.insert(std::make_pair(std::make_pair(i, j), computeGain(i, j)));
	}
#if 0
	for (std::map<std::pair<int, int>, float >::iterator i = grouptogroup2Gain.begin(); i != grouptogroup2Gain.end(); i++)
	{
		std::cout << i->first.first << " " << i->first.second << " " << i->second << std::endl;
	}
#endif
	//挑选收益最大的进行粗化group
	while (curGroupNum > boundaryGroupNum)
	{
		//当group的数目小于boundaryGroupNum时停止粗化
		//从中找收益最大的可粗化（有拓扑）的节点，同时根据粗化后结果更新相关的信息
		std::multimap<float, std::pair<int, int> > tmpGain2grouptogroup;
		for (std::map<std::pair<int, int>, float >::iterator g2p_iter = grouptogroup2Gain.begin(); g2p_iter != grouptogroup2Gain.end(); g2p_iter++)
			tmpGain2grouptogroup.insert(std::make_pair(g2p_iter->second, g2p_iter->first));
		std::multimap<float, std::pair<int, int> >::reverse_iterator r_p2g_iter = tmpGain2grouptogroup.rbegin();
		while (r_p2g_iter != tmpGain2grouptogroup.rend())
		{
			//考察p2g_iter所指的一对group能否被粗化在一起（主要是通过拓扑排序来考察和负载均衡的角度来考察）
			//如果一对节点合并后的节点的负载大于均值则不能够组在一起
			if (m_groupWorkload[(r_p2g_iter->second).first] + m_groupWorkload[(r_p2g_iter->second).second] > m_avgWorkload) r_p2g_iter++;
			else if (topoSortGroup((r_p2g_iter->second).first, (r_p2g_iter->second).second, originalGroupNum))//将这两个group组在一起
				break;
			else r_p2g_iter++;
		}
#if 0
		for (int i = 0; i != _groupNo2actualNo.size(); i++) std::cout << _groupNo2actualNo[i] << " ";
		std::cout << std::endl;
#endif
		if (r_p2g_iter != tmpGain2grouptogroup.rend())
		{
			//根据粗化结果更新修改_groupNo2actualNo，_groupWorkload，_groupCommunication，_groupAdjMatrix，grouptogroup2Gain
			//要将所有的值为(r_p2g_iter->second).second的位置的值改为 (r_p2g_iter->second).first;
			for (int i = 0; i != m_groupNo2actualNo.size(); i++)
				if (m_groupNo2actualNo[i] == (r_p2g_iter->second).second) m_groupNo2actualNo[i] = (r_p2g_iter->second).first;
			m_groupWorkload[(r_p2g_iter->second).first] += m_groupWorkload[(r_p2g_iter->second).second];
			m_groupWorkload[(r_p2g_iter->second).second] = 0;
			m_groupCommunication[(r_p2g_iter->second).first] += m_groupCommunication[(r_p2g_iter->second).second] - 2 * m_groupAdjMatrix[(r_p2g_iter->second).first][(r_p2g_iter->second).second];
			m_groupCommunication[(r_p2g_iter->second).second] = 0;
			//删除grouptogroup2Gain中过时的信息（以group1，group2为头和为的pair）
			std::map<std::pair<int, int>, float >::iterator del_g2p_iter = grouptogroup2Gain.begin();
			while (del_g2p_iter != grouptogroup2Gain.end()) {
				if (del_g2p_iter->first.first == (r_p2g_iter->second).first || del_g2p_iter->first.first == (r_p2g_iter->second).second || del_g2p_iter->first.second == (r_p2g_iter->second).first || del_g2p_iter->first.second == (r_p2g_iter->second).second)
				{//删除当前的
					std::map<std::pair<int, int>, float >::iterator tmp_del = del_g2p_iter;
					del_g2p_iter++;
					grouptogroup2Gain.erase(tmp_del);
				}
				else del_g2p_iter++;
			}
			//更新_groupAdjMatrix
			updateAdjMatrixInfo((r_p2g_iter->second).first, (r_p2g_iter->second).second, m_groupAdjMatrix, originalGroupNum);
			//向grouptogroup2Gain添加新的信息——根据_groupAdjMatrix，将与(r_p2g_iter->second).first有依赖关系的点插入到grouptogroup2Gain中
			//遍历邻接矩阵中(r_p2g_iter->second).first的行和列
			for (int i = (r_p2g_iter->second).first; i != originalGroupNum; i++)
			{
				if (m_groupAdjMatrix[(r_p2g_iter->second).first][i] == 0 || m_groupAdjMatrix[(r_p2g_iter->second).first][i] == -1) continue;
				grouptogroup2Gain.insert(std::make_pair(std::make_pair((r_p2g_iter->second).first, i), computeGain((r_p2g_iter->second).first, i)));
			}
			for (int i = 0; i != (r_p2g_iter->second).first; i++)
			{
				if (m_groupAdjMatrix[i][(r_p2g_iter->second).first] == 0 || m_groupAdjMatrix[i][(r_p2g_iter->second).first] == -1) continue;
				grouptogroup2Gain.insert(std::make_pair(std::make_pair(i, (r_p2g_iter->second).first), computeGain(i, (r_p2g_iter->second).first)));
			}
		}
		else break;//粗化结束，再也找不到能够被粗化的group了
		curGroupNum--;
	}
	//根据粗化的结果重构group图
	updataGroupGraphByCoarsening(originalGroupNum);
		
	std::vector<FlatNodeGroup *> groupOrginal(m_groups.begin(), m_groups.end());
	std::vector<FlatNodeGroup *> groupTopo = topoSortGroup(groupOrginal);
	assert(m_groups.size() == groupTopo.size());//保证粗化结果的group图是没有环的
}

void HClusterProcessGroupPartition::initialPartition()
{
	std::vector<FlatNodeGroup *> tmp_groups(m_groups.begin(), m_groups.end());
	std::vector<FlatNodeGroup *> topoGroups = topoSortGroup(tmp_groups);
	assert(topoGroups.size() == m_groups.size());
	//下面的操作都是针对topoGroups进行的——按照拓扑的序列来决定节点被划分的次序，同时在确定一个group在那个划分上时，在对通信量进行一定的考察（可以向上调整）
	std::map<FlatNodeGroup *, int> initGroup2partition;//group与划分编号之间的map
													   //记录当前划分需要的信息
	std::vector<long> partition_workload(m_nClusters, 0);//记录各个划分的负载
	int p_no = 0;//当前确定划分子集的编号（当前处理的是第几个划分，该值一定小于_ncluster）
	std::vector<FlatNodeGroup *> tailGroup;
	for (int i = 0; i != topoGroups.size(); i++)
	{
		if (p_no == m_nClusters) { tailGroup.push_back(topoGroups[i]); continue; }
		long tmp_partition_workload = partition_workload[p_no] + topoGroups[i]->getWorkload();

		if ((double)partition_workload[p_no] - (double)m_avgWorkload * 0.7 <= EXP || (double)tmp_partition_workload - (double)m_avgWorkload * MINBALANCEFACTOR <= EXP)
		{
			//如果当前的负载小于(当前划分上的负载比平均的一般还小或者在当前的划分中加上group的工作量后还没有达到均值)应该得到的最小值则直接添加
			partition_workload[p_no] += topoGroups[i]->getWorkload();
		}
		else if ((double)tmp_partition_workload - (double)m_avgWorkload * MAXBALANCEFACTOR <= EXP)
		{
			//参考通信的开销——判断当前节点需不需要放在该划分中
			//取当前group的上端
			std::map<FlatNodeGroup*, int> cur_upGroup2weight = topoGroups[i]->getUpGroup2Weight();
			//判断当前group与p_no中的通信量
			int initPTcomm = 0;
			std::set<int> cur_upcluster;
			for (std::map<FlatNodeGroup*, int>::iterator cur_up_iter = cur_upGroup2weight.begin(); cur_up_iter != cur_upGroup2weight.end(); cur_up_iter++)
			{
				std::map<FlatNodeGroup *, int>::iterator up_pt_iter = initGroup2partition.find(cur_up_iter->first);
				if (up_pt_iter != initGroup2partition.end()) cur_upcluster.insert(up_pt_iter->second);
				if (up_pt_iter != initGroup2partition.end() && up_pt_iter->second == p_no)initPTcomm += cur_up_iter->second;
			}
			int curComm = topoGroups[i]->getTotalCommunicateData();//取当前节点总的通信量
			int curCommEdge = cur_upcluster.size() + topoGroups[i]->getDownGroup2Weight().size();//计算可能与多少个cluster通信
																								 //std::cout<<curComm / curCommEdge<<" "<<initPTcomm<<"\n";
			if (curComm / curCommEdge <= initPTcomm)
			{
				partition_workload[p_no] += topoGroups[i]->getWorkload();
			}
			else
			{
				p_no++;
				if (p_no != m_nClusters) { partition_workload[p_no] = topoGroups[i]->getWorkload(); }
				else { tailGroup.push_back(topoGroups[i]); continue; }
			}
		}
		else
		{
			p_no++;
			if (p_no != m_nClusters) { partition_workload[p_no] = topoGroups[i]->getWorkload(); }
			else { tailGroup.push_back(topoGroups[i]); continue; }
		}
		initGroup2partition.insert(std::make_pair(topoGroups[i], p_no));
	}
	if (!tailGroup.empty())
	{
		for (int kk = 0; kk != tailGroup.size(); kk++)
		{
			initGroup2partition.insert(std::make_pair(tailGroup[kk], m_nClusters - 1));
			partition_workload[m_nClusters - 1] += tailGroup[kk]->getWorkload();
		}
	}
#if 0
	for (std::vector<long>::iterator pw_iter = partition_workload.begin(); pw_iter != partition_workload.end(); pw_iter++)
		std::cout << *pw_iter << " ";
	std::cout << std::endl;
#endif
	assert(initGroup2partition.size() == m_groups.size());
	assert(p_no <= m_nClusters);
	//将initGroup2partition初始化分转换成flatNode与cluster之间的map
	for (std::map<FlatNodeGroup *, int>::iterator initPT_iter = initGroup2partition.begin(); initPT_iter != initGroup2partition.end(); initPT_iter++)
	{
		std::set<FlatNode *> flatNodeSet = initPT_iter->first->getFlatNodes();
		for (std::set<FlatNode *>::iterator fn_iter = flatNodeSet.begin(); fn_iter != flatNodeSet.end(); fn_iter++)
		{
			m_cluster2flatNode.insert(std::make_pair(initPT_iter->second, *fn_iter));
			m_flatNode2cluster.insert(std::make_pair(*fn_iter, initPT_iter->second));
		}
	}
	//将初始划分后在一个cluster上的group融合成一个group
	std::multimap<int, FlatNodeGroup*> partition2initGroup;//划分编号与group之间的map
	for (std::map<FlatNodeGroup *, int>::iterator initPT_iter = initGroup2partition.begin(); initPT_iter != initGroup2partition.end(); initPT_iter++)
		partition2initGroup.insert(std::make_pair(initPT_iter->second, initPT_iter->first));
	for (int i = 0; i != m_nClusters; i++)
	{
		std::pair<std::multimap<int, FlatNodeGroup *>::iterator, std::multimap<int, FlatNodeGroup *>::iterator> pos = partition2initGroup.equal_range(i);
		assert(pos.first != partition2initGroup.end());//确定每一个集群节点上都有group
		FlatNodeGroup *curGroup = pos.first->second;
		++pos.first;
		while (pos.first != pos.second)
		{
			m_groups.erase(pos.first->second);
			curGroup->fusingGroup(pos.first->second);
			pos.first++;
		}
		m_cluster2group.push_back(curGroup);
	}
	assert(m_cluster2group.size() == m_nClusters);
	//------确定经过初始划分完成后flatNode与group之间的map
	m_flatNode2group.clear();
	m_groups.clear();
	for (int i = 0; i != m_cluster2group.size(); i++)
	{
		m_groups.insert(m_cluster2group[i]);
		std::set<FlatNode *> tmpflatNodes_Group = m_cluster2group[i]->getFlatNodes();
		for (std::set<FlatNode *>::iterator tmp_iter = tmpflatNodes_Group.begin(); tmp_iter != tmpflatNodes_Group.end(); tmp_iter++)
			m_flatNode2group.insert(std::make_pair(*tmp_iter, m_cluster2group[i]));
	}
	assert(m_groups.size() == m_cluster2group.size());
	assert(m_flatNode2group.size() == m_sssg->flatNodes.size());
}

void HClusterProcessGroupPartition::fineGrainedPartition()
{
	std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > flatNode2AdjPT2Gain;//flatNode与相对于目标partition而言产生的收益（flatNode，相邻的划分，收益）
	std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> > gain2FlatNode2AdjPT;//有gain来维护的优先队列（收益，flatNode，相邻的划分）
	refiningPhase_CollectBoundFlatNode(flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	assert(flatNode2AdjPT2Gain.size() == gain2FlatNode2AdjPT.size());
	//下面对优先队列gain2FlatNode2AdjPT中的元素进行迭代处理，在处理的过程中要更新优先队列（插入ED > ID的节点，删除ED > ID的节点）
	//迭代终止的条件是：①不破坏图的平衡，就没有节点可以移动；②不破引入环，就没有节点可以移动；③队列为空；④队列中所有节点的移动都会使全局通信边增多
	while (!gain2FlatNode2AdjPT.empty())
	{//处理细粒度调整gain2FlatNode2AdjPT,flatNode2AdjPT2Gain
		std::multimap<int, std::pair<FlatNode *, FlatNodeGroup *> >::reverse_iterator adjIter = gain2FlatNode2AdjPT.rbegin();
		FlatNode *adjFlatNode = adjIter->second.first;//取当前要被调整的flatNode
		FlatNodeGroup* srcGroup = m_flatNode2group.find(adjFlatNode)->second;
		if (refiningPhase_IsAdjustableFlatNode(adjFlatNode, adjIter->second.second, adjIter->first))
		{
			//如果能够被调整，则更新gain2FlatNode2AdjPT,flatNode2AdjPT2Gain
			//adjFlatNode调整后所在的group
			//std::cout<<"移动flatNode		"<<adjFlatNode->name<<"\n";
			refiningPhase_UpdatePriQueue(adjFlatNode, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
		}
		//不管调整是否成功将flatNode从队列中删除
		flatNode2AdjPT2Gain.erase(adjIter->second);
		gain2FlatNode2AdjPT.erase(--adjIter.base());
	}
	assert(m_groups.size() == m_nClusters);
	//检测划分——在划分中不允许有有这样的节点存在，即一个节点的输入输出均不在当前的划分中，如果出现这样的节点则要将该节点向上端节点调整或者像下端节点调整
	std::map<FlatNode *, FlatNodeGroup *>::iterator r_iter = m_flatNode2group.begin();
	for (; r_iter != m_flatNode2group.end(); ++r_iter)
	{
		if (r_iter->second->getFlatNodes().size() <= 1) continue;
		std::vector<FlatNode *> outflatNodes = r_iter->first->outFlatNodes;
		std::set<FlatNodeGroup *> objGroupSet;
		int obj_flag = 0;
		for (int i = 0; i != outflatNodes.size(); ++i)
		{
			FlatNodeGroup * tmp_downGroup = m_flatNode2group.find(outflatNodes[i])->second;
			if (r_iter->second == tmp_downGroup) { obj_flag = 1; break; }
			else objGroupSet.insert(tmp_downGroup);
		}
		if (obj_flag) { objGroupSet.clear(); obj_flag = 0; continue; }
		std::vector<FlatNode *> inflatNodes = r_iter->first->inFlatNodes;
		for (int i = 0; i != inflatNodes.size(); ++i)
		{
			FlatNodeGroup * tmp_upGroup = m_flatNode2group.find(inflatNodes[i])->second;
			if (r_iter->second == tmp_upGroup) { obj_flag = 1; break; }
			else objGroupSet.insert(tmp_upGroup);
		}
		if (obj_flag) { objGroupSet.clear(); obj_flag = 0; continue; }
		//为了特殊处理在一个划分存在一个节点与该划分中其他所有的节点都不相邻的情况-------begin
		//存在单独的一个flatNode在一个group中--------------------------------------
		//找其相邻节点所在group中工作量最小的将其移入
		std::set<FlatNodeGroup *>::iterator obj_iter = objGroupSet.begin();
		FlatNodeGroup *objG = *obj_iter; int objW = (*obj_iter)->getWorkload();
		for (; obj_iter != objGroupSet.end(); ++obj_iter)
		{
			if (objW > (*obj_iter)->getWorkload()) { objW = (*obj_iter)->getWorkload(); objG = *obj_iter; }
		}
		std::cout << "\n--------XXXXXXXXXX特殊处理XXXXXXXXXXXX--------\n" << std::endl;
		movingFlatNodeInterGroup(r_iter->first, r_iter->second, objG);
		objGroupSet.clear();
	}//---------------------------------------------------------------------------------------end
	 //将group中的内容转换到group2cluster和cluster2group中
	m_cluster2flatNode.clear();
	m_flatNode2cluster.clear();
	for (int i = 0; i != m_cluster2group.size(); i++)
	{
		std::set<FlatNode *> g_flatNodes = m_cluster2group[i]->getFlatNodes();
		for (std::set<FlatNode *>::iterator f_iter = g_flatNodes.begin(); f_iter != g_flatNodes.end(); f_iter++)
		{
			m_cluster2flatNode.insert(std::make_pair(i, *f_iter));
			m_flatNode2cluster.insert(std::make_pair(*f_iter, i));
		}
	}
}

void HClusterProcessGroupPartition::adjustBoundaryFlatNodeWorkload()
{
	//根据划分结果修改边界节点计算量（将与外界的通行量转换成计算量）-20131201
	for (int i = 0; i != m_nClusters; i++)
	{
		std::pair<std::multimap<int, FlatNode *>::iterator, std::multimap<int, FlatNode *>::iterator> ret = m_cluster2flatNode.equal_range(i);
		std::vector<FlatNode *> flatNodes;
		while (ret.first != ret.second)
		{
			flatNodes.push_back(ret.first->second);
			ret.first++;
		}
		updateFlatNodeWorkloadSSG(flatNodes);
	}
}

void HClusterProcessGroupPartition::updateFlatNodeWorkloadSSG(std::vector<FlatNode*> flatNodeV)
{
	//对于边界节点要根据通信数据量增大actor的负载
	std::set<FlatNode *>flatNodes(flatNodeV.begin(), flatNodeV.end());
	for (std::set<FlatNode *>::iterator s_iter = flatNodes.begin(); s_iter != flatNodes.end(); s_iter++)
	{
		long oldWorkload = m_sssg->GetSteadyWork(*s_iter);
		long curWorkload = oldWorkload;
		//考察flatNode的输出节点
		for (int i = 0; i != (*s_iter)->outFlatNodes.size(); i++)
		{
			if (flatNodes.count((*s_iter)->outFlatNodes[i])) continue;
			else
			{
				curWorkload += OVERHEAD_PER_COMMUNICATION_DATA * (*s_iter)->outPushWeights[i];//+ COMM_STRATUP_TIME;
			}
		}
		//考察flatNode的输入节点
		for (int i = 0; i != (*s_iter)->inFlatNodes.size(); i++)
		{
			if (flatNodes.count((*s_iter)->inFlatNodes[i])) continue;
			else
			{
				curWorkload += OVERHEAD_PER_COMMUNICATION_DATA * (*s_iter)->inPopWeights[i];// + COMM_STRATUP_TIME;
			}
		}
		//修改SSG中的flatNode对应的负载
		if (curWorkload != oldWorkload)
		{
			std::map<FlatNode *, int>::iterator fn_iter = m_sssg->mapSteadyWork2FlatNode.find(*s_iter);
			assert(fn_iter != m_sssg->mapSteadyWork2FlatNode.end());
			fn_iter->second = curWorkload;
			std::cout << (*s_iter)->name << "负载的变化" << oldWorkload << "------>" << curWorkload << "	delt " << curWorkload - oldWorkload << std::endl;
		}
	}
}

long HClusterProcessGroupPartition::computeWorkload(std::vector<FlatNode*> flatNodes)
{
	if (flatNodes.empty()) return 0;
	long workload = 0;
	for (int i = 0; i != flatNodes.size(); i++)
	{
		workload += m_sssg->GetSteadyWork(flatNodes[i]) * m_sssg->GetSteadyCount(flatNodes[i]);
	}
	return workload;
}

std::vector<FlatNodeGroup*> HClusterProcessGroupPartition::topoSortGroup(std::vector<FlatNodeGroup*> original)
{
	std::vector<FlatNodeGroup *>::iterator iter;
	std::vector<FlatNodeGroup *> topoGroup;
	std::vector<int> nInDegree;//用于保存各节点的入度
	std::vector<FlatNodeGroup *> groupStack;
	int nsize = original.size();
	int count = 0;
	for (iter = original.begin(); iter != original.end(); ++iter)
	{
		nInDegree.push_back(((*iter)->getUpGroup()).size());//将各节点入度保存
	}
	for (int i = 0; i != nInDegree.size(); i++)
	{
		if (!nInDegree[i]) groupStack.push_back(original[i]);
	}
	while (!groupStack.empty())
	{
		FlatNodeGroup *group = groupStack.back();// 取将要出栈的节点
		std::vector<FlatNodeGroup *>tmpProClusterGroup = group->getDownGroup();//取group的所有后继
		groupStack.pop_back();
		topoGroup.push_back(group);
		for (int i = 0; i != tmpProClusterGroup.size(); i++)//对所有group的邻接点的入度减1
		{
			for (int j = 0; j != original.size(); j++)
			{
				if (original[j] == tmpProClusterGroup[i])
				{
					if (!(--nInDegree[j])) groupStack.push_back(original[j]);//入度为0点进栈
				}
			}
		}
	}
	return topoGroup;
}

Bool HClusterProcessGroupPartition::topoSortGroup(int group1, int group2, int nsize)
{
	//根据邻接矩阵做拓扑排序，判断在划分过程中会不会出现死锁
	std::vector<int> TopoOrder;
	std::vector<int> nInDegree(nsize, 0);//用于保存各节点的入度
	std::vector<int> groupStack;
	//要先对矩阵进行修改（然后在判断是否存在环，可以对一个副本做修改）
	for (int i = 0; i != nsize; i++)
		for (int j = 0; j < nsize; j++)
			m_groupAdjMatrix_copy[i][j] = m_groupAdjMatrix[i][j];
	updateAdjMatrixInfo(group1, group2, m_groupAdjMatrix_copy, nsize);

	for (int i = 0; i != nsize; ++i)
	{
		//统计各个节点的入度（矩阵的列）
		if (m_groupAdjMatrix_copy[i][i] == -1)
		{
			nInDegree[i] = -1;//入度为-1表示该值是不存在的 
			continue;//当前的节点是无效的
		}
		int inCount = 0;
		for (int j = 0; j < nsize; j++)
		{
			if (m_groupAdjMatrix_copy[j][i] != 0 && m_groupAdjMatrix_copy[j][i] != -1) inCount++;
		}
		nInDegree[i] = inCount;//将各节点入度保存
	}
	int invalid_group = 0;
	for (int i = 0; i != nsize; i++)
	{
		if (!nInDegree[i]) groupStack.push_back(i);//将入度为0的节点先放到groupStack中
		else if (nInDegree[i] == -1) invalid_group++;//记录无效节点
	}
	while (!groupStack.empty())
	{
		int tmpGroupNo = groupStack.back();// 取将要出栈的节点
		TopoOrder.push_back(tmpGroupNo);
		groupStack.pop_back();
		if (nInDegree[tmpGroupNo] == -1) continue;
		for (int i = 0; i != nsize; i++)//对所有group的邻接点的入度减1
		{
			if (m_groupAdjMatrix_copy[tmpGroupNo][i] != 0 && m_groupAdjMatrix_copy[tmpGroupNo][i] != -1)
			{
				if (!(--nInDegree[i])) groupStack.push_back(i);//入度为0点进栈
			}
		}
	}
	if (TopoOrder.size() + invalid_group == nsize)return TRUE;
	else return FALSE;
}

void HClusterProcessGroupPartition::updateGroupGraph(std::map<FlatNodeGroup*, std::set<FlatNodeGroup*>> Preprocess)
{
	//根据预处理的结果修改GroupGraph（连接关系在fusingGroup中已经维护好了）
	for (std::map<FlatNodeGroup *, std::set<FlatNodeGroup *> >::iterator iter = Preprocess.begin(); iter != Preprocess.end(); iter++)
	{
		for (std::set<FlatNodeGroup *>::iterator pp_iter = (iter->second).begin(); pp_iter != (iter->second).end(); pp_iter++)
		{
			//先从_group中删除需要被融合的FlatNodeGroup
			if (*pp_iter != iter->first) {
				m_groups.erase(*pp_iter);
				iter->first->fusingGroup(*pp_iter);
			}
		}
	}
}

void HClusterProcessGroupPartition::updateAdjMatrixInfo(int group1, int group2, int ** adjMatrix, int size)
{
	for (int i = 0; i != size; i++)
	{
		if (adjMatrix[group2][i] != -1)
		{
			adjMatrix[group1][i] += adjMatrix[group2][i];
		}
		if (adjMatrix[i][group2] != -1)
		{
			adjMatrix[i][group1] += adjMatrix[i][group2];
		}
	}
	adjMatrix[group1][group1] = 0;
	for (int i = 0; i != size; i++)
	{
		adjMatrix[group2][i] = -1;//-1表明该点存储的值是无效的
		adjMatrix[i][group2] = -1;
	}
}

void HClusterProcessGroupPartition::updataGroupGraphByCoarsening(int original_size)
{
	std::map<int, std::set<int> > actualNo2originalNoSet;//group当前的编号与原始编号之间的map
	for (int i = 0; i != original_size; i++)
	{
		std::set<int>tmpOriginalSet;
		tmpOriginalSet.insert(i);
		std::pair<std::map<int, std::set<int> >::iterator, bool> ret = actualNo2originalNoSet.insert(std::make_pair(m_groupNo2actualNo[i], tmpOriginalSet));
		if (!ret.second) ret.first->second.insert(i);
	}
	for (std::map<int, std::set<int> >::iterator iter = actualNo2originalNoSet.begin(); iter != actualNo2originalNoSet.end(); iter++)
	{
		FlatNodeGroup *curGroup = m_groupNo2group.find(iter->first)->second;
		for (std::set<int>::iterator pp_iter = (iter->second).begin(); pp_iter != (iter->second).end(); pp_iter++)
		{
			FlatNodeGroup *tmpGroup = m_groupNo2group.find(*pp_iter)->second;
			//先从_group中删除需要被融合的FlatNodeGroup
			if (*pp_iter != iter->first) {
				m_groups.erase(tmpGroup);
				curGroup->fusingGroup(tmpGroup);
			}
		}
	}
}

void HClusterProcessGroupPartition::refiningPhase_CollectBoundFlatNode(std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> > &gain2FlatNode2AdjPT)
{
	assert(m_sssg->flatNodes.size() == m_flatNode2cluster.size());
	for (std::map<FlatNode *, FlatNodeGroup*>::iterator f_iter = m_flatNode2group.begin(); f_iter != m_flatNode2group.end(); f_iter++)
	{
		insertNewFlatNodeGain(f_iter->first, f_iter->second, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	}
}

Bool HClusterProcessGroupPartition::refiningPhase_IsAdjustableFlatNode(FlatNode * flatNode, FlatNodeGroup * snkGroup, int gain)
{
	FlatNodeGroup* srcGroup = m_flatNode2group.find(flatNode)->second;//取flatNode所在的partition

	int srcGroup_commData = srcGroup->getTotalCommunicateData() * OVERHEAD_PER_COMMUNICATION_DATA;
	int snkGroup_commData = snkGroup->getTotalCommunicateData() * OVERHEAD_PER_COMMUNICATION_DATA;
	int srcGroup_workload = srcGroup->getWorkload();
	int snkGroup_workload = snkGroup->getWorkload();

	movingFlatNodeInterGroup(flatNode, srcGroup, snkGroup);
	std::vector<FlatNodeGroup *> groupVec(m_groups.begin(), m_groups.end());
	std::vector<FlatNodeGroup *> groupTopo = topoSortGroup(groupVec);
	if (groupTopo.size() != m_groups.size())
	{//有环
		movingFlatNodeInterGroup(flatNode, snkGroup, srcGroup);//撤销上次的移动
		return FALSE;
	}
	//如何评价移动的效果--------（需要根据实验结果来做调整）
	if (((float)snkGroup->getWorkload()) / srcGroup->getWorkload() > BALANCE_FACTOE)
	{
		movingFlatNodeInterGroup(flatNode, snkGroup, srcGroup);//撤销上次的移动
		return FALSE;
	}
	if (gain > 0)
		return TRUE;
	else//gain <= 0
	{
		//相关的两个group的计算通信比的增量的角度来考察
		int srcGroup_commData_after = srcGroup->getTotalCommunicateData() * OVERHEAD_PER_COMMUNICATION_DATA;
		int snkGroup_commData_after = snkGroup->getTotalCommunicateData() * OVERHEAD_PER_COMMUNICATION_DATA;
		int srcGroup_workload_after = srcGroup->getWorkload();
		int snkGroup_workload_after = snkGroup->getWorkload();
		float radio = (float)srcGroup_workload / srcGroup_commData + (float)snkGroup_workload / snkGroup_commData;
		float radio_after = (float)srcGroup_workload_after / srcGroup_commData_after + (float)snkGroup_workload_after / snkGroup_commData_after;
		if (radio_after - radio > 0.1)
			return TRUE;//注意float与0的比较
	}
	movingFlatNodeInterGroup(flatNode, snkGroup, srcGroup);//撤销上次的移动
	return FALSE;
}

void HClusterProcessGroupPartition::refiningPhase_UpdatePriQueue(FlatNode * flatNode, std::map<std::pair<FlatNode*, FlatNodeGroup*>, int>& flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode*, FlatNodeGroup*>>& gain2FlatNode2AdjPT)
{
	refiningPhase_RemoveOldGain(flatNode, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	insertNewFlatNodeGain(flatNode, m_flatNode2group.find(flatNode)->second, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	//flatNode上端节点在flatNode移动过后收益的变化情况
	for (int i = 0; i != flatNode->inFlatNodes.size(); i++)
	{
		refiningPhase_RemoveOldGain(flatNode->inFlatNodes[i], flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
		insertNewFlatNodeGain(flatNode->inFlatNodes[i], m_flatNode2group.find(flatNode->inFlatNodes[i])->second, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	}
	//flatNode下端节点在flatNode移动过后收益的变化情况
	for (int i = 0; i != flatNode->outFlatNodes.size(); i++)
	{
		refiningPhase_RemoveOldGain(flatNode->outFlatNodes[i], flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
		insertNewFlatNodeGain(flatNode->outFlatNodes[i], m_flatNode2group.find(flatNode->outFlatNodes[i])->second, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	}
}

void HClusterProcessGroupPartition::refiningPhase_RemoveOldGain(FlatNode * flatNode, std::map<std::pair<FlatNode*, FlatNodeGroup*>, int>& flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode*, FlatNodeGroup*>>& gain2FlatNode2AdjPT)
{
	std::map<std::pair<FlatNode *, FlatNodeGroup*>, int >::iterator flatNode2AdjPT2Gain_iter = flatNode2AdjPT2Gain.begin();
	while (flatNode2AdjPT2Gain_iter != flatNode2AdjPT2Gain.end())
	{
		if ((flatNode2AdjPT2Gain_iter->first).first == flatNode)
		{//删除
			std::map<std::pair<FlatNode *, FlatNodeGroup*>, int >::iterator tmp = flatNode2AdjPT2Gain_iter;
			flatNode2AdjPT2Gain_iter++;
			flatNode2AdjPT2Gain.erase(tmp);
		}
		else flatNode2AdjPT2Gain_iter++;
	}
	std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> >::iterator gain2FlatNode2AdjPT_iter = gain2FlatNode2AdjPT.begin();
	while (gain2FlatNode2AdjPT_iter != gain2FlatNode2AdjPT.end())
	{
		if ((gain2FlatNode2AdjPT_iter->second).first == flatNode)
		{//删除
			std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> >::iterator tmp = gain2FlatNode2AdjPT_iter;
			gain2FlatNode2AdjPT_iter++;
			gain2FlatNode2AdjPT.erase(tmp);
		}
		else gain2FlatNode2AdjPT_iter++;
	}
}

void HClusterProcessGroupPartition::movingFlatNodeInterGroup(FlatNode* flatNode, FlatNodeGroup* srcGroup, FlatNodeGroup* snkGroup)
{
	std::map<FlatNode*, FlatNodeGroup*>::iterator f_iter = m_flatNode2group.find(flatNode);
	assert(f_iter !=m_flatNode2group.end());
	assert(f_iter->second == srcGroup);
	f_iter->second = snkGroup;
	srcGroup->deleteFlatNode(flatNode);
	snkGroup->insertFlatNode(flatNode);
	//更新与srcGroup和snkGroup相关的group图的依赖关系——要修改的关系主要就是srcGroup，snkGroup的上下端group以及srcGroup，snkGroup的上下端的上下端
	std::set<FlatNodeGroup *> modifyAdjDownGroup;//记录需要修改的group的下端group
	std::set<FlatNodeGroup *> modifyAdjUpGroup;//记录需要修改的group的上端group
											   //src和snk上下端都可能需要改变
	modifyAdjDownGroup.insert(srcGroup);
	modifyAdjDownGroup.insert(snkGroup);
	modifyAdjUpGroup.insert(srcGroup);
	modifyAdjUpGroup.insert(snkGroup);
	std::vector<FlatNodeGroup *>srcUpGroup = srcGroup->getUpGroup();
	std::vector<FlatNodeGroup *>srcDownGroup = srcGroup->getDownGroup();
	std::vector<FlatNodeGroup *>snkUpGroup = snkGroup->getUpGroup();
	std::vector<FlatNodeGroup *>snkDownGroup = snkGroup->getDownGroup();
	//src和snk上端group，只需要修改其下端
	for (int i = 0; i != srcUpGroup.size(); i++)
		modifyAdjDownGroup.insert(srcUpGroup[i]);
	for (int i = 0; i != snkUpGroup.size(); i++)
		modifyAdjDownGroup.insert(snkUpGroup[i]);
	//src和snk下端group，只需要修改其上端
	for (int i = 0; i != srcDownGroup.size(); i++)
		modifyAdjUpGroup.insert(srcDownGroup[i]);
	for (int i = 0; i != snkDownGroup.size(); i++)
		modifyAdjUpGroup.insert(snkDownGroup[i]);
	//修改需要修改其下端的group
	for (std::set<FlatNodeGroup *>::iterator dg_iter = modifyAdjDownGroup.begin(); dg_iter != modifyAdjDownGroup.end(); dg_iter++)
	{
		//先删除当前的group中的下端group
		std::vector<FlatNodeGroup *> down_downGroup = (*dg_iter)->getDownGroup();
		for (int i = 0; i != down_downGroup.size(); i++) (*dg_iter)->deletedownGroup(down_downGroup[i]);
		std::set<FlatNode *> dg_flatNodes = (*dg_iter)->getFlatNodes();
		for (std::set<FlatNode *>::iterator dgf_iter = dg_flatNodes.begin(); dgf_iter != dg_flatNodes.end(); dgf_iter++)
		{//取dg_flatNodes的输出边
			for (int i = 0; i != (*dgf_iter)->outFlatNodes.size(); i++)
			{
				FlatNodeGroup *cur_tmp_downGroup = m_flatNode2group.find((*dgf_iter)->outFlatNodes[i])->second;
				if (cur_tmp_downGroup != *dg_iter)(*dg_iter)->insertdownGroup(cur_tmp_downGroup);
			}
		}
	}
	//修改需要修改其上端的group
	for (std::set<FlatNodeGroup *>::iterator ug_iter = modifyAdjUpGroup.begin(); ug_iter != modifyAdjUpGroup.end(); ug_iter++)
	{
		//先删除当前的group中的上端group
		std::vector<FlatNodeGroup *> up_upGroup = (*ug_iter)->getUpGroup();
		for (int i = 0; i != up_upGroup.size(); i++) (*ug_iter)->deleteupGroup(up_upGroup[i]);
		std::set<FlatNode *> ug_flatNodes = (*ug_iter)->getFlatNodes();
		for (std::set<FlatNode *>::iterator ugf_iter = ug_flatNodes.begin(); ugf_iter != ug_flatNodes.end(); ugf_iter++)
		{//取dg_flatNodes的输入边
			for (int i = 0; i != (*ugf_iter)->inFlatNodes.size(); i++)
			{
				FlatNodeGroup *cur_tmp_upGroup = m_flatNode2group.find((*ugf_iter)->inFlatNodes[i])->second;
				if (cur_tmp_upGroup != *ug_iter) (*ug_iter)->insertupGroup(cur_tmp_upGroup);
			}
		}
	}
}

void HClusterProcessGroupPartition::insertNewFlatNodeGain(FlatNode * flatNode, FlatNodeGroup * curGroup, std::map<std::pair<FlatNode*, FlatNodeGroup*>, int>& flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode*, FlatNodeGroup*>>& gain2FlatNode2AdjPT)
{
	int internalDegree = 0;//记录节点对内的度数
	std::map<FlatNodeGroup *, int> adjPT2ExternalDegree;//当前节点相邻划分与度数之间连接边的数目（节点的相邻的划分，节点与该划分相连边的数目）
	for (int i = 0; i != flatNode->inFlatNodes.size(); i++)
	{
		std::map<FlatNode *, FlatNodeGroup * >::iterator pt_iter = m_flatNode2group.find(flatNode->inFlatNodes[i]);
		assert(pt_iter != m_flatNode2group.end());
		if (pt_iter->second == curGroup) internalDegree++;
		else
		{//更新adjPT2ExternalDegree
			std::pair<std::map<FlatNodeGroup *, int>::iterator, bool> ret = adjPT2ExternalDegree.insert(std::make_pair(pt_iter->second, 1));
			if (!ret.second) ret.first->second += 1;
		}
	}
	for (int i = 0; i != flatNode->outFlatNodes.size(); i++)
	{
		std::map<FlatNode *, FlatNodeGroup * >::iterator pt_iter = m_flatNode2group.find(flatNode->outFlatNodes[i]);
		assert(pt_iter != m_flatNode2group.end());
		if (pt_iter->second == curGroup) internalDegree++;
		else
		{//更新adjPT2ExternalDegree
			std::pair<std::map<FlatNodeGroup *, int>::iterator, bool> ret = adjPT2ExternalDegree.insert(std::make_pair(pt_iter->second, 1));
			if (!ret.second) ret.first->second += 1;
		}
	}
	//根据adjPT2ExternalDegree和internalDegree构造flatNode2AdjPT2Gain和gain2FlatNode2AdjPT
	for (std::map<FlatNodeGroup *, int>::iterator ED_iter = adjPT2ExternalDegree.begin(); ED_iter != adjPT2ExternalDegree.end(); ED_iter++)
	{
		if (ED_iter->second == 0) continue;//对于非边界节点可以暂时不放入优先的对列中，以免增加查找和更新时间
		flatNode2AdjPT2Gain.insert(std::make_pair(std::make_pair(flatNode, ED_iter->first), ED_iter->second - internalDegree));
		gain2FlatNode2AdjPT.insert(std::make_pair(ED_iter->second - internalDegree, std::make_pair(flatNode, ED_iter->first)));
	}
}

void HClusterProcessGroupPartition::constructAdjMatrixInfo()
{
	assert(m_groupAdjMatrix != NULL);
	int num = 0;
	std::vector<FlatNodeGroup *> groupOrginal(m_groups.begin(), m_groups.end());
	std::vector<FlatNodeGroup *> groupTopo = topoSortGroup(groupOrginal);
	assert(m_groups.size() == groupTopo.size());//保证进入粗化的group图是没有环的

	for (std::vector<FlatNodeGroup *>::iterator iter = groupTopo.begin(); iter != groupTopo.end(); iter++)
	{
		m_group2groupNo.insert(std::make_pair(*iter, num));
		m_groupNo2group.insert(std::make_pair(num, *iter));
		m_groupNo2actualNo.push_back(num);
		//确定各个group的负载_groupWorkload
		m_groupWorkload.push_back((*iter)->getWorkload());
		//确定各个group对外的通信量_groupCommunication
		m_groupCommunication.push_back((*iter)->getTotalCommunicateData());
		num++;
	}
	for (std::map<FlatNodeGroup *, int>::iterator iter = m_group2groupNo.begin(); iter != m_group2groupNo.end(); iter++)
	{//根据每个group的下端的group构造邻接矩阵，矩阵上的权值是一对group之间的通行量
		std::map<FlatNodeGroup*, int> curDownGroup2weight = iter->first->getDownGroup2Weight();
		for (std::map<FlatNodeGroup*, int>::iterator down_iter = curDownGroup2weight.begin(); down_iter != curDownGroup2weight.end(); down_iter++)
		{
			int downNo = m_group2groupNo.find(down_iter->first)->second;
			m_groupAdjMatrix[iter->second][downNo] = down_iter->second;
		}
	}
}

float HClusterProcessGroupPartition::computeGain(int group1No, int group2No)
{
	float deltComm = 2 * m_groupAdjMatrix[group1No][group2No];
	float gain = (deltComm *OVERHEAD_PER_COMMUNICATION_DATA) / (float)(m_groupWorkload[group1No] + m_groupWorkload[group2No]);
	//std::cout<<"进程划分收益 "<<gain<<std::endl;
	return gain;
}

FlatNodeGroup::FlatNodeGroup(std::set<FlatNode *> flatNodes, SchedulerSSG *sssg):m_sssg(sssg), m_flatNodes(flatNodes.begin(), flatNodes.end()), m_workload(0), m_totalData(0)
{
	//将vector中的flatnode以及sssg构造成一个group
	std::set<FlatNode *>::const_iterator const_iter = m_flatNodes.begin();
	for (const_iter = m_flatNodes.begin(); const_iter != m_flatNodes.end(); const_iter++)
	{
		m_workload += m_sssg->GetSteadyWork(*const_iter) * m_sssg->GetSteadyCount(*const_iter);
	}
}

int FlatNodeGroup::computeTotalCommunicateData()
{
	//计算当前group中节点对外的总通行量
	int tmpTotalData = 0;
	std::set<FlatNode *>::const_iterator const_iter = m_flatNodes.begin();
	for (const_iter = m_flatNodes.begin(); const_iter != m_flatNodes.end(); const_iter++)
	{
		int steadyCount = m_sssg->GetSteadyCount(*const_iter);
		//输出边
		for (int i = 0; i != (*const_iter)->nOut; i++)
		{
			if (m_flatNodes.count((*const_iter)->outFlatNodes[i]) == 0)
			{//对应的输出端节点不在该group中
				tmpTotalData += steadyCount * (*const_iter)->outPushWeights[i];
			}
		}
		//输入边
		for (int i = 0; i != (*const_iter)->nIn; i++)
		{
			if (m_flatNodes.count((*const_iter)->inFlatNodes[i]) == 0)
			{//对应的输出端节点不在该group中
				tmpTotalData += steadyCount * (*const_iter)->inPopWeights[i];
			}
		}
	}
	return tmpTotalData;
}

inline long FlatNodeGroup::getWorkload()
{
	//取当前group的负载
	return m_workload;
}

int FlatNodeGroup::getCurCommunicateData(FlatNodeGroup *objGroup)
{
	//取当前的group与objGroup之间的通信数据
	std::map<FlatNodeGroup *, int >::iterator iter = m_upGroup2Weight.find(objGroup);
	if (iter != m_upGroup2Weight.end()) return iter->second;
	iter = m_downGroup2Weight.find(objGroup);
	if (iter != m_downGroup2Weight.end()) return iter->second;
	else return 0;
}

FlatNodeGroup * FlatNodeGroup::fusingGroup(FlatNodeGroup *objGroup)
{
	//将当前的group与objGroup融合成一个group同时释放objGroup
	if (this == objGroup) return this;
	std::set<FlatNode *> objFlatNodes = objGroup->getFlatNodes();
	m_flatNodes.insert(objFlatNodes.begin(), objFlatNodes.end());
	m_workload += objGroup->getWorkload();
	//先从当前group的上下端中删除objGroup,同时更新了_totalData
	deletedownGroup(objGroup);
	deleteupGroup(objGroup);
	//修改融合后的group的上端的Group,同时更新_totalData
	std::map<FlatNodeGroup *, int> objUpGroup2Weight = objGroup->getUpGroup2Weight();
	for (std::map<FlatNodeGroup *, int>::iterator up_iter = objUpGroup2Weight.begin(); up_iter != objUpGroup2Weight.end(); up_iter++)
	{
		if (up_iter->first == this) continue;
		std::pair<std::map<FlatNodeGroup *, int>::iterator, int> ret = m_upGroup2Weight.insert(std::make_pair(up_iter->first, up_iter->second));
		if (!ret.second) ret.first->second += up_iter->second;
		m_totalData += up_iter->second;
	}
	//修改融合后的group的下端group,同时更新_totalData
	std::map<FlatNodeGroup *, int> objDownGroup2Weight = objGroup->getDownGroup2Weight();
	for (std::map<FlatNodeGroup *, int>::iterator down_iter = objDownGroup2Weight.begin(); down_iter != objDownGroup2Weight.end(); down_iter++)
	{
		if (down_iter->first == this) continue;
		std::pair<std::map<FlatNodeGroup *, int>::iterator, int> ret = m_downGroup2Weight.insert(std::make_pair(down_iter->first, down_iter->second));
		if (!ret.second) ret.first->second += down_iter->second;
		m_totalData += down_iter->second;
	}
	//修改objGroup的上下端group的上下端group
	for (std::map<FlatNodeGroup *, int>::iterator objup_iter = objUpGroup2Weight.begin(); objup_iter != objUpGroup2Weight.end(); objup_iter++)
	{//要修改objGroup的上端group的_dowmGroup2Weight
		if (objup_iter->first == this) continue;
		objup_iter->first->deletedownGroup(objGroup);//删除旧的
		objup_iter->first->insertdownGroup(this);//插入新的
	}
	for (std::map<FlatNodeGroup *, int>::iterator objdown_iter = objDownGroup2Weight.begin(); objdown_iter != objDownGroup2Weight.end(); objdown_iter++)
	{//要修改objGroup的下端group的_upGroup2Weight
		if (objdown_iter->first == this) continue;
		objdown_iter->first->deleteupGroup(objGroup);//删除旧的
		objdown_iter->first->insertupGroup(this);//插入新的
	}
	delete(objGroup);
	return this;
}

void FlatNodeGroup::insertupGroup(FlatNodeGroup *objGroup)
{
	//向当前的group的上端节点添加group
	std::set<FlatNode *> tmpObjFlatNodes = objGroup->getFlatNodes();
	int tmpTotalData = 0;
	std::set<FlatNode *>::const_iterator const_iter = m_flatNodes.begin();
	for (const_iter = m_flatNodes.begin(); const_iter != m_flatNodes.end(); const_iter++)
	{
		int steadyCount = m_sssg->GetSteadyCount(*const_iter);
		//输入边
		for (int i = 0; i != (*const_iter)->nIn; i++)
		{
			if (tmpObjFlatNodes.count((*const_iter)->inFlatNodes[i]))
			{//flatNode对应的上端节点在tmpObjFlatNodes中，他们之间有通信
				tmpTotalData += steadyCount * (*const_iter)->inPopWeights[i];
			}
		}
	}
	std::pair<std::map<FlatNodeGroup *, int>::iterator, int> ret = m_upGroup2Weight.insert(std::make_pair(objGroup, tmpTotalData));
	if (ret.second)m_totalData += tmpTotalData;//插入成功修改总的通信量
	else
	{
		m_totalData += tmpTotalData - ret.first->second;
		ret.first->second = tmpTotalData;
	}
}

void FlatNodeGroup::insertdownGroup(FlatNodeGroup *objGroup)
{
	//向当前group的下端节点添加group
	std::set<FlatNode *> tmpObjFlatNodes = objGroup->getFlatNodes();
	int tmpTotalData = 0;
	std::set<FlatNode *>::const_iterator const_iter = m_flatNodes.begin();
	for (const_iter = m_flatNodes.begin(); const_iter != m_flatNodes.end(); const_iter++)
	{
		int steadyCount = m_sssg->GetSteadyCount(*const_iter);
		//输出边
		for (int i = 0; i != (*const_iter)->nOut; i++)
		{
			if (tmpObjFlatNodes.count((*const_iter)->outFlatNodes[i]))
			{//对应的输出端节点在该group中
				tmpTotalData += steadyCount * (*const_iter)->outPushWeights[i];
			}
		}
	}
	std::pair<std::map<FlatNodeGroup *, int>::iterator, int> ret = m_downGroup2Weight.insert(std::make_pair(objGroup, tmpTotalData));
	if (ret.second)m_totalData += tmpTotalData;//插入成功修改总的通信量
	else
	{
		m_totalData += tmpTotalData - ret.first->second;
		ret.first->second = tmpTotalData;
	}
}

std::vector<FlatNodeGroup*> FlatNodeGroup::getUpGroup()
{
	std::vector<FlatNodeGroup *> upGroup;
	for (std::map<FlatNodeGroup *, int>::const_iterator const_iter = m_upGroup2Weight.begin(); const_iter != m_upGroup2Weight.end(); const_iter++)
		upGroup.push_back(const_iter->first);
	return upGroup;
}

std::vector<FlatNodeGroup*> FlatNodeGroup::getDownGroup()
{
	std::vector<FlatNodeGroup *> downGroup;
	for (std::map<FlatNodeGroup *, int>::const_iterator const_iter = m_downGroup2Weight.begin(); const_iter != m_downGroup2Weight.end(); const_iter++)
		downGroup.push_back(const_iter->first);
	return downGroup;
}

void FlatNodeGroup::insertFlatNode(FlatNode *objFlatNode)
{
	//向当前group中添加一个flatNode节点
	m_flatNodes.insert(objFlatNode);
	m_workload += m_sssg->GetSteadyWork(objFlatNode) * m_sssg->GetSteadyCount(objFlatNode);
	m_totalData = computeTotalCommunicateData();
	//使当前group的上下端的group无效,关系的维护交给该接口的调用者
	m_upGroup2Weight.clear();
	m_downGroup2Weight.clear();
}

void FlatNodeGroup::deleteFlatNode(FlatNode *objFlatNode)
{
	//从当前group中删除一个flatNode节点
	m_flatNodes.erase(objFlatNode);
	m_workload -= m_sssg->GetSteadyWork(objFlatNode) * m_sssg->GetSteadyCount(objFlatNode);
	m_totalData = computeTotalCommunicateData();
	//使当前group的上下端的group无效,关系的维护交给该接口的调用者
	m_upGroup2Weight.clear();
	m_downGroup2Weight.clear();
}

