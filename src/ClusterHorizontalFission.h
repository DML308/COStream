#ifndef  _CLUSTER_HORIZONTAL_FISSION_H_
#define	 _CLUSTER_HORIZONTAL_FISSION_H_

#include "schedulerSSG.h"
#include "ast.h"
#include <vector>
#include <map>
#include <set>
using namespace std;
class ClusterHorizontialFission
{//只考虑分裂单入单出的节点
private:
	typedef struct {
		FlatNode *fissingFlatNode;//将要分的的节点
		Node *actualOperatorNode;//复制分裂后对应的operator
		int fissedSteadycount;//分裂后当前副本的执行次数
		int operatorWeight;//原始的operator在一次稳态执行是的工作量（fiss_state的状态为false则该值为1）
		vector<int> fissed_peek;//分裂后节点执行一次peek值（fiss_state的状态为false则该值就是一次稳态的值）
		vector<int> fissed_pop; //分裂后节点执行一次pop值（同上）
		vector<int> fissed_push; //分裂后节点执行一次push值（同上）
		int npart;             //记录当前的fissionNode所在的划分编号
	//	SplitStyle style;//记录副本产生的方式（初始值是的设定，如果其上端节点是上split并且是roudrobin类型，则该值是roundrobin,否则是duplicate类型，
							 /*对于副本值的设定要考虑到peek与pop的关系,如果一个副本自身的style是S_Duplicate，而其上端是S_RoundRobin那么就要引入一个额外的类型是duplicate的split节点）*/
		Bool fiss_state; //当前节点分裂的状态（主要体现在是否将稳态执行的次数移到work内，使该副本的work在一次稳态周期执行中只运行一次(False),TURE表示不止一次）
	}FissionNodeInfo;

	float balanceFactor; //平衡因子
	multimap<int ,FissionNodeInfo *>PartitonNum2FissionNode;//划分的编号与分裂中间节点之间的映射（确定分裂后的副本放在那个划分中的）
	map<FlatNode* ,set<FissionNodeInfo *> >FlatNode2FissionNode;//待分裂的节点与分裂中间节点之间的映射(只存放被分裂的节点，用于最后修改SDF图)
	
	map<FlatNode *,Bool> _FlatNode2makeUpNodeTag;//用于标识是在上端建造一个flatNode还是修改上端的节点（主要对上端是split节点而言）
	map<FlatNode *,Bool> _FlatNode2makeDownNodeTag;//主要用于标识是在下端建造一个flatNode还是修改下端的节点（主要对上端是join节点而言）
	map<FlatNode *,SplitStyle>_FlatNode2splitStyle;//确定节点的分裂方式
	map<Node *,int> newOperNode2partitionNum;//分裂产生的新的operator Node对应的划分编号
	map<Node *,int> newOperNode2steadycount;//记录所有新产生的operator的稳定状态次数

	map<FlatNode *,int> _flatNode2partitionNum;//flatNode与划分编号之间的map
	map<FlatNode *,int> _flatNode2steadycount;//flatNode与当前稳定状态的执行次数的map
	
	

	SchedulerSSG *sssg;
	int npartition;//划分的份数

public:
	ClusterHorizontialFission(map<FlatNode *,int>flatNode2partitionNum,map<FlatNode *,int> flatNode2steadycount,SchedulerSSG *tmpsssg, int np,float bf);//构造函数
	~ClusterHorizontialFission(){};//析构函数
	inline map<FlatNode *,int>GetFlatNode2partitionNum()
	{//返回分裂后flatNode与划分编号之间的map
		return _flatNode2partitionNum;
	}
	inline map<FlatNode *,int>GetFlatNode2steadycount()
	{//返回分裂后flatNode与稳定执行次数之间的map
		return _flatNode2steadycount;
	}

