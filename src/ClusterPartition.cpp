#include "ClusterPartition.h"
#include <map>
#include <set>
#include <vector>
#include <limits>
#include <float.h>
using namespace std;

//确定计算收益的方式
#define SUBTRACT FALSE

PRIVATE ClusterPartition* cpartition = NULL;

ClusterPartition::ClusterPartition(ClusterGroupGraph* tmpgroupGraph)//完成group的编号
{
	groupGraph = tmpgroupGraph;
	std::vector<ClusterGroup *> clusterGroupSet = groupGraph->GetClusterGroupSet();
}

void ClusterPartition::CreateClusterGraph()
{//根据cluster与group之间的映射关系构造cluster之间的依赖
	cluster2PrecedenceCluster.clear();
	cluster2SuccessorCluster.clear();
	typedef multimap<int,ClusterGroup*>::iterator Num2GroupIter;
	for(int clusterNum = 0; clusterNum != mnclusters; clusterNum++)
	{
		pair<Num2GroupIter,Num2GroupIter>range=cluster2Group.equal_range(clusterNum);
		vector<ClusterGroup *>tmpGroupVec;
		set<int >tmpPrecedenceSet;
		set<int >tmpSuccessorSet;
		for(Num2GroupIter iter=range.first;iter!=range.second;++iter)
		{
			tmpGroupVec.push_back(iter->second);
		}
		for(int i = 0; i != tmpGroupVec.size(); i++)
		{
			int curCluster = GetClusterNum(tmpGroupVec[i]);
			vector<ClusterGroup *> precedenceClusterGroup = tmpGroupVec[i]->GetPrecedenceClusterGroup();
			vector<ClusterGroup *> successorClusterGroup = tmpGroupVec[i]->GetSuccessorClusterGroup();
			for(int j = 0; j != precedenceClusterGroup.size();j++)
			{
				int preCluster = GetClusterNum(precedenceClusterGroup[j]); 
				if(curCluster != preCluster)tmpPrecedenceSet.insert(preCluster);
			}
			for(int k = 0; k != successorClusterGroup.size(); k++)
			{
				int sucCluster = GetClusterNum(successorClusterGroup[k]);
				if(curCluster != sucCluster)tmpSuccessorSet.insert(sucCluster);
			}
		}
		vector<int >tmpPrecedenceVec(tmpPrecedenceSet.begin(),tmpPrecedenceSet.end());
		vector<int >tmpSuccessorVec(tmpSuccessorSet.begin(),tmpSuccessorSet.end());
		cluster2PrecedenceCluster.insert(make_pair(clusterNum,tmpPrecedenceVec));
		cluster2SuccessorCluster.insert(make_pair(clusterNum,tmpSuccessorVec));
	}
}

//第一级划分(将group映射集群节点)
void ClusterPartition::InitPartition(ClusterGroupGraph* groupGraph)//初始化分（当K不确定或者mnparts大于mnclusters时需要完成group与cluster节点之间的划分）
{//将groupGraph中的group映射到cluster的节点上
	std::vector<ClusterGroup *> clusterGroupSet = groupGraph->GetClusterGroupSet();//将所有的group节点取出来
	if(clusterGroupSet.size()<= mnclusters)//直接完成映射
	{
		for (int i = 0; i != clusterGroupSet.size(); i++)
		{
			cluster2Group.insert(make_pair(i,clusterGroupSet[i]));
			group2Cluster.insert(make_pair(clusterGroupSet[i],i));
			std::vector<FlatNode *> tmpflatNode = clusterGroupSet[i]->GetFlatNodes();
			for(int j = 0; j != tmpflatNode.size(); j++)
			{
				cluster2FlatNode.insert(make_pair(i,tmpflatNode[j]));
				flatNode2Cluster.insert(make_pair(tmpflatNode[j],i));
			}
		}
		CreateClusterSteadyCount();//初始化分完成后进行节点内部进行稳态调度（保证在没有进行细化的前提下程序也能正常运行）
	}
	else//要有一个初始化分(group的数目不等于cluster数目)==================暂未使用
	{//暂时采用metis划分
		MetisPartitionGroupGraph(groupGraph);//完成group与cluster之间的映射
		//flatNode与cluster之间的映射
		for (std::map<ClusterGroup*, int>::iterator iter = group2Cluster.begin();iter != group2Cluster.end();iter++)
		{
			std::vector<FlatNode *> tmpflatNode = iter->first->GetFlatNodes();
			for (int j = 0; j != tmpflatNode.size(); j++)
			{
				cluster2FlatNode.insert(make_pair(iter->second,tmpflatNode[j]));
				flatNode2Cluster.insert(make_pair(tmpflatNode[j],iter->second));
			}
		}
	}
}

