#ifndef _RRS_PARTITION_H_
#define _RRS_PARTITION_H_

#include "Partition.h"

class RRSPartiton:public Partition
{
public:
	RRSPartiton();
	void SssgPartition(SchedulerSSG *sssg ,int level);
private:
	//ѭ���ַ����ȣ�RRS������Ҫ�õ��ĳ�Ա����
	int totalActors;//��actor��Ŀ
	int perParts;//ÿ�����ֵ�actor��Ŀ
};

#endif 