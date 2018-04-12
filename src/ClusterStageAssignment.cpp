#include "ClusterStageAssignment.h"
#include <map>
#include <vector>
using namespace std;

ClusterStageAssignment::ClusterStageAssignment(ClusterPartition *cp):cpartition(cp)
{
}

void ClusterStageAssignment::CreateStageAssignment()
{//��������Ⱥ������л����ϵĽڵ���н׶λ���
	map<FlatNode *,int>actor2StageSwap;//���ڴ洢�׶θ�ֵ�Ľ��
	multimap<int,FlatNode*>stage2ActorSwap;//���ڴ洢�׶θ�ֵ�Ľ��
	for(int i = 0 ;i != cpartition->GetClusters(); i++ )
	{
		stage2Actor.swap(stage2ActorSwap);
		actor2Stage.swap(actor2StageSwap);
		vector<FlatNode *> tmpFlatNodes = cpartition->GetFlatNodes(i);
		map<FlatNode *,int> tmpFlatNode2Core = cpartition->GetFlatNode2Core(i);
		CreateFlatNode2StageMap(tmpFlatNode2Core);
		cluster2Stage2Actor.insert(make_pair(i,stage2Actor));
		cluster2Actor2Stage.insert(make_pair(i,actor2Stage));
	}
	for(map<int,map<FlatNode * ,int> >::iterator iter = cluster2Actor2Stage.begin(); iter != cluster2Actor2Stage.end(); iter++)
	{
		for(map<FlatNode *, int>::iterator iter1 = (iter->second).begin(); iter1 != (iter->second).end(); iter1++)
		{
			actor2Cluster2stage.insert(make_pair(iter1->first,make_pair(iter->first,iter1->second)));
		}
	}
}

vector<FlatNode *> ClusterStageAssignment::CreateTopoLogicalOrder(vector<FlatNode *>original)
{//�ڼ�Ⱥ��һ���ڵ�����һ����������
	vector<FlatNode *> actorTopoVec;
	actorTopoOrder.swap(actorTopoVec);
	vector<int> nInDegree;//���ڱ�����ڵ�����
	vector<FlatNode *> flatNodeStack;
	int nsize=original.size();
	for (int i = 0;i != nsize;++i)
	{
		int inCount = 0;
		for(int j = 0; j < original[i]->nIn; j++)
		{
			for(int k = 0; k != nsize; k++)
			{
				if ( original[i]->inFlatNodes[j] == original[k]) inCount++;
			}
		}
		nInDegree.push_back(inCount);//�����ڵ���ȱ���
	}
	for (int i = 0; i != nInDegree.size();i++)
	{
		if(!nInDegree[i]) flatNodeStack.push_back(original[i]);
	}

	while (!flatNodeStack.empty())
	{
		FlatNode *tmpFlatNode = flatNodeStack.back();// ȡ��Ҫ��ջ�Ľڵ�
		actorTopoOrder.push_back(tmpFlatNode);
		flatNodeStack.pop_back();
		for(int i= 0; i != tmpFlatNode->nOut;i++)//������group���ڽӵ����ȼ�1
		{
			for (int j =0;j != original.size(); j++)
			{
				if(original[j] == tmpFlatNode->outFlatNodes[i])
				{
					if(!(--nInDegree[j])) flatNodeStack.push_back(original[j]);//���Ϊ0���ջ
				}
			}
		}
	}
	assert(actorTopoOrder.size() == original.size());
	return actorTopoOrder;
}


