#ifndef _RRS_PARTITION_H_
#define _RRS_PARTITION_H_

#include "Partition.h"

class RRSPartiton:public Partition
{
public:
	RRSPartiton();
	void SssgPartition(SchedulerSSG *sssg ,int level);
private:
	//循环分发调度（RRS）会需要用到的成员变量
	int totalActors;//总actor数目
	int perParts;//每个部分的actor数目
};

#endif 