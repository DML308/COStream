#include "ClusterGroupGraph.h"
#include <limits>
#include <float.h>
using namespace std;

const float MIN_WEIGHT = numeric_limits <float> ::min();

ClusterGroupGraph::ClusterGroupGraph(SchedulerSSG *SSSG)
{//先根据SSG中flatNode构造一个group graph
	for(int i = 0; i!= SSSG->flatNodes.size();i++)
	{
		ClusterGroup *group = new ClusterGroup(SSSG->flatNodes[i]);
		clusterGroup2Radio.insert(make_pair(group,(float)(group->GetWorkload())/group->GetCommunicationCost()));
		flatNode2ClusterGroup.insert(make_pair(SSSG->flatNodes[i],group));
	}
	//构建group间的依赖关系（在这里要处理一对节点他们有边的连接但是没有数据传输（push = 0,pop = 0））
	for (std::map<ClusterGroup *,float>::iterator iter = clusterGroup2Radio.begin();iter != clusterGroup2Radio.end(); iter++)
	{
		std::vector<FlatNode *> precedenceFlatNode = iter->first->GetPrecedenceFlatNode();
		std::vector<FlatNode *> successorFlatNode = iter->first->GetSuccessorFlatNode();
		std::map<FlatNode *,ClusterGroup *>::iterator pos;
		//前驱
		for (int i = 0; i != precedenceFlatNode.size(); i++)
		{
			pos = flatNode2ClusterGroup.find(precedenceFlatNode[i]);
			assert(pos != flatNode2ClusterGroup.end());
			iter->first->AddPrecedenceClusterGroup(pos->second);//将group的前驱插入到clusterGroup的precedenceFlatNode中
		}
		//后继
		for (int i = 0; i != successorFlatNode.size(); i++)
		{
			pos = flatNode2ClusterGroup.find(successorFlatNode[i]);
			assert(pos != flatNode2ClusterGroup.end());
			iter->first->AddSuccessorClusterGroup(pos->second);
			groupSrc2SnkChannel.insert(make_pair(iter->first, pos->second));
		}
	}	
}

//group间的融合
ClusterGroup* ClusterGroupGraph::FussGroup(ClusterGroup *group1,ClusterGroup *group2)
{ 
	ClusterGroup *newGroup = new ClusterGroup(group1,group2);
	ModifyConnectionOfGroup(group1,group2,newGroup);
	//修改group graph的信息
	/*修改clusterGroup2Radio的信息*/
	std::map<ClusterGroup *,float>::iterator pos1;
	std::map<ClusterGroup *,float>::iterator pos2;
	pos1 = this->clusterGroup2Radio.find(group1);
	this->clusterGroup2Radio.erase(pos1);
	pos2 = this->clusterGroup2Radio.find(group2);
	this->clusterGroup2Radio.erase(pos2);
	this->clusterGroup2Radio.insert(make_pair(newGroup, CompToCommRadio(newGroup)));
	//修改flatNode2ClusterGroup
	std::vector<FlatNode *> group1FlatNode = group1->GetFlatNodes();
	std::vector<FlatNode *> group2FlatNode = group2->GetFlatNodes();
	std::map<FlatNode *,ClusterGroup *>::iterator pos3;
	for(int i = 0; i != group1FlatNode.size(); i++)
	{
		pos3 = this->flatNode2ClusterGroup.find(group1FlatNode[i]);
		pos3->second = newGroup;
	}
	for(int i = 0; i != group2FlatNode.size(); i++ )
	{
		pos3 =this->flatNode2ClusterGroup.find(group2FlatNode[i]);
		pos3->second = newGroup;
	}
	//修改groupSrc2SnkChannel中的信息
	this->groupSrc2SnkChannel.erase(group1);//删除以group1作为起点的边
	this->groupSrc2SnkChannel.erase(group2);//删除以Group2作为起点的边
	//要改成删除以group1，group2 作为终点的边
	std::multimap<ClusterGroup*, ClusterGroup*>::iterator iter = this->groupSrc2SnkChannel.begin();
	Bool eraseflag = false;
	while (iter !=  this->groupSrc2SnkChannel.end())
	{
		eraseflag = false;
		std::multimap<ClusterGroup*, ClusterGroup*>::iterator iter1;
		if(iter->second == group1 || iter->second == group2) {eraseflag = TRUE; iter1 = iter;}
		iter++;
		if(eraseflag){ this->groupSrc2SnkChannel.erase(iter1);}
		
	}
	//向groupSrc2SnkChannel中插入以newgroup作为前驱的边
	std::vector<ClusterGroup *> successorClusterGroup = newGroup->GetSuccessorClusterGroup();
	for(int i =0; i != successorClusterGroup.size(); i++)
	{
		this->groupSrc2SnkChannel.insert(make_pair(newGroup, successorClusterGroup[i]));
	}	
	std::vector<ClusterGroup *> procedenceClusterGroup = newGroup->GetPrecedenceClusterGroup();
	for(int i =0; i != procedenceClusterGroup.size(); i++)
	{
		this->groupSrc2SnkChannel.insert(make_pair( procedenceClusterGroup[i],newGroup));
	}	
	return newGroup;
}

