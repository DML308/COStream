#include "TDOBPartition.h"
static int *processed_place = NULL; 
static int *processed_flatnode = NULL;
static int select_state = 0;
using namespace std;

TDOBPartition::TDOBPartition():Partition()
{
	total_work = 0;
}

bool TDOBPartition::Select(const FlatNode *src, const double current_work, const double aver)
{
	double x[2] = {0}, max = 0; 

	x[0] = current_work / aver;
	x[1] = (current_work + src->work_estimate) / aver;
	for (int i = 0; i < 2; ++i)
	{
		if (x[i] > 1) 
			x[i] = 1/x[i];
	}
	if (x[0] > x[1])//选择该节点导致负载平衡差距增大
	{
		select_state = 1;
		return false;
	}
		

	int count = 0;
	for (int i = 0; i < src->inFlatNodes.size(); ++i)
	{
		if (processed_flatnode[src->inFlatNodes[i]->num] == 1)
		{
			++count;
		}
	}

	if (count == src->inFlatNodes.size())//该节点的每个输入节点都已被处理了
	{
		select_state = 1;
		return true;
	}
		
	else
	{
		select_state = 2;
		return false;
	}
}

bool TDOBPartition::FirstPhasePartition(SchedulerSSG *sssg)
{
	double aver = total_work/CpuCoreNum;
	total_node = sssg->GetFlatNodes(-1).size();

	std::vector<FlatNode *> tmpFlatNodes = sssg->GetFlatNodes(-1);

	for (int place = 0; place < CpuCoreNum; ++place)
	{
		std::vector<FlatNode *> groups;
		for (int ki = 0; ki < tmpFlatNodes.size(); ++ki)
		{
			if (processed_flatnode[ki] == 0)//未处理
				groups.push_back(tmpFlatNodes[ki]);
		}

		if (place == (CpuCoreNum-1)) // 最后一个place
		{
			for (int ki = 0; ki < groups.size(); ++ki)
			{
				groups[ki]->place_id = place;
				groups[ki]->thread_id = 0;
				processed_flatnode[groups[ki]->num] = 1;
			}
			continue;
		}

		//找到拓扑排序的顶端节点
		FlatNode *top = FindTopFlatnode(groups);

		if (top != NULL)
		{
			double current_work = 0;//节点集合的当前工作量
			std::list<FlatNode *> tmp_list;
			tmp_list.push_back(top);
			current_work += top->work_estimate;
			processed_flatnode[top->num] = 1;
			top->place_id = place;
			top->thread_id = 0;
			while (!tmp_list.empty())
			{
				top = tmp_list.front();
				tmp_list.pop_front();

				for (int i = 0; i < top->outFlatNodes.size(); ++i)
					if (Select(top->outFlatNodes[i], current_work, aver))
					{
						tmp_list.push_back(top->outFlatNodes[i]);
						current_work += top->outFlatNodes[i]->work_estimate;
						processed_flatnode[top->outFlatNodes[i]->num] = 1;
						top->outFlatNodes[i]->place_id = place;
						top->outFlatNodes[i]->thread_id = 0;
					}

					if(tmp_list.empty() && select_state == 2)
					{
						groups.clear();
						for (int ki = 0; ki < tmpFlatNodes.size(); ++ki)
						{
							if (processed_flatnode[ki] == 0)//未处理
								groups.push_back(tmpFlatNodes[ki]);
						}
						top = FindTopFlatnode(groups);
						if (top != NULL && Select(top, current_work, aver))
						{
							tmp_list.push_back(top);
							current_work += top->work_estimate;
							processed_flatnode[top->num] = 1;
							top->place_id = place;
							top->thread_id = 0;
						}
					}
			}
		}

	}

	return true;
}

bool TDOBPartition::SecondPhasePartiton(SchedulerSSG *sssg, const std::vector<FlatNode *> &src)
{
	double aver = total_work/NThreads;

	for (int thread = 0; thread < NThreads; ++thread)
	{
		std::vector<FlatNode *> groups;
		for (int ki = 0; ki < src.size(); ++ki)
		{
			if (processed_flatnode[src[ki]->num] == 0)//未处理
				groups.push_back(src[ki]);
		}

		if (thread == (NThreads-1)) // 最后一个place
		{
			for (int ki = 0; ki < groups.size(); ++ki)
			{
				groups[ki]->thread_id = thread;
				processed_flatnode[groups[ki]->num] = 1;
			}
			continue;
		}

		//找到拓扑排序的顶端节点
		FlatNode *top = FindTopFlatnode(groups);

		if (top != NULL)
		{
			double current_work = 0;//节点集合的当前工作量
			std::list<FlatNode *> tmp_list;
			tmp_list.push_back(top);
 			processed_flatnode[top->num] = 1;
			top->thread_id = thread;
			current_work += top->work_estimate;
			while (!tmp_list.empty())
			{
				top = tmp_list.front();
				tmp_list.pop_front();

				for (int i = 0; i < top->outFlatNodes.size(); ++i)
					if (Select(top->outFlatNodes[i], current_work, aver))
					{
						tmp_list.push_back(top->outFlatNodes[i]);
						current_work += top->outFlatNodes[i]->work_estimate;
						processed_flatnode[top->outFlatNodes[i]->num] = 1;
						top->outFlatNodes[i]->thread_id = thread;
					}

					if(tmp_list.empty() && select_state == 2)
					{
						groups.clear();
						for (int ki = 0; ki < src.size(); ++ki)
						{
							if (processed_flatnode[src[ki]->num] == 0)//未处理
								groups.push_back(src[ki]);
						}
						top = FindTopFlatnode(groups);
						if (top != NULL && Select(top, current_work, aver))
						{
							tmp_list.push_back(top);
							current_work += top->work_estimate;
							processed_flatnode[top->num] = 1;
							top->thread_id = thread;
						}
					}
			}
		}

	}
	return true;
}


void TDOBPartition::SssgPartition(SchedulerSSG *sssg, int level)
{
	std::vector<FlatNode *> tmpFlatNodes = sssg->GetFlatNodes(-1);
	processed_flatnode = new int[tmpFlatNodes.size()];

	for (int i = 0; i < tmpFlatNodes.size(); ++i)
		processed_flatnode[i] = 0;

	if(level == 1)
	{
		total_work = sssg->total_work;
		FirstPhasePartition(sssg);

#if 1 //打印图
		//DumpStreamGraph(sssg,this,"TDOBPartitionGraph-1.dot", NULL);
#endif
		
	}
	else
	{
		for (int place = 0; place < CpuCoreNum; ++place)
		{
			total_work = 0;
			std::vector<FlatNode *> src = sssg->GetFlatNodes(place);
			for (int i = 0; i < src.size(); ++i)
				total_work += src[i]->work_estimate;
			SecondPhasePartiton(sssg, src);
		}

#if 1 //打印图
		//DumpStreamGraph(sssg,this,"TDOBPartitionGraph-2.dot", NULL);
#endif
	}

}