#ifndef _STATIC_STREAM_GRAPH_H_
#define _STATIC_STREAM_GRAPH_H_

#include "flatNode.h"
#include <set>

extern "C"
{
	extern int NThreads;
	extern GLOBAL Bool X86Backend;
	extern GLOBAL Bool GPUBackend;
	extern GLOBAL int GpuNum;
};

class StaticStreamGraph
{
protected:
	std::string comName;
	FlatNode *topLevel; // SDF图的起始节点，假设只有一个输入为0的节点

	std::vector<FlatNode *> tmpFlatNodes;// SDF图某place上或某thread上的节点集合

public://将访问属性由protected修改成public，以便能对其进行修改
	
	std::vector<FlatNode *> flatNodes;// SDF图所有节点集合
	
	std::map<Node *, FlatNode *>  mapEdge2UpFlatNode; // 将有向边与其上端绑定
	std::multimap<Node *, FlatNode *>  mapEdge2DownFlatNode;//将有向边与其下端绑定
	
	std::map<FlatNode *, int> mapSteadyWork2FlatNode; // 存放各个operator的workestimate（稳态工作量估计）
	std::map<FlatNode *, int> mapInitWork2FlatNode; // 存放各个operator的workestimate（初态）

public:
	StaticStreamGraph()	{	}
	void SetName(const char *);
	inline void AddFlatNode(FlatNode *flatNode)
	{
		flatNodes.push_back(flatNode);
	}
	inline void SetTopLevel()
	{
		assert (flatNodes[0] != NULL && flatNodes[0]->nIn == 0); 
		topLevel = flatNodes[0];
	}
	void PrintFlatNodes(); // 打印SDF图
	void GenerateFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite); // 构建SDF图，初始化flatNodes, mapEdge2FlatNode
	
	FlatNode *GenerateFlatNodes(operatorNode *u);//zww 20120917 添加
	FlatNode * InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,FlatNode *oldFlatNode);
	FlatNode * InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,std::set<FlatNode *> oldFlatNodeSet);//zww 20121011 添加

	void SetFlatNodesWeights(); // 给各节点设置权重
	void SetFlatNodesWeights(FlatNode *flatNode); // 重载，为了使每个FlatNode能够单独设置权重，给各节点设置权重   zww-20120312
	void ResetFlatNodeNames(); // 重置每个flatnode的name值
	bool IsUpBorder(FlatNode *);   //cwb判断父结点是否有边界结点
	bool IsDownBorder(FlatNode *); //cwb判断子结点是否有边界结点
	void GetPreName();   //cwb获取各actor被重命名前的名字
	void ResetFlatNodeVisitTimes(); // 重置每个flastnode的visttimes值
	inline FlatNode *GetTopLevel(void)
	{ return topLevel;}
	void AddSteadyWork(FlatNode *,int); // 存放稳态调度工作量
	void AddInitWork(FlatNode *,int); // 存放初态调度工作量
	int GetSteadyWork(FlatNode *n);
	int GetInitWork(FlatNode *n);
	std::vector<FlatNode *> GetFlatNodes(int place_id); //lxx.2012.03.28
	std::vector<FlatNode *> GetFlatNodes(int place_id, int thread_id); //lxx.2012.03.28
	inline std::string GetName(){ return comName;}
	inline std::vector<FlatNode *> GetFlatNodes(void)
	{ return flatNodes;}
	
	inline std::map<Node *,FlatNode *>  GetMapEdge2UpFlatNode(void)
	{ return mapEdge2UpFlatNode;}
	inline std::multimap<Node *,FlatNode *>  GetMapEdge2DownFlatNode(void)
	{ return mapEdge2DownFlatNode;}
	inline std::map<FlatNode *, int> GetSteadyWorkMap(void)
	{ return mapSteadyWork2FlatNode;}
	inline std::map<FlatNode *, int> GetInitWorkMap(void)
	{ return mapInitWork2FlatNode;}
};

GLOBAL extern StaticStreamGraph *SSG;
GLOBAL StaticStreamGraph *AST2FlatStaticStreamGraph(Node *mainComposite);
GLOBAL void GenerateWorkEst(StaticStreamGraph *ssg,bool WorkEstimateByDataFlow);

#endif