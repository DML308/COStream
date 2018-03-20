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
	HorizontalFusionSSG(SchedulerSSG *sssg,int clusterNum);//���캯��
	void selectFusingSSG();//������ssg���ں�
	~HorizontalFusionSSG();//��������
	SchedulerSSG *hfsssg;
private:

	vector<FlatNode *> TopoSortFilter();//+��SDFͼ�е����нڵ���һ����������
	void collectFusionFlatNodesInfo(vector<FlatNode *>);
	void addCandidateFusionFlatNodes(vector<vector<FlatNode *> >flatNodes);//��sssgͼ�����ܹ����ںϵĽڵ㣬���ս������prority2candidateFlatNodes��


	Bool detectHorizontalFusionEligible(FlatNode *flatNode);//���һ��flatNode�ڵ��ܷ��ں�

	void initMetisPartitionParamiter(vector<FlatNode *>);//�����ںϵĽ���趨metis��������Ĳ���
	void updateMetisPartitionParamiter();//�����ںϵĽ������metis���ֵĲ���
	void backupPartitionInfo();//�ڸ����йز�������Ϣǰ��������

	void partitionSSG();//Ϊ��ȷ���ںϵĶȶ�SSG�����֣����������metis���֣�

	int findMaxPartitionWeight();//�һ��ֽ����Ȩֵ�������������Ļ��ֵĹ�����



	void selectFusionFlatNodes(int num);//ѡ��Ҫ���ںϵĽڵ�
	//void updateCandidateFusionFlatNodes();//���º�ѡ��
	void fusingFlatNodes(vector<FlatNode *> flatNodes);//��flatNotes�еĽڵ���ˮƽ�ں�
	void undoSelectFusionFlatNode();//�����ںϲ���

	//��Դ��ڵ�Ĳ�ͬ������ͬ���ںϲ���
	void fusingFlatNodesCommon(vector<FlatNode *> flatNodes);
	void fusingFlatNodesJoin(FlatNode *joinflatNode,vector<FlatNode *> flatNodes);
	void fusingFlatNodesSplit(FlatNode *splitflatNode,vector<FlatNode *> flatNodes);
	void fusingFlatNodesSplitJoin(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);
	vector<operatorNode *> fusionFlatNodesSplitJoin_FSFJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);//_ Fuse Split Fuse Join
	vector<operatorNode *> fusionFlatNodesSplitJoin_MSFJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);//_ Modify Split Fuse Join
	vector<operatorNode *> fusionFlatNodesSplitJoin_FSMJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);//_ Fuse Split Modify Join
	vector<operatorNode *> fusionFlatNodesSplitJoin_MSMJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes);//_ Modify Split Modify Join

	Node *makeHFusionJoin(List *joinInputList, List *joinPopArg, Node *joinOutputNode, Node *joinPushArg);//����join operator
	Node *makeHFusionSplit(Node *splitInputNode,Node *splitPopArg,List *splitOutputList, List *splitPushArg, SplitStyle style);//����split operator

	//ע����window�ĺ�������ͳһ��һ������
	Node *HorizontalFusionSSG::MakeWindowNode(Node *id,Node *decl, Node *count, int style);
	List *makeHFusionJoinWindow(List *inputList,List *outputList,List *pop_arg, Node *pushArg);//����join��window
	List *makeHFusionRoundRobinWindow(List *inputList,List *outputList,List *push_arg, Node *popArg);
	List *makeHFusionDuplicateWindow(List *inputList,List *outputList,Node *pop_value);
	List *makeHFusionSInSOutWindow(Node *inputNode,Node *outputNode,int popValue,int peekValue,int pushValue);

	//���ڵ����ں�
	Node *fusingNodes(vector<FlatNode *>flatNodes,Node *inputNode,Node *outputNode);
	Node *fusingOperators(vector<operatorNode *>operNodes, int popvalue,int pushvalue);//��ת�����operator�ڵ��ںϵ�һ��
	void commonOperatorTransform(operatorNode *operNode,int popvalue, int pushvalue,Node *inputStream,Node *output,int popOffset,int pushOffset,int steadyCount,int index);//�޸���ͨ��operator�����ݣ���������ߣ��Լ�������С�ȶ��ĵ���ѭ�����޸����ݷ��ʵ�ƫ��ֵ��
	void MWIOS_astwalk(Node *n,Node *oldInputDecl,Node *oldOutputDecl,Node *newInputDecl,Node *newOutputDecl,Node *iterNode,int pushvalue,int popvalue,int pushOffset,int popOffset);
	void MWIOS_List(List *l,Node *oldInputDecl,Node *oldOutputDecl,Node *newInputDecl,Node *newOutputDecl,Node *iterNode,int pushvalue,int popvalue,int pushOffset,int popOffset);
	vector<FlatNode *> replaceHfusionFlatNodes(vector<FlatNode *>oldFlatNodes,vector<operatorNode *>newNodes);
	void InsertExternalOutputStreamToSSG(FlatNode *flatNode);

	Node *reorderFusionJoinNode(Node *operNode,FlatNode *joinFlatNode,vector<FlatNode *> flatNodes);//����flatNodes��join�Ĺ�ϵ��������flatNodes�ںϵõ���operNode������ߵ����ݽ���������
	Node *reorderFusionSplitNode(Node *operNode,FlatNode *splitFlatNode,vector<FlatNode *> flatNodes);//����flatNodes��split�Ĺ�ϵ��������flatNodes�ںϵõ���operNode������ߵ����ݽ���������

private:
	int partitionNum;//����SDFͼҪ�����ֵķ���
	map<FlatNode *,int> _sjflatNode2smallSteadyCount;//��¼�ڵ����С�ȶ�״̬����
	multimap<int ,vector<FlatNode *> > priority2candidateFlatNodes;//���ȼ����ѡ�ڵ㼯��map�����ȼ����ǵ�������Ҫ��splitjoin��Ƕ�׹�ϵ���Լ��ڵ�Ĺ�������
	map<FlatNode *, int >FlatNode2No;//flatNode��ڵ���֮���map
	vector<vector<FlatNode *> > fusingNo2FlatNodes;//�ڵ��ںϵĴ�����ñ������Ҫ���ںϽڵ�ı��
	map<int, vector<FlatNode *> > curFusingNo2FlatNodes;//������ʱ�洢���µ��ںϽڵ��Լ���ţ��ڳ����ں�����Ҫ�õ�
	map<int,int>flatNodeOldNo2NewNo;//��Ϊ�˷�������ںϽڵ㣩flatNode�ڵ�ľɱ�����±��֮���map
	multimap<int,int>flatNodeNewNo2OldNo;//��Ϊ�˷��㳷���ںϲ�����flatNode�ڵ���±����ɱ��֮���map
	//++++++++++++++++++++++metis �Լ������ں��õ���һЩ��Ϣ++++++++++++
	//ʹ��metisҪ�õ���һЩ����
	int edgenum;//ͼ�бߵ���Ŀ
	int nvtxs;//ͼ�ĵ�ǰ�ڵ����Ŀ
	int *xadj;
	int *adjncy;//xadj,adjncy��Žڵ��������ϵ
	int *vwgt;//�ڵ�Ĺ�����(��̬����*work�����Ĺ�����)
	int *vsize;//�ڵ��ͨ��������
	int *mpart;//���ֵĽ������mpart��

	//����metis�õ���һЩ����
	int bak_nvtxs;//ͼ�Ķ�����Ŀ
	int bak_edgenum;//ͼ�бߵ���Ŀ
	int *bak_xadj;
	int *bak_adjncy;//xadj,adjncy��Žڵ��������ϵ
	int *bak_vwgt;//�ڵ�Ĺ�����(��̬����*work�����Ĺ�����)
	int *bak_vsize;//�ڵ��ͨ��������
	map<int,int>bak_flatNodeOldNo2NewNo;//���ݽڵ�ı��
	map<FlatNode *, int >bak_FlatNode2No;
};

GLOBAL SchedulerSSG *HorizontalFusionTransform(SchedulerSSG *sssg, int clusterNum );

#endif