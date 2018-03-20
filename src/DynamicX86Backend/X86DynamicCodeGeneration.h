#ifndef _X86DYNAMIC_H_
#define _X86DYNAMIC_H_

#include "../X86CodeGenerate.h"
#include "../ActorEdgeInfo.h"
#include "DividedStaticGraph.h"
#include "DynamicX86Backend.h"

using namespace std;
class DynamicX86CodeGenerate
{
private:
	//vector<X86CodeGenerate*> inCodeGenerate;
	
	stringstream globalvarbuf, temp_declInitList;//globalvarbuf����ȫ�ֱ���������
	
	bool extractDecl, isInParam;
	stringstream strScheduler, parameterBuf, thisBuf;
	stringstream ExternTypeBuf; //Ƕ��ʽ�еĽӿ�����
	FlatNode* curactor;		//��ǰ���ڽ���/���ɵ�actor
	//Դ�������Ƿ����C�������ö����־
	string dir_;//�����ļ�·��
	int flag;
	//��¼declInitList��ʼ���ȣ������ָ���������ǰ������
	ActorEdgeInfo* pEdgeInfo;//��Ÿ����ߵ�������Ϣ��by lihe 2012-09-04
	std::vector<FlatNode *> flatNodes_;//���нڵ�
	int declInitLen;
	stringstream declInitList_temp;
	vector<string> nDeclDim;//���ڴ��δ�����ȫ������ά��
	vector<string> staticNameInit;	//���actor��state��var��Ա�ĳ�ʼ�����������actor��Ӧ��Դ�ļ���
	map<operatorNode *, std::string> mapOperator2ClassName; // ��Ÿ� class ��Ӧ�� composite����

	
	int nCpucore_;		//cpu����
	int nActors_;		//����flatNode����Ŀ
	vector<map<int, set<int> > > mapNum2StageList;
	multimap<FlatNode*, string> mapActor2InEdge;
	multimap<FlatNode*, string> mapActor2OutEdge;

	map<int, set<int> > mapNum2Stage;//?û��ʼ��/��������ŵ�stage�Ķ�Ӧ��ϵ
	//��Ҫһ����ͼ��Ӧ�Ķ�̬��������
	map<int, string> graphIndex2InName;
	map<int, string> graphIndex2OutName;



	FlatNode *readerActor, *writerActor;//��ʶ��д�ļ�������actor
	//��¼C���������ĸ�����������ȷ�淶�������������ʽ
	int n_fdcl;
	vector<StageAssignment*> psalist;		//����ͼ�׶λ��ֵĴ洢
	vector<MetisPartiton*> mplist;			//����ͼmetis���ֵĴ洢
	DividedStaticGraph* dsg_;				//�����SDFͼ���ں�����ͼ��SSSGͼ
	int threadNum;							//�߳�����


	//��Ҫһ����ͼʵ��ʹ�ú�����ӳ��
	map<int, int> graphIndex2coreNum;
public:
	DynamicX86CodeGenerate(DividedStaticGraph*dsg, int, const char*, vector<StageAssignment*>, vector<MetisPartiton*>);
	//���ߺ���
	int getThreadIndex2SSSGindex(int index);		//�����̺߳�����index��ö�ӦSSSGͼ��index

	bool isMainThreadInGraph(int index);			//�ж��Ƿ������߳�

	int getindexinGraph(int index);			//�����̺߳�����index��ø��߳�����ͼ�еı��
	
	string getGraphInName(int graphindex);		//���һ����ͼ�����������

	string getGraphOutName(int graphindex);		//���һ����ͼ�����������

	void setGraphindex2coreNum();

	void DYGlobalVar();	//����ȫ�ֱ��������ļ�
	void DYGlobalVarExtern();	//����ȫ�ֱ��������ļ�
	void DYGlobalHeader();	//����stream�����ͺ�ȫ������������������
	void DYGlobalCpp();		//����������������
	
	void DYGlobalActors(); //���������ʾ�ļ��㵥Ԫactor
	void DYMain();		//���������̵߳�main�ļ�
	void DYAllActorHeader();		//����һ����������actorͷ�ļ���ͷ�ļ�
	void CGMakefile();	//Ϊlinux�´�������makefile�ļ�
	void DYactors();		//����actors
	void DYactor(FlatNode*actor, OperatorType ot);		//����ÿһ��actor
	void DYwork(FlatNode*actor, OperatorType ot, stringstream &);		//����work����
	void DYrunSteadyScheduleWork(FlatNode*actor, stringstream &buf);		//������̬���Ⱥ���
	void DYrunInitScheduleWork(FlatNode*actor, stringstream &buf);		//���ɳ�̬���Ⱥ���
	void DYrun(stringstream&buf, string initFun);
	void DYinitWork(stringstream&);			//��ʼ������init��initvar�ĺ���		
	void DYinitPeek(stringstream&, string);		//peek��������ʵ��

	void DYpopToken(FlatNode*, stringstream&);
	void DYpushToken(FlatNode*, stringstream&);
	void DYdeclList(FlatNode*actor, OperatorType ot, stringstream&);
	void DYinitVarAndState(FlatNode*actor, OperatorType ot, stringstream&);
	void DYlogicInit(FlatNode *actor, OperatorType ot, stringstream &);
	void DYEdgeParam(FlatNode *actor, stringstream &);
	void DYthis(FlatNode *actor, OperatorType ot, stringstream &);

	void DYThreads();
	void DYThread(int index, stringstream&buf);	//�߳����ɴ���
	void DYThreadFaker(int index, stringstream&buf);	//�߳����ɴ���
public:
	
