#include "HClusterProcessGroupPartition.h"

HClusterProcessGroupPartition::HClusterProcessGroupPartition(SchedulerSSG * _sssg, int _nClusters):m_sssg(_sssg),m_nClusters(_nClusters)
{
	std::vector<FlatNode *> flatNodes = _sssg->GetFlatNodes();
	m_avgWorkload = computeWorkload(flatNodes) / m_nClusters;//���������ϵ�ƽ������

	constructGroups();
	//Ϊ�ڽӾ���m_groupAdjMatrix���ٿռ�
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
	//std::cout << "�칹��Ⱥ���--�黮��---Ԥ����׶Σ�" << endl;
	preProcess();
	//std::cout << "�칹��Ⱥ���---�黮��---�����Ƚ׶Σ�" << endl;
	coarseGrainedPartition();
	//std::cout << "�칹��Ⱥ���---�黮��---��ʼ���ֽ׶Σ�" << endl;
	initialPartition();
	//cout << "�칹��Ⱥ���---�黮��---ϸ�������׶Σ�" << endl;
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
	//����������Ҫ���ܣ��齨��ʼ��groupͼ��һ��groupֻ��һ���ڵ㣩��ȷ��_groups��_flatNode2group��ֵ
	std::set<FlatNodeGroup *> basicGroup;//��ʱ�����ʼ��group
	std::vector<FlatNode *> flatNodes = m_sssg->GetFlatNodes();
	std::map<FlatNode *, FlatNodeGroup*> tmpFlatNode2group;
	//���ɳ�ʼ��flatNodeGroup���ڴ�ʱ��group����һ��groupֻ��һ��flatNode��
	for (int i = 0; i != flatNodes.size(); i++)
	{
		std::set<FlatNode *> tmpFlatNodeSet;
		tmpFlatNodeSet.insert(flatNodes[i]);
		FlatNodeGroup *tmpGroup = new FlatNodeGroup(tmpFlatNodeSet, m_sssg);
		basicGroup.insert(tmpGroup);
		tmpFlatNode2group.insert(std::make_pair(flatNodes[i], tmpGroup));
	}
	//ȷ��group��������ϵ��ȷ��flatNodeGroup�е����¶�group��
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

	std::set<FlatNodeGroup *> IsProcessed;//�����Ѿ���Ԥ�������group�ļ��ϣ���ֹ���ظ�����
	std::map<FlatNodeGroup *, std::set<FlatNodeGroup *> > mainGroup2slaveGroups;//��¼�ܹ����ֻ���һ���group(�����set�а���mainGroup)
	std::vector<FlatNodeGroup *> groupOrginal(m_groups.begin(), m_groups.end());
	std::vector<FlatNodeGroup *> groupTopo = topoSortGroup(groupOrginal);
	assert(m_groups.size() == groupTopo.size());

	for (std::vector<FlatNodeGroup *>::iterator gIter = groupTopo.begin(); gIter != groupTopo.end(); gIter++)
	{//���ǵ�ǰ�ڵ㣨����Ԥ����������Ԥ����
		FlatNodeGroup* cur_group = *gIter;//ȡ��ǰ��group
		if (IsProcessed.count(cur_group)) continue;//��ǰ�ڵ㱻������ˡ��������Ѿ����������group
		if (cur_group->getDownGroup().size() <= 1 && cur_group->getUpGroup().size() <= 1) continue;//������ҪԤ�����group
		long groupSetWorkload = cur_group->getWorkload();
		if (groupSetWorkload > m_avgWorkload) continue;//��ǰgroup�ĸ����Ѿ����ھ�ֵ
		std::set<FlatNodeGroup*> potential_Group;//ȡ�ܹ�������һ���group�ļ��ϣ��϶����¶˵��ܺͣ�
		potential_Group.clear();
		//�Կ�����Ҫ��Ԥ����Ľڵ����Ԥ����������ͨ�бߵ���Ŀ
		std::vector<FlatNodeGroup*> all_upGroup = cur_group->getUpGroup();
		//�ռ���ǰ�ڵ���϶˽ڵ������������ĿΪ1�Ľڵ㣬����һ��set�У�
		//������еĽڵ���϶˽ڵ㶼������һ��set��,���ж��϶˵��϶˽ڵ��ǲ���ͬһ���������Ҳ�ӽ���
		if (all_upGroup.size() > 1)
		{
			//�϶˽ڵ���Ŀ����1
			std::multimap<long, FlatNodeGroup*> potential_upGroup;//(������group֮���map)ȡ�ܹ�������һ����϶�group�ļ���
			for (int up_iter = 0; up_iter != all_upGroup.size(); up_iter++)
			{
				//����ڵ��ǵ��뵥���ļӵ�potential_upGroup��
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
					//�ж����϶˽ڵ��ܲ��ܱ��Ž����������϶˵����ԣ�potential_upGroup�еĽڵ�Ķ����亢�ӽڵ㣬������û�������ĺ���ʱ������룬�������
					std::vector<FlatNodeGroup *>upupGroupVec = potential_upGroup.begin()->second->getUpGroup();
					FlatNodeGroup *upupGroup = NULL;
					if (!upupGroupVec.empty()) upupGroup = upupGroupVec[0];
					std::vector<FlatNodeGroup *>upupdownGroupVec = upupGroup->getDownGroup();
					//�ж�upupdownGroupVec��potential_upGroup�еĽڵ��ǲ���һ��
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
				//�����ش�Ľڵ����ߣ����µĽڵ����ֱ�����뵱ǰ�ڵ���
				//�������potential_upGroup�����߸��ش��group
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
		{//�¶˽ڵ����Ŀ����1������ǰ�ڵ����¶˽ڵ���д����϶˵Ľڵ����¶˽ڵ���Կ����Ƕ����ģ�
			std::multimap<long, FlatNodeGroup*> potential_downGroup;//(������group֮���map)ȡ�ܹ�������һ����϶�group�ļ���
			for (int down_iter = 0; down_iter != all_downGroup.size(); down_iter++)
			{//����ڵ��ǵ��뵥���Ĳ���û�б�Ԥ��������ӵ�potential_upGroup��
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
				{//�ж����϶˽ڵ��ܲ��ܱ��Ž����������϶˵����ԣ�potential_upGroup�еĽڵ�Ķ����亢�ӽڵ㣬������û�������ĺ���ʱ������룬�������
					std::vector<FlatNodeGroup *>downdownGroupVec = potential_downGroup.begin()->second->getDownGroup();
					FlatNodeGroup *downdownGroup = NULL;
					if (!downdownGroupVec.empty()) downdownGroup = downdownGroupVec[0];
					std::vector<FlatNodeGroup *>downdownupGroupVec = downdownGroup->getUpGroup();
					//�ж�upupdownGroupVec��potential_upGroup�еĽڵ��ǲ���һ��
					std::set<FlatNodeGroup *> downdownupGroupSet(downdownupGroupVec.begin(), downdownupGroupVec.end());
					for (std::multimap<long, FlatNodeGroup*>::iterator pg_iter = potential_downGroup.begin(); pg_iter != potential_downGroup.end(); pg_iter++)
					{
						if (!downdownupGroupSet.count(pg_iter->second)) { FG_up = TRUE; break; }
					}
					if (!FG_up) potential_downGroup.insert(std::make_pair(downdownGroup->getWorkload(), downdownGroup));
				}
			}
			else
			{//�����ش�Ľڵ����ߣ����µĽڵ����ֱ�����뵱ǰ�ڵ���
			 //�������potential_upGroup�����߸��ش��group
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
		//����potential_Group�޸�IsProcessed
		IsProcessed.insert(potential_Group.begin(), potential_Group.end());
		mainGroup2slaveGroups.insert(std::make_pair(cur_group, potential_Group));
	}
	if (!mainGroup2slaveGroups.empty()) 
		updateGroupGraph(mainGroup2slaveGroups);//����Ԥ������������groupGraph
}

void HClusterProcessGroupPartition::coarseGrainedPartition()
{
	int boundaryGroupNum = m_nClusters * COARSEN_FACTOR;//�ֻ�ʱgroup����������
	int originalGroupNum = m_groups.size();//Ԥ�����ͼ��group����Ŀ
	if (originalGroupNum <= boundaryGroupNum)return;
	constructAdjMatrixInfo();//����Ԥ������ͼ�����ڽӾ���(Ԥ����Ĵ�������ǿ���ѡ���)
							 //�������漰�������ݽṹ��_groupAdjMatrix��_groupWorkload��_groupNo2actualNo���Լ��¶���Ľṹ(map<pair<int,int>,float>)���������ڽڵ�ֻ�����������
	std::map<std::pair<int, int>, float > grouptogroup2Gain;//����һ��group������һ�����������
	//���ӹ�ϵ�������ڽӾ��󱣴棬�������map���������������������������ݽṹ���ֲ���
	int curGroupNum = originalGroupNum;//��¼��ǰ�ڵ����Ŀ
	//���ݳ�ʼ��groupͼ����ֻ�һ�����ٽڵ�ĳ�ʼ������
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
	//��ѡ�������Ľ��дֻ�group
	while (curGroupNum > boundaryGroupNum)
	{
		//��group����ĿС��boundaryGroupNumʱֹͣ�ֻ�
		//�������������Ŀɴֻ��������ˣ��Ľڵ㣬ͬʱ���ݴֻ�����������ص���Ϣ
		std::multimap<float, std::pair<int, int> > tmpGain2grouptogroup;
		for (std::map<std::pair<int, int>, float >::iterator g2p_iter = grouptogroup2Gain.begin(); g2p_iter != grouptogroup2Gain.end(); g2p_iter++)
			tmpGain2grouptogroup.insert(std::make_pair(g2p_iter->second, g2p_iter->first));
		std::multimap<float, std::pair<int, int> >::reverse_iterator r_p2g_iter = tmpGain2grouptogroup.rbegin();
		while (r_p2g_iter != tmpGain2grouptogroup.rend())
		{
			//����p2g_iter��ָ��һ��group�ܷ񱻴ֻ���һ����Ҫ��ͨ����������������͸��ؾ���ĽǶ������죩
			//���һ�Խڵ�ϲ���Ľڵ�ĸ��ش��ھ�ֵ���ܹ�����һ��
			if (m_groupWorkload[(r_p2g_iter->second).first] + m_groupWorkload[(r_p2g_iter->second).second] > m_avgWorkload) r_p2g_iter++;
			else if (topoSortGroup((r_p2g_iter->second).first, (r_p2g_iter->second).second, originalGroupNum))//��������group����һ��
				break;
			else r_p2g_iter++;
		}
#if 0
		for (int i = 0; i != _groupNo2actualNo.size(); i++) std::cout << _groupNo2actualNo[i] << " ";
		std::cout << std::endl;
#endif
		if (r_p2g_iter != tmpGain2grouptogroup.rend())
		{
			//���ݴֻ���������޸�_groupNo2actualNo��_groupWorkload��_groupCommunication��_groupAdjMatrix��grouptogroup2Gain
			//Ҫ�����е�ֵΪ(r_p2g_iter->second).second��λ�õ�ֵ��Ϊ (r_p2g_iter->second).first;
			for (int i = 0; i != m_groupNo2actualNo.size(); i++)
				if (m_groupNo2actualNo[i] == (r_p2g_iter->second).second) m_groupNo2actualNo[i] = (r_p2g_iter->second).first;
			m_groupWorkload[(r_p2g_iter->second).first] += m_groupWorkload[(r_p2g_iter->second).second];
			m_groupWorkload[(r_p2g_iter->second).second] = 0;
			m_groupCommunication[(r_p2g_iter->second).first] += m_groupCommunication[(r_p2g_iter->second).second] - 2 * m_groupAdjMatrix[(r_p2g_iter->second).first][(r_p2g_iter->second).second];
			m_groupCommunication[(r_p2g_iter->second).second] = 0;
			//ɾ��grouptogroup2Gain�й�ʱ����Ϣ����group1��group2Ϊͷ��Ϊ��pair��
			std::map<std::pair<int, int>, float >::iterator del_g2p_iter = grouptogroup2Gain.begin();
			while (del_g2p_iter != grouptogroup2Gain.end()) {
				if (del_g2p_iter->first.first == (r_p2g_iter->second).first || del_g2p_iter->first.first == (r_p2g_iter->second).second || del_g2p_iter->first.second == (r_p2g_iter->second).first || del_g2p_iter->first.second == (r_p2g_iter->second).second)
				{//ɾ����ǰ��
					std::map<std::pair<int, int>, float >::iterator tmp_del = del_g2p_iter;
					del_g2p_iter++;
					grouptogroup2Gain.erase(tmp_del);
				}
				else del_g2p_iter++;
			}
			//����_groupAdjMatrix
			updateAdjMatrixInfo((r_p2g_iter->second).first, (r_p2g_iter->second).second, m_groupAdjMatrix, originalGroupNum);
			//��grouptogroup2Gain����µ���Ϣ��������_groupAdjMatrix������(r_p2g_iter->second).first��������ϵ�ĵ���뵽grouptogroup2Gain��
			//�����ڽӾ�����(r_p2g_iter->second).first���к���
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
		else break;//�ֻ���������Ҳ�Ҳ����ܹ����ֻ���group��
		curGroupNum--;
	}
	//���ݴֻ��Ľ���ع�groupͼ
	updataGroupGraphByCoarsening(originalGroupNum);
		
	std::vector<FlatNodeGroup *> groupOrginal(m_groups.begin(), m_groups.end());
	std::vector<FlatNodeGroup *> groupTopo = topoSortGroup(groupOrginal);
	assert(m_groups.size() == groupTopo.size());//��֤�ֻ������groupͼ��û�л���
}

void HClusterProcessGroupPartition::initialPartition()
{
	std::vector<FlatNodeGroup *> tmp_groups(m_groups.begin(), m_groups.end());
	std::vector<FlatNodeGroup *> topoGroups = topoSortGroup(tmp_groups);
	assert(topoGroups.size() == m_groups.size());
	//����Ĳ����������topoGroups���еġ����������˵������������ڵ㱻���ֵĴ���ͬʱ��ȷ��һ��group���Ǹ�������ʱ���ڶ�ͨ��������һ���Ŀ��죨�������ϵ�����
	std::map<FlatNodeGroup *, int> initGroup2partition;//group�뻮�ֱ��֮���map
													   //��¼��ǰ������Ҫ����Ϣ
	std::vector<long> partition_workload(m_nClusters, 0);//��¼�������ֵĸ���
	int p_no = 0;//��ǰȷ�������Ӽ��ı�ţ���ǰ������ǵڼ������֣���ֵһ��С��_ncluster��
	std::vector<FlatNodeGroup *> tailGroup;
	for (int i = 0; i != topoGroups.size(); i++)
	{
		if (p_no == m_nClusters) { tailGroup.push_back(topoGroups[i]); continue; }
		long tmp_partition_workload = partition_workload[p_no] + topoGroups[i]->getWorkload();

		if ((double)partition_workload[p_no] - (double)m_avgWorkload * 0.7 <= EXP || (double)tmp_partition_workload - (double)m_avgWorkload * MINBALANCEFACTOR <= EXP)
		{
			//�����ǰ�ĸ���С��(��ǰ�����ϵĸ��ر�ƽ����һ�㻹С�����ڵ�ǰ�Ļ����м���group�Ĺ�������û�дﵽ��ֵ)Ӧ�õõ�����Сֵ��ֱ�����
			partition_workload[p_no] += topoGroups[i]->getWorkload();
		}
		else if ((double)tmp_partition_workload - (double)m_avgWorkload * MAXBALANCEFACTOR <= EXP)
		{
			//�ο�ͨ�ŵĿ��������жϵ�ǰ�ڵ��費��Ҫ���ڸû�����
			//ȡ��ǰgroup���϶�
			std::map<FlatNodeGroup*, int> cur_upGroup2weight = topoGroups[i]->getUpGroup2Weight();
			//�жϵ�ǰgroup��p_no�е�ͨ����
			int initPTcomm = 0;
			std::set<int> cur_upcluster;
			for (std::map<FlatNodeGroup*, int>::iterator cur_up_iter = cur_upGroup2weight.begin(); cur_up_iter != cur_upGroup2weight.end(); cur_up_iter++)
			{
				std::map<FlatNodeGroup *, int>::iterator up_pt_iter = initGroup2partition.find(cur_up_iter->first);
				if (up_pt_iter != initGroup2partition.end()) cur_upcluster.insert(up_pt_iter->second);
				if (up_pt_iter != initGroup2partition.end() && up_pt_iter->second == p_no)initPTcomm += cur_up_iter->second;
			}
			int curComm = topoGroups[i]->getTotalCommunicateData();//ȡ��ǰ�ڵ��ܵ�ͨ����
			int curCommEdge = cur_upcluster.size() + topoGroups[i]->getDownGroup2Weight().size();//�����������ٸ�clusterͨ��
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
	//��initGroup2partition��ʼ����ת����flatNode��cluster֮���map
	for (std::map<FlatNodeGroup *, int>::iterator initPT_iter = initGroup2partition.begin(); initPT_iter != initGroup2partition.end(); initPT_iter++)
	{
		std::set<FlatNode *> flatNodeSet = initPT_iter->first->getFlatNodes();
		for (std::set<FlatNode *>::iterator fn_iter = flatNodeSet.begin(); fn_iter != flatNodeSet.end(); fn_iter++)
		{
			m_cluster2flatNode.insert(std::make_pair(initPT_iter->second, *fn_iter));
			m_flatNode2cluster.insert(std::make_pair(*fn_iter, initPT_iter->second));
		}
	}
	//����ʼ���ֺ���һ��cluster�ϵ�group�ںϳ�һ��group
	std::multimap<int, FlatNodeGroup*> partition2initGroup;//���ֱ����group֮���map
	for (std::map<FlatNodeGroup *, int>::iterator initPT_iter = initGroup2partition.begin(); initPT_iter != initGroup2partition.end(); initPT_iter++)
		partition2initGroup.insert(std::make_pair(initPT_iter->second, initPT_iter->first));
	for (int i = 0; i != m_nClusters; i++)
	{
		std::pair<std::multimap<int, FlatNodeGroup *>::iterator, std::multimap<int, FlatNodeGroup *>::iterator> pos = partition2initGroup.equal_range(i);
		assert(pos.first != partition2initGroup.end());//ȷ��ÿһ����Ⱥ�ڵ��϶���group
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
	//------ȷ��������ʼ������ɺ�flatNode��group֮���map
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
	std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > flatNode2AdjPT2Gain;//flatNode�������Ŀ��partition���Բ��������棨flatNode�����ڵĻ��֣����棩
	std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> > gain2FlatNode2AdjPT;//��gain��ά�������ȶ��У����棬flatNode�����ڵĻ��֣�
	refiningPhase_CollectBoundFlatNode(flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	assert(flatNode2AdjPT2Gain.size() == gain2FlatNode2AdjPT.size());
	//��������ȶ���gain2FlatNode2AdjPT�е�Ԫ�ؽ��е��������ڴ���Ĺ�����Ҫ�������ȶ��У�����ED > ID�Ľڵ㣬ɾ��ED > ID�Ľڵ㣩
	//������ֹ�������ǣ��ٲ��ƻ�ͼ��ƽ�⣬��û�нڵ�����ƶ����ڲ������뻷����û�нڵ�����ƶ����۶���Ϊ�գ��ܶ��������нڵ���ƶ�����ʹȫ��ͨ�ű�����
	while (!gain2FlatNode2AdjPT.empty())
	{//����ϸ���ȵ���gain2FlatNode2AdjPT,flatNode2AdjPT2Gain
		std::multimap<int, std::pair<FlatNode *, FlatNodeGroup *> >::reverse_iterator adjIter = gain2FlatNode2AdjPT.rbegin();
		FlatNode *adjFlatNode = adjIter->second.first;//ȡ��ǰҪ��������flatNode
		FlatNodeGroup* srcGroup = m_flatNode2group.find(adjFlatNode)->second;
		if (refiningPhase_IsAdjustableFlatNode(adjFlatNode, adjIter->second.second, adjIter->first))
		{
			//����ܹ��������������gain2FlatNode2AdjPT,flatNode2AdjPT2Gain
			//adjFlatNode���������ڵ�group
			//std::cout<<"�ƶ�flatNode		"<<adjFlatNode->name<<"\n";
			refiningPhase_UpdatePriQueue(adjFlatNode, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
		}
		//���ܵ����Ƿ�ɹ���flatNode�Ӷ�����ɾ��
		flatNode2AdjPT2Gain.erase(adjIter->second);
		gain2FlatNode2AdjPT.erase(--adjIter.base());
	}
	assert(m_groups.size() == m_nClusters);
	//��⻮�֡����ڻ����в��������������Ľڵ���ڣ���һ���ڵ��������������ڵ�ǰ�Ļ����У�������������Ľڵ���Ҫ���ýڵ����϶˽ڵ�����������¶˽ڵ����
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
		//Ϊ�����⴦����һ�����ִ���һ���ڵ���û������������еĽڵ㶼�����ڵ����-------begin
		//���ڵ�����һ��flatNode��һ��group��--------------------------------------
		//�������ڽڵ�����group�й�������С�Ľ�������
		std::set<FlatNodeGroup *>::iterator obj_iter = objGroupSet.begin();
		FlatNodeGroup *objG = *obj_iter; int objW = (*obj_iter)->getWorkload();
		for (; obj_iter != objGroupSet.end(); ++obj_iter)
		{
			if (objW > (*obj_iter)->getWorkload()) { objW = (*obj_iter)->getWorkload(); objG = *obj_iter; }
		}
		std::cout << "\n--------XXXXXXXXXX���⴦��XXXXXXXXXXXX--------\n" << std::endl;
		movingFlatNodeInterGroup(r_iter->first, r_iter->second, objG);
		objGroupSet.clear();
	}//---------------------------------------------------------------------------------------end
	 //��group�е�����ת����group2cluster��cluster2group��
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
	//���ݻ��ֽ���޸ı߽�ڵ����������������ͨ����ת���ɼ�������-20131201
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
	//���ڱ߽�ڵ�Ҫ����ͨ������������actor�ĸ���
	std::set<FlatNode *>flatNodes(flatNodeV.begin(), flatNodeV.end());
	for (std::set<FlatNode *>::iterator s_iter = flatNodes.begin(); s_iter != flatNodes.end(); s_iter++)
	{
		long oldWorkload = m_sssg->GetSteadyWork(*s_iter);
		long curWorkload = oldWorkload;
		//����flatNode������ڵ�
		for (int i = 0; i != (*s_iter)->outFlatNodes.size(); i++)
		{
			if (flatNodes.count((*s_iter)->outFlatNodes[i])) continue;
			else
			{
				curWorkload += OVERHEAD_PER_COMMUNICATION_DATA * (*s_iter)->outPushWeights[i];//+ COMM_STRATUP_TIME;
			}
		}
		//����flatNode������ڵ�
		for (int i = 0; i != (*s_iter)->inFlatNodes.size(); i++)
		{
			if (flatNodes.count((*s_iter)->inFlatNodes[i])) continue;
			else
			{
				curWorkload += OVERHEAD_PER_COMMUNICATION_DATA * (*s_iter)->inPopWeights[i];// + COMM_STRATUP_TIME;
			}
		}
		//�޸�SSG�е�flatNode��Ӧ�ĸ���
		if (curWorkload != oldWorkload)
		{
			std::map<FlatNode *, int>::iterator fn_iter = m_sssg->mapSteadyWork2FlatNode.find(*s_iter);
			assert(fn_iter != m_sssg->mapSteadyWork2FlatNode.end());
			fn_iter->second = curWorkload;
			std::cout << (*s_iter)->name << "���صı仯" << oldWorkload << "------>" << curWorkload << "	delt " << curWorkload - oldWorkload << std::endl;
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
	std::vector<int> nInDegree;//���ڱ�����ڵ�����
	std::vector<FlatNodeGroup *> groupStack;
	int nsize = original.size();
	int count = 0;
	for (iter = original.begin(); iter != original.end(); ++iter)
	{
		nInDegree.push_back(((*iter)->getUpGroup()).size());//�����ڵ���ȱ���
	}
	for (int i = 0; i != nInDegree.size(); i++)
	{
		if (!nInDegree[i]) groupStack.push_back(original[i]);
	}
	while (!groupStack.empty())
	{
		FlatNodeGroup *group = groupStack.back();// ȡ��Ҫ��ջ�Ľڵ�
		std::vector<FlatNodeGroup *>tmpProClusterGroup = group->getDownGroup();//ȡgroup�����к��
		groupStack.pop_back();
		topoGroup.push_back(group);
		for (int i = 0; i != tmpProClusterGroup.size(); i++)//������group���ڽӵ����ȼ�1
		{
			for (int j = 0; j != original.size(); j++)
			{
				if (original[j] == tmpProClusterGroup[i])
				{
					if (!(--nInDegree[j])) groupStack.push_back(original[j]);//���Ϊ0���ջ
				}
			}
		}
	}
	return topoGroup;
}

Bool HClusterProcessGroupPartition::topoSortGroup(int group1, int group2, int nsize)
{
	//�����ڽӾ��������������ж��ڻ��ֹ����л᲻���������
	std::vector<int> TopoOrder;
	std::vector<int> nInDegree(nsize, 0);//���ڱ�����ڵ�����
	std::vector<int> groupStack;
	//Ҫ�ȶԾ�������޸ģ�Ȼ�����ж��Ƿ���ڻ������Զ�һ���������޸ģ�
	for (int i = 0; i != nsize; i++)
		for (int j = 0; j < nsize; j++)
			m_groupAdjMatrix_copy[i][j] = m_groupAdjMatrix[i][j];
	updateAdjMatrixInfo(group1, group2, m_groupAdjMatrix_copy, nsize);

	for (int i = 0; i != nsize; ++i)
	{
		//ͳ�Ƹ����ڵ����ȣ�������У�
		if (m_groupAdjMatrix_copy[i][i] == -1)
		{
			nInDegree[i] = -1;//���Ϊ-1��ʾ��ֵ�ǲ����ڵ� 
			continue;//��ǰ�Ľڵ�����Ч��
		}
		int inCount = 0;
		for (int j = 0; j < nsize; j++)
		{
			if (m_groupAdjMatrix_copy[j][i] != 0 && m_groupAdjMatrix_copy[j][i] != -1) inCount++;
		}
		nInDegree[i] = inCount;//�����ڵ���ȱ���
	}
	int invalid_group = 0;
	for (int i = 0; i != nsize; i++)
	{
		if (!nInDegree[i]) groupStack.push_back(i);//�����Ϊ0�Ľڵ��ȷŵ�groupStack��
		else if (nInDegree[i] == -1) invalid_group++;//��¼��Ч�ڵ�
	}
	while (!groupStack.empty())
	{
		int tmpGroupNo = groupStack.back();// ȡ��Ҫ��ջ�Ľڵ�
		TopoOrder.push_back(tmpGroupNo);
		groupStack.pop_back();
		if (nInDegree[tmpGroupNo] == -1) continue;
		for (int i = 0; i != nsize; i++)//������group���ڽӵ����ȼ�1
		{
			if (m_groupAdjMatrix_copy[tmpGroupNo][i] != 0 && m_groupAdjMatrix_copy[tmpGroupNo][i] != -1)
			{
				if (!(--nInDegree[i])) groupStack.push_back(i);//���Ϊ0���ջ
			}
		}
	}
	if (TopoOrder.size() + invalid_group == nsize)return TRUE;
	else return FALSE;
}

void HClusterProcessGroupPartition::updateGroupGraph(std::map<FlatNodeGroup*, std::set<FlatNodeGroup*>> Preprocess)
{
	//����Ԥ����Ľ���޸�GroupGraph�����ӹ�ϵ��fusingGroup���Ѿ�ά�����ˣ�
	for (std::map<FlatNodeGroup *, std::set<FlatNodeGroup *> >::iterator iter = Preprocess.begin(); iter != Preprocess.end(); iter++)
	{
		for (std::set<FlatNodeGroup *>::iterator pp_iter = (iter->second).begin(); pp_iter != (iter->second).end(); pp_iter++)
		{
			//�ȴ�_group��ɾ����Ҫ���ںϵ�FlatNodeGroup
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
		adjMatrix[group2][i] = -1;//-1�����õ�洢��ֵ����Ч��
		adjMatrix[i][group2] = -1;
	}
}

void HClusterProcessGroupPartition::updataGroupGraphByCoarsening(int original_size)
{
	std::map<int, std::set<int> > actualNo2originalNoSet;//group��ǰ�ı����ԭʼ���֮���map
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
			//�ȴ�_group��ɾ����Ҫ���ںϵ�FlatNodeGroup
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
	FlatNodeGroup* srcGroup = m_flatNode2group.find(flatNode)->second;//ȡflatNode���ڵ�partition

	int srcGroup_commData = srcGroup->getTotalCommunicateData() * OVERHEAD_PER_COMMUNICATION_DATA;
	int snkGroup_commData = snkGroup->getTotalCommunicateData() * OVERHEAD_PER_COMMUNICATION_DATA;
	int srcGroup_workload = srcGroup->getWorkload();
	int snkGroup_workload = snkGroup->getWorkload();

	movingFlatNodeInterGroup(flatNode, srcGroup, snkGroup);
	std::vector<FlatNodeGroup *> groupVec(m_groups.begin(), m_groups.end());
	std::vector<FlatNodeGroup *> groupTopo = topoSortGroup(groupVec);
	if (groupTopo.size() != m_groups.size())
	{//�л�
		movingFlatNodeInterGroup(flatNode, snkGroup, srcGroup);//�����ϴε��ƶ�
		return FALSE;
	}
	//��������ƶ���Ч��--------����Ҫ����ʵ��������������
	if (((float)snkGroup->getWorkload()) / srcGroup->getWorkload() > BALANCE_FACTOE)
	{
		movingFlatNodeInterGroup(flatNode, snkGroup, srcGroup);//�����ϴε��ƶ�
		return FALSE;
	}
	if (gain > 0)
		return TRUE;
	else//gain <= 0
	{
		//��ص�����group�ļ���ͨ�űȵ������ĽǶ�������
		int srcGroup_commData_after = srcGroup->getTotalCommunicateData() * OVERHEAD_PER_COMMUNICATION_DATA;
		int snkGroup_commData_after = snkGroup->getTotalCommunicateData() * OVERHEAD_PER_COMMUNICATION_DATA;
		int srcGroup_workload_after = srcGroup->getWorkload();
		int snkGroup_workload_after = snkGroup->getWorkload();
		float radio = (float)srcGroup_workload / srcGroup_commData + (float)snkGroup_workload / snkGroup_commData;
		float radio_after = (float)srcGroup_workload_after / srcGroup_commData_after + (float)snkGroup_workload_after / snkGroup_commData_after;
		if (radio_after - radio > 0.1)
			return TRUE;//ע��float��0�ıȽ�
	}
	movingFlatNodeInterGroup(flatNode, snkGroup, srcGroup);//�����ϴε��ƶ�
	return FALSE;
}

void HClusterProcessGroupPartition::refiningPhase_UpdatePriQueue(FlatNode * flatNode, std::map<std::pair<FlatNode*, FlatNodeGroup*>, int>& flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode*, FlatNodeGroup*>>& gain2FlatNode2AdjPT)
{
	refiningPhase_RemoveOldGain(flatNode, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	insertNewFlatNodeGain(flatNode, m_flatNode2group.find(flatNode)->second, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	//flatNode�϶˽ڵ���flatNode�ƶ���������ı仯���
	for (int i = 0; i != flatNode->inFlatNodes.size(); i++)
	{
		refiningPhase_RemoveOldGain(flatNode->inFlatNodes[i], flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
		insertNewFlatNodeGain(flatNode->inFlatNodes[i], m_flatNode2group.find(flatNode->inFlatNodes[i])->second, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	}
	//flatNode�¶˽ڵ���flatNode�ƶ���������ı仯���
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
		{//ɾ��
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
		{//ɾ��
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
	//������srcGroup��snkGroup��ص�groupͼ��������ϵ����Ҫ�޸ĵĹ�ϵ��Ҫ����srcGroup��snkGroup�����¶�group�Լ�srcGroup��snkGroup�����¶˵����¶�
	std::set<FlatNodeGroup *> modifyAdjDownGroup;//��¼��Ҫ�޸ĵ�group���¶�group
	std::set<FlatNodeGroup *> modifyAdjUpGroup;//��¼��Ҫ�޸ĵ�group���϶�group
											   //src��snk���¶˶�������Ҫ�ı�
	modifyAdjDownGroup.insert(srcGroup);
	modifyAdjDownGroup.insert(snkGroup);
	modifyAdjUpGroup.insert(srcGroup);
	modifyAdjUpGroup.insert(snkGroup);
	std::vector<FlatNodeGroup *>srcUpGroup = srcGroup->getUpGroup();
	std::vector<FlatNodeGroup *>srcDownGroup = srcGroup->getDownGroup();
	std::vector<FlatNodeGroup *>snkUpGroup = snkGroup->getUpGroup();
	std::vector<FlatNodeGroup *>snkDownGroup = snkGroup->getDownGroup();
	//src��snk�϶�group��ֻ��Ҫ�޸����¶�
	for (int i = 0; i != srcUpGroup.size(); i++)
		modifyAdjDownGroup.insert(srcUpGroup[i]);
	for (int i = 0; i != snkUpGroup.size(); i++)
		modifyAdjDownGroup.insert(snkUpGroup[i]);
	//src��snk�¶�group��ֻ��Ҫ�޸����϶�
	for (int i = 0; i != srcDownGroup.size(); i++)
		modifyAdjUpGroup.insert(srcDownGroup[i]);
	for (int i = 0; i != snkDownGroup.size(); i++)
		modifyAdjUpGroup.insert(snkDownGroup[i]);
	//�޸���Ҫ�޸����¶˵�group
	for (std::set<FlatNodeGroup *>::iterator dg_iter = modifyAdjDownGroup.begin(); dg_iter != modifyAdjDownGroup.end(); dg_iter++)
	{
		//��ɾ����ǰ��group�е��¶�group
		std::vector<FlatNodeGroup *> down_downGroup = (*dg_iter)->getDownGroup();
		for (int i = 0; i != down_downGroup.size(); i++) (*dg_iter)->deletedownGroup(down_downGroup[i]);
		std::set<FlatNode *> dg_flatNodes = (*dg_iter)->getFlatNodes();
		for (std::set<FlatNode *>::iterator dgf_iter = dg_flatNodes.begin(); dgf_iter != dg_flatNodes.end(); dgf_iter++)
		{//ȡdg_flatNodes�������
			for (int i = 0; i != (*dgf_iter)->outFlatNodes.size(); i++)
			{
				FlatNodeGroup *cur_tmp_downGroup = m_flatNode2group.find((*dgf_iter)->outFlatNodes[i])->second;
				if (cur_tmp_downGroup != *dg_iter)(*dg_iter)->insertdownGroup(cur_tmp_downGroup);
			}
		}
	}
	//�޸���Ҫ�޸����϶˵�group
	for (std::set<FlatNodeGroup *>::iterator ug_iter = modifyAdjUpGroup.begin(); ug_iter != modifyAdjUpGroup.end(); ug_iter++)
	{
		//��ɾ����ǰ��group�е��϶�group
		std::vector<FlatNodeGroup *> up_upGroup = (*ug_iter)->getUpGroup();
		for (int i = 0; i != up_upGroup.size(); i++) (*ug_iter)->deleteupGroup(up_upGroup[i]);
		std::set<FlatNode *> ug_flatNodes = (*ug_iter)->getFlatNodes();
		for (std::set<FlatNode *>::iterator ugf_iter = ug_flatNodes.begin(); ugf_iter != ug_flatNodes.end(); ugf_iter++)
		{//ȡdg_flatNodes�������
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
	int internalDegree = 0;//��¼�ڵ���ڵĶ���
	std::map<FlatNodeGroup *, int> adjPT2ExternalDegree;//��ǰ�ڵ����ڻ��������֮�����ӱߵ���Ŀ���ڵ�����ڵĻ��֣��ڵ���û��������ߵ���Ŀ��
	for (int i = 0; i != flatNode->inFlatNodes.size(); i++)
	{
		std::map<FlatNode *, FlatNodeGroup * >::iterator pt_iter = m_flatNode2group.find(flatNode->inFlatNodes[i]);
		assert(pt_iter != m_flatNode2group.end());
		if (pt_iter->second == curGroup) internalDegree++;
		else
		{//����adjPT2ExternalDegree
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
		{//����adjPT2ExternalDegree
			std::pair<std::map<FlatNodeGroup *, int>::iterator, bool> ret = adjPT2ExternalDegree.insert(std::make_pair(pt_iter->second, 1));
			if (!ret.second) ret.first->second += 1;
		}
	}
	//����adjPT2ExternalDegree��internalDegree����flatNode2AdjPT2Gain��gain2FlatNode2AdjPT
	for (std::map<FlatNodeGroup *, int>::iterator ED_iter = adjPT2ExternalDegree.begin(); ED_iter != adjPT2ExternalDegree.end(); ED_iter++)
	{
		if (ED_iter->second == 0) continue;//���ڷǱ߽�ڵ������ʱ���������ȵĶ����У��������Ӳ��Һ͸���ʱ��
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
	assert(m_groups.size() == groupTopo.size());//��֤����ֻ���groupͼ��û�л���

	for (std::vector<FlatNodeGroup *>::iterator iter = groupTopo.begin(); iter != groupTopo.end(); iter++)
	{
		m_group2groupNo.insert(std::make_pair(*iter, num));
		m_groupNo2group.insert(std::make_pair(num, *iter));
		m_groupNo2actualNo.push_back(num);
		//ȷ������group�ĸ���_groupWorkload
		m_groupWorkload.push_back((*iter)->getWorkload());
		//ȷ������group�����ͨ����_groupCommunication
		m_groupCommunication.push_back((*iter)->getTotalCommunicateData());
		num++;
	}
	for (std::map<FlatNodeGroup *, int>::iterator iter = m_group2groupNo.begin(); iter != m_group2groupNo.end(); iter++)
	{//����ÿ��group���¶˵�group�����ڽӾ��󣬾����ϵ�Ȩֵ��һ��group֮���ͨ����
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
	//std::cout<<"���̻������� "<<gain<<std::endl;
	return gain;
}

FlatNodeGroup::FlatNodeGroup(std::set<FlatNode *> flatNodes, SchedulerSSG *sssg):m_sssg(sssg), m_flatNodes(flatNodes.begin(), flatNodes.end()), m_workload(0), m_totalData(0)
{
	//��vector�е�flatnode�Լ�sssg�����һ��group
	std::set<FlatNode *>::const_iterator const_iter = m_flatNodes.begin();
	for (const_iter = m_flatNodes.begin(); const_iter != m_flatNodes.end(); const_iter++)
	{
		m_workload += m_sssg->GetSteadyWork(*const_iter) * m_sssg->GetSteadyCount(*const_iter);
	}
}

int FlatNodeGroup::computeTotalCommunicateData()
{
	//���㵱ǰgroup�нڵ�������ͨ����
	int tmpTotalData = 0;
	std::set<FlatNode *>::const_iterator const_iter = m_flatNodes.begin();
	for (const_iter = m_flatNodes.begin(); const_iter != m_flatNodes.end(); const_iter++)
	{
		int steadyCount = m_sssg->GetSteadyCount(*const_iter);
		//�����
		for (int i = 0; i != (*const_iter)->nOut; i++)
		{
			if (m_flatNodes.count((*const_iter)->outFlatNodes[i]) == 0)
			{//��Ӧ������˽ڵ㲻�ڸ�group��
				tmpTotalData += steadyCount * (*const_iter)->outPushWeights[i];
			}
		}
		//�����
		for (int i = 0; i != (*const_iter)->nIn; i++)
		{
			if (m_flatNodes.count((*const_iter)->inFlatNodes[i]) == 0)
			{//��Ӧ������˽ڵ㲻�ڸ�group��
				tmpTotalData += steadyCount * (*const_iter)->inPopWeights[i];
			}
		}
	}
	return tmpTotalData;
}

inline long FlatNodeGroup::getWorkload()
{
	//ȡ��ǰgroup�ĸ���
	return m_workload;
}

int FlatNodeGroup::getCurCommunicateData(FlatNodeGroup *objGroup)
{
	//ȡ��ǰ��group��objGroup֮���ͨ������
	std::map<FlatNodeGroup *, int >::iterator iter = m_upGroup2Weight.find(objGroup);
	if (iter != m_upGroup2Weight.end()) return iter->second;
	iter = m_downGroup2Weight.find(objGroup);
	if (iter != m_downGroup2Weight.end()) return iter->second;
	else return 0;
}

FlatNodeGroup * FlatNodeGroup::fusingGroup(FlatNodeGroup *objGroup)
{
	//����ǰ��group��objGroup�ںϳ�һ��groupͬʱ�ͷ�objGroup
	if (this == objGroup) return this;
	std::set<FlatNode *> objFlatNodes = objGroup->getFlatNodes();
	m_flatNodes.insert(objFlatNodes.begin(), objFlatNodes.end());
	m_workload += objGroup->getWorkload();
	//�ȴӵ�ǰgroup�����¶���ɾ��objGroup,ͬʱ������_totalData
	deletedownGroup(objGroup);
	deleteupGroup(objGroup);
	//�޸��ںϺ��group���϶˵�Group,ͬʱ����_totalData
	std::map<FlatNodeGroup *, int> objUpGroup2Weight = objGroup->getUpGroup2Weight();
	for (std::map<FlatNodeGroup *, int>::iterator up_iter = objUpGroup2Weight.begin(); up_iter != objUpGroup2Weight.end(); up_iter++)
	{
		if (up_iter->first == this) continue;
		std::pair<std::map<FlatNodeGroup *, int>::iterator, int> ret = m_upGroup2Weight.insert(std::make_pair(up_iter->first, up_iter->second));
		if (!ret.second) ret.first->second += up_iter->second;
		m_totalData += up_iter->second;
	}
	//�޸��ںϺ��group���¶�group,ͬʱ����_totalData
	std::map<FlatNodeGroup *, int> objDownGroup2Weight = objGroup->getDownGroup2Weight();
	for (std::map<FlatNodeGroup *, int>::iterator down_iter = objDownGroup2Weight.begin(); down_iter != objDownGroup2Weight.end(); down_iter++)
	{
		if (down_iter->first == this) continue;
		std::pair<std::map<FlatNodeGroup *, int>::iterator, int> ret = m_downGroup2Weight.insert(std::make_pair(down_iter->first, down_iter->second));
		if (!ret.second) ret.first->second += down_iter->second;
		m_totalData += down_iter->second;
	}
	//�޸�objGroup�����¶�group�����¶�group
	for (std::map<FlatNodeGroup *, int>::iterator objup_iter = objUpGroup2Weight.begin(); objup_iter != objUpGroup2Weight.end(); objup_iter++)
	{//Ҫ�޸�objGroup���϶�group��_dowmGroup2Weight
		if (objup_iter->first == this) continue;
		objup_iter->first->deletedownGroup(objGroup);//ɾ���ɵ�
		objup_iter->first->insertdownGroup(this);//�����µ�
	}
	for (std::map<FlatNodeGroup *, int>::iterator objdown_iter = objDownGroup2Weight.begin(); objdown_iter != objDownGroup2Weight.end(); objdown_iter++)
	{//Ҫ�޸�objGroup���¶�group��_upGroup2Weight
		if (objdown_iter->first == this) continue;
		objdown_iter->first->deleteupGroup(objGroup);//ɾ���ɵ�
		objdown_iter->first->insertupGroup(this);//�����µ�
	}
	delete(objGroup);
	return this;
}

void FlatNodeGroup::insertupGroup(FlatNodeGroup *objGroup)
{
	//��ǰ��group���϶˽ڵ����group
	std::set<FlatNode *> tmpObjFlatNodes = objGroup->getFlatNodes();
	int tmpTotalData = 0;
	std::set<FlatNode *>::const_iterator const_iter = m_flatNodes.begin();
	for (const_iter = m_flatNodes.begin(); const_iter != m_flatNodes.end(); const_iter++)
	{
		int steadyCount = m_sssg->GetSteadyCount(*const_iter);
		//�����
		for (int i = 0; i != (*const_iter)->nIn; i++)
		{
			if (tmpObjFlatNodes.count((*const_iter)->inFlatNodes[i]))
			{//flatNode��Ӧ���϶˽ڵ���tmpObjFlatNodes�У�����֮����ͨ��
				tmpTotalData += steadyCount * (*const_iter)->inPopWeights[i];
			}
		}
	}
	std::pair<std::map<FlatNodeGroup *, int>::iterator, int> ret = m_upGroup2Weight.insert(std::make_pair(objGroup, tmpTotalData));
	if (ret.second)m_totalData += tmpTotalData;//����ɹ��޸��ܵ�ͨ����
	else
	{
		m_totalData += tmpTotalData - ret.first->second;
		ret.first->second = tmpTotalData;
	}
}

void FlatNodeGroup::insertdownGroup(FlatNodeGroup *objGroup)
{
	//��ǰgroup���¶˽ڵ����group
	std::set<FlatNode *> tmpObjFlatNodes = objGroup->getFlatNodes();
	int tmpTotalData = 0;
	std::set<FlatNode *>::const_iterator const_iter = m_flatNodes.begin();
	for (const_iter = m_flatNodes.begin(); const_iter != m_flatNodes.end(); const_iter++)
	{
		int steadyCount = m_sssg->GetSteadyCount(*const_iter);
		//�����
		for (int i = 0; i != (*const_iter)->nOut; i++)
		{
			if (tmpObjFlatNodes.count((*const_iter)->outFlatNodes[i]))
			{//��Ӧ������˽ڵ��ڸ�group��
				tmpTotalData += steadyCount * (*const_iter)->outPushWeights[i];
			}
		}
	}
	std::pair<std::map<FlatNodeGroup *, int>::iterator, int> ret = m_downGroup2Weight.insert(std::make_pair(objGroup, tmpTotalData));
	if (ret.second)m_totalData += tmpTotalData;//����ɹ��޸��ܵ�ͨ����
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
	//��ǰgroup�����һ��flatNode�ڵ�
	m_flatNodes.insert(objFlatNode);
	m_workload += m_sssg->GetSteadyWork(objFlatNode) * m_sssg->GetSteadyCount(objFlatNode);
	m_totalData = computeTotalCommunicateData();
	//ʹ��ǰgroup�����¶˵�group��Ч,��ϵ��ά�������ýӿڵĵ�����
	m_upGroup2Weight.clear();
	m_downGroup2Weight.clear();
}

void FlatNodeGroup::deleteFlatNode(FlatNode *objFlatNode)
{
	//�ӵ�ǰgroup��ɾ��һ��flatNode�ڵ�
	m_flatNodes.erase(objFlatNode);
	m_workload -= m_sssg->GetSteadyWork(objFlatNode) * m_sssg->GetSteadyCount(objFlatNode);
	m_totalData = computeTotalCommunicateData();
	//ʹ��ǰgroup�����¶˵�group��Ч,��ϵ��ά�������ýӿڵĵ�����
	m_upGroup2Weight.clear();
	m_downGroup2Weight.clear();
}

