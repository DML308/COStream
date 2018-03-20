#ifndef  _CLUSTER_HORIZONTAL_FISSION_H_
#define	 _CLUSTER_HORIZONTAL_FISSION_H_

#include "../schedulerSSG.h"
#include "../ast.h"
#include <vector>
#include <map>
#include <set>
using namespace std;
class ClusterHorizontialFission
{//ֻ���Ƿ��ѵ��뵥���Ľڵ�
private:
	typedef struct {
		FlatNode *fissingFlatNode;//��Ҫ�ֵĵĽڵ�
		Node *actualOperatorNode;//���Ʒ��Ѻ��Ӧ��operator
		int fissedSteadycount;//���Ѻ�ǰ������ִ�д���
		int operatorWeight;//ԭʼ��operator��һ����ִ̬���ǵĹ�������fiss_state��״̬Ϊfalse���ֵΪ1��
		vector<int> fissed_peek;//���Ѻ�ڵ�ִ��һ��peekֵ��fiss_state��״̬Ϊfalse���ֵ����һ����̬��ֵ��
		vector<int> fissed_pop; //���Ѻ�ڵ�ִ��һ��popֵ��ͬ�ϣ�
		vector<int> fissed_push; //���Ѻ�ڵ�ִ��һ��pushֵ��ͬ�ϣ�
		int npart;             //��¼��ǰ��fissionNode���ڵĻ��ֱ��
	//	SplitStyle style;//��¼���������ķ�ʽ����ʼֵ�ǵ��趨��������϶˽ڵ�����split������roudrobin���ͣ����ֵ��roundrobin,������duplicate���ͣ�
							 /*���ڸ���ֵ���趨Ҫ���ǵ�peek��pop�Ĺ�ϵ,���һ�����������style��S_Duplicate�������϶���S_RoundRobin��ô��Ҫ����һ�������������duplicate��split�ڵ㣩*/
		Bool fiss_state; //��ǰ�ڵ���ѵ�״̬����Ҫ�������Ƿ���ִ̬�еĴ����Ƶ�work�ڣ�ʹ�ø�����work��һ����̬����ִ����ֻ����һ��(False),TURE��ʾ��ֹһ�Σ�
	}FissionNodeInfo;

	float balanceFactor; //ƽ������
	multimap<int ,FissionNodeInfo *>PartitonNum2FissionNode;//���ֵı��������м�ڵ�֮���ӳ�䣨ȷ�����Ѻ�ĸ��������Ǹ������еģ�
	map<FlatNode* ,set<FissionNodeInfo *> >FlatNode2FissionNode;//�����ѵĽڵ�������м�ڵ�֮���ӳ��(ֻ��ű����ѵĽڵ㣬��������޸�SDFͼ)
	
	map<FlatNode *,Bool> _FlatNode2makeUpNodeTag;//���ڱ�ʶ�����϶˽���һ��flatNode�����޸��϶˵Ľڵ㣨��Ҫ���϶���split�ڵ���ԣ�
	map<FlatNode *,Bool> _FlatNode2makeDownNodeTag;//��Ҫ���ڱ�ʶ�����¶˽���һ��flatNode�����޸��¶˵Ľڵ㣨��Ҫ���϶���join�ڵ���ԣ�
	map<FlatNode *,SplitStyle>_FlatNode2splitStyle;//ȷ���ڵ�ķ��ѷ�ʽ
	map<Node *,int> newOperNode2partitionNum;//���Ѳ������µ�operator Node��Ӧ�Ļ��ֱ��
	map<Node *,int> newOperNode2steadycount;//��¼�����²�����operator���ȶ�״̬����

	map<FlatNode *,int> _flatNode2partitionNum;//flatNode�뻮�ֱ��֮���map
	map<FlatNode *,int> _flatNode2steadycount;//flatNode�뵱ǰ�ȶ�״̬��ִ�д�����map
	
	

	SchedulerSSG *sssg;
	int npartition;//���ֵķ���

public:
	ClusterHorizontialFission(map<FlatNode *,int>flatNode2partitionNum,map<FlatNode *,int> flatNode2steadycount,SchedulerSSG *tmpsssg, int np,float bf);//���캯��
	~ClusterHorizontialFission(){};//��������
	inline map<FlatNode *,int>GetFlatNode2partitionNum()
	{//���ط��Ѻ�flatNode�뻮�ֱ��֮���map
		return _flatNode2partitionNum;
	}
	inline map<FlatNode *,int>GetFlatNode2steadycount()
	{//���ط��Ѻ�flatNode���ȶ�ִ�д���֮���map
		return _flatNode2steadycount;
	}

	float fakementHorizontalFission();//��α���ѣ����ػ�����ɺ�ͼ��ƽ��״̬
	void HorizontalFissionPartitions();//ˮƽ����
	
private:
	/*��ʼ�����Ա�е�PartitonNum2FissionNode��FlatNode2FissionNode*/
	void initFissionNodeMap();
	map<int,int> computeMapPlace2Weight();//����������ֵ�Ȩֵ
	/*�ҵ�ǰȨֵ���Ļ��֣����ص��ǻ��ֱ��*/
	int NextMaxWeightPartition(map<int,int>partition2weight,vector<Bool> PartiotionFlag); 
	vector<int>SortPartitionbyWeight(map<int,int>partition2weight);//�Ի��ְ��������Ӵ�С����
	vector<FissionNodeInfo *> findFissionflatNodesInPartition(int npart);//ȡ��ǰ�����пɱ����ѵ�����flatNode�ڵ�
	Bool SInSOutOPerator(FlatNode *operFlatNode);
	void SplitFissionNode(FissionNodeInfo *fissionNode,int replicationFactor,int max_partiotion,int min_partiotion);
	void ReplicationFissing();//����FlatNode2FissionNode�е���Ϣ����ʵ�ʵķ���
	void modifyFissionNodeInfo();//���ݸ���flatNode��������Ϣȷ��pop��peek�Լ�push��ֵ
	/*��һ��flatnode���з��ѣ�������ѵ���Ϣ����������ɵ�Node�ڵ�*/
	void horizontalFissOperator(FlatNode* flatNode, set<FissionNodeInfo *> &splitedNodeVec,SchedulerSSG *sssg);
	Node *MakeCopyOperatorNode_Duplicate(FissionNodeInfo *copyNodeinfo, Node *inputNode,Node *outputNode,int oldpopValue, int oldpeekValue,int oldpushValue,int deltpeek);//���츱��operator�ڵ㣬ͬʱ�滻����������ߣ��Լ�peek��ƫ��
	Node *MakeCopyOperatorNode_RoundRobin(FissionNodeInfo *copyNodeinfo, Node *inputNode,Node *outputNode,int oldpopValue, int oldpeekValue,int oldpushValue,int deltpeek);//���츱��operator�ڵ㣬ͬʱ�滻����������ߣ��Լ�peek��ƫ��
	
	List *ModifyWindow(List *inputList,List *outputList,int popValue,int peekValue,int pushValue);//�����µĴ��ڴ�С����window

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