//计算一个group的计算和通信比
float ClusterGroupGraph::CompToCommRadio(ClusterGroup *group)
{
	float commCost = group->GetCommunicationCost();
	int workload = group->GetWorkload();
	return ((float)workload)/commCost;
}

//对group graph进行拓扑，无环则返回true，否则返回false
Bool ClusterGroupGraph::HasGroupTopologicalSort(std::vector<ClusterGroup *>original)
{
	vector<ClusterGroup *>::iterator iter1,iter2,iter4;
	vector<ClusterGroup *>::iterator iter;
	vector<int> nInDegree;//用于保存各节点的入度
	vector<ClusterGroup *> groupStack;
	vector<int>::iterator iter3;
	int nsize=original.size();
	int count = 0;
	for (iter1=original.begin();iter1!=original.end();++iter1)
	{
		nInDegree.push_back(((*iter1)->GetPrecedenceClusterGroup()).size());//将各节点入度保存
	}
	for (int i = 0; i != nInDegree.size();i++)
	{
		if(!nInDegree[i]) groupStack.push_back(original[i]);
	}

	while (!groupStack.empty())
	{
		ClusterGroup *group = groupStack.back();// 取将要出栈的节点
		std::vector<ClusterGroup *>tmpProClusterGroup = group->GetSuccessorClusterGroup();//取group的所有后继
		groupStack.pop_back();
		++count;
		for(int i= 0; i != tmpProClusterGroup.size();i++)//对所有group的邻接点的入度减1
		{
			for (int j =0;j != original.size(); j++)
			{
				if(original[j] == tmpProClusterGroup[i])
				{
					if(!(--nInDegree[j])) groupStack.push_back(original[j]);//入度为0点进栈
				}
			}
		}
	}
	if(count < original.size()) return FALSE;
	else return TRUE;
}