	void CGFrta();
	void OutputToFile(string fileName, string oldContents);
	void AdclInit(Node *from, int offset);
	void OutputArgList(List *list, int offset);
	void OutputCRSpaceAndTabs(int tabs);
	void OutputStmt(Node *node, int offset);
	void OutputTabs(int tabs);
	void OutputConstant(Node *, Bool);
	int OutputString(const char *s);
	void OutputStmtList(List *list, int offset);
	int OutputChar(char val);
	int OutpusString(const char *s);
	void CharToText(char *, unsigned char);
	void OutputFloat(float);
	void OutputDouble(double);
	void ExtractDeclVariables(Node *from);
	void RecursiveAdclInit(List *init);

	//���ݻ�ȡ����
	//string GetPrimDataType(Node *);
	//string GetArrayDataType(Node *node);
	string GetArrayDim(Node *from);
	
	string GetOpType(OpType op);


private:
	void COSX86_Node(Node*,int);
	void COSX86_List(List*,int);
	//ʹ��X86��AST��������
	void COSX86_Const(Node *node, ConstNode *u, int offset);
	void COSX86_Id(Node *node, idNode *u, int offset);
	void COSX86_Binop(Node *node, binopNode *u, int offset);
	void COSX86_Unary(Node *node, unaryNode *u, int offset);
	void COSX86_Cast(Node *node, castNode *u, int offset);
	void COSX86_Comma(Node *node, commaNode *u, int offset);
	void COSX86_Ternary(Node *node, ternaryNode *u, int offset);
	void COSX86_Array(Node *node, arrayNode *u, int offset);
	void COSX86_Call(Node *node, callNode *u, int offset);
	void COSX86_Initializer(Node *node, initializerNode *u, int offset);
	void COSX86_ImplicitCast(Node *node, implicitcastNode *u, int offset);
	void COSX86_Label(Node *node, labelNode *u, int offset);
	void COSX86_Switch(Node *node, SwitchNode *u, int offset);
	void COSX86_Case(Node *node, CaseNode *u, int offset);
	void COSX86_Default(Node *node, DefaultNode *u, int offset);
	void COSX86_If(Node *node, IfNode *u, int offset);
	void COSX86_IfElse(Node *node, IfElseNode *u, int offset);
	void COSX86_While(Node *node, WhileNode *u, int offset);
	void COSX86_Do(Node *node, DoNode *u, int offset);
	void COSX86_For(Node *node, ForNode *u, int offset);
	void COSX86_Goto(Node *node, GotoNode *u, int offset);
	void COSX86_Continue(Node *node, ContinueNode *u, int offset);
	void COSX86_Break(Node *node, BreakNode *u, int offset);
	void COSX86_Return(Node *node, ReturnNode *u, int offset);	
	void COSX86_Block(Node *node, BlockNode *u, int offset);
	void COSX86_Prim(Node *node, primNode *u, int offset);
	void COSX86_Tdef(Node *node, tdefNode *u, int offset);
	void COSX86_Ptr(Node *node, ptrNode *u, int offset);
	void COSX86_Adcl(Node *node, adclNode *u, int offset);
	void COSX86_Fdcl(Node *node, fdclNode *u, int offset);
	void COSX86_Sdcl(Node *node, sdclNode *u, int offset);
	void COSX86_Udcl(Node *node, udclNode *u, int offset);
	void COSX86_Edcl(Node *node, edclNode *u, int offset);
	void COSX86_Decl(Node *node, declNode *u, int offset);
	void COSX86_Attrib(Node *node, attribNode *u, int offset);
	void COSX86_Proc(Node *node, procNode *u, int offset);
	void COSX86_Text(Node *node, textNode *u, int offset);
	/*****--------------Define For SPL----------******/
	void COSX86_STRdcl(Node *node, strdclNode *u, int offset);
	void COSX86_Comdcl(Node *node, comDeclNode *u, int offset) { }
	void COSX86_Composite(Node *node, compositeNode *u, int offset) { }
	void COSX86_ComInOut(Node *node, comInOutNode *u, int offset) { }
	void COSX86_ComBody(Node *node, comBodyNode *u, int offset) { }
	void COSX86_Param(Node *node, paramNode *u, int offset){ }
	void COSX86_OperBody(Node *node, operBodyNode *u, int offset) { }
	void COSX86_Operdcl(Node *node, operDeclNode *u, int offset) { }
	void COSX86_Operator_(Node *node, operatorNode *u, int offset) { }
	void COSX86_Window(Node *node, windowNode *u, int offset) { }
	void COSX86_Sliding(Node *node, slidingNode *u, int offset) { }
	void COSX86_Tumbling(Node *node, tumblingNode *u, int offset) { }
	void COSX86_Uncertainty(Node *node, uncertaintyNode *u, int offset){}
	/*****--------------New For SPL----------*****/
	void COSX86_CompositeCall(Node *node, comCallNode *u, int offset) { }
	void COSX86_Pipeline(Node *node, PipelineNode *u, int offset) { }
	void COSX86_SplitJoin(Node *node, SplitJoinNode *u, int offset) { }
	void COSX86_Split(Node *node, splitNode *u, int offset) { }
	void COSX86_Join(Node *node, joinNode *u, int offset) { }
	void COSX86_RoundRobin(Node *node, roundrobinNode *u, int offset) { }
	void COSX86_Duplicate(Node *node, duplicateNode *u, int offset) { }
	/*****************���ķ��������***********/
	void COSX86_Add(Node *node, addNode *u, int offset) { }
private:
	stringstream declList, declInitList, stateInit;
};

#endif // !_X86DYNAMIC_H_
