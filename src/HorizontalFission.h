#ifndef  _HORIZONTAL_FISSION_H_
#define	 _HORIZONTAL_FISSION_H_

#include "MetisPartiton.h"
#include "Partition.h"
#include "schedulerSSG.h"
#include "ast.h"
#include <vector>
#include <map>
using namespace std;


class HorizontalFission
{
private:
	typedef struct {
		FlatNode *fissingFlatNode;//将要分的的节点
		int fissedSteadycount;//分裂后当前副本的执行次数
		int operatorWeight;//原始的operator在一次稳态执行是的工作量
		vector<int> fissed_peek;//分裂后节点在一个稳态周期执行中的peek的总值
		vector<int> fissed_pop; //分裂后节点在一个稳态周期执行中的pop的总值
		vector<int> fissed_push; //分裂后节点在一个稳态周期执行中的push的总值
		int npart;             //记录当前的fissionNode所在的划分编号
		Bool fiss_state; //当前节点分裂的状态（主要体现在是否将稳态执行的次数移到work内，使该副本的work在一次稳态周期执行中只运行一次(Falst),TURE表示不止一次）
	}FissionNodeInfo;

	Partition *partition;  // 保存初始的划分
	float balanceFactor; //平衡因子
	multimap<int ,FissionNodeInfo *>PartitonNum2FissionNode;//划分的编号与分裂中间节点之间的映射
	multimap<FlatNode* ,FissionNodeInfo *>FlatNode2FissionNode;//待分裂的节点与分裂中间节点之间的映射(只存放被分裂的节点，用于最后修改SDF图)

public:
	/*构造函数*/
	HorizontalFission(Partition *p,float balane);

	/*初始化类成员中的PartitonNum2FissionNode和FlatNode2FissionNode*/
	void initFissionNodeMap(SchedulerSSG *sssg);
	/*查找在同一划分中的FissionNodeInfo节点*/
	vector<FissionNodeInfo *> findfissionNodeSetInPartition(int partitionNum);


	/*判断actor是否是stateful*/
	void hasMutableState(operatorNode *opNode);	
	/*判断一个变量是否是mutable变量*/
	void IsMutableVar(List *list,Node *node);
	/*用于遍历work函数内的信息*/
	void MS_astwalk(Node *n,List *list);  
	void MS_listwalk(List *l,List *list);
	/*单入单出*/
	Bool SInSOutOPerator(operatorNode *oper);

	/*计算每一个核(一个划分)的工作量的总和，参数是核的编号*/
	int computeSumWeightofPlace(int npart);
	/*得到划分和权重的映射*/
	map<int ,int> computeMapPlace2Weight();
	/*按核的工作量进行排序，返回的是核编号组成的数组*/
	vector<int> SortPartitionsByWeight(map<int,int>partition2weight);
	/*找权值最小的划分，返回的是划分编号*/
	int MinPartitionWeight(map<int,int>partition2weight);
	/*找当前权值最大的划分，返回的是划分编号*/
	int NextMaxWeightPartition(map<int,int>partition2weight,vector<Bool> PartiotionFlag); 

	/*将flatNode进行伪分裂，主要是在PartitonNum2FissionNode和FlatNode2FissionNode中进行相关信息的修改*/
	void SplitFissionNode(FissionNodeInfo *,int ,int ,int );

	/*修改peek信息*/
	void MWISD_List(List *l,List *inputlist,List *outputlist,Node *inputDelt,Node *outputDelt);
	void MWISD_astwalk(Node *n,List *inputlist,List *outputlist,Node *inputDelt,Node *outputDelt);
	List *ModifyWorkInOutStreamDim(Node *node,List *inputList,List *outputList,Node* inputdelt,Node* outputdelt);
	List *ModifyWindow(List *inputList,List *outputList,int popValue,int peekValue,int pushValue);
	Node *ModifyCompositeName(Node *composite);

	/*将operator节点转换成composite节点*/
	List *MakeReplicateOper(vector<FissionNodeInfo *> splitedNodeVec,FlatNode *flatnode, List *inputList, int steadyCount,SplitStyle style);
	Node *MakeCompositeNode(Node *operNode);

	Node *MakeFissionSplitComposite(Node *inputNode,List *outputList,  List *pop_arg,SplitStyle style);
	Node *MakeFissionJoinComposite(List *inputList,Node *outputNode, List *pop_arg);

	List *MakeDuplicateWindow(List *inputList,List *outputList,Node *pop_value);
	List *MakeRoundRobinWindow(List *inputList,List *outputList,List *pop_arg);
	List *MakeJoinWindow(List *inputList,List *outputList,List *pop_arg);
	/*构造split的输出边*/
	List *MakeSplitOutputStream(Node *input,int count);
	/*构造split节点,输入内容是输入边（一条），输出边（多条），每一条边向每一节点条边发送数据的数目*/
	Node *MakeFissionDuplicate(Node *inputNode,List *outputList, Node *pop_value);
	Node *MakeFissionRoundRobin(Node *inputNode,List *outputList, List *pop_arg);
	/*构造join节点,输入内容是输入边（多条），输出边（一条），每一条边向每一条边发送数据的数目*/
	Node *MakeFissionJoin(List *inputlist,Node *outputNode, List *pop_arg);

	/*构造composite*/
	Node *ConstructNewComposite(Node *node,List *inputList,List *outputList,int count,int popValue,int peekValue,int pushValue,int oldpopValue,int oldpeekValue,int oldpushValue,int deltpeek);

	/*对一个flatnode进行分裂，输入是待分裂的节点，以及分裂次数，输出分裂后节点flatnode组成的vector；*/
	List *horizontalFissOperator(vector<FissionNodeInfo *> splitedNodeVec,SchedulerSSG *sssg);

	/*修改以分裂的flatnode的信息*/
	void Operator2FlatNodeModifyInfo(FlatNode *curFlatNode, List *childOperList,SchedulerSSG *sssg, vector<int >npartVec);
	void InsertJoinOutStreamToSSG(Node *streamNode,FlatNode *topFlatNode,SchedulerSSG *sssg);
	void InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,FlatNode *oldFlatNode,SchedulerSSG *sssg);

	/*分裂算法*/
	void HorizontalFissionPRPartitions(SchedulerSSG *sssg,float balanceFactor);
	/*析构函数*/
	~HorizontalFission(){};
};

#endif /* _FLAT_NODE_H_ */