//************************************
// Qualifier: 将group graph 融合成K份，即最终的group只含有K节点
//************************************
void ClusterGroupGraph::CreateCoarseGraph(int k)
{
	coarseGroupSort.clear();//使用前先将其清空
	coarseGroupResult.clear();

	while( clusterGroup2Radio.size() > k)
	{
		//找一对group进行合并，要求通信比的增量最大，并且合并过后不会引入环
		//计算每一对group的计算通信比
		std::vector<float> coarsingWeight;

		std::vector<ClusterGroup *> currentGroups;
		for(std::map<ClusterGroup *, float>::iterator cluster_iter = this->clusterGroup2Radio.begin();cluster_iter != this->clusterGroup2Radio.end();cluster_iter++)
		{
			currentGroups.push_back(cluster_iter->first);
		}		
		//计算每一对相邻group的计算通信比
		for(std::multimap<ClusterGroup* ,ClusterGroup *>::iterator iter= this->groupSrc2SnkChannel.begin();iter != this->groupSrc2SnkChannel.end(); iter++)
		{
			ClusterGroup* tmpGroup = new ClusterGroup(iter->first,iter->second);
			ModifyConnectionOfGroup(iter->first,iter->second,tmpGroup);
			std::vector<ClusterGroup *>::iterator pos1,pos2;
			for (std::vector<ClusterGroup *>::iterator iter1 = currentGroups.begin(); iter1 != currentGroups.end(); iter1++)
			{
				if((*iter1) == iter->first) {pos1 = iter1;break;}
			}
			assert(pos1 != currentGroups.end() && (*pos1) == iter->first);
			currentGroups.erase(pos1);
			for (std::vector<ClusterGroup *>::iterator iter1 = currentGroups.begin(); iter1 != currentGroups.end(); iter1++)
			{
				if((*iter1) == (iter->second)) {pos2 = iter1;break;}
			}
			assert(pos2 != currentGroups.end() && (*pos2) == (iter->second) );
			currentGroups.erase(pos2);
			currentGroups.push_back(tmpGroup);

			if(HasGroupTopologicalSort(currentGroups))
			{
				coarsingWeight.push_back((iter->first->GetCompCommRadio() + iter->second->GetCompCommRadio()) - tmpGroup->GetCompCommRadio() );//无环，插入
			}
			else 
			{
				coarsingWeight.push_back(MIN_WEIGHT);
			}
			currentGroups.pop_back();
			currentGroups.push_back(iter->first);
			currentGroups.push_back(iter->second);
			
			//由于在构造新的group时修改了iter->first，和iter->second中的前驱group的后继，和后继group中的前驱，现在要还原
			vector<ClusterGroup *>tmpPreGroup = tmpGroup->GetPrecedenceClusterGroup();
			vector<ClusterGroup *>tmpSuccGroup = tmpGroup->GetSuccessorClusterGroup();
			vector<FlatNode *> tmpFlatNodes1 = iter->first->GetFlatNodes();
			vector<FlatNode *> tmpFlatNodes2 = iter->second->GetFlatNodes();
			Bool flag1 = FALSE;
			Bool flag2 = FALSE;
			for(int i = 0;i !=tmpPreGroup.size(); i++)
			{
				flag1 = FALSE;
				flag2 = FALSE;
				if(tmpPreGroup[i]->DeleteSuccessorClusterGroup(tmpGroup))
				{
					//前驱的snk节点				
					vector<FlatNode *> tmpSnkFlatNode = tmpPreGroup[i]->GetSnkFlatNode();
					for(int j =0; j != tmpSnkFlatNode.size();j++)
					{
						
						for(int k = 0; k < tmpSnkFlatNode[j]->nOut;k++)
						{
							if(tmpSnkFlatNode[j]->outPushWeights[k] == 0)  continue;//20130119 zww 添加
							for (int l = 0; l != tmpFlatNodes1.size(); l++)
							{
								if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
							}
							for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
							{
								if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
							}
						}
					}
					if(flag1)  tmpPreGroup[i]->AddSuccessorClusterGroup(iter->first);
					if(flag2)  tmpPreGroup[i]->AddSuccessorClusterGroup(iter->second);
				}						
			}
			for(int i = 0;i !=tmpSuccGroup.size(); i++)
			{
				flag1 = FALSE;
				flag2 = FALSE;
				if(tmpSuccGroup[i]->DeletePrecedenceClusterGroup(tmpGroup))
				{
					//前驱的src节点				
					vector<FlatNode *> tmpSrcFlatNode = tmpSuccGroup[i]->GetSrcFlatNode();

					for(int j =0; j != tmpSrcFlatNode.size();j++)
					{
						for(int k = 0; k < tmpSrcFlatNode[j]->nIn;k++)
						{
							if(tmpSrcFlatNode[j]->inPopWeights[k] == 0)  continue;//20130119 zww 添加
							for (int l = 0; l != tmpFlatNodes1.size(); l++)
							{
								if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
							}
							for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
							{
								if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
							}
						}
					}
					if(flag1)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(iter->first);
					if(flag2)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(iter->second);
				}								
			}
			assert(currentGroups.size() == this->clusterGroup2Radio.size());//保证经过上面的操作后group的数目没有变
		}
		//找最大边的位置
		float maxWeight = MIN_WEIGHT;
		int maxIter = 0;
		for(int i = 0; i!= coarsingWeight.size();i++)
		{
			if(coarsingWeight[i]>maxWeight)
			{
					maxWeight = coarsingWeight[i];
					maxIter = i;
			}
		}
		int p = 0;
		std::multimap<ClusterGroup* ,ClusterGroup *>::iterator wait_pos;
		for(std::multimap<ClusterGroup* ,ClusterGroup *>::iterator iter3= this->groupSrc2SnkChannel.begin();iter3 != this->groupSrc2SnkChannel.end();iter3++)
		{	
			if(p == maxIter){wait_pos = iter3;break;}
			p++;
		}
		coarseGroupSort.push_back(make_pair(wait_pos->first, wait_pos->second));
		//融合
		ClusterGroup *resultGroup = FussGroup(wait_pos->first, wait_pos->second);	
		coarseGroupResult.push_back(resultGroup);
		coarsingWeight.clear();
		currentGroups.clear();
	}
	mngroups = k;
}

