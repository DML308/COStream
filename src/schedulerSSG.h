
#ifndef _SCHEDULER_SSG_H_
#define _SCHEDULER_SSG_H_


#include "staticStreamGraph.h"

class SchedulerSSG : public StaticStreamGraph 
{

public:
	std::map<FlatNode *,int> mapInitCount2FlatNode; // SDFͼ���нڵ��ʼ����������
	std::map<FlatNode *,int> mapSteadyCount2FlatNode; // SDFͼ���нڵ��ȶ�״̬��������<�ڵ㣬ִ�д���>
	double total_work;//SDF�ܵĹ�����
	double edge_work;// add by wangliang SDF�ܵıߴ�С 

	/*�����������Ϊ��ȺSDF��ͼ����*/
	std::map<FlatNode *, std::vector<Node *> > mapDownBordNode2Edge; //�߽�ڵ� by yqj 2015-3-5
	std::map<FlatNode *, std::vector<Node *> > mapUpBordNode2Edge;
	std::map<FlatNode *, std::map<FlatNode *, int> >  mapDownNode2OutNode2Nplace; // ��A->B->2��A���ⲿ�ڵ㣬B���ڲ��ڵ㣬2��A���ڵĻ��ֺ�
	std::map<FlatNode *, std::map<FlatNode *, int> >  mapUpNode2OutNode2Nplace; //��A->B->2 A���ڲ���,B���ⲿ�ߣ�2��B���ڵĻ��ֺ�
	std::map<std::string, std::string> mapBoardEdgeDecl2EdgeDefine;
	std::map<FlatNode *, FlatNode *> mapOldNode2NewNode;
	std::map<FlatNode *, FlatNode *> mapNewNode2OldNode;
	int currNplace; //��sssg���ڼ�Ⱥ�еĽڵ���

public:
	SchedulerSSG(StaticStreamGraph *ssg);
	int lcm(int a, int b); // ��a,b����С������
	int gcd(int a, int b); // ��a,b�����Լ��
	bool InitScheduling();
	bool SteadyScheduling();

	/*ysz��̬������ר��*/
	bool InitScheduling2();
	bool SteadyScheduling2();

	std::map<FlatNode *, int> SteadySchedulingGroup(std::vector<FlatNode *> );//��vector�еĵ㼯���оֲ���̬����
	int GetInitCount(FlatNode *node);
	int GetSteadyCount(FlatNode *node);

	/*����Ӧ����SDFͼ*/
	void hClusterAdjust();
};

GLOBAL SchedulerSSG *SchedulingSSG(StaticStreamGraph *ssg);
//GLOBAL void DumpStreamGraph(SchedulerSSG *ssg,const char *fileName);

#endif