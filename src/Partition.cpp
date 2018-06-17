#include "Partition.h"

using namespace std;

static SchedulerSSG *Sssg = NULL;
static int *processed = NULL; 

Partition::Partition(){
	mnparts=1;//初始化划分数目为1
}

int Partition::getParts(){
	return this->mnparts;
}
//设置place数目（即进程数目）
void Partition::setCpuCoreNum(int nplaces,SchedulerSSG *sssg)
{
	this->mnparts = 2;
	if(nplaces != 0){
		vector<FlatNode *> tmp = sssg->GetFlatNodes();
		if(nplaces > tmp.size())
			this->mnparts = tmp.size();//如果图中结点数少于place数目，则取结点数目即可
		else
			mnparts = nplaces;
	}
}
//根据flatnode找到其下标号 如source_0中的0
int Partition::findID(SchedulerSSG *sssg,FlatNode *flatnode)
{
	for (int i=0;i<sssg->GetFlatNodes().size();i++)
		if(strcmp(sssg->GetFlatNodes()[i]->name.c_str(),flatnode->name.c_str())==0)
			return i;
}
//根据编号num查找其中的节点(个数大于等于1个)，将节点集合返回给PartitonNumSet(编号->节点)
vector<FlatNode *> Partition::findNodeSetInPartition(int partitionNum)
{
	vector<FlatNode *> vecswap;
	PartitonNumSet.swap(vecswap);//清空原来的内容，并释放内存
	typedef multimap<int,FlatNode*>::iterator Num2NodeIter;
	pair<Num2NodeIter,Num2NodeIter>range=PartitonNum2FlatNode.equal_range(partitionNum);
	for(Num2NodeIter iter=range.first;iter!=range.second;++iter)
	{
		/*cout<<"$$: "<<iter->second->name<<endl;*/
		PartitonNumSet.push_back(iter->second);
	}
	return PartitonNumSet;
}
//根据节点返回其所在划分区的编号(节点->编号)
int Partition::findPartitionNumForFlatNode(FlatNode *flatnode)
{
	map<FlatNode *,int>::iterator iter=FlatNode2PartitionNum.find(flatnode);
	if (iter==FlatNode2PartitionNum.end())
		return -1;
	else
		return iter->second;
}
int Partition::FindFlatnode(const std::vector<FlatNode *> &src, const FlatNode *tmp)
{
	for (int i = 0; i < src.size(); ++i)
	{
		if (src[i] == tmp)
			return i;
	}

	return -1;
}
FlatNode *Partition::FindTopFlatnode(const std::vector<FlatNode *> &src)
{
	for (int i = 0; i < src.size(); ++i)
	{
		int x = 0;
		FlatNode *temp_i = src[i];

		for (int j = 0; j < temp_i->inFlatNodes.size(); ++j)
		{
			FlatNode *temp_j = temp_i->inFlatNodes[j];
			if (FindFlatnode(src, temp_j) == -1)
			{
				++x;
			}
		}

		if (x == temp_i->inFlatNodes.size())
			return temp_i;
	}

	return NULL;

}

double Partition::SumOfWork(const std::vector<FlatNode *> &src)
{
	double sum = 0.0;
	for (int i = 0; i < src.size(); ++i)
		sum += src[i]->work_estimate;

	return sum;
}
void Partition::AddDownFlatnodes(std::vector<FlatNode *> &dest, const std::vector<FlatNode *> &src)
{
	for (int i = 0; i < src.size(); ++i)
	{
		for (int j = 0; j < dest.size(); ++j)
		{
			FlatNode *d1 = src[i], *d2 = dest[j];
			for (int k = 0; k < d2->outFlatNodes.size(); ++k)
			{
				if (d1 == d2->outFlatNodes[k])
				{
					if (FindFlatnode(dest, d1) == -1)
					{
						dest.push_back(d1);
					}
				}
			}
		}
	}
}

void Partition::AddUpFlatnodes(std::vector<FlatNode *> &dest, const std::vector<FlatNode *> &src)
{
	for (int i = 0; i < src.size(); ++i)
	{
		for (int j = 0; j < dest.size(); ++j)
		{
			FlatNode *d1 = src[i], *d2 = dest[j];
			for (int k = 0; k < d2->inFlatNodes.size(); ++k)
			{
				if (d1 == d2->inFlatNodes[k])
				{
					if (FindFlatnode(dest, d1) == -1)
					{
						dest.push_back(d1);
					}
				}
			}
		}
	}
}