/*
//融合，最终融合的结果是不定的，即K的不是预先输入的，而是根据图的性质来确定的（不过最终融合后的group的数目要大于集群节点的数目）
void ClusterGroupGraph::CreateCoarseGraph()
{//(该函数未调试)
	std::vector< std::pair<ClusterGroup*,ClusterGroup*> > vecswap;
	coarseGroupSort.swap(vecswap);//使用前先将其清空
	coarseGroupResult.clear();
	int coarsingcount = 0;

	while(1)
	{
		int groupcount = 0;

		//找一对group进行合并，要求通信比的增量最大，并且合并过后不会引入环
		//计算每一对group的计算通信比
		std::vector<float> coarsingWeight;

		std::vector<ClusterGroup *> currentGroups;
		for(std::map<ClusterGroup *, float>::iterator cluster_iter = this->clusterGroup2Radio.begin();cluster_iter != this->clusterGroup2Radio.end();cluster_iter++)
		{
			currentGroups.push_back(cluster_iter->first);
		}
		cout<<"===当前group的数目  "<<clusterGroup2Radio.size()<<"  边的数目  "<<groupSrc2SnkChannel.size() <<endl;			
		//计算每一对相邻group的计算通信比
		for(std::multimap<ClusterGroup* ,ClusterGroup *>::iterator iter= this->groupSrc2SnkChannel.begin();iter != this->groupSrc2SnkChannel.end();iter++)
		{
			groupcount++;
			coarsingcount++;

			ClusterGroup* tmpGroup = new ClusterGroup(iter->first,iter->second);
			ModifyConnectionOfGroup(iter->first,iter->second,tmpGroup);
			std::vector<ClusterGroup *>::iterator pos1,pos2;
			for (std::vector<ClusterGroup *>::iterator iter1 = currentGroups.begin(); iter1 != currentGroups.end(); iter1++)
			{
				if((*iter1) == iter->first) {pos1 = iter1;break;}
			}
			assert(pos1 != currentGroups.end() && (*pos1) == iter->first);
			currentGroups.erase(pos1);
			for (std::vector<ClusterGroup *>::iterator iter1 = currentGroups.begin(); iter1 != currentGroups.end(); iter1++)
			{
				if((*iter1) == (iter->second)) {pos2 = iter1;break;}
			}
			assert(pos2 != currentGroups.end() && (*pos2) == (iter->second) );
			currentGroups.erase(pos2);
			currentGroups.push_back(tmpGroup);
			cout<<"   计算通信比增量 "<<tmpGroup->GetCompCommRadio()-(iter->first->GetCompCommRadio() + iter->second->GetCompCommRadio())<<endl;
			if(HasGroupTopologicalSort(currentGroups))
			{
				coarsingWeight.push_back(tmpGroup->GetCompCommRadio()-(iter->first->GetCompCommRadio() + iter->second->GetCompCommRadio()));//无环，插入
				cout<<"   无环  "<<groupcount<<"   "<<coarsingcount<<endl;
			}
			else 
			{
				cout<<"   有环   "<<groupcount<<"   "<<coarsingcount<<endl;
				coarsingWeight.push_back(MIN_WEIGHT);
			}
			currentGroups.pop_back();
			currentGroups.push_back(iter->first);
			currentGroups.push_back(iter->second);

			//由于在构造新的group时修改了iter->first，和iter->second中的前驱group的后继，和后继group中的前驱，现在要还原
			vector<ClusterGroup *>tmpPreGroup = tmpGroup->GetPrecedenceClusterGroup();
			vector<ClusterGroup *>tmpSuccGroup = tmpGroup->GetSuccessorClusterGroup();
			vector<FlatNode *> tmpFlatNodes1 = iter->first->GetFlatNodes();
			vector<FlatNode *> tmpFlatNodes2 = iter->second->GetFlatNodes();
			Bool flag1 = FALSE;
			Bool flag2 = FALSE;
			for(int i = 0;i !=tmpPreGroup.size(); i++)
			{
				flag1 = FALSE;
				flag2 = FALSE;
				if(tmpPreGroup[i]->DeleteSuccessorClusterGroup(tmpGroup))
				{
					//前驱的snk节点				
					vector<FlatNode *> tmpSnkFlatNode = tmpPreGroup[i]->GetSnkFlatNode();

					for(int j =0; j != tmpSnkFlatNode.size();j++)
					{
						for(int k = 0; k < tmpSnkFlatNode[j]->nOut;k++)
						{
							for (int l = 0; l != tmpFlatNodes1.size(); l++)
							{
								if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
							}
							for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
							{
								if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
							}
						}

					}
					if(flag1)  tmpPreGroup[i]->AddSuccessorClusterGroup(iter->first);
					if(flag2)  tmpPreGroup[i]->AddSuccessorClusterGroup(iter->second);
				}						
			}
			for(int i = 0;i !=tmpSuccGroup.size(); i++)
			{
				flag1 = FALSE;
				flag2 = FALSE;
				if(tmpSuccGroup[i]->DeletePrecedenceClusterGroup(tmpGroup))
				{
					//前驱的snk节点				
					vector<FlatNode *> tmpSrcFlatNode = tmpSuccGroup[i]->GetSrcFlatNode();

					for(int j =0; j != tmpSrcFlatNode.size();j++)
					{
						for(int k = 0; k < tmpSrcFlatNode[j]->nIn;k++)
						{
							for (int l = 0; l != tmpFlatNodes1.size(); l++)
							{
								if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
							}
							for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
							{
								if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
							}
						}
					}
					if(flag1)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(iter->first);
					if(flag2)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(iter->second);
				}								
			}
			assert(currentGroups.size() == this->clusterGroup2Radio.size());//保证经过上面的操作后group的数目没有变
		}
		//找最大边的位置
		float maxWeight = MIN_WEIGHT;
		int maxIter = 0;
		for(int i = 0; i!= coarsingWeight.size();i++)
		{
			if(coarsingWeight[i]>maxWeight)
			{
				maxWeight = coarsingWeight[i];
				maxIter = i;
			}
		}
		if(maxWeight < THRESHOD_COMP_COMM_RADIO) break;
		int p = 0;
		std::multimap<ClusterGroup* ,ClusterGroup *>::iterator wait_pos;
		for(std::multimap<ClusterGroup* ,ClusterGroup *>::iterator iter3= this->groupSrc2SnkChannel.begin();iter3 != this->groupSrc2SnkChannel.end();iter3++)
		{	
			if(p == maxIter){wait_pos = iter3;break;}
			p++;
		}
		coarseGroupSort.push_back(make_pair(wait_pos->first, wait_pos->second));
		//融合
		ClusterGroup *resultGroup = FussGroup(wait_pos->first, wait_pos->second);	
		//currentGroups.push_back(resultGroup);
		coarseGroupResult.push_back(resultGroup);
		vector<float> swapVec1;
		coarsingWeight.swap(swapVec1);
		vector<ClusterGroup *> swapVec;
		currentGroups.swap(swapVec);

	}
	SetGroupNum(clusterGroup2Radio.size());//设置最终的group数目
}
*/

