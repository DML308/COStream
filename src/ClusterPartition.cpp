#include "ClusterPartition.h"
#include <map>
#include <set>
#include <vector>
#include <limits>
#include <float.h>
using namespace std;

//ȷ����������ķ�ʽ
#define SUBTRACT FALSE

PRIVATE ClusterPartition* cpartition = NULL;

ClusterPartition::ClusterPartition(ClusterGroupGraph* tmpgroupGraph)//���group�ı��
{
	groupGraph = tmpgroupGraph;
	std::vector<ClusterGroup *> clusterGroupSet = groupGraph->GetClusterGroupSet();
}

void ClusterPartition::CreateClusterGraph()
{//����cluster��group֮���ӳ���ϵ����cluster֮�������
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

//��һ������(��groupӳ�伯Ⱥ�ڵ�)
void ClusterPartition::InitPartition(ClusterGroupGraph* groupGraph)//��ʼ���֣���K��ȷ������mnparts����mnclustersʱ��Ҫ���group��cluster�ڵ�֮��Ļ��֣�
{//��groupGraph�е�groupӳ�䵽cluster�Ľڵ���
	std::vector<ClusterGroup *> clusterGroupSet = groupGraph->GetClusterGroupSet();//�����е�group�ڵ�ȡ����
	if(clusterGroupSet.size()<= mnclusters)//ֱ�����ӳ��
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
		CreateClusterSteadyCount();//��ʼ������ɺ���нڵ��ڲ�������̬���ȣ���֤��û�н���ϸ����ǰ���³���Ҳ���������У�
	}
	else//Ҫ��һ����ʼ����(group����Ŀ������cluster��Ŀ)==================��δʹ��
	{//��ʱ����metis����
		MetisPartitionGroupGraph(groupGraph);//���group��cluster֮���ӳ��
		//flatNode��cluster֮���ӳ��
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
{//����group����(��group��ͬһ��cluster��)�Ͷ���(��group����ͬһ��cluster��)ͨ�����Ĳ�ֵ
	int curCluster = group2Cluster.find(group)->second;
	vector<FlatNode *>curFlatNodes = group->GetFlatNodes();
	assert(curFlatNodes.size() != 0);
	vector<FlatNode *>srcFlatNode = group->GetSrcFlatNode();
	vector<FlatNode *>snkFlatNode = group->GetSnkFlatNode();
	std::map<FlatNode *,int> flatNode2SteadyCount = group->GetSteadyCountMap();
	std::map<FlatNode *,int>::iterator pos;
	Bool flag;
	int externalCommCost = 0;//����group�����������
	int internalCommCost = 0;//����group���ڵ�������
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
#if SUBTRACT //���ö�������ڵ�
	cout<<"����ͨ���� "<<externalCommCost<<"   ����ͨ���� "<<internalCommCost<<"  ��ֵ"<<externalCommCost - internalCommCost<<endl;
	return externalCommCost - internalCommCost;
#else //���ö�������ڱ�ֵ
	if(internalCommCost == 0) return numeric_limits <float> ::max();
	else return externalCommCost / internalCommCost;
#endif
}

int ClusterPartition::ComputeWorkloadOfCluster(int clusterNum)
{//���㼯Ⱥ�ڵ��ϵĹ�����
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
{//��src����dest�ϲ������޸�graph�������Ϣ�����ص����ƶ��������(ע��move�Ĺ����в�û���޸�group�ı߽�)
	std::multimap<int,ClusterGroup*>::iterator cluster2Group_Iter;
	std::map<ClusterGroup*, int>::iterator group2Cluster_Iter;
	std::map<FlatNode *,int>::iterator flatNode2Cluster_Iter;
	std::multimap<int,FlatNode*>::iterator cluster2FlatNode_Iter;
	//�޸�group��cluster֮���map
	group2Cluster_Iter = group2Cluster.find(srcGroup);
	group2Cluster_Iter->second = destCluster;
	for(cluster2Group_Iter = cluster2Group.begin();cluster2Group_Iter != cluster2Group.end(); cluster2Group_Iter++)
	{
		if(cluster2Group_Iter->second == srcGroup)break;
	}
	assert(cluster2Group_Iter != cluster2Group.end() );
	cluster2Group.erase(cluster2Group_Iter);
	cluster2Group.insert(make_pair(destCluster, srcGroup));
	//�޸�flatNode��cluster֮���map
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
{	//���cluster���Ƿ��л�
	vector<int> nInDegree;//���ڱ�����ڵ�����
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
			if(!(--nInDegree[tmpProClusterVec[i]])) clusterStack.push_back(tmpProClusterVec[i]);//���Ϊ0���ջ
		}
	}
	if(count < mnclusters) return FALSE;
	else return TRUE;
}

//************************************
// Qualifier: srcGroup��Ҫ�ƶ���group��destClusterVecĿ�Ļ���(��ѡ)���ϣ�graph��group��ͼ��gain���ƶ������棬���ص���src���ܱ��ƶ�����cluster���
//************************************
int ClusterPartition::RefineMoveGroup(ClusterGroup *srcGroup, vector<ClusterGroup *>destGroupVec, ClusterGroupGraph *graph)
{
	//Ԥ��������srcGroup��Ҫ���ƶ�����Ŀ��cluster���
	int srcCluster = group2Cluster.find(srcGroup)->second;
	int srcClusterWorkload_begin = ComputeWorkloadOfCluster(srcCluster);//��¼��srcGroup�ƶ�֮ǰcluster�ϵĹ�����
	int srcClusterWorkload_end;//��¼��srcGroup�ƶ�֮��cluster�ϵĹ�����
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
		//0.�ƶ�+�������棨�����ͨ�����Ͷ���ͨ�����Ĳ�ͼ�����֮��ıȣ�
		MovingGroup(srcGroup,destClusterVec[destClusterNum],graph);
		//1.����move���cluster��group֮���map����cluster֮���������ϵ
		CreateClusterGraph();
		//2.����ƶ��Ƿ�Ϸ�����cluster�䲻�����뻷��
		if(HasClusterTopologicalSort())
		{
#if SUBTRACT
			float gain = - ComputeCommDeltWeight(srcGroup);//������ͨ������ֵ���෴��
#else
			float gain = ComputeCommDeltWeight(srcGroup); //gainԽ�����ߵĿ����Ծ�Խ��
#endif
			//cout<<"�ƶ�����  "<<gain<<endl;
			cluster2Gain.insert(make_pair(destClusterVec[destClusterNum], gain));
			srcClusterWorkload_end = ComputeWorkloadOfCluster(srcCluster);//��srcCluster���Ƴ�srcGroup��cluster�ϵĹ�����
			destCluster2Workload.insert(make_pair(destClusterVec[destClusterNum],ComputeWorkloadOfCluster(destClusterVec[destClusterNum])+(srcClusterWorkload_begin-srcClusterWorkload_end)));//�ƶ���Ӹ��صĽǶȲ���������
		}
		//3.�����ƶ�
		MovingGroup(srcGroup,srcCluster,graph);
		
	}
	assert(cluster2Gain.size() == destCluster2Workload.size());
	map<int,int>::iterator workload_pos = destCluster2Workload.begin();
	map<int,float>::iterator max_pos = cluster2Gain.end();
		//4.����������ֵ
#if SUBTRACT
	float max_gain = -numeric_limits <float> ::max();//ȡ��С������
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
	//4.����������ֵ
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
	//5.����groupҪ���ƶ����ĵ����λ�ã���һ������������λ�ã�
	if(max_pos != cluster2Gain.end())
	{
		//cout<<"�ƶ��������"<<max_pos->second<<"  Ŀ��cluster���  "<<max_pos->first<<"   Դcluster���"<<srcCluster<<endl;
		return max_pos->first;		
	}
	else return -1;
}
//*************ϸ���ȵ���*********************
// Qualifier: ϸ����չ��ʱ����ͬһ����cluster��group����ڵ���ʱ���Բ��ƶ�����ͬ��Ⱥ�ϵ�Ҫ�Ը���cluster�ϵı߽�group�ڵ���п����Ƿ�Ҫ�ƶ���һ��չ��һ���ƶ��߽�group��
			  /*ÿ��groupֻ�ƶ�һ��,ϸ����ѭ����ֵ��������ÿ��group��ֻ��һ��flatNode*/
//************************************
void ClusterPartition::RefinePartition(ClusterGroupGraph *groupGraph)//ϸ����������Ҫ��Ե��Ǳ߽��Ͻڵ㣩
{	
	std::vector< std::pair<ClusterGroup*,ClusterGroup*> > coarseGroupSort = groupGraph->GetCoarseGroupSort();//ȡ�ֻ���group�ϲ���˳��
	std::vector<ClusterGroup* > coarseGroupResult = groupGraph->GetCoarseGroupResult();//ȡÿһ���ֻ��ϲ����γɵ�group
	assert(coarseGroupSort.size() == coarseGroupResult.size());
	//����ϸ���Ĺ���,��ֻ���˳���෴
	for(int coarseIter = coarseGroupSort.size() - 1; coarseIter >= 0; coarseIter-- )
	{
		ClusterGroup *currentGroup = coarseGroupResult[coarseIter];//ȡ��ǰҪϸ�������group
		assert(coarseGroupResult[coarseIter] != NULL);
		//1.�Ƚ�currentGroup�𿪣�����coarseGroupSort[coarseIter]���ƶ�,��ԭ��coarseIter���ƶ����޸Ļ�ԭ��Ӧ�ߵ���Ϣ��
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
				//ǰ����snk�ڵ�				
				vector<FlatNode *> tmpSnkFlatNode = tmpPreGroup[i]->GetSnkFlatNode();
				for(int j =0; j != tmpSnkFlatNode.size();j++)
				{
					for(int k = 0; k < tmpSnkFlatNode[j]->nOut;k++)
					{
						for (int l = 0; l != tmpFlatNodes1.size(); l++)
						{
							//if(tmpSnkFlatNode[j]->outPushWeights[k] == 0)continue; ////20121204��ӣ������б����ӵ�û�����ݴ��䣩
							if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
						}
						for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
						{
							//if(tmpSnkFlatNode[j]->outPushWeights[k] == 0)continue; ////20121204��ӣ������б����ӵ�û�����ݴ��䣩
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
				//ǰ����snk�ڵ�				
				vector<FlatNode *> tmpSrcFlatNode = tmpSuccGroup[i]->GetSrcFlatNode();
				for(int j =0; j != tmpSrcFlatNode.size();j++)
				{
					for(int k = 0; k < tmpSrcFlatNode[j]->nIn;k++)
					{
						for (int l = 0; l != tmpFlatNodes1.size(); l++)
						{
							//if(tmpSrcFlatNode[j]->inPopWeights[k] == 0)continue;////20121204��ӣ������б����ӵ�û�����ݴ��䣩
							if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
						}
						for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
						{
							//if(tmpSrcFlatNode[j]->inPopWeights[k] == 0)continue;////20121204��ӣ������б����ӵ�û�����ݴ��䣩
							if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
						}
					}
				}
				if(flag1)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(coarseGroupSort[coarseIter].first);
				if(flag2)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(coarseGroupSort[coarseIter].second);
			}								
		}
		//�޸�cluster2Group��group2Cluster�е�����(����i���groupֱ�ӷŵ�map�У�����i+1���group��map�в���)
		group2Cluster.erase(currentGroup);
		std::multimap<int,ClusterGroup*>::iterator curiter = cluster2Group.end();
		for(curiter = cluster2Group.begin();curiter != cluster2Group.end(); curiter++)
		{
			if(curiter->second == currentGroup) break;
		}
		assert(curiter != cluster2Group.end());
		int curClusterNum = curiter->first;//��i+1���group���ڼ�Ⱥ�ڵ��cluster���
		cluster2Group.erase(curiter);
		cluster2Group.insert(make_pair(curClusterNum,coarseGroupSort[coarseIter].first ));
		cluster2Group.insert(make_pair(curClusterNum,coarseGroupSort[coarseIter].second ));
		group2Cluster.insert(make_pair(coarseGroupSort[coarseIter].first,curClusterNum));
		group2Cluster.insert(make_pair(coarseGroupSort[coarseIter].second,curClusterNum));
		//ÿ��group(��i+1��)�����ֻ��2����group(��i��)������ȡ����������group���ڵ�����group(��i��)
		//ȡ���ĵ�i���group�뵱ǰ�ĵ�i+1���group����ͬһ��partition��
		vector<ClusterGroup *>neighborhoodGroup_first;//���һ��group�����ڵ�����group�������Ҫ���뵱ǰ�ĵ�i+1���group����ͬһ��partition�У�
		vector<ClusterGroup *>neighborhoodGroup_second;
		vector<ClusterGroup *>tmpSubGroupSuccessor;//��group�к��group
		vector<ClusterGroup *>tmpSubGroupPrecedence;//��group��ǰ��group
		tmpSubGroupPrecedence.clear();
		tmpSubGroupSuccessor.clear();
		tmpSubGroupPrecedence = coarseGroupSort[coarseIter].first->GetPrecedenceClusterGroup();
		tmpSubGroupSuccessor = coarseGroupSort[coarseIter].first->GetSuccessorClusterGroup();
		for (int i = 0; i != tmpSubGroupPrecedence.size(); i++)
		{
			int tmpClusterNum = group2Cluster.find(tmpSubGroupPrecedence[i])->second;//��group���ڵ�cluster��
			if(tmpClusterNum != curClusterNum)
			{
				neighborhoodGroup_first.push_back(tmpSubGroupPrecedence[i]);
			}
		}
		for (int i = 0; i != tmpSubGroupSuccessor.size(); i++)
		{
			int tmpClusterNum = group2Cluster.find(tmpSubGroupSuccessor[i])->second;//��group���ڵ�cluster��
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
			int tmpClusterNum = group2Cluster.find(tmpSubGroupSuccessor[i])->second;//��group���ڵ�cluster��
			if(tmpClusterNum != curClusterNum)
			{
				neighborhoodGroup_second.push_back(tmpSubGroupSuccessor[i]);
			}
		}
		//������Ե�����
		float gain_first = ComputeCommDeltWeight(coarseGroupSort[coarseIter].first);
		float gain_second = ComputeCommDeltWeight(coarseGroupSort[coarseIter].second);
		int destClusterNum;
		//��group�����ƶ����ҳ���������һ���ƶ�
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
	CreateClusterSteadyCount();//ϸ����ɹ����ڼ�Ⱥ�Ľڵ��ڲ�������̬����
// 	for (int i = 0; i < mnclusters;i++)
// 	{
// 		cout<<ComputeWorkloadOfCluster(i)<<endl;
// 	}
}

int ClusterPartition::GetClusterNum(ClusterGroup* group)
{//����group����cluster�ı��
	std::map<ClusterGroup*, int>::iterator pos;
	pos = group2Cluster.find(group);
	assert(pos != group2Cluster.end());
	return pos->second;	
}

std::pair<int, int> ClusterPartition::GetClusterCoreNum(FlatNode *flatNode)
{//����flatNode����cluster���
	std::map<FlatNode *,std::pair<int ,int> >::iterator pos = flatNode2Cluster2Core.find(flatNode);
	assert(pos != flatNode2Cluster2Core.end());
	return pos->second;
}

std::vector<FlatNode*> ClusterPartition::GetFlatNodesInGroups(int clusterNum)
{//����cluster�����flatNode--------������ڲ�ʹ�ã�private��
	std::vector<FlatNode *> vecswap;
	flatNodeSet.swap(vecswap);//���ԭ�������ݣ����ͷ��ڴ�
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
	flatNodeSet.swap(vecswap);//���ԭ�������ݣ����ͷ��ڴ�
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
{//����cluster�����flatNode
	std::vector<FlatNode *> vecswap;
	flatNodeSet.swap(vecswap);//���ԭ�������ݣ����ͷ��ڴ�
	map<int ,map<FlatNode *,int> >::iterator _cluster_iter = cluster2FlatNode2Core.find(clusterNum);
	for(map<FlatNode *,int>::iterator _flatNode_iter = _cluster_iter->second.begin(); _flatNode_iter != _cluster_iter->second.end(); _flatNode_iter++)
		flatNodeSet.push_back(_flatNode_iter->first);
	return flatNodeSet;
}
std::map<FlatNode *,int > ClusterPartition::GetFlatNode2Core(int clusterNum)
{//���ݸ����ļ�Ⱥ�ڵ�ı�ţ�����ڸû����ϵĻ��֣�FlatNode��core֮���ӳ�䣩
	std::map<int, std::map<FlatNode *, int > >::iterator pos; 
	pos = cluster2FlatNode2Core.find(clusterNum);
	assert(pos != cluster2FlatNode2Core.end());
	return pos->second;
}

std::multimap<int, FlatNode *> ClusterPartition::GetCore2FlatNode(int clusterNum)
{//���ݸ����ļ�Ⱥ�ڵ�ı�ţ�����ڸû����ϵĻ��֣�core��FlatNode֮���ӳ�䣩
	std::map<int, std::multimap<int, FlatNode *> >::iterator pos;
	pos = cluster2Core2FlatNode.find(clusterNum);
	assert(pos != cluster2Core2FlatNode.end());
	return pos->second;
}

std::vector<ClusterGroup *> ClusterPartition::GetGroups(int clusterNum)
{	//����Cluster����Ҹýڵ��ϵ�����Group
	std::vector<ClusterGroup *> vecswap;
	clusterGroupSet.swap(vecswap);//���ԭ�������ݣ����ͷ��ڴ�
	typedef std::multimap<int,ClusterGroup*>::iterator Num2NodeIter;
	pair<Num2NodeIter,Num2NodeIter>range=cluster2Group.equal_range(clusterNum);
	for(Num2NodeIter iter=range.first;iter!=range.second;++iter)
	{
		clusterGroupSet.push_back(iter->second);
	}
	return clusterGroupSet;
}

int ClusterPartition::GetClusters()
{//���ػ��ָ���mnclusters
	return mnclusters;
}
void ClusterPartition::SetClusters(int k)//���ü�Ⱥ�ڵ���Ŀ����������Ŀ��
{
	mnclusters = k;
}

int ClusterPartition::GetCores()
{//���ػ��ָ���mnclusters
	return mnCores;
}
void ClusterPartition::SetCores(int k)//���ü�Ⱥÿ���ڵ����Ŀ����������Ŀ��
{
	mnCores = k;
}

int ClusterPartition::findID(FlatNode *flatnode ,vector<FlatNode *> original)
{//����flatnode�ҵ�����vector�еı��
	for (int i=0;i<original.size();i++)
		if(flatnode == original[i])
			return i;
}

void ClusterPartition::CreateClusterSteadyCount()
{//�����ȶ�״̬
	cluster2flatNodeSteadyCount.clear();
	for(int i = 0; i < mnclusters; i++)
	{
		vector<FlatNode *> flatNodes = GetFlatNodesInGroups(i);
		map<FlatNode*, int >flatNode2SteadyCount = SteadySchedulingGroup(flatNodes);
		cluster2flatNodeSteadyCount.push_back(flatNode2SteadyCount);
	}
}
int ClusterPartition::GetSteadyCount(FlatNode *flatNode)
{//ȡһ��FlatNode�ϵľֲ���̬����
	std::map<FlatNode *,int>::iterator pos = flatNode2Cluster.find(flatNode);
	assert(pos != flatNode2Cluster.end());
	std::map<FlatNode *,int> tmpSteadyCountMap = cluster2flatNodeSteadyCount[pos->second];
	std::map<FlatNode *,int>::iterator steady_iter = tmpSteadyCountMap.find(flatNode);
	return steady_iter->second;
}


void ClusterPartition::MetisPartitionGroupGraph(ClusterGroupGraph* groupGraph)
{//��ʱû���õ�
}

//��ʱֻ����һ��������ֻ��һ��group�����
void ClusterPartition::MetisPartitionFlatNodeInCluster()
{//�ڶ������֣���flatNodeӳ�䵽���ϣ�
	//assert(mnclusters == clusterGroup2NO.size());
	int *mxadj;//����ָ��ָ��xadj����
	int *madjncy;//����ָ��ָ��adjncy����
	int *mobjval;//����ָ��ָ��objval
	int *mpart;//����ָ��ָ��part����
	int *mvwgt;//����ָ��ָ��vwgt���飬���ߴ洢ÿ�������Ȩֵ
	int *madjwgt;//����ָ��ָ��adjwgt����
	int nvtxs;//���嶥�����
	float *tpwgts;
	int *mvsize;//����ָ��ָ��vsize����
	float *ubvec;
	int mncon;
	int objval;
	int options[40];//��������
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
		if(mnCores == 1)//���ֻ��һ���ڵ��һ��thread��������
		{
			for (int i=0;i<nvtxs;i++){
				//cout<<part[i]<<endl;
				tmpflatNode2Core.insert(make_pair(tmpFlatNodes[i],0));//�����ڵ㵽���ֱ�ŵ�ӳ��
				tmpcore2FlatNode.insert(make_pair(0,tmpFlatNodes[i]));
				flatNode2Cluster2Core.insert(make_pair(tmpFlatNodes[i],make_pair(groupNum, 0)));
			}
			cluster2Core2FlatNode.insert(make_pair(groupNum,tmpcore2FlatNode));
			cluster2FlatNode2Core.insert(make_pair(groupNum,tmpflatNode2Core));
			continue;
		}
		if(nvtxs == 1)//ֻ��һ������
		{
			tmpflatNode2Core.insert(make_pair(tmpFlatNodes[0],0));//�����ڵ㵽���ֱ�ŵ�ӳ��
			tmpcore2FlatNode.insert(make_pair(0,tmpFlatNodes[0]));
			flatNode2Cluster2Core.insert(make_pair(tmpFlatNodes[0],make_pair(groupNum, 0)));
			cluster2Core2FlatNode.insert(make_pair(groupNum,tmpcore2FlatNode));
			cluster2FlatNode2Core.insert(make_pair(groupNum,tmpflatNode2Core));
			continue;
		}
		/************************************************************************/
		/* metis ������ȣ���ϱ�ͨ���Լ����ؾ���                               */
		/************************************************************************/
		vector<int>xadj(nvtxs+1,0);//��̬����xadj����
		vector<int>vwgt(nvtxs);
		vector<int>part(nvtxs);
		vector<int>vsize(nvtxs,0);
		int edgenum = 0;//ͼ�ı���,������ѡ�ô�ͼ�ı���Ҳ���ԣ���Ӱ����
		edgenum=SSG->GetMapEdge2DownFlatNode().size();//ͼ�ı���
		vector<int>adjncy(edgenum*2);
		vector<long>adjwgt(edgenum*2);//���ڴ洢�ߵ�Ȩ��
		int k=0;//k���ڼ�¼flatnode�����ڽڵ���
		map<FlatNode *, int>::iterator iter;
		typedef multimap<int,FlatNode *>::iterator iter1;
		map<FlatNode *, int> tmpflatNode2Steadycount = cluster2flatNodeSteadyCount[groupNum];
		for(int i=0;i<nvtxs;i++)
		{
			FlatNode *node = tmpFlatNodes[i];
			int flag = 0;//��֤sumֻ��һ��
			int sum = 0;
			sum += xadj[i];
			int nOut = 0;
			for (int j = 0;j < node->nOut;j++)
			{
				if(tmpFlatNodeSet.count(node->outFlatNodes[j]))	 //20121128 ע��������жϣ�Ŀ������������ʱ����
					++nOut; //���ҳɹ�
			}
			if (nOut != 0)
			{  
				flag = 1;
				xadj[i+1] = sum + nOut;
				for (int j = 0;j < node->nOut;j++)
				{
					if(!tmpFlatNodeSet.count(node->outFlatNodes[j]))  //20121128 ע��������жϣ�Ŀ������������ʱ����	
						continue;//������ڵ㲻��place��
					adjncy[k] = findID(node->outFlatNodes[j],tmpFlatNodes);
					adjwgt[k] = node->outPushWeights[j] * tmpflatNode2Steadycount.find(node)->second;
					vsize[i] += adjwgt[k];
					k++;
				}
			}

			int nIn = 0;
			for (int j = 0;j < node->nIn;j++)
			{
				if(tmpFlatNodeSet.count(node->inFlatNodes[j]))	 //20121128 ע��������жϣ�Ŀ������������ʱ����
						++nIn; //���ҳɹ�
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
					if(!tmpFlatNodeSet.count(node->inFlatNodes[j])) //20121128 ע��������жϣ�Ŀ������������ʱ����
						continue;//������ڵ㲻��place��
					adjncy[k] = findID(node->inFlatNodes[j],tmpFlatNodes);
					adjwgt[k] = node->inPopWeights[j] * tmpflatNode2Steadycount.find(node)->second;
					vsize[i] += adjwgt[k];
					k++;
				}
			}
			iter = tmpflatNode2Steadycount.find(node);
			vwgt[i] = SSG->GetSteadyWork(node)*iter->second;
		} 
		mxadj = &xadj[0]; //�������,���������ڽӱ������еķ�Χ
		madjncy = &adjncy[0]; // �����, adjncy: adjacency
		madjwgt = NULL;//�ߵ�Ȩ��
		mvsize = &vsize[0];//���ڵ��ͨ����(�ڵ㷢�͵�������)
		mpart = &part[0];//���ڵ�����Ӧ�Ļ��ֱ��
		mvwgt = &vwgt[0];//���ڵ�Ĺ�����
		if(METIS_OK == METIS_PartGraphKway(&nvtxs,&mncon,mxadj,madjncy,mvwgt,mvsize,madjwgt,&mnCores,tpwgts,ubvec,options,&objval,mpart))
		{
			//��part[i]��һ���жϣ����ֵȨ��ͬ�����һ���µķ����㷨
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
				tmpflatNode2Core.insert(make_pair(tmpFlatNodes[i],part[i]));//�����ڵ㵽���ֱ�ŵ�ӳ��
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
{//����cluster2FlatNode2Core��sssg��Ϣ����_tmpcp���ֵĽ��----------20121128���
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
{//���м�Ⱥ��Ķ�������
	ClusterGroupGraph *groupGraph = new ClusterGroupGraph(SSSG);
	groupGraph->CreateCoarseGraph(nclusters);//��ͼ���г�ʼ��group����
	//groupGraph->CreateCoarseGraph();//��ͼ���г�ʼ��group����(��ȷ�����ַ���)
	cpartition = new ClusterPartition(groupGraph);
	cpartition->SetClusters(nclusters);//���ü�Ⱥ�����ڵ���Ŀ
	cpartition->SetCores(nplaces); //�趨������Ⱥ�ڵ��core����Ŀ
	cpartition->InitPartition(groupGraph);//���г�ʼ����(��һ�����֣����ֵ���Ⱥ�л����Ľڵ�)
	cpartition->RefinePartition(groupGraph);//ϸ���ȵ���
	cpartition->MetisPartitionFlatNodeInCluster();//�ڶ�������(��FlatNodeӳ�䵽����)
	return cpartition;
}

