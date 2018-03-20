
#ifndef _SCHEDULER_SSG_H_
#define _SCHEDULER_SSG_H_


#include "staticStreamGraph.h"

class SchedulerSSG : public StaticStreamGraph 
{

public:
	std::map<FlatNode *,int> mapInitCount2FlatNode; // SDF图所有节点初始化调度序列
	std::map<FlatNode *,int> mapSteadyCount2FlatNode; // SDF图所有节点稳定状态调度序列<节点，执行次数>
	double total_work;//SDF总的工作量
	double edge_work;// add by wangliang SDF总的边大小 

	/*以下类的属性为集群SDF子图设立*/
	std::map<FlatNode *, std::vector<Node *> > mapDownBordNode2Edge; //边界节点 by yqj 2015-3-5
	std::map<FlatNode *, std::vector<Node *> > mapUpBordNode2Edge;
	std::map<FlatNode *, std::map<FlatNode *, int> >  mapDownNode2OutNode2Nplace; // 如A->B->2，A是外部节点，B是内部节点，2是A所在的划分号
	std::map<FlatNode *, std::map<FlatNode *, int> >  mapUpNode2OutNode2Nplace; //如A->B->2 A是内部边,B是外部边，2是B所在的划分号
	std::map<std::string, std::string> mapBoardEdgeDecl2EdgeDefine;
	std::map<FlatNode *, FlatNode *> mapOldNode2NewNode;
	std::map<FlatNode *, FlatNode *> mapNewNode2OldNode;
	int currNplace; //该sssg所在集群中的节点编号

public:
	SchedulerSSG(StaticStreamGraph *ssg);
	int lcm(int a, int b); // 求a,b的最小公倍数
	int gcd(int a, int b); // 求a,b的最大公约数
	bool InitScheduling();
	bool SteadyScheduling();

	/*ysz动态数据流专用*/
	bool InitScheduling2();
	bool SteadyScheduling2();

	std::map<FlatNode *, int> SteadySchedulingGroup(std::vector<FlatNode *> );//对vector中的点集进行局部稳态调度
	int GetInitCount(FlatNode *node);
	int GetSteadyCount(FlatNode *node);

	/*自适应调整SDF图*/
	void hClusterAdjust();
};

GLOBAL SchedulerSSG *SchedulingSSG(StaticStreamGraph *ssg);
//GLOBAL void DumpStreamGraph(SchedulerSSG *ssg,const char *fileName);

#endif