float ClusterPartition::ComputeCommDeltWeight(ClusterGroup *group)
{//计算group对外(与group在同一个cluster中)和对内(与group不在同一个cluster中)通信量的差值
	int curCluster = group2Cluster.find(group)->second;
	vector<FlatNode *>curFlatNodes = group->GetFlatNodes();
	assert(curFlatNodes.size() != 0);
	vector<FlatNode *>srcFlatNode = group->GetSrcFlatNode();
	vector<FlatNode *>snkFlatNode = group->GetSnkFlatNode();
	std::map<FlatNode *,int> flatNode2SteadyCount = group->GetSteadyCountMap();
	std::map<FlatNode *,int>::iterator pos;
	Bool flag;
	int externalCommCost = 0;//保存group对外的数据量
	int internalCommCost = 0;//保存group对内的数据量
	for (int i = 0; i != srcFlatNode.size();i++)
	{
		flag = FALSE;
		pos = flatNode2SteadyCount.find(srcFlatNode[i]);
		assert(pos != flatNode2SteadyCount.end());
		for (int j = 0;j != srcFlatNode[i]->inFlatNodes.size(); j++)
		{
			int tmpCluster = flatNode2Cluster.find(srcFlatNode[i]->inFlatNodes[j])->second;
			if(tmpCluster != curCluster) externalCommCost += (pos->second * srcFlatNode[i]->inPopWeights[j]);
			else internalCommCost += (pos->second * srcFlatNode[i]->inPopWeights[j]);
		}		
	}
	for (int i = 0; i != snkFlatNode.size();i++)
	{	
		flag = FALSE;
		pos = flatNode2SteadyCount.find(snkFlatNode[i]);
		assert(pos != flatNode2SteadyCount.end());
		for (int j = 0;j != snkFlatNode[i]->outFlatNodes.size(); j++)
		{
			std::map<FlatNode *,int>::iterator tmpIter = flatNode2Cluster.find(snkFlatNode[i]->outFlatNodes[j]);
			assert(tmpIter != flatNode2Cluster.end());
			int tmpCluster = tmpIter->second;
			if(tmpCluster != curCluster) externalCommCost += pos->second * snkFlatNode[i]->outPushWeights[j];
			else internalCommCost += (pos->second * snkFlatNode[i]->outPushWeights[j]);
		}
	}
#if SUBTRACT //采用对外减对内的
	cout<<"对外通信量 "<<externalCommCost<<"   对内通信量 "<<internalCommCost<<"  差值"<<externalCommCost - internalCommCost<<endl;
	return externalCommCost - internalCommCost;
#else //采用对外与对内比值
	if(internalCommCost == 0) return numeric_limits <float> ::max();
	else return externalCommCost / internalCommCost;
#endif
}

int ClusterPartition::ComputeWorkloadOfCluster(int clusterNum)
{//计算集群节点上的工作量
	vector<FlatNode *> flatNodes = GetFlatNodesInGroups(clusterNum);
	map<FlatNode*, int >flatNode2SteadyCount = SteadySchedulingGroup(flatNodes);
	int workload = 0;
	for(int i = 0; i != flatNodes.size();i++)
	{
		workload += SSG->GetSteadyWork(flatNodes[i]) * flatNode2SteadyCount.find(flatNodes[i])->second;
	}
	return workload;
}

void ClusterPartition::MovingGroup(ClusterGroup *srcGroup,int destCluster,ClusterGroupGraph *graph )
{//将src，与dest合并，并修改graph的相关信息，返回的是移动后的收益(注意move的过程中并没有修改group的边界)
	std::multimap<int,ClusterGroup*>::iterator cluster2Group_Iter;
	std::map<ClusterGroup*, int>::iterator group2Cluster_Iter;
	std::map<FlatNode *,int>::iterator flatNode2Cluster_Iter;
	std::multimap<int,FlatNode*>::iterator cluster2FlatNode_Iter;
	//修改group与cluster之间的map
	group2Cluster_Iter = group2Cluster.find(srcGroup);
	group2Cluster_Iter->second = destCluster;
	for(cluster2Group_Iter = cluster2Group.begin();cluster2Group_Iter != cluster2Group.end(); cluster2Group_Iter++)
	{
		if(cluster2Group_Iter->second == srcGroup)break;
	}
	assert(cluster2Group_Iter != cluster2Group.end() );
	cluster2Group.erase(cluster2Group_Iter);
	cluster2Group.insert(make_pair(destCluster, srcGroup));
	//修改flatNode与cluster之间的map
	vector<FlatNode *>srcFlatNodes = srcGroup->GetFlatNodes();
	assert(srcFlatNodes.size() != 0);
	for(int i = 0; i != srcFlatNodes.size(); i++)
	{
		flatNode2Cluster_Iter = flatNode2Cluster.find(srcFlatNodes[i]);
		assert(flatNode2Cluster_Iter != flatNode2Cluster.end());
		flatNode2Cluster_Iter->second = destCluster;
	}
	for(int i = 0; i != srcFlatNodes.size(); i++)
	{
		cluster2FlatNode_Iter = cluster2FlatNode.begin();
		while(cluster2FlatNode_Iter != cluster2FlatNode.end())
		{
			if(cluster2FlatNode_Iter->second == srcFlatNodes[i]) {cluster2FlatNode.erase(cluster2FlatNode_Iter++);break;}
			else cluster2FlatNode_Iter++;
		}
	}
	for(int i = 0; i != srcFlatNodes.size(); i++)
	{
		cluster2FlatNode.insert(make_pair(destCluster, srcFlatNodes[i]));
	}
}

Bool ClusterPartition::HasClusterTopologicalSort()
{	//检测cluster间是否有环
	vector<int> nInDegree;//用于保存各节点的入度
	vector<int>clusterStack;
	int count = 0;
	for(std::map<int, std::vector<int> >::iterator iter = cluster2PrecedenceCluster.begin(); iter != cluster2PrecedenceCluster.end(); iter++)
	{
		nInDegree.push_back((iter->second).size());
	}
	assert(nInDegree.size() == mnclusters);
	for (int i = 0; i != nInDegree.size();i++)
	{
		if(!nInDegree[i]) clusterStack.push_back(i);
	}
	while (!clusterStack.empty())
	{
		int clusterNum = clusterStack.back();
		assert(clusterNum < cluster2SuccessorCluster.size() );
		vector<int > tmpProClusterVec = cluster2SuccessorCluster[clusterNum];
		clusterStack.pop_back();
		++count;
		for(int i = 0; i != tmpProClusterVec.size(); i++)
		{
			if(!(--nInDegree[tmpProClusterVec[i]])) clusterStack.push_back(tmpProClusterVec[i]);//入度为0点进栈
		}
	}
	if(count < mnclusters) return FALSE;
	else return TRUE;
}

