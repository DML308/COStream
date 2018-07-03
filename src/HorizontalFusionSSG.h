#ifndef HORIZONTIAL_FUSION_SSG_H
#define HORIZONTIAL_FUSION_SSG_H

#include "schedulerSSG.h"
#include "MetisPartiton.h"
#include <iostream>
#include <vector>

using namespace std;

class HorizontalFusionSSG
{
public:
	HorizontalFusionSSG(SchedulerSSG *sssg,int clusterNum);//构造函数
	void selectFusingSSG();//对整个ssg做融合
	~HorizontalFusionSSG();//析构函数
	SchedulerSSG *hfsssg;
private:

	vector<FlatNode *> TopoSortFilter();//+对SDF图中的所有节点做一个拓扑排序
	void collectFusionFlatNodesInfo(vector<FlatNode *>);
	void addCandidateFusionFlatNodes(vector<vector<FlatNode *> >flatNodes);//从sssg图中找能够被融合的节点，最终结果放在prority2candidateFlatNodes中


	Bool detectHorizontalFusionEligible(FlatNode *flatNode);//检测一个flatNode节点能否被融合

	void initMetisPartitionParamiter(vector<FlatNode *>);//根据融合的结果设定metis划分所需的参数
	void updateMetisPartitionParamiter();//根据融合的结果更新metis划分的参数
	void backupPartitionInfo();//在更新有关参数的信息前先做备份

	void partitionSSG();//为了确定融合的度对SSG做划分（本程序采用metis划分）

	int findMaxPartitionWeight();//找划分结果中权值（工作量）最大的划分的工作量



	void selectFusionFlatNodes(int num);//选择将要被融合的节点
	//void updateCandidateFusionFlatNodes();//更新候选集
	void fusingFlatNodes(vector<FlatNode *> flatNodes);//将flatNotes中的节点做水平融合
	void undoSelectFusionFlatNode();//撤销融合操作

	//针对待节点的不同，做不同的融合操作
	void fusingFlatNodesCommon(vector<FlatNode *> flatNodes);
	void fusingFlatNodesJoin(FlatNode *joinflatNode,vector<FlatNode *> flatNodes);
	void fusingFlatNodesSplit(FlatNode *splitflatNode,vector<FlatNode *> flatNodes);
	void fusingFlatNodesSplitJoin(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);
	vector<operatorNode *> fusionFlatNodesSplitJoin_FSFJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);//_ Fuse Split Fuse Join
	vector<operatorNode *> fusionFlatNodesSplitJoin_MSFJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);//_ Modify Split Fuse Join
	vector<operatorNode *> fusionFlatNodesSplitJoin_FSMJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);//_ Fuse Split Modify Join
	vector<operatorNode *> fusionFlatNodesSplitJoin_MSMJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);//_ Modify Split Modify Join

	Node *makeHFusionJoin(List *joinInputList, List *joinPopArg, Node *joinOutputNode, Node *joinPushArg);//构造join operator
	Node *makeHFusionSplit(Node *splitInputNode,Node *splitPopArg,List *splitOutputList, List *splitPushArg, SplitStyle style);//构造split operator

	//注：做window的函数可以统一成一个函数
	Node *MakeWindowNode(Node *id,Node *decl, Node *count, int style);
	List *makeHFusionJoinWindow(List *inputList,List *outputList,List *pop_arg, Node *pushArg);//构造join的window
	List *makeHFusionRoundRobinWindow(List *inputList,List *outputList,List *push_arg, Node *popArg);
	List *makeHFusionDuplicateWindow(List *inputList,List *outputList,Node *pop_value);
	List *makeHFusionSInSOutWindow(Node *inputNode,Node *outputNode,int popValue,int peekValue,int pushValue);

	//做节点间的融合
	Node *fusingNodes(vector<FlatNode *>flatNodes,Node *inputNode,Node *outputNode);
	Node *fusingOperators(vector<operatorNode *>operNodes, int popvalue,int pushvalue);//将转换后的operator节点融合到一起
	void commonOperatorTransform(operatorNode *operNode,int popvalue, int pushvalue,Node *inputStream,Node *output,int popOffset,int pushOffset,int steadyCount,int index);//修改普通的operator的内容（输入输出边，以及构造最小稳定的迭代循环，修改数据访问的偏移值）
	void MWIOS_astwalk(Node *n,Node *oldInputDecl,Node *oldOutputDecl,Node *newInputDecl,Node *newOutputDecl,Node *iterNode,int pushvalue,int popvalue,int pushOffset,int popOffset);
	void MWIOS_List(List *l,Node *oldInputDecl,Node *oldOutputDecl,Node *newInputDecl,Node *newOutputDecl,Node *iterNode,int pushvalue,int popvalue,int pushOffset,int popOffset);
	vector<FlatNode *> replaceHfusionFlatNodes(vector<FlatNode *>oldFlatNodes,vector<operatorNode *>newNodes);
	void InsertExternalOutputStreamToSSG(FlatNode *flatNode);

	Node *reorderFusionJoinNode(Node *operNode,FlatNode *joinFlatNode,vector<FlatNode *> flatNodes);//根据flatNodes与join的关系，将根据flatNodes融合得到的operNode的输出边的数据进行重排序
	Node *reorderFusionSplitNode(Node *operNode,FlatNode *splitFlatNode,vector<FlatNode *> flatNodes);//根据flatNodes与split的关系，将根据flatNodes融合得到的operNode的输出边的数据进行重排序

private:
	int partitionNum;//最终SDF图要被划分的份数
	map<FlatNode *,int> _sjflatNode2smallSteadyCount;//记录节点的最小稳定状态次数
	multimap<int ,vector<FlatNode *> > priority2candidateFlatNodes;//优先级与候选节点集的map（优先级考虑的因素主要是splitjoin的嵌套关系，以及节点的工作量）
	map<FlatNode *, int >FlatNode2No;//flatNode与节点编号之间的map
	vector<vector<FlatNode *> > fusingNo2FlatNodes;//节点融合的次序与该编号下需要被融合节点的编号
	map<int, vector<FlatNode *> > curFusingNo2FlatNodes;//用于临时存储的新的融合节点以及编号，在撤销融合是需要用到
	map<int,int>flatNodeOldNo2NewNo;//（为了方便更新融合节点）flatNode节点的旧编号与新编号之间的map
	multimap<int,int>flatNodeNewNo2OldNo;//（为了方便撤销融合操作）flatNode节点的新编号与旧编号之间的map
	//++++++++++++++++++++++metis 以及撤销融合用到的一些信息++++++++++++
	//使用metis要用到的一些参数
	int edgenum;//图中边的数目
	int nvtxs;//图的当前节点的数目
	int *xadj;
	int *adjncy;//xadj,adjncy存放节点的依赖关系
	int *vwgt;//节点的工作量(稳态次数*work函数的工作量)
	int *vsize;//节点的通信数据量
	int *mpart;//划分的结果放在mpart中

	//备份metis用到的一些参数
	int bak_nvtxs;//图的顶点数目
	int bak_edgenum;//图中边的数目
	int *bak_xadj;
	int *bak_adjncy;//xadj,adjncy存放节点的依赖关系
	int *bak_vwgt;//节点的工作量(稳态次数*work函数的工作量)
	int *bak_vsize;//节点的通信数据量
	map<int,int>bak_flatNodeOldNo2NewNo;//备份节点的编号
	map<FlatNode *, int >bak_FlatNode2No;
};

GLOBAL SchedulerSSG *HorizontalFusionTransform(SchedulerSSG *sssg, int clusterNum );

#endif
