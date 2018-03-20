#ifndef _DIVIDED_STATIC_GRAPH_
#define _DIVIDED_STATIC_GRAPH_
#include "../flatNode.h"
#include <set>
#include "../staticStreamGraph.h"
#include "../schedulerSSG.h"
#include <vector>
using namespace std;

extern "C"
{
	extern int NThreads;
}

enum CombineState
{
	FULLCORE, MERGEONE,MERGETWO, MERGEUPONE, MERGEUPTWO,MERGEDOWNONE,MERGEDOWNTWO, NOTMERGE, NOTJUDGE
	//FULLCORE表示是8核，不进行合并，MERGE表示要合并，合并的对象是
	//下一个MERGE.NOTMERGE表示经过计算合并开销更大，不进行合并，
	//或者是最后剩下一个子图不进行合并,NOTJUDGE表示没进行处理
	//MERGEUP表示上游子图压缩，MERGEDOWN表示下游子图压缩
};
enum ActorPosition		//用于表示算子在子图中的位置
{
	FIRST, LAST, MIDDLE,BOTH//BOTH表示一个算子其上端算子输出边和本身输出边都是动态，这表示该子图只有一个算子
};
class DividedStaticGraph
{
public:
	std::vector<StaticStreamGraph*> staticChildGraph;	//静态子图列表
	std::vector<SchedulerSSG*> scheStaticChildGraph;	//调度后子图列表
	std::vector<int> maxUseCoreNumList;		//每个子图使用的核数，未压缩之前
	std::vector<CombineState> combineList;//该成员存储对应子图是否被combine，其下标与scheStaticChildGraph与staticChildGraph相同
	std::vector<std::pair<SchedulerSSG*, int> >ssg2coreNum;//默认是8，压缩后变为重新分配的核数，这是一个子图与使用核数的映射
	std::map<FlatNode*, int> flatNode2graphindex;		//算子与子图编号的映射
	std::map<FlatNode*, ActorPosition> flatNode2position;		//一个算子与位置的映射
	//目前combine的机制是从第一个子图开始
	//如果核为8，跳过设置为0
	int graphCount;			//子图数量
	int lastGraph;			//最后一个子图
public:
	DividedStaticGraph(StaticStreamGraph*ssg);			//构造函数，划分子图
	void setCount(int);		//X
	int getCount();				//X获得子图数量
	int getlastGraph();		//X获得最后一个子图下标
	bool isLastGraph(int);		//X判断是否是最后一个子图
	void updateSsg2Core(SchedulerSSG*graph, int coreNum);
	std::vector<FlatNode*> getAllFlatNodes();			//X返回DSG图中全部节点
	void allocatePosition();						//分配算子位置,即算子是在FIRST,LAST,MIDDLE还是BOTH
	ActorPosition getActorPosition(FlatNode*);		//判断算子位置函数
	bool isSink(FlatNode*);			//判断位置为LAST的结点是否是最后的sink结点
	bool isSource(FlatNode*);		//判断位置为FIRST的结点是否是source结点

	int getSteadyCount(FlatNode*);		//该函数根据一个actor查找对应SSSG图并获得其稳态调度次数

	int getNextGraphFirstNodePop(FlatNode*);	//获取下一个子图的起始actor的需求数据量，这个数据量是包含稳态次数乘积的
	int getThisNodePop(FlatNode*);		//获取本算子的数据需求量，这个数据量是包含稳态次数乘积的
		
	int getGraphIndexByFlatNode(FlatNode*);		//根据flatnode获取其子图编号
};


GLOBAL void SchedulingSSG(DividedStaticGraph*dsg);
GLOBAL extern DividedStaticGraph *DSG;

GLOBAL void GenerateWorkEst(DividedStaticGraph*dsg, bool WorkEstimateByDataFlow);
GLOBAL void ReviseDSGFunction(DividedStaticGraph*dsg);

#endif