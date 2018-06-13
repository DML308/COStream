#include "Partition.h"

using namespace std;

static SchedulerSSG *Sssg = NULL;
static int *processed = NULL; 

Partition::Partition(){
	mnparts=1;//��ʼ��������ĿΪ1
}

int Partition::getParts(){
	return this->mnparts;
}
//����place��Ŀ����������Ŀ��
void Partition::setCpuCoreNum(int nplaces,SchedulerSSG *sssg)
{
	this->mnparts = 2;
	if(nplaces != 0){
		vector<FlatNode *> tmp = sssg->GetFlatNodes();
		if(nplaces > tmp.size())
			this->mnparts = tmp.size();//���ͼ�н��������place��Ŀ����ȡ�����Ŀ����
		else
			mnparts = nplaces;
	}
}
//����flatnode�ҵ����±�� ��source_0�е�0
int Partition::findID(SchedulerSSG *sssg,FlatNode *flatnode)
{
	for (int i=0;i<sssg->GetFlatNodes().size();i++)
		if(strcmp(sssg->GetFlatNodes()[i]->name.c_str(),flatnode->name.c_str())==0)
			return i;
}
//���ݱ��num�������еĽڵ�(�������ڵ���1��)�����ڵ㼯�Ϸ��ظ�PartitonNumSet(���->�ڵ�)
vector<FlatNode *> Partition::findNodeSetInPartition(int partitionNum)
{
	vector<FlatNode *> vecswap;
	PartitonNumSet.swap(vecswap);//���ԭ�������ݣ����ͷ��ڴ�
	typedef multimap<int,FlatNode*>::iterator Num2NodeIter;
	pair<Num2NodeIter,Num2NodeIter>range=PartitonNum2FlatNode.equal_range(partitionNum);
	for(Num2NodeIter iter=range.first;iter!=range.second;++iter)
	{
		/*cout<<"$$: "<<iter->second->name<<endl;*/
		PartitonNumSet.push_back(iter->second);
	}
	return PartitonNumSet;
}
//���ݽڵ㷵�������ڻ������ı��(�ڵ�->���)
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


//�������������˸���ƽ�⣬�Ե����ϣ��ϲ�ڵ㡰�ԡ��²�ڵ�
int Partition::UnLock(const std::vector<FlatNode *> &src,const std::vector<FlatNode *> &dest, const int mode)
{
	assert(mode == 1 || mode == 0);// mode == 0��place֮�䣬 mode == 1��place�ڲ�thread֮��
	double sumSrc = 0.0, sumDown = 0.0, sumSrcDown = 0.0, sumDestDown = 0.0, sumSrcUp = 0.0, sumDestUp = 0.0;
	int place = 0; 
	int thread = 0;
	sumSrc = SumOfWork(src);
	sumDown = SumOfWork(dest);

	//�ϲ�ڵ㡰�ԡ��²�ڵ�,��ȫ�ԡ�
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
	case 0://�̵�currentSrcDown
		place = dest[0]->place_id;
		thread = dest[0]->thread_id;
		for (int i = 0; i < currentSrcDown.size(); ++i)
		{
			currentSrcDown[i]->place_id = place;
			currentSrcDown[i]->thread_id = thread;
		}
		break;
	case 1://�̵�currentDestUp
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

//Ŀǰ�ж���ֱ����������A,B����֮����໥����
bool Partition::IsDeadLock(const std::vector<FlatNode *> &src, const std::vector<FlatNode *> &dest,const int mode)
{
	assert(mode == 1 || mode == 0);// mode == 0��place֮�䣬 mode == 1��place�ڲ�thread֮��
	currentSrc2Dest.clear();//�����
	currentDest2Src.clear();//�����
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

//ÿ��asyncֻ��������һ����ͨ��ͼ
void Partition::FinalAdjust(SchedulerSSG *sssg)
{
	std::vector<FlatNode *> nodes = sssg->GetFlatNodes(-1);
	for (int i = 0; i < nodes.size(); ++i) //����join�ڵ㣬������peek>pop�����
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
	assert(mode == 1 || mode == 0);// mode == 0��place֮�䣬 mode == 1��place�ڲ�thread֮��
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
				if (processed[i] == 0)//δ����
					AddFlatnodes(groups, sssg->GetFlatNodes(i));
			}
			//�ҵ���������Ķ��˽ڵ�
			FlatNode *top = FindTopFlatnode(groups);

			if (top != NULL)
			{
				int place = top->place_id;
				for (int ki = 0; ki < CpuCoreNum; ki++)
				{
					if (ki == place || processed[ki] == 1)
						continue;
					while (1) //place֮����������
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
					if (processed[kj] == 0)//δ����
						AddFlatnodes(groups, sssg->GetFlatNodes(i, kj));
				}
				//�ҵ���������Ķ��˽ڵ�
				FlatNode *top = FindTopFlatnode(groups);
				if (top != NULL)
				{
					int thread = top->thread_id;

					for (int kj = 0; kj < NThreads; ++kj)
					{
						if (kj == thread || processed[kj] == 1)
							continue;
						while (1) //place�ڲ�thread֮����������
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
