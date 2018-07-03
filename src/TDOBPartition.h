#ifndef _TDOB_PARTITION_H_
#define _TDOB_PARTITION_H_

#include "Partition.h"

//TDOB: Top-down overhead balance
class TDOBPartition:public Partition
{
public:
	TDOBPartition();
	void SssgPartition(SchedulerSSG *sssg, int level);
	bool Select(const FlatNode *, const double, const double);
	bool FirstPhasePartition(SchedulerSSG *sssg);
	bool SecondPhasePartiton(SchedulerSSG *sssg, const std::vector<FlatNode *> &);
private:
	//自顶向下负载均衡划分算法(TDOB）用到的成员变量
	double total_work;//总的工作量
	int total_node;
};

#endif 