std::vector< std::pair<ClusterGroup*,ClusterGroup*> > ClusterGroupGraph::GetCoarseGroupSort()
{//返回融合的顺序
	return coarseGroupSort;
}

std::vector<ClusterGroup *> ClusterGroupGraph::GetCoarseGroupResult()
{//返回融合的顺序
	return coarseGroupResult;
}

std::vector<ClusterGroup *> ClusterGroupGraph::GetClusterGroupSet()
{
	std::vector<ClusterGroup *> vecswap;
	clusterGroupVec.swap(vecswap);//清空原来的内容，并释放内存
	typedef std::map<ClusterGroup *,float>::iterator Num2NodeIter;
	for(Num2NodeIter iter=clusterGroup2Radio.begin();iter!=clusterGroup2Radio.end();++iter)
	{
		clusterGroupVec.push_back(iter->first);
	}
	return clusterGroupVec;
}

ClusterGroup *ClusterGroupGraph::GetClusterGroup(FlatNode *floatNode)
{
	std::map<FlatNode *,ClusterGroup *>::iterator pos;
	pos = flatNode2ClusterGroup.find(floatNode);
	return pos->second;	
}

int ClusterGroupGraph::GetGroupNum()
{
	return mngroups;
}

void ClusterGroupGraph::SetGroupNum(int n)
{
	mngroups = n;
}

