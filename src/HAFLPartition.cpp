#include "HAFLPartition.h"

using namespace std;

void HAFLPartition::SssgPartition(SchedulerSSG *sssg,int level)
{
	assert(level==1);
	for (int i = 0; i < nvtxs; i++)
	{
		flatNodes_.push_back(sssg->GetFlatNodes()[i]);
	}
	vector<FlatNode *>::iterator iter_1;
	for (iter_1=flatNodes_.begin();iter_1!=flatNodes_.end();++iter_1)//遍历所有结点
	{
		if (!DetectiveActorState(*iter_1) && (UporDownStatelessNode((*iter_1)) != 3))
		{
			FlatNode2PartitionNum.insert(make_pair((*iter_1),0));//建立节点到划分编号的映射,划分到CPU端
			PartitonNum2FlatNode.insert(make_pair(0,(*iter_1)));
			(*iter_1)->GPUPart = 0;
			GPUNodes.push_back((*iter_1));
			//MultiNum2FlatNode.insert(make_pair(sssg->GetFlatNodes()[i],MultiNum/this->mnparts));
		}
		else
		{
			FlatNode2PartitionNum.insert(make_pair((*iter_1),GpuNum));//建立节点到划分编号的映射，划分到GPU端
			PartitonNum2FlatNode.insert(make_pair(GpuNum,(*iter_1)));
			(*iter_1)->GPUPart = GpuNum;
			CPUNodes.push_back((*iter_1));
			//MultiNum2FlatNode.insert(make_pair(sssg->GetFlatNodes()[i],MultiNum));
		}
	}
}
void HAFLPartition::SetMultiNum2FlatNode()
{
	vector<FlatNode *>::iterator iter_1;
	for (iter_1=flatNodes_.begin();iter_1!=flatNodes_.end();++iter_1)//遍历所有结点
	{
		if((*iter_1)->GPUPart >= GpuNum)
			MultiNum2FlatNode.insert(make_pair((*iter_1),MultiNum));
		else
			MultiNum2FlatNode.insert(make_pair((*iter_1),MultiNum/GpuNum));
	}
}
int HAFLPartition::UporDownStatelessNode(FlatNode *node)
{
	int marks=0,marks1 = 0,marks2 = 0;
	vector<FlatNode*>::iterator iter1;
	for (iter1 = node->inFlatNodes.begin(); iter1 != node ->inFlatNodes.end(); ++iter1)
	{
		if (!DetectiveActorState(*iter1))
		{
			marks1 = 0;
			break;
		}
		else
			marks1 = 1;
	}
	for (iter1 = node->outFlatNodes.begin(); iter1 != node->outFlatNodes.end(); ++iter1)
	{
		if (!DetectiveActorState(*iter1))
		{
			marks2 = 0;
			break;
		}
		else
			marks2 = 2;
	}
	if(marks1 != 0 && marks2 == 0)marks = marks1;
	else if (marks1 == 0 && marks2 != 0)
	{
		marks = marks2;
	}
	else if (marks1 != 0 && marks2 != 0)
	{
		marks = 3;
	}
	return marks;
}

vector<FlatNode *> HAFLPartition::FindNodeInOneDevice(int parts)
{
	if (parts == GpuNum || parts >= 2*GpuNum+1)
	{
		return CPUNodes;
	}
	else
		return GPUNodes;
}

int HAFLPartition::Index2Device(int index)
{
	if (index == GpuNum || index >= 2*GpuNum+1 )
	{
		return GpuNum;
	}
	else 
		return 0;
}

void HAFLPartition::SetGpuNum(SchedulerSSG *sssg)
{
	bool ispeekbig = false;
	nvtxs=sssg->GetFlatNodes().size();
	for (int i=0;i<nvtxs;i++)
	{
		FlatNode * actor = sssg->GetFlatNodes()[i];
		for (int i = 0; i < actor->nIn;++i)
		{
			if (actor->inPeekWeights[i] > actor->inPopWeights[i])
			{
				ispeekbig = true;
				break;
			}
		}
		if (ispeekbig)
		{
			GpuNum = 1;
			break;
		}
	}
	if (GpuNum != 1)
	{
		vector<FlatNode *> tmp = sssg->GetFlatNodes();
		if(GpuNum > tmp.size())
			GpuNum = tmp.size();//如果图中结点数少于place数目，则取结点数目即可
	}
}