//************************************
// Qualifier: srcGroup将要移动的group，destClusterVec目的划分(候选)集合，graph是group的图，gain是移动的收益，返回的是src可能被移动到的cluster编号
//************************************
int ClusterPartition::RefineMoveGroup(ClusterGroup *srcGroup, vector<ClusterGroup *>destGroupVec, ClusterGroupGraph *graph)
{
	//预处理，查找srcGroup将要被移动到的目标cluster编号
	int srcCluster = group2Cluster.find(srcGroup)->second;
	int srcClusterWorkload_begin = ComputeWorkloadOfCluster(srcCluster);//记录在srcGroup移动之前cluster上的工作量
	int srcClusterWorkload_end;//记录在srcGroup移动之后cluster上的工作量
	set<int> destClusterSet;
	for(int i = 0; i != destGroupVec.size(); i++)
	{
		std::map<ClusterGroup*, int>::iterator pos = group2Cluster.find(destGroupVec[i]);
		assert(pos != group2Cluster.end());
		destClusterSet.insert(pos->second);
	}
	vector<int > destClusterVec(destClusterSet.begin(),destClusterSet.end());
	if(destClusterVec.size() == 0) return -1;

	map<int,float>cluster2Gain;
	map<int,int>destCluster2Workload;
	for (int destClusterNum = 0; destClusterNum != destClusterVec.size(); destClusterNum++)
	{
		//0.移动+计算收益（对外的通信量和对内通行量的差和计算量之间的比）
		MovingGroup(srcGroup,destClusterVec[destClusterNum],graph);
		//1.根据move后的cluster与group之间的map构造cluster之间的依赖关系
		CreateClusterGraph();
		//2.检测移动是否合法（在cluster间不能引入环）
		if(HasClusterTopologicalSort())
		{
#if SUBTRACT
			float gain = - ComputeCommDeltWeight(srcGroup);//收益是通信量差值的相反数
#else
			float gain = ComputeCommDeltWeight(srcGroup); //gain越大被移走的可能性就越大
#endif
			//cout<<"移动收益  "<<gain<<endl;
			cluster2Gain.insert(make_pair(destClusterVec[destClusterNum], gain));
			srcClusterWorkload_end = ComputeWorkloadOfCluster(srcCluster);//从srcCluster中移除srcGroup后cluster上的工作量
			destCluster2Workload.insert(make_pair(destClusterVec[destClusterNum],ComputeWorkloadOfCluster(destClusterVec[destClusterNum])+(srcClusterWorkload_begin-srcClusterWorkload_end)));//移动后从负载的角度产生的收益
		}
		//3.撤销移动
		MovingGroup(srcGroup,srcCluster,graph);
		
	}
	assert(cluster2Gain.size() == destCluster2Workload.size());
	map<int,int>::iterator workload_pos = destCluster2Workload.begin();
	map<int,float>::iterator max_pos = cluster2Gain.end();
		//4.找收益的最大值
#if SUBTRACT
	float max_gain = -numeric_limits <float> ::max();//取最小浮点数
	for(map<int,float>::iterator iter = cluster2Gain.begin(); iter != cluster2Gain.end(); iter++)
	{
		cout<<workload_pos->second<<"  "<<srcClusterWorkload_end<<endl;
		if(iter->second > max_gain && workload_pos->second / srcClusterWorkload >= MINBALANCEFACTOR && workload_pos->second / srcClusterWorkload <= MAXBALANCEFACTOR)
		{
			max_gain = iter->second;
			max_pos = iter;
		}
		workload_pos++;
	}
#else
	//4.找收益的最大值
	float max_gain = COMMUNICATION_FACTOR;
	for(map<int,float>::iterator iter = cluster2Gain.begin(); iter != cluster2Gain.end(); iter++)
	{
		float workload_balance = ((float)workload_pos->second / srcClusterWorkload_end);
		if(iter->second <= max_gain &&  workload_balance>= MINBALANCEFACTOR && workload_balance <= MAXBALANCEFACTOR)
		{
			//cout<<workload_pos->second<<"  "<<srcClusterWorkload_end<<"  "<<workload_balance<<endl;
			max_gain = iter->second;
			max_pos = iter;
		}
		workload_pos++;
	}
#endif	
	//5.返回group要被移动到的的最佳位置（不一定是收益最大的位置）
	if(max_pos != cluster2Gain.end())
	{
		//cout<<"移动后的收益"<<max_pos->second<<"  目标cluster编号  "<<max_pos->first<<"   源cluster编号"<<srcCluster<<endl;
		return max_pos->first;		
	}
	else return -1;
}
//*************细粒度调整*********************
// Qualifier: 细粒度展开时，在同一个的cluster上group间的在调整时可以不移动，不同集群上的要对各个cluster上的边界group节点进行考虑是否要移动（一边展开一边移动边界group）
			  /*每个group只移动一次,细粒度循环中值的条件是每个group中只有一个flatNode*/