std::multimap<ClusterGroup*, ClusterGroup*> ClusterGroupGraph::GetGroupSrc2SnkChannel()
{
	return groupSrc2SnkChannel;
}


void ClusterGroupGraph::ModifyConnectionOfGroup(ClusterGroup *group1,ClusterGroup *group2,ClusterGroup* group)
{
	//前驱clusterGroup
	//两个group合并过后要修改他们的前驱的后继以及后继的前驱，（注意之后还要还原）
	vector<ClusterGroup*> precedenceClusterGroup1 = group1->GetPrecedenceClusterGroup();
	vector<ClusterGroup*> precedenceClusterGroup2 = group2->GetPrecedenceClusterGroup();
	vector<ClusterGroup*> successorClusterGroup1 = group1->GetSuccessorClusterGroup();
	vector<ClusterGroup*> successorClusterGroup2 = group2->GetSuccessorClusterGroup();;
	for(int i = 0; i != precedenceClusterGroup1.size(); i++)
	{
		if(precedenceClusterGroup1[i] != group2) 
		{
			group->AddPrecedenceClusterGroup(precedenceClusterGroup1[i]);
			//要修改前驱的后继
			if(precedenceClusterGroup1[i]->DeleteSuccessorClusterGroup(group1))//修改前驱的后继
				precedenceClusterGroup1[i]->AddSuccessorClusterGroup(group);
		}
	}
	for (int i = 0; i !=precedenceClusterGroup2.size(); i++ )
	{
		if(precedenceClusterGroup2[i] != group1)
		{
			group->AddPrecedenceClusterGroup(precedenceClusterGroup2[i]);
			//要修改前驱的后继
			if(precedenceClusterGroup2[i]->DeleteSuccessorClusterGroup(group2))//修改前驱的后继
				precedenceClusterGroup2[i]->AddSuccessorClusterGroup(group);
		}		
	}
	//后继clusterGroup
	for(int i = 0; i != successorClusterGroup1.size(); i++)
	{
		if(successorClusterGroup1[i] != group2) 
		{
			group->AddSuccessorClusterGroup(successorClusterGroup1[i]);
			//要修改后继的前驱
			if(successorClusterGroup1[i]->DeletePrecedenceClusterGroup(group1))//修改后继的前驱
				successorClusterGroup1[i]->AddPrecedenceClusterGroup(group);
		}
	}
	for (int i = 0; i !=successorClusterGroup2.size(); i++ )
	{
		if(successorClusterGroup2[i] != group1)
		{
			group->AddSuccessorClusterGroup(successorClusterGroup2[i]);
			if(successorClusterGroup2[i]->DeletePrecedenceClusterGroup(group2))//修改后继的前驱
				successorClusterGroup2[i]->AddPrecedenceClusterGroup(group);
		}		
	}
}

std::map<FlatNode *,ClusterGroup *> ClusterGroupGraph::GetFlatNode2ClusterGroup()
{	//返回flatNode与group之间的map
	return flatNode2ClusterGroup;
}