void Partition::AddFlatnodes(std::vector<FlatNode *> &groups, const std::vector<FlatNode *> &src)
{
	for (int i = 0; i < src.size(); ++i)
	{
		groups.push_back(src[i]);
	}
}


//死锁消除考虑了负载平衡，自底向上，上层节点“吃”下层节点
int Partition::UnLock(const std::vector<FlatNode *> &src,const std::vector<FlatNode *> &dest, const int mode)
{
	assert(mode == 1 || mode == 0);// mode == 0，place之间， mode == 1，place内部thread之间
	double sumSrc = 0.0, sumDown = 0.0, sumSrcDown = 0.0, sumDestDown = 0.0, sumSrcUp = 0.0, sumDestUp = 0.0;
	int place = 0; 
	int thread = 0;
	sumSrc = SumOfWork(src);
	sumDown = SumOfWork(dest);

	//上层节点“吃”下层节点,“全吃”
	if (mode == 0)
	{
		place = currentSrcDown[0]->place_id;
		std::vector<FlatNode *> tmp = Sssg->GetFlatNodes(place);
		AddDownFlatnodes(currentSrcDown, tmp);
		//AddUpFlatnodes(currentSrcUp, tmp);
		if (currentDestDown.size() > 0)
		{
			place = currentDestDown[0]->place_id;
		}
		else
		{
			place = currentDestUp[0]->place_id;
		}

		tmp = Sssg->GetFlatNodes(place);
		//AddDownFlatnodes(currentDestDown, tmp);
		AddUpFlatnodes(currentDestUp, tmp);
	}
	else
	{
		place = currentSrcDown[0]->place_id;
		thread = currentSrcDown[0]->thread_id;
		std::vector<FlatNode *> tmp = Sssg->GetFlatNodes(place, thread);
		AddDownFlatnodes(currentSrcDown, tmp);
		//AddUpFlatnodes(currentSrcUp, tmp);

		if (currentDestDown.size() > 0)
		{
			place = currentDestDown[0]->place_id;
			thread = currentDestDown[0]->thread_id;
		}
		else
		{
			place = currentDestUp[0]->place_id;
			thread = currentDestUp[0]->thread_id;
		}
		tmp = Sssg->GetFlatNodes(place, thread);
		//AddDownFlatnodes(currentDestDown, tmp);
		AddUpFlatnodes(currentDestUp, tmp);
	}

	sumSrcDown = SumOfWork(currentSrcDown);
	//sumDestDown = SumOfWork(currentDestDown);
	//sumSrcUp = SumOfWork(currentSrcUp);
	sumDestUp = SumOfWork(currentDestUp);

	double x[2] = {0}, max = 0; 
	int solution = 0; 
	x[0] = (sumSrc - sumSrcDown)/(sumDown + sumSrcDown);
	x[1] = (sumDown - sumDestUp)/(sumSrc + sumDestUp);

	for (int i = 0; i < 2; ++i)
	{
		if (x[i] > 1) 
			x[i] = 1/x[i];

		if (x[i] > max)
		{
			max = x[i];
			solution = i;
		}
	}

	switch(solution)
	{
	case 0://吞掉currentSrcDown
		place = dest[0]->place_id;
		thread = dest[0]->thread_id;
		for (int i = 0; i < currentSrcDown.size(); ++i)
		{
			currentSrcDown[i]->place_id = place;
			currentSrcDown[i]->thread_id = thread;
		}
		break;
	case 1://吞掉currentDestUp
		place = src[0]->place_id;
		thread = src[0]->thread_id;
		for (int i = 0; i < currentDestUp.size(); ++i)
		{
			currentDestUp[i]->place_id = place;
			currentDestUp[i]->thread_id = thread;
		}
		break;
	default:
		break;
	}

	return solution;
}