	float fakementHorizontalFission();//做伪分裂，返回划分完成后图的平衡状态
	void HorizontalFissionPartitions();//水平分裂
	
private:
	/*初始化类成员中的PartitonNum2FissionNode和FlatNode2FissionNode*/
	void initFissionNodeMap();
	map<int,int> computeMapPlace2Weight();//计算各个划分的权值
	/*找当前权值最大的划分，返回的是划分编号*/
	int NextMaxWeightPartition(map<int,int>partition2weight,vector<Bool> PartiotionFlag); 
	vector<int>SortPartitionbyWeight(map<int,int>partition2weight);//对划分按工作量从大到小排序
	vector<FissionNodeInfo *> findFissionflatNodesInPartition(int npart);//取当前划分中可被分裂的所有flatNode节点
	Bool SInSOutOPerator(FlatNode *operFlatNode);
	void SplitFissionNode(FissionNodeInfo *fissionNode,int replicationFactor,int max_partiotion,int min_partiotion);
	void ReplicationFissing();//根据FlatNode2FissionNode中的信息进行实际的分裂
	void modifyFissionNodeInfo();//根据各个flatNode副本的信息确定pop，peek以及push的值
	/*对一个flatnode进行分裂，输入分裂的信息，输出新生成的Node节点*/
	void horizontalFissOperator(FlatNode* flatNode, set<FissionNodeInfo *> &splitedNodeVec,SchedulerSSG *sssg);
	Node *MakeCopyOperatorNode_Duplicate(FissionNodeInfo *copyNodeinfo, Node *inputNode,Node *outputNode,int oldpopValue, int oldpeekValue,int oldpushValue,int deltpeek);//制造副本operator节点，同时替换完输入输出边，以及peek的偏移
	Node *MakeCopyOperatorNode_RoundRobin(FissionNodeInfo *copyNodeinfo, Node *inputNode,Node *outputNode,int oldpopValue, int oldpeekValue,int oldpushValue,int deltpeek);//制造副本operator节点，同时替换完输入输出边，以及peek的偏移
	
	List *ModifyWindow(List *inputList,List *outputList,int popValue,int peekValue,int pushValue);//根据新的窗口大小构造window

	void MWISD_astwalk(Node *n,Node *old_inputDecl,Node *old_outputDecl,Node *inputId,Node *outputId,Node *inputDelt,Node *outputDelt);
	void MWISD_List(List *l,Node *old_inputDecl,Node *old_outputDecl,Node *inputId,Node *outputId,Node *inputDelt,Node *outputDelt);

	void modifySplitFlatNode(map<FlatNode*,set<FlatNode *> > upflatNode2flatNodes,map<FlatNode *,vector<FlatNode *> > flatNode2FissionOperatorNode);
	void modifyJoinFlatNode(map<FlatNode*,set<FlatNode *> > upflatNode2flatNodes,map<FlatNode *,vector<FlatNode *> > flatNode2FissionOperatorNode);
	map<FlatNode *,vector<Node *> > makeSplitJoinExtractOperators();
	map<FlatNode *,vector<FlatNode *> > transOperatorNodetoFlatNode(map<FlatNode *,vector<Node *> > flatNode2FissionOperatorNode);

	Node *makeHFissionSplit(Node *splitInputNode,Node *splitPopArg,List *splitOutputList, List *splitPushArg, SplitStyle style);
	List *makeHFissionJoinWindow(List *inputList,List *outputList,List *pop_arg, Node *pushArg);
	Node *MakeWindowNode(Node *id,Node *decl, Node *count, int style);
	List *makeHFissionRoundRobinWindow(List *inputList,List *outputList,List *push_arg, Node *popArg);
	List *makeHFissionDuplicateWindow(List *inputList,List *outputList,Node *pop_value);
	Node *makeHFissionJoin(List *joinInputList, List *joinPopArg, Node *joinOutputNode, Node *joinPushArg);

	FlatNode* replaceHfisionFlatNodes(FlatNode *oldFlatNodes,operatorNode *newNodes);

};

GLOBAL map<FlatNode *,int> ClusterHFissing(map<FlatNode *,int>flatNode2partitionNum,SchedulerSSG *sssg, int _nplace, float bf);



#endif