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
		FlatNode *fissingFlatNode;//��Ҫ�ֵĵĽڵ�
		int fissedSteadycount;//���Ѻ�ǰ������ִ�д���
		int operatorWeight;//ԭʼ��operator��һ����ִ̬���ǵĹ�����
		vector<int> fissed_peek;//���Ѻ�ڵ���һ����̬����ִ���е�peek����ֵ
		vector<int> fissed_pop; //���Ѻ�ڵ���һ����̬����ִ���е�pop����ֵ
		vector<int> fissed_push; //���Ѻ�ڵ���һ����̬����ִ���е�push����ֵ
		int npart;             //��¼��ǰ��fissionNode���ڵĻ��ֱ��
		Bool fiss_state; //��ǰ�ڵ���ѵ�״̬����Ҫ�������Ƿ���ִ̬�еĴ����Ƶ�work�ڣ�ʹ�ø�����work��һ����̬����ִ����ֻ����һ��(Falst),TURE��ʾ��ֹһ�Σ�
	}FissionNodeInfo;

	Partition *partition;  // �����ʼ�Ļ���
	float balanceFactor; //ƽ������
	multimap<int ,FissionNodeInfo *>PartitonNum2FissionNode;//���ֵı��������м�ڵ�֮���ӳ��
	multimap<FlatNode* ,FissionNodeInfo *>FlatNode2FissionNode;//�����ѵĽڵ�������м�ڵ�֮���ӳ��(ֻ��ű����ѵĽڵ㣬��������޸�SDFͼ)

public:
	/*���캯��*/
	HorizontalFission(Partition *p,float balane);

	/*��ʼ�����Ա�е�PartitonNum2FissionNode��FlatNode2FissionNode*/
	void initFissionNodeMap(SchedulerSSG *sssg);
	/*������ͬһ�����е�FissionNodeInfo�ڵ�*/
	vector<FissionNodeInfo *> findfissionNodeSetInPartition(int partitionNum);


	/*�ж�actor�Ƿ���stateful*/
	void hasMutableState(operatorNode *opNode);	
	/*�ж�һ�������Ƿ���mutable����*/
	void IsMutableVar(List *list,Node *node);
	/*���ڱ���work�����ڵ���Ϣ*/
	void MS_astwalk(Node *n,List *list);  
	void MS_listwalk(List *l,List *list);
	/*���뵥��*/
	Bool SInSOutOPerator(operatorNode *oper);

	/*����ÿһ����(һ������)�Ĺ��������ܺͣ������Ǻ˵ı��*/
	int computeSumWeightofPlace(int npart);
	/*�õ����ֺ�Ȩ�ص�ӳ��*/
	map<int ,int> computeMapPlace2Weight();
	/*���˵Ĺ������������򣬷��ص��Ǻ˱����ɵ�����*/
	vector<int> SortPartitionsByWeight(map<int,int>partition2weight);
	/*��Ȩֵ��С�Ļ��֣����ص��ǻ��ֱ��*/
	int MinPartitionWeight(map<int,int>partition2weight);
	/*�ҵ�ǰȨֵ���Ļ��֣����ص��ǻ��ֱ��*/
	int NextMaxWeightPartition(map<int,int>partition2weight,vector<Bool> PartiotionFlag); 

	/*��flatNode����α���ѣ���Ҫ����PartitonNum2FissionNode��FlatNode2FissionNode�н��������Ϣ���޸�*/
	void SplitFissionNode(FissionNodeInfo *,int ,int ,int );

	/*�޸�peek��Ϣ*/
	void MWISD_List(List *l,List *inputlist,List *outputlist,Node *inputDelt,Node *outputDelt);
	void MWISD_astwalk(Node *n,List *inputlist,List *outputlist,Node *inputDelt,Node *outputDelt);
	List *ModifyWorkInOutStreamDim(Node *node,List *inputList,List *outputList,Node* inputdelt,Node* outputdelt);
	List *ModifyWindow(List *inputList,List *outputList,int popValue,int peekValue,int pushValue);
	Node *ModifyCompositeName(Node *composite);

	/*��operator�ڵ�ת����composite�ڵ�*/
	List *MakeReplicateOper(vector<FissionNodeInfo *> splitedNodeVec,FlatNode *flatnode, List *inputList, int steadyCount,SplitStyle style);
	Node *MakeCompositeNode(Node *operNode);

	Node *MakeFissionSplitComposite(Node *inputNode,List *outputList,  List *pop_arg,SplitStyle style);
	Node *MakeFissionJoinComposite(List *inputList,Node *outputNode, List *pop_arg);

	List *MakeDuplicateWindow(List *inputList,List *outputList,Node *pop_value);
	List *MakeRoundRobinWindow(List *inputList,List *outputList,List *pop_arg);
	List *MakeJoinWindow(List *inputList,List *outputList,List *pop_arg);
	/*����split�������*/
	List *MakeSplitOutputStream(Node *input,int count);
	/*����split�ڵ�,��������������ߣ�һ����������ߣ���������ÿһ������ÿһ�ڵ����߷������ݵ���Ŀ*/
	Node *MakeFissionDuplicate(Node *inputNode,List *outputList, Node *pop_value);
	Node *MakeFissionRoundRobin(Node *inputNode,List *outputList, List *pop_arg);
	/*����join�ڵ�,��������������ߣ�������������ߣ�һ������ÿһ������ÿһ���߷������ݵ���Ŀ*/
	Node *MakeFissionJoin(List *inputlist,Node *outputNode, List *pop_arg);

	/*����composite*/
	Node *ConstructNewComposite(Node *node,List *inputList,List *outputList,int count,int popValue,int peekValue,int pushValue,int oldpopValue,int oldpeekValue,int oldpushValue,int deltpeek);

	/*��һ��flatnode���з��ѣ������Ǵ����ѵĽڵ㣬�Լ����Ѵ�����������Ѻ�ڵ�flatnode��ɵ�vector��*/
	List *horizontalFissOperator(vector<FissionNodeInfo *> splitedNodeVec,SchedulerSSG *sssg);

	/*�޸��Է��ѵ�flatnode����Ϣ*/
	void Operator2FlatNodeModifyInfo(FlatNode *curFlatNode, List *childOperList,SchedulerSSG *sssg, vector<int >npartVec);
	void InsertJoinOutStreamToSSG(Node *streamNode,FlatNode *topFlatNode,SchedulerSSG *sssg);
	void InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,FlatNode *oldFlatNode,SchedulerSSG *sssg);

	/*�����㷨*/
	void HorizontalFissionPRPartitions(SchedulerSSG *sssg,float balanceFactor);
	/*��������*/
	~HorizontalFission(){};
};

#endif /* _FLAT_NODE_H_ */