//目前判断了直接死锁，即A,B集合之间的相互链接
bool Partition::IsDeadLock(const std::vector<FlatNode *> &src, const std::vector<FlatNode *> &dest,const int mode)
{
	assert(mode == 1 || mode == 0);// mode == 0，place之间， mode == 1，place内部thread之间
	currentSrc2Dest.clear();//先清空
	currentDest2Src.clear();//先清空
	currentSrcUp.clear();
	currentSrcDown.clear();
	currentDestUp.clear();
	currentDestDown.clear();

	if (src.size() == 0 || dest.size() == 0)
		return false;

	for (int i = 0; i < src.size(); ++i)
	{
		FlatNode *node = src[i];
		for (int j = 0; j < node->inFlatNodes.size(); ++j)
		{
			FlatNode *tmp = node->inFlatNodes[j];
			int k = 0;
			for (;k < dest.size() && tmp != dest[k]; ++k);

			if (k != dest.size())
			{
				currentDest2Src.insert(make_pair(tmp, node));
				currentDestUp.push_back(tmp);
				currentSrcDown.push_back(node);
			}

		}

		for (int j = 0; j < node->outFlatNodes.size(); j++)
		{
			FlatNode *tmp = node->outFlatNodes[j];
			int k = 0;
			for (;k < dest.size() && tmp != dest[k]; ++k);

			if (k != dest.size())
			{
				currentSrc2Dest.insert(make_pair(node, tmp));
				currentSrcUp.push_back(node);
				currentDestDown.push_back(tmp);
			}

		}
	}

	if (currentDest2Src.empty())
		return false;
	else
		return true;
}

//每个async只能允许有一个连通子图
void Partition::FinalAdjust(SchedulerSSG *sssg)
{
	std::vector<FlatNode *> nodes = sssg->GetFlatNodes(-1);
	for (int i = 0; i < nodes.size(); ++i) //对于join节点，不存在peek>pop的情况
	{
		if(nodes[i]->inFlatNodes.size() == 1)
		{
			if (nodes[i]->inPeekWeights[0] > nodes[i]->inPopWeights[0])
			{
				nodes[i]->place_id = nodes[i]->inFlatNodes[0]->place_id;
				nodes[i]->thread_id = nodes[i]->inFlatNodes[0]->thread_id;
			}
		}

	}

}

int Partition::Adjust(SchedulerSSG *sssg, int mode)
{
	assert(mode == 1 || mode == 0);// mode == 0，place之间， mode == 1，place内部thread之间
	Sssg = sssg;
	assert(X10DistributedBackEnd == TRUE);
	if (mode == 0)
	{
		processed = new int[CpuCoreNum];

		for (int i = 0; i < CpuCoreNum; ++i)
			processed[i] = 0;

		for (int i = 0; i < CpuCoreNum; ++i)
		{
			std::vector<FlatNode *> groups;
			for (int ki = 0; ki < CpuCoreNum; ++ki)
			{
				if (processed[i] == 0)//未处理
					AddFlatnodes(groups, sssg->GetFlatNodes(i));
			}
			//找到拓扑排序的顶端节点
			FlatNode *top = FindTopFlatnode(groups);

			if (top != NULL)
			{
				int place = top->place_id;
				for (int ki = 0; ki < CpuCoreNum; ki++)
				{
					if (ki == place || processed[ki] == 1)
						continue;
					while (1) //place之间死锁消除
					{
						std::vector<FlatNode *> src = sssg->GetFlatNodes(place);
						std::vector<FlatNode *> dest = sssg->GetFlatNodes(ki);
						if (IsDeadLock(src, dest, 0))
						{
							UnLock(src, dest, 0);
						}
						else
							break;
					}
				}

				processed[place] = 1;
			}
		}
	}
	else
	{
		processed = new int[NThreads];

		for (int i = 0; i < CpuCoreNum; i++)
		{
			for (int ii = 0; ii < NThreads; ++ii)
				processed[ii] = 0;

			for (int j = 0; j < NThreads; j++)
			{
				std::vector<FlatNode *> groups;

				for (int kj = 0; kj < NThreads; ++kj)
				{
					if (processed[kj] == 0)//未处理
						AddFlatnodes(groups, sssg->GetFlatNodes(i, kj));
				}
				//找到拓扑排序的顶端节点
				FlatNode *top = FindTopFlatnode(groups);
				if (top != NULL)
				{
					int thread = top->thread_id;

					for (int kj = 0; kj < NThreads; ++kj)
					{
						if (kj == thread || processed[kj] == 1)
							continue;
						while (1) //place内部thread之间死锁消除
						{
							std::vector<FlatNode *> src = sssg->GetFlatNodes(i, thread);
							std::vector<FlatNode *> dest = sssg->GetFlatNodes(i, kj);
							if (IsDeadLock(src, dest, 1))
							{
								UnLock(src, dest, 1);
							}
							else
								break;
						}
					}
					processed[thread] = 1;
				}
			}
		}
		FinalAdjust(sssg);
	}

	return 1;
}
