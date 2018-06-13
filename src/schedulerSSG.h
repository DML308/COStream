
#ifndef _SCHEDULER_SSG_H_
#define _SCHEDULER_SSG_H_


#include "staticStreamGraph.h"

class SchedulerSSG : public StaticStreamGraph 
{

public:
	std::map<FlatNode *,int> mapInitCount2FlatNode; // SDFͼ���нڵ��ʼ����������
	std::map<FlatNode *,int> mapSteadyCount2FlatNode; // SDFͼ���нڵ��ȶ�״̬��������<�ڵ㣬ִ�д���>
	double total_work;//SDF�ܵĹ�����
public:
	SchedulerSSG(StaticStreamGraph *ssg);
	int lcm(int a, int b); // ��a,b����С������
	int gcd(int a, int b); // ��a,b�����Լ��
	bool InitScheduling();
	bool SteadyScheduling();
	std::map<FlatNode *, int> SteadySchedulingGroup(std::vector<FlatNode *> );//��vector�еĵ㼯���оֲ���̬����
	int GetInitCount(FlatNode *node);
	int GetSteadyCount(FlatNode *node);

};

GLOBAL SchedulerSSG *SchedulingSSG(StaticStreamGraph *ssg);
//GLOBAL void DumpStreamGraph(SchedulerSSG *ssg,const char *fileName);

#endif