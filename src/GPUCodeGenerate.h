#ifndef _GPUCODEGENERATE_H_
#define _GPUCODEGENERATE_H_

#include "schedulerSSG.h"
#include "ActorStageAssignment.h"
#include "MetisPartiton.h"
#include "GPULibCopy.h"
#include "stdio.h"
#include "set"
#include <stack>
#include "GPUClusterPartition.h"
#include "MAFLPartition.h"
#include "ActorEdgeInfo.h"
//#include "process.h"
using namespace std;
/***************************************************/
/*********************GPU(OpenCL)��������*******************/
/***************************************************/

//���ɶ���ļ�
class GPUCodeGenerate
{
public:
	/******************���캯��******************/
	GPUCodeGenerate(SchedulerSSG *, int, int, const char *,StageAssignment *,MAFLPartition*);
	/********************************************/
	void OutputToFile(std::string, std::string);//������ļ�
	//string GetPrimDataType(Node *);
	//string GetNodeDataType(Node *);
	//string GetArrayDataType(Node *);
	string GetArrayDim(Node *);
	string GetDataInitVal(string );
	void GetArrayDataInitVal(Node *, stringstream &);
	string GetOpType(OpType op);
	void ExtractDeclVariables(Node *);
	void RecursiveAdclInit(List *init);
	void AdclInit(Node * from,int offset);
	/***********************************************/
	void CGactors(); // ���ɸ���actor����
	void CGactor(FlatNode *actor, OperatorType ot);//����ָ��actor����
	void CGwork(FlatNode *actor, OperatorType ot, stringstream &);
	/***********************************************/
	void OutputCRSpaceAndTabs(int );
	void OutputTabs(int );
	int OutputChar(char val);
	int OutputString(const char *s);
	void OutputConstant(Node *, Bool );
	void CharToText(char *, unsigned char );
	void OutputFloat(float );
	void OutputDouble(double );
	void OutputStmt(Node *node, int);
	void OutputStmtList(List *, int);
	void OutputArgList(List *list,int offset);
	void AddSemicolon();
	string MakeTabs(int );
	/********************************************/
	void CGdataGetforGPU(FlatNode *actor,stringstream &buf);
	void CGdataSendforGPU(FlatNode *actor,stringstream &buf);
	void CGexternbuffer(FlatNode *actor,stringstream &buf);
	void CGrun(stringstream &buf, string initFun); // ����run����
	void CGrunInitScheduleWork(FlatNode *actor,stringstream &buf);
	void CGrunInitScheduleWorkforGPU(FlatNode *actor,stringstream &buf);//
	void CGrunSteadyScheduleWork(FlatNode *actor,stringstream &buf);
	void CGrunSteadyScheduleWorkforGPU(FlatNode *actor,stringstream &buf);//
	void CGrecv(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGsend(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGflush(stringstream &, string);// ����flush����
	void CGinitWork(stringstream &, string);
	void CGinitPeek(stringstream &, string);
	void CGinitPush(stringstream &, string);
	void CGpopToken(FlatNode *,stringstream &, string);
	void CGpushToken(FlatNode *,stringstream &, string);
	void CGpopTokenForGPU(FlatNode *,stringstream &, string);//��������GPU��actor��pop����
	void CGpushTokenForGPU(FlatNode *,stringstream &, string);//��������GPU��actor��push����
	void CGpushToken(stringstream &, string);
	void CGdeclList(FlatNode *actor, OperatorType ot, stringstream &);
	void CGinitVarAndState(FlatNode *actor, OperatorType ot, stringstream &);
	void CGlogicInit(FlatNode *actor, OperatorType ot, stringstream &, stringstream &);
	void CGthis(FlatNode *actor, OperatorType ot, stringstream &);
	void CGEdgeParam(FlatNode *actor,stringstream &);
	/********************************************/
	void CGGlobalvar(); // ����ȫ�ֱ���
	void CGAllKernel();//����OpenCL kernel�ļ�
	void CGGlobalvarextern();
	void CGglobalHeader();//����ȫ�ֱ��������ߵ���Ϣ
	void CGglobalCpp();
	void CGAllActor();//��������actor����
	void CGThreads();//���������߳��ļ�
	void CGThread(int,stringstream&);
	void CGdatatag(FlatNode *,stringstream &);//Ϊ������GPU�ϵ�Actor���ɶ�д��־
	void CGdatataginit(FlatNode *,stringstream &);//�ڹ��캯���ж�������־λ���г�ʼ��
	void InitBufferSize();//��Edge2Buffersize��������
	void SetMultiNum();//�Զ�������������




	void CGAllActorHeader();//Ϊÿ��actor����һ���ಢ����ͬһ���ļ���,�˺�������ͷ�ļ�
	void CGAllActorCpp();//Ϊÿ��actor����һ���ಢ����ͬһ���ļ���,�˺�������.cpp�ļ�
	void CGMain();//�������������̵߳ĺ���
	void CGTestFile();//�����ļ����������Ĺ�����˵û������
	/**********************************************/
	void CGMakefile();//Ϊlinux�µĴ�������Makefile�ļ�
	/**********************************************/
	string FindNumofNode(FlatNode *);//���ؽ������
	int ReturnBufferSize(FlatNode *,FlatNode *);//�����������֮��Ļ�������С
	int ReturnPushWeight(FlatNode *actorA,FlatNode *actorB);//����A push��B��������
	int ReturnPeekWeight(FlatNode *actorA,FlatNode *actorB);//����B��A��peek��������
	string ReturnNameOfEdge(FlatNode *actorA,FlatNode *actorB);//����AB֮��ṹ������ı�������x
	string ReturnTypeOfEdge(FlatNode *actorA,FlatNode *actorB);//����AB֮��ṹ������ı������ͣ���double
	string ReturnNameOfEdgestr(string namestr);
	string ReturnTypeOfEdgestr(string typestr);
	string ReturnNameOfTheEdge(string searchstring);//���رߵ�����
	int ReturnNumBiger(int size);//���ر�size�����С������������2���ݼ���
	bool IsUpBorder(FlatNode *);   //cwb�жϸ�����Ƿ��б߽���
	bool IsDownBorder(FlatNode *); //cwb�ж��ӽ���Ƿ��б߽���
	bool DataDependence(FlatNode *,FlatNode *); //cwb�ж�����actor������������ϵ��peek>pop
	bool ExistDependence(FlatNode *); //cwb�жϸ�actor�д���peek>pop
private:
	StageAssignment *pSa;
	MAFLPartition *Maflp;
	//MetisPartiton *pPp;
	//GPUClusterPartition *pPp;
	SchedulerSSG *sssg_;	
	std::vector<FlatNode *> flatNodes_;
	map<string,string> mapOperator2EdgeName;
	int nGpu_;
	int nActors_;
	string dir_;
	int buffer_size_;
	bool extractDecl, isInParam;
	FlatNode* curactor;
	std::map<FlatNode *, int> mapFlatNode2Place; // ��Ÿ� FlatNode ��Ӧ�� place���
	stringstream declList, declInitList, stateInit;
	stringstream edgexy;
	stringstream strScheduler, parameterBuf, thisBuf;
	stringstream globalvarbuf,temp_declInitList;
	stringstream declInitList_temp;
	string OutputPath;
	string InputPath;
	map<operatorNode *, std::string> mapOperator2ClassName; // ��Ÿ� class ��Ӧ�� composite
	multimap<FlatNode*,string> mapActor2InEdge;
	multimap<FlatNode*,string> mapActor2OutEdge;
	map<int,set<int> > mapNum2Stage;//��������ŵ�stage�Ķ�Ӧ��ϵ
	multimap<string,string> mapedge2xy;//��Ÿ��ߵ�����
	multimap<string,string> mapedge2xy2;//mapedge2xy�ĸ���
	map<map<FlatNode *,FlatNode *>,map<string,string>>edge2nameandtype;//�洢�ߵ�����,ǰ������FlatNode��ʾ��㣬��һ��string��ʾ�ñ߶�Ӧ�Ľṹ�����������������double���ڶ���string��ʾ��������x
	map<string,string>nameandtypeofedge;//�ߵ������Լ�����,Ŀǰֻ֧�ֵ������ͣ���֧������ͽṹ��
	map<string,map<string,string>>edgestrnameandtype;//
	map<string,string>edge2name;//�洢���������ƵĶ�Ӧ��ϵ
	map<string,string>allnameandtype;//���ڴ洢���еıߵ����ƺ����ͣ���Ҫ����������globalheader�е��Զ���ṹ��
	//map<string,string> mapStruct2Struct;//���ÿ��operate��struct��ӳ��
	vector<string> staticname2value;
	vector<string> ptrname;//���ڴ��ÿ��actor�ж�̬���ɵ���������ƣ��Ա��ں����delete����
	vector<string> ptrtype;//���ڴ��������vector��Ӧ����������
	multimap<string,string>ptrdim;//���ڱ����������Ӧ�������λ��
	multimap<FlatNode *,map<string,string>>actor2ptr;//���ڴ�����е�actor�ж�̬���ɵ����������Ϣ
	vector<string> kernelparam;//���ڴ�Ŵ���ÿ��actor��kernel�Ĳ������˲������������Լ���д��־��Ŀǰ��������ÿ��actor��logic state���ֵĲ���
	vector<string> kernelparamtype;//������������Ӧ����¼�˲���������
	map<FlatNode *,vector<string>>Actor2KernelPar;//���ڴ�Ŵ���ÿ��actor����kernel�Ĳ���ӳ���ϵ
	map<FlatNode *,vector<string>>Actor2KernelParType;//���ڴ�Ŵ���ÿ��actor����kernel�Ĳ�������ӳ���ϵ
	map<FlatNode*,vector<string>>OutbufferOfActor;//���ڴ��ÿ��actor���buffer
	map<FlatNode*,vector<string>>InbufferOfActor;//���ڴ��ÿ��actor��Ϊ����actor��CPU��ʱ����������buffer
	map<map<FlatNode*,FlatNode*>,string>BufferNameOfEdge;//���ڴ�Ž��֮��ߵ�����
	map<int,int>thread2queue;//���̺߳�ӳ�䵽���к�
	string structdatatype;//�ṹ��������(x,y)������
	multimap<FlatNode *,map<string,map<string,string>>>alllocalvariable;//���ڴ洢����actor��̬���ɵľֲ�����
	multimap<FlatNode *,map<string,map<string,string>>>allstaticvariable;//���ڴ洢����actor�ľ�̬����
	vector<map<string,map<string,string>>>staticvariable,staticvariablefinal;//���ڴ洢actor�ľ�̬��������һ��string�����Ǳ��������ڶ���map�У�������Ϊ��ά������string�洢��ά����Ϊһά������ڶ���stringΪ0��Ϊ����������string��Ϊ0
	vector<string>staticvartype,staticvartypefinal;//���ڴ洢������map��Ӧ�Ĳ���������
	multimap<FlatNode *,vector<string>>allstatictype;
	vector<map<string,map<string,string>>>globalvariable,globalvariablefinal;//ͬ��
	vector<string>globalvartype,globalvartypefinal;//ͬ��
	set<string>array2Dname,array2Dnamefinal;//��static��global�еĶ�ά�������ƴ洢����������kernel������
	set<string>array1Dname,array1Dnamefinal;//
	set<string>arraylocal2Dname;//���ֲ���ά��̬���������������kernel������
	map<string,int>Edge2Buffersize;//��¼ÿ���߶�Ӧbuffer�Ĵ�С
	map<FlatNode *,int>Node2QueueID;//��¼��㵽GPU���е�ӳ���ϵ
	int RetrunBufferSizeOfGpu(int num);//���ر��Ϊnum��gpu��buffer���ܴ�С
	//string structdataname;
	/*********************************************/
	void UpdateFlatNodeEdgeName(vector<FlatNode *>::iterator flatnode,string edgeName);
	void SPL2GPU_Node(Node *, int);
	void SPL2GPU_List(List *, int);
	// -----------------------------------------------
	void SPL2GPU_Const(Node *node, ConstNode *u, int offset);
	void SPL2GPU_Id(Node *node, idNode *u, int offset);
	void SPL2GPU_Binop(Node *node, binopNode *u, int offset);
	void SPL2GPU_Unary(Node *node, unaryNode *u, int offset);
	void SPL2GPU_Cast(Node *node, castNode *u, int offset);
	void SPL2GPU_Comma(Node *node, commaNode *u, int offset);
	void SPL2GPU_Ternary(Node *node, ternaryNode *u, int offset);
	void SPL2GPU_Array(Node *node, arrayNode *u, int offset);
	void SPL2GPU_Call(Node *node, callNode *u, int offset);
	void SPL2GPU_Initializer(Node *node, initializerNode *u, int offset);
	void SPL2GPU_ImplicitCast(Node *node, implicitcastNode *u, int offset);
	void SPL2GPU_Label(Node *node, labelNode *u, int offset);
	void SPL2GPU_Switch(Node *node, SwitchNode *u, int offset);
	void SPL2GPU_Case(Node *node, CaseNode *u, int offset);
	void SPL2GPU_Default(Node *node, DefaultNode *u, int offset);
	void SPL2GPU_If(Node *node, IfNode *u, int offset);
	void SPL2GPU_IfElse(Node *node, IfElseNode *u, int offset);
	void SPL2GPU_While(Node *node, WhileNode *u, int offset);
	void SPL2GPU_Do(Node *node, DoNode *u, int offset);
	void SPL2GPU_For(Node *node, ForNode *u, int offset);
	void SPL2GPU_Goto(Node *node, GotoNode *u, int offset);
	void SPL2GPU_Continue(Node *node, ContinueNode *u, int offset);
	void SPL2GPU_Break(Node *node, BreakNode *u, int offset);
	void SPL2GPU_Return(Node *node, ReturnNode *u, int offset);
	void SPL2GPU_Block(Node *node, BlockNode *u, int offset);
	void SPL2GPU_Prim(Node *node, primNode *u, int offset);
	void SPL2GPU_Tdef(Node *node, tdefNode *u, int offset);
	void SPL2GPU_Ptr(Node *node, ptrNode *u, int offset);
	void SPL2GPU_Adcl(Node *node, adclNode *u, int offset);
	void SPL2GPU_Fdcl(Node *node, fdclNode *u, int offset);
	void SPL2GPU_Sdcl(Node *node, sdclNode *u, int offset);
	void SPL2GPU_Udcl(Node *node, udclNode *u, int offset);
	void SPL2GPU_Edcl(Node *node, edclNode *u, int offset);
	void SPL2GPU_Decl(Node *node, declNode *u, int offset);
	void SPL2GPU_Attrib(Node *node, attribNode *u, int offset);
	void SPL2GPU_Proc(Node *node, procNode *u, int offset);
	void SPL2GPU_Text(Node *node, textNode *u, int offset);
	/*****--------------Define For SPL----------******/
	void SPL2GPU_STRdcl(Node *node, strdclNode *u, int offset);
	void SPL2GPU_Comdcl(Node *node, comDeclNode *u, int offset) { }
	void SPL2GPU_Composite(Node *node, compositeNode *u, int offset) { }
	void SPL2GPU_ComInOut(Node *node, comInOutNode *u, int offset) { }
	void SPL2GPU_ComBody(Node *node, comBodyNode *u, int offset) { }
	void SPL2GPU_Param(Node *node, paramNode *u, int offset){ }
	//void SPL2GPU_Var(Node *node, varNode *u, int offset){ }
	//void SPL2GPU_Graph(Node *node, graphNode *u, int offset) { }
	void SPL2GPU_OperBody(Node *node, operBodyNode *u, int offset) { }
	void SPL2GPU_Operdcl(Node *node, operDeclNode *u, int offset) { }
	void SPL2GPU_Operator_(Node *node, operatorNode *u, int offset) { }
	//void SPL2GPU_Logic(Node *node, logicNode *u, int offset) { }
	void SPL2GPU_Window(Node *node, windowNode *u, int offset) { }
	//void SPL2GPU_Eviction(Node *node, evictionNode *u, int offset) { }
	//void SPL2GPU_Trigger(Node *node, triggerNode *u, int offset) { }
	void SPL2GPU_Sliding(Node *node, slidingNode *u, int offset) { }
	void SPL2GPU_Tumbling(Node *node, tumblingNode *u, int offset) { }
	/*****--------------New For SPL----------*****/
	void SPL2GPU_CompositeCall(Node *node, comCallNode *u, int offset) { }
	void SPL2GPU_Pipeline(Node *node, PipelineNode *u, int offset) { }
	void SPL2GPU_SplitJoin(Node *node, SplitJoinNode *u, int offset) { }
	void SPL2GPU_Split(Node *node, splitNode *u, int offset) { }
	void SPL2GPU_Join(Node *node, joinNode *u, int offset) { }
	void SPL2GPU_RoundRobin(Node *node, roundrobinNode *u, int offset) { }
	void SPL2GPU_Duplicate(Node *node, duplicateNode *u, int offset) { }
	void SPL2GPU_Uncertainty(Node*node, uncertaintyNode *u, int offset){}
	//void SPL2GPU_StreamFor(Node *node, StreamForNode *u, int offset) { }
	//void SPL2GPU_StreamIf(Node *node, StreamIfNode *u, int offset) { }
	//void SPL2GPU_StreamIfElse(Node *node, StreamIfElseNode *u, int offset) { }
	void SPL2GPU_Add(Node *node, addNode *u, int offset) { }
};
extern "C"
{
	extern Bool Win;
	extern Bool Linux;
	extern Bool TRACE;
	extern Bool FileRW;
	extern Bool CHECKBARRIERTIME;
	extern Bool CHECKEACHACTORTIME;
	extern GLOBAL const char *output_file;
	extern GLOBAL int MultiNum;
	extern GLOBAL int LocalSize;
	extern GLOBAL int LocalSizeofWork;
	extern GLOBAL Bool IsMod;
	extern GLOBAL int PrintFlag;
	extern GLOBAL Bool TimeCount;
	extern GLOBAL int Comm_num;
	extern GLOBAL Bool PrintTime;
	extern GLOBAL int NCpuThreads;
};
#endif