void ClusterStageAssignment::CreateFlatNode2StageMap(map<FlatNode *,int>FlatNode2Core)
{
	int maxstage,stage;
	bool flag;
	vector<FlatNode *>::iterator iter1,iter2;
	map<FlatNode *,int>::iterator iter3,iter4,iter5;
	map<FlatNode *,int>::iterator iter;
	Bool inTag = FALSE;
	vector<FlatNode *> tmpFlatNodes;
	for(map<FlatNode *,int>::iterator iter6 = FlatNode2Core.begin(); iter6 != FlatNode2Core.end(); iter6++)
	{
		tmpFlatNodes.push_back(iter6->first);
	}
	vector<FlatNode *> actorTopoOrder = CreateTopoLogicalOrder(tmpFlatNodes);
#if 0
	cout<<"########"<<endl;
	for(int i = 0; i<actorTopoOrder.size();i++)
	{
		cout<<actorTopoOrder[i]->name<<endl;
	}
	cout<<"########"<<endl;
#endif
	

	for (iter1=actorTopoOrder.begin();iter1!=actorTopoOrder.end();++iter1)
	{
		maxstage=0;
		flag=false;
		for (iter2=(*iter1)->inFlatNodes.begin(); iter2!=(*iter1)->inFlatNodes.end(); ++iter2)
		{
			inTag = FALSE;
			for(int i = 0; i != tmpFlatNodes.size(); i++)
			{
				if( *iter2 == tmpFlatNodes[i]) { inTag = TRUE; break;}
			}
			if (inTag)
			{
				iter=actor2Stage.find(*iter2);
				if ( (iter->second) >= maxstage )
				{
					maxstage=iter->second;
				}
				iter3=FlatNode2Core.find(*iter1);//����*iter1����Ӧ�Ļ��ֺ�
				iter4=FlatNode2Core.find(*iter2);//����*iter2����Ӧ�Ļ��ֺ�
				if (iter3->second != iter4->second)
				{
					flag=true;
				}
			}
			
		}
		if (flag)
		{
			stage=maxstage+1;
		}
		else
		{
			stage=maxstage;
		}
		actor2Stage.insert(make_pair(*iter1,stage));
		stage2Actor.insert(make_pair(stage,*iter1));
	}

}

int ClusterStageAssignment::GetStageNum(FlatNode* actor)
{
	map<FlatNode *,pair<int, int> >::iterator iter=actor2Cluster2stage.find(actor);
	if (iter!=actor2Cluster2stage.end())
	{
		return (iter->second).second;
	}
	else return -1;
}

multimap<int,FlatNode *> ClusterStageAssignment::GetCluster2Stage2Actor(int clusterNum)//ȡ��Ⱥ��һ���ڵ�Ľ׶θ��ƵĽ��
{
	map<int,multimap<int, FlatNode *> >::iterator pos;
	pos = cluster2Stage2Actor.find(clusterNum);
	assert(pos != cluster2Stage2Actor.end());
	return pos->second;
}

map<FlatNode*, int> ClusterStageAssignment::GetCluster2Actor2Stage(int clusterNum)
{
	map<int,map<FlatNode *, int> >::iterator pos;
	pos = cluster2Actor2Stage.find(clusterNum);
	assert(pos != cluster2Actor2Stage.end());
	return pos->second;
}

map<int,multimap<int ,FlatNode *> > ClusterStageAssignment::GetAllCluster2Stage2Actor()
{//�������еĽ��
	return this->cluster2Stage2Actor;
}

map<int,map<FlatNode * ,int> > ClusterStageAssignment::GetAllCluster2Actor2Stage()
{
	return this->cluster2Actor2Stage;
}

vector<FlatNode*> ClusterStageAssignment::GetActors(int clusterNum,int stage)
{
	vector<FlatNode *> swapVec;
	actorSet.swap(swapVec);
	multimap<int,FlatNode *> tmpStage2Actor = GetCluster2Stage2Actor(clusterNum); 
	typedef multimap<int,FlatNode*>::iterator Num2NodeIter;
	pair<Num2NodeIter,Num2NodeIter>range=tmpStage2Actor.equal_range(stage);
	for(Num2NodeIter iter=range.first;iter!=range.second;++iter)
	{
		actorSet.push_back(iter->second);
	}
	return actorSet;
}

int ClusterStageAssignment::GetMaxStageNum(int clusterNum)
{
	multimap<int,FlatNode *> tmpStage2Actor = GetCluster2Stage2Actor(clusterNum); 
	multimap<int,FlatNode*>::iterator iter1 = tmpStage2Actor.end();
	iter1--;
	return iter1->first + 1; 
}
