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
	//�Զ����¸��ؾ��⻮���㷨(TDOB���õ��ĳ�Ա����
	double total_work;//�ܵĹ�����
	int total_node;
};

#endif 