//************************************
void ClusterPartition::RefinePartition(ClusterGroupGraph *groupGraph)//细化调整（主要针对的是边界上节点）
{	
	std::vector< std::pair<ClusterGroup*,ClusterGroup*> > coarseGroupSort = groupGraph->GetCoarseGroupSort();//取粗化是group合并的顺序
	std::vector<ClusterGroup* > coarseGroupResult = groupGraph->GetCoarseGroupResult();//取每一步粗化合并后形成的group
	assert(coarseGroupSort.size() == coarseGroupResult.size());
	//具体细化的过程,与粗化的顺序相反
	for(int coarseIter = coarseGroupSort.size() - 1; coarseIter >= 0; coarseIter-- )
	{
		ClusterGroup *currentGroup = coarseGroupResult[coarseIter];//取当前要细化处理的group
		assert(coarseGroupResult[coarseIter] != NULL);
		//1.先将currentGroup拆开（撤销coarseGroupSort[coarseIter]的移动,还原第coarseIter次移动，修改还原相应边的信息）
		vector<ClusterGroup *>tmpPreGroup = currentGroup->GetPrecedenceClusterGroup();
		vector<ClusterGroup *>tmpSuccGroup = currentGroup->GetSuccessorClusterGroup();
		vector<FlatNode *> tmpFlatNodes1 = coarseGroupSort[coarseIter].first->GetFlatNodes();
		vector<FlatNode *> tmpFlatNodes2 = coarseGroupSort[coarseIter].second->GetFlatNodes();
		Bool flag1 = FALSE;
		Bool flag2 = FALSE;
		for(int i = 0;i !=tmpPreGroup.size(); i++)
		{
			flag1 = FALSE;
			flag2 = FALSE;
			if(tmpPreGroup[i]->DeleteSuccessorClusterGroup(currentGroup))
			{
				//前驱的snk节点				
				vector<FlatNode *> tmpSnkFlatNode = tmpPreGroup[i]->GetSnkFlatNode();
				for(int j =0; j != tmpSnkFlatNode.size();j++)
				{
					for(int k = 0; k < tmpSnkFlatNode[j]->nOut;k++)
					{
						for (int l = 0; l != tmpFlatNodes1.size(); l++)
						{
							//if(tmpSnkFlatNode[j]->outPushWeights[k] == 0)continue; ////20121204添加（处理有边连接但没有数据传输）
							if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
						}
						for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
						{
							//if(tmpSnkFlatNode[j]->outPushWeights[k] == 0)continue; ////20121204添加（处理有边连接但没有数据传输）
							if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
						}
					}
				}
				if(flag1)  tmpPreGroup[i]->AddSuccessorClusterGroup(coarseGroupSort[coarseIter].first);
				if(flag2)  tmpPreGroup[i]->AddSuccessorClusterGroup(coarseGroupSort[coarseIter].second);
			}						
		}
		for(int i = 0;i !=tmpSuccGroup.size(); i++)
		{
			flag1 = FALSE;
			flag2 = FALSE;
			if(tmpSuccGroup[i]->DeletePrecedenceClusterGroup(currentGroup))
			{
				//前驱的snk节点				
				vector<FlatNode *> tmpSrcFlatNode = tmpSuccGroup[i]->GetSrcFlatNode();
				for(int j =0; j != tmpSrcFlatNode.size();j++)
				{
					for(int k = 0; k < tmpSrcFlatNode[j]->nIn;k++)
					{
						for (int l = 0; l != tmpFlatNodes1.size(); l++)
						{
							//if(tmpSrcFlatNode[j]->inPopWeights[k] == 0)continue;////20121204添加（处理有边连接但没有数据传输）
							if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
						}
						for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
						{
							//if(tmpSrcFlatNode[j]->inPopWeights[k] == 0)continue;////20121204添加（处理有边连接但没有数据传输）
							if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
						}
					}
				}
				if(flag1)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(coarseGroupSort[coarseIter].first);
				if(flag2)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(coarseGroupSort[coarseIter].second);
			}								
		}
		//修改cluster2Group，group2Cluster中的内容(将第i层的group直接放到map中，将第i+1层的group从map中擦除)
		group2Cluster.erase(currentGroup);
		std::multimap<int,ClusterGroup*>::iterator curiter = cluster2Group.end();
		for(curiter = cluster2Group.begin();curiter != cluster2Group.end(); curiter++)
		{
			if(curiter->second == currentGroup) break;
		}
		assert(curiter != cluster2Group.end());
		int curClusterNum = curiter->first;//第i+1层的group所在集群节点的cluster编号
		cluster2Group.erase(curiter);
		cluster2Group.insert(make_pair(curClusterNum,coarseGroupSort[coarseIter].first ));
		cluster2Group.insert(make_pair(curClusterNum,coarseGroupSort[coarseIter].second ));
		group2Cluster.insert(make_pair(coarseGroupSort[coarseIter].first,curClusterNum));
		group2Cluster.insert(make_pair(coarseGroupSort[coarseIter].second,curClusterNum));
		//每个group(第i+1层)中最多只有2个子group(第i层)，下面取与这两个子group相邻的所有group(第i层)
		//取出的第i层的group与当前的第i+1层的group不在同一个partition中
		vector<ClusterGroup *>neighborhoodGroup_first;//与第一个group的相邻的所有group（满足的要求与当前的第i+1层的group不在同一个partition中）
		vector<ClusterGroup *>neighborhoodGroup_second;
		vector<ClusterGroup *>tmpSubGroupSuccessor;//子group中后继group
		vector<ClusterGroup *>tmpSubGroupPrecedence;//子group的前驱group
		tmpSubGroupPrecedence.clear();
		tmpSubGroupSuccessor.clear();
		tmpSubGroupPrecedence = coarseGroupSort[coarseIter].first->GetPrecedenceClusterGroup();
		tmpSubGroupSuccessor = coarseGroupSort[coarseIter].first->GetSuccessorClusterGroup();
		for (int i = 0; i != tmpSubGroupPrecedence.size(); i++)
		{
			int tmpClusterNum = group2Cluster.find(tmpSubGroupPrecedence[i])->second;//找group所在的cluster号
			if(tmpClusterNum != curClusterNum)
			{
				neighborhoodGroup_first.push_back(tmpSubGroupPrecedence[i]);
			}
		}
		for (int i = 0; i != tmpSubGroupSuccessor.size(); i++)
		{
			int tmpClusterNum = group2Cluster.find(tmpSubGroupSuccessor[i])->second;//找group所在的cluster号
			if(tmpClusterNum != curClusterNum)
			{
				neighborhoodGroup_first.push_back(tmpSubGroupSuccessor[i]);
			}
		}
		tmpSubGroupPrecedence.clear();
		tmpSubGroupSuccessor.clear();
		tmpSubGroupPrecedence = coarseGroupSort[coarseIter].second->GetPrecedenceClusterGroup();
		tmpSubGroupSuccessor = coarseGroupSort[coarseIter].second->GetSuccessorClusterGroup();
		for (int i = 0; i != tmpSubGroupPrecedence.size(); i++)
		{
			int tmpClusterNum = group2Cluster.find(tmpSubGroupPrecedence[i])->second;
			if(tmpClusterNum != curClusterNum)
			{
				neighborhoodGroup_second.push_back(tmpSubGroupPrecedence[i]);
			}
		}
		for (int i = 0; i != tmpSubGroupSuccessor.size(); i++)
		{
			int tmpClusterNum = group2Cluster.find(tmpSubGroupSuccessor[i])->second;//找group所在的cluster号
			if(tmpClusterNum != curClusterNum)
			{
				neighborhoodGroup_second.push_back(tmpSubGroupSuccessor[i]);
			}
		}
		//计算各自的收益
		float gain_first = ComputeCommDeltWeight(coarseGroupSort[coarseIter].first);
		float gain_second = ComputeCommDeltWeight(coarseGroupSort[coarseIter].second);
		int destClusterNum;
		//将group进行移动，找出收益最大的一次移动
		if (gain_first >= gain_second && gain_first >= 0)
		{
			 destClusterNum = RefineMoveGroup(coarseGroupSort[coarseIter].first,neighborhoodGroup_first ,groupGraph);
			 if(destClusterNum != -1) MovingGroup(coarseGroupSort[coarseIter].first,destClusterNum,groupGraph);
		}
		else if (gain_second > gain_first && gain_second >= 0)
		{
			destClusterNum = RefineMoveGroup(coarseGroupSort[coarseIter].second,neighborhoodGroup_second ,groupGraph);
			if(destClusterNum != -1) MovingGroup(coarseGroupSort[coarseIter].second,destClusterNum,groupGraph);
		}
		else continue;
	}
	CreateClusterSteadyCount();//细化完成过后在集群的节点内部进行稳态调度
// 	for (int i = 0; i < mnclusters;i++)
// 	{
// 		cout<<ComputeWorkloadOfCluster(i)<<endl;
// 	}
}

int ClusterPartition::GetClusterNum(ClusterGroup* group)
{//根据group查找cluster的编号
	std::map<ClusterGroup*, int>::iterator pos;
	pos = group2Cluster.find(group);
	assert(pos != group2Cluster.end());
	return pos->second;	
}

std::pair<int, int> ClusterPartition::GetClusterCoreNum(FlatNode *flatNode)
{//根据flatNode查找cluster编号
	std::map<FlatNode *,std::pair<int ,int> >::iterator pos = flatNode2Cluster2Core.find(flatNode);
	assert(pos != flatNode2Cluster2Core.end());
	return pos->second;
}

std::vector<FlatNode*> ClusterPartition::GetFlatNodesInGroups(int clusterNum)
{//根据cluster编号找flatNode--------在类的内部使用（private）
	std::vector<FlatNode *> vecswap;
	flatNodeSet.swap(vecswap);//清空原来的内容，并释放内存
	std::vector<ClusterGroup *> tmpGroup = GetGroups(clusterNum);
	for(int i = 0; i != tmpGroup.size(); i++)
	{
		std::vector<FlatNode *> tmpFlatNodeVec = tmpGroup[i]->GetFlatNodes();
		for(int j = 0; j != tmpFlatNodeVec.size(); j++)
		{
			flatNodeSet.push_back(tmpFlatNodeVec[j]);
		}
	}
	return flatNodeSet;
}

std::vector<FlatNode *>ClusterPartition::GetFlatNodes(int clusterNum, int coreNum )
{
	std::vector<FlatNode *> vecswap;
	flatNodeSet.swap(vecswap);//清空原来的内容，并释放内存
	std::map<int, std::multimap<int, FlatNode *> >::iterator pos = cluster2Core2FlatNode.find(clusterNum);
	
	typedef multimap<int,FlatNode*>::iterator Num2NodeIter;
	pair<Num2NodeIter,Num2NodeIter>range=pos->second.equal_range(coreNum);
	for(Num2NodeIter iter=range.first; iter != range.second; ++iter)
	{
		flatNodeSet.push_back(iter->second);
	}	
	return flatNodeSet;
}

std::vector<FlatNode*> ClusterPartition::GetFlatNodes(int clusterNum)
{//根据cluster编号找flatNode
	std::vector<FlatNode *> vecswap;
	flatNodeSet.swap(vecswap);//清空原来的内容，并释放内存
	map<int ,map<FlatNode *,int> >::iterator _cluster_iter = cluster2FlatNode2Core.find(clusterNum);
	for(map<FlatNode *,int>::iterator _flatNode_iter = _cluster_iter->second.begin(); _flatNode_iter != _cluster_iter->second.end(); _flatNode_iter++)
		flatNodeSet.push_back(_flatNode_iter->first);
	return flatNodeSet;
}
std::map<FlatNode *,int > ClusterPartition::GetFlatNode2Core(int clusterNum)
{//根据给定的集群节点的编号，获得在该机器上的划分（FlatNode与core之间的映射）
	std::map<int, std::map<FlatNode *, int > >::iterator pos; 
	pos = cluster2FlatNode2Core.find(clusterNum);
	assert(pos != cluster2FlatNode2Core.end());
	return pos->second;
}

std::multimap<int, FlatNode *> ClusterPartition::GetCore2FlatNode(int clusterNum)
{//根据给定的集群节点的编号，获得在该机器上的划分（core与FlatNode之间的映射）
	std::map<int, std::multimap<int, FlatNode *> >::iterator pos;
	pos = cluster2Core2FlatNode.find(clusterNum);
	assert(pos != cluster2Core2FlatNode.end());
	return pos->second;
}

std::vector<ClusterGroup *> ClusterPartition::GetGroups(int clusterNum)
{	//根据Cluster编号找该节点上的所有Group
	std::vector<ClusterGroup *> vecswap;
	clusterGroupSet.swap(vecswap);//清空原来的内容，并释放内存
	typedef std::multimap<int,ClusterGroup*>::iterator Num2NodeIter;
	pair<Num2NodeIter,Num2NodeIter>range=cluster2Group.equal_range(clusterNum);
	for(Num2NodeIter iter=range.first;iter!=range.second;++iter)
	{
		clusterGroupSet.push_back(iter->second);
	}
	return clusterGroupSet;
}

int ClusterPartition::GetClusters()
{//返回划分个数mnclusters
	return mnclusters;
}
void ClusterPartition::SetClusters(int k)//设置集群节点数目（即进程数目）
{
	mnclusters = k;
}

int ClusterPartition::GetCores()
{//返回划分个数mnclusters
	return mnCores;
}
void ClusterPartition::SetCores(int k)//设置集群每个节点核数目（即进程数目）
{
	mnCores = k;
}

int ClusterPartition::findID(FlatNode *flatnode ,vector<FlatNode *> original)
{//根据flatnode找到其在vector中的编号
	for (int i=0;i<original.size();i++)
		if(flatnode == original[i])
			return i;
}

void ClusterPartition::CreateClusterSteadyCount()
{//构造稳定状态
	cluster2flatNodeSteadyCount.clear();
	for(int i = 0; i < mnclusters; i++)
	{
		vector<FlatNode *> flatNodes = GetFlatNodesInGroups(i);
		map<FlatNode*, int >flatNode2SteadyCount = SteadySchedulingGroup(flatNodes);
		cluster2flatNodeSteadyCount.push_back(flatNode2SteadyCount);
	}
}
int ClusterPartition::GetSteadyCount(FlatNode *flatNode)
{//取一个FlatNode上的局部稳态次数
	std::map<FlatNode *,int>::iterator pos = flatNode2Cluster.find(flatNode);
	assert(pos != flatNode2Cluster.end());
	std::map<FlatNode *,int> tmpSteadyCountMap = cluster2flatNodeSteadyCount[pos->second];
	std::map<FlatNode *,int>::iterator steady_iter = tmpSteadyCountMap.find(flatNode);
	return steady_iter->second;
}


void ClusterPartition::MetisPartitionGroupGraph(ClusterGroupGraph* groupGraph)
{//暂时没有用到
}

//暂时只处理一个机器上只有一个group的情况
void ClusterPartition::MetisPartitionFlatNodeInCluster()
{//第二级划分（将flatNode映射到核上）
	//assert(mnclusters == clusterGroup2NO.size());
	int *mxadj;//定义指针指向xadj数组
	int *madjncy;//定义指针指向adjncy数组
	int *mobjval;//定义指针指向objval
	int *mpart;//定义指针指向part数组
	int *mvwgt;//定义指针指向vwgt数组，后者存储每个顶点的权值
	int *madjwgt;//定义指针指向adjwgt数组
	int nvtxs;//定义顶点个数
	float *tpwgts;
	int *mvsize;//定义指针指向vsize数组
	float *ubvec;
	int mncon;
	int objval;
	int options[40];//参数数组
	for (int i = 0; i < METIS_NOPTIONS; i++)
		options[i] = 0;

	options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY;//
	options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;//Specifies the type of objective
	options[METIS_OPTION_CTYPE] = METIS_CTYPE_SHEM;//Specifies the matching scheme to be used during coarsening.
	options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_METISRB;//Determines the algorithm used during initial partitioning.
	options[METIS_OPTION_RTYPE] = METIS_RTYPE_GREEDY;//Determines the algorithm used for refinement.
	options[METIS_OPTION_DBGLVL] = 0;//Specifies the amount of progress/debugging information will be printed during the execution of the algorithms.
	options[METIS_OPTION_NITER] = 10;//Specifies the number of iterations for the refinement algorithms at each stage of the uncoarsening process.Default is 10.
	options[METIS_OPTION_NCUTS] = 1;//Specifies the number of different partitionings that it will compute. The final partitioning is the one that achieves the best edgecut or communication volume. Default is 1.
	options[METIS_OPTION_SEED] = -1;//Specifies the seed for the random number generator
	options[METIS_OPTION_MINCONN] = 0;//Specifies that the partitioning routines should try to minimize the maximum degree of the subdomain graph,	i.e., the graph in which each partition is a node, and edges connect subdomains with a shared interface.
	options[METIS_OPTION_CONTIG] = 0;//Specifies that the partitioning routines should try to produce partitions that are contiguous. Note that if the input graph is not connected this option is ignored.
	options[METIS_OPTION_UFACTOR] = 30;//Specifies the maximum allowed load imbalance among the partitions.
	options[METIS_OPTION_NUMBERING] = 0;//C-style numbering is assumed that starts from 0.
	options[METIS_OPTION_NOOUTPUT] = 0;//
	options[METIS_OPTION_BALANCE] = 0;
	tpwgts = NULL;
	ubvec = NULL;
	mncon=1;

	for(int groupNum = 0; groupNum < mnclusters; groupNum++)
	{
		
		//ClusterGroup *group = GetClusterGroup(groupNum);
		std::map<FlatNode *,int> tmpflatNode2Core;
		std::multimap<int ,FlatNode*>tmpcore2FlatNode;
		std::vector<FlatNode *> tmpFlatNodes = GetFlatNodesInGroups(groupNum);
		std::set<FlatNode *> tmpFlatNodeSet(tmpFlatNodes.begin(),tmpFlatNodes.end()); 
		nvtxs = tmpFlatNodes.size();
		if(mnCores == 1)//如果只有一个节点或一个thread则不作划分
		{
			for (int i=0;i<nvtxs;i++){
				//cout<<part[i]<<endl;
				tmpflatNode2Core.insert(make_pair(tmpFlatNodes[i],0));//建立节点到划分编号的映射
				tmpcore2FlatNode.insert(make_pair(0,tmpFlatNodes[i]));
				flatNode2Cluster2Core.insert(make_pair(tmpFlatNodes[i],make_pair(groupNum, 0)));
			}
			cluster2Core2FlatNode.insert(make_pair(groupNum,tmpcore2FlatNode));
			cluster2FlatNode2Core.insert(make_pair(groupNum,tmpflatNode2Core));
			continue;
		}
		if(nvtxs == 1)//只有一个顶点
		{
			tmpflatNode2Core.insert(make_pair(tmpFlatNodes[0],0));//建立节点到划分编号的映射
			tmpcore2FlatNode.insert(make_pair(0,tmpFlatNodes[0]));
			flatNode2Cluster2Core.insert(make_pair(tmpFlatNodes[0],make_pair(groupNum, 0)));
			cluster2Core2FlatNode.insert(make_pair(groupNum,tmpcore2FlatNode));
			cluster2FlatNode2Core.insert(make_pair(groupNum,tmpflatNode2Core));
			continue;
		}
		/************************************************************************/
		/* metis 任务调度，结合边通信以及负载均衡                               */
		/************************************************************************/
		vector<int>xadj(nvtxs+1,0);//动态定义xadj数组
		vector<int>vwgt(nvtxs);
		vector<int>part(nvtxs);
		vector<int>vsize(nvtxs,0);
		int edgenum = 0;//图的边数,在这里选用大图的边数也可以，不影响结果
		edgenum=SSG->GetMapEdge2DownFlatNode().size();//图的边数
		vector<int>adjncy(edgenum*2);
		vector<long>adjwgt(edgenum*2);//用于存储边的权重
		int k=0;//k用于记录flatnode的相邻节点数
		map<FlatNode *, int>::iterator iter;
		typedef multimap<int,FlatNode *>::iterator iter1;
		map<FlatNode *, int> tmpflatNode2Steadycount = cluster2flatNodeSteadyCount[groupNum];
		for(int i=0;i<nvtxs;i++)
		{
			FlatNode *node = tmpFlatNodes[i];
			int flag = 0;//保证sum只加一次
			int sum = 0;
			sum += xadj[i];
			int nOut = 0;
			for (int j = 0;j < node->nOut;j++)
			{
				if(tmpFlatNodeSet.count(node->outFlatNodes[j]))	 //20121128 注释上面的判断，目的是消除编译时错误
					++nOut; //查找成功
			}
			if (nOut != 0)
			{  
				flag = 1;
				xadj[i+1] = sum + nOut;
				for (int j = 0;j < node->nOut;j++)
				{
					if(!tmpFlatNodeSet.count(node->outFlatNodes[j]))  //20121128 注释上面的判断，目的是消除编译时错误	
						continue;//该输出节点不在place中
					adjncy[k] = findID(node->outFlatNodes[j],tmpFlatNodes);
					adjwgt[k] = node->outPushWeights[j] * tmpflatNode2Steadycount.find(node)->second;
					vsize[i] += adjwgt[k];
					k++;
				}
			}

			int nIn = 0;
			for (int j = 0;j < node->nIn;j++)
			{
				if(tmpFlatNodeSet.count(node->inFlatNodes[j]))	 //20121128 注释上面的判断，目的是消除编译时错误
						++nIn; //查找成功
			}
			if (nIn != 0)
			{
				if (flag == 0)
				{
					xadj[i+1] = sum + nIn;
				}
				else
				{
					xadj[i+1] += nIn;
				}
				for (int j = 0; j < node->nIn; j++)
				{
					if(!tmpFlatNodeSet.count(node->inFlatNodes[j])) //20121128 注释上面的判断，目的是消除编译时错误
						continue;//该输出节点不在place中
					adjncy[k] = findID(node->inFlatNodes[j],tmpFlatNodes);
					adjwgt[k] = node->inPopWeights[j] * tmpflatNode2Steadycount.find(node)->second;
					vsize[i] += adjwgt[k];
					k++;
				}
			}
			iter = tmpflatNode2Steadycount.find(node);
			vwgt[i] = SSG->GetSteadyWork(node)*iter->second;
		} 
		mxadj = &xadj[0]; //顶点相关,顶点编号在邻接边数组中的范围
		madjncy = &adjncy[0]; // 边相关, adjncy: adjacency
		madjwgt = NULL;//边的权重
		mvsize = &vsize[0];//各节点的通信量(节点发送的数据量)
		mpart = &part[0];//各节点所对应的划分编号
		mvwgt = &vwgt[0];//各节点的工作量
		if(METIS_OK == METIS_PartGraphKway(&nvtxs,&mncon,mxadj,madjncy,mvwgt,mvsize,madjwgt,&mnCores,tpwgts,ubvec,options,&objval,mpart))
		{
			//对part[i]做一个判断，如果值权相同则调用一个新的分配算法
			int partTag = 0;
			for (int i = 0; i < nvtxs -1; i++)
			{
				if(part[i] != part[i+1] ){partTag = 1;break;}
			}
			if(!partTag)
			{
				int perParts = (int)ceil((float)nvtxs / mnCores);
				for (int i=0,total=nvtxs;i<mnCores;i++)
					for (int j=0;j<perParts && total>0;j++,total--)
						part[perParts*i+j]=i;
			}
			for (int i=0;i<nvtxs;i++)
			{
				tmpflatNode2Core.insert(make_pair(tmpFlatNodes[i],part[i]));//建立节点到划分编号的映射
				tmpcore2FlatNode.insert(make_pair(part[i],tmpFlatNodes[i]));
				flatNode2Cluster2Core.insert(make_pair(tmpFlatNodes[i],make_pair(groupNum, part[i])));
			}
			cluster2Core2FlatNode.insert(make_pair(groupNum,tmpcore2FlatNode));
			cluster2FlatNode2Core.insert(make_pair(groupNum,tmpflatNode2Core));
		}
		else
		{
			cout<<"Warning: METIS_PartGraphKway failed!\n";
		}
	}
}

ClusterPartition* ClusterPartition::RevisionClusterPartition(ClusterPartition* _tmpcp,SchedulerSSG* sssg, std::map<int, std::map<FlatNode *, int > >& _tmpcluster2FlatNode2Core)
{//根据cluster2FlatNode2Core和sssg信息修正_tmpcp划分的结果----------20121128添加
	_tmpcp->flatNode2Cluster2Core.clear();
	_tmpcp->cluster2Core2FlatNode.clear();
	_tmpcp->cluster2FlatNode2Core.clear();
	_tmpcp->flatNode2Cluster.clear();
	_tmpcp->cluster2FlatNode.clear();
	_tmpcp->cluster2flatNodeSteadyCount.clear();
	_tmpcp->cluster2FlatNode2Core = _tmpcluster2FlatNode2Core;
	multimap<int,FlatNode *>_tmpCore2FlatNode;
	map<FlatNode*, int> _tmpflatNode2steadycount;
	for(map<int, map<FlatNode*,int> >::iterator _iter = _tmpcluster2FlatNode2Core.begin(); _iter != _tmpcluster2FlatNode2Core.end();_iter++)
	{
		_tmpCore2FlatNode.clear();
		_tmpflatNode2steadycount.clear();
		for(map<FlatNode *,int>::iterator _core_iter = _iter->second.begin(); _core_iter != _iter->second.end(); _core_iter++)
		{
			_tmpCore2FlatNode.insert(make_pair(_core_iter->second,_core_iter->first));
			_tmpcp->flatNode2Cluster2Core.insert(make_pair(_core_iter->first,make_pair(_iter->first, _core_iter->second)));
			_tmpcp->flatNode2Cluster.insert(make_pair(_core_iter->first, _iter->first));
			_tmpcp->cluster2FlatNode.insert(make_pair(_iter->first, _core_iter->first));
			_tmpflatNode2steadycount.insert(make_pair(_core_iter->first, sssg->mapSteadyCount2FlatNode.find(_core_iter->first)->second));
		}
		_tmpcp->cluster2Core2FlatNode.insert(make_pair(_iter->first,_tmpCore2FlatNode));
		_tmpcp->cluster2flatNodeSteadyCount.push_back(_tmpflatNode2steadycount);
	}
	return _tmpcp;
}

GLOBAL ClusterPartition* SSGPartitionCluster(int nclusters, int nplaces)
{//进行集群间的二级划分
	ClusterGroupGraph *groupGraph = new ClusterGroupGraph(SSSG);
	groupGraph->CreateCoarseGraph(nclusters);//对图进行初始组group划分
	//groupGraph->CreateCoarseGraph();//对图进行初始组group划分(不确定划分分数)
	cpartition = new ClusterPartition(groupGraph);
	cpartition->SetClusters(nclusters);//设置集群机器节点数目
	cpartition->SetCores(nplaces); //设定机器集群节点的core的数目
	cpartition->InitPartition(groupGraph);//进行初始化分(第一级划分，划分到集群中机器的节点)
	cpartition->RefinePartition(groupGraph);//细粒度调整
	cpartition->MetisPartitionFlatNodeInCluster();//第二级划分(将FlatNode映射到核中)
	return cpartition;
}

