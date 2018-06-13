#ifndef _X86CODEGENERATE_H_
#define _X86CODEGENERATE_H_

#include "schedulerSSG.h"
#include "X86LibCopy.h"
#include "ActorStageAssignment.h"
#include "MetisPartiton.h"
#include "stdio.h"
#include "list.h"
#include "set"
#include "ActorEdgeInfo.h"
#include "TemplateClass.h"
extern bool libPath;
//#include "process.h"
using namespace std;
						/***************************************************/
						/*********************X86��������*******************/
						/***************************************************/

//���ɶ���ļ�
class X86CodeGenerate
{
public:
	/******************���캯��******************/
	X86CodeGenerate(SchedulerSSG *, int, const char *,StageAssignment *,MetisPartiton *,TemplateClass*);
	/********************************************/
	void OutputToFile(std::string, std::string);//������ļ�
	string GetPrimDataType(Node *);
	string GetNodeDataType(Node *);
	string GetArrayDataType(Node *);
	string GetArrayDim(Node *);
	string GetDataInitVal(string );
	string GetOpType(OpType op);
	void ExtractDeclVariables(Node *);
	void RecursiveAdclInit(List *init);
	void AdclInit(Node * from,int offset);
/***********************************************/
	void CGactors(); // ���ɸ���actor����
	void CGactor(FlatNode *actor,string name, OperatorType ot);//����ָ��actor����
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
	void CGrun(stringstream &buf, string initFun); // ����run����
	void CGrunInitScheduleWork(FlatNode *actor,stringstream &buf);
	void CGrunSteadyScheduleWork(FlatNode *actor,stringstream &buf);
	void CGrecv(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGsend(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGflush(stringstream &, string);// ����flush����
	void CGinitWork(stringstream &);
	void CGinitPeek(stringstream &, string);
	void CGinitPush(stringstream &, string);
	void CGpopToken(FlatNode *,stringstream &);
	void CGpushToken(FlatNode *,stringstream &);
	void CGpushToken(stringstream &, string);
	void CGdeclList(FlatNode *actor, OperatorType ot, stringstream &);
	void CGinitVarAndState(FlatNode *actor, OperatorType ot, stringstream &);
	void CGlogicInit(FlatNode *actor, OperatorType ot, stringstream &);
	void CGthis(FlatNode *actor, OperatorType ot, stringstream &,string name);
	void CGEdgeParam(FlatNode *actor,stringstream &);
/********************************************/
	void CGGlobalvar(); // ����ȫ�ֱ���
	void CGGlobalvarextern();
	void CGExternType(); //Ƕ��ʽ�����ɽӿ�����
	void CGglobalHeader();//����ȫ�ֱ��������ߵ���Ϣ
	void CGglobalCpp();
	void CGAllActor();//��������actor����
	void CGThreads();//���������߳��ļ�
	void CGThread(int,stringstream&);
	void CGAllActorHeader();//Ϊÿ��actor����һ���ಢ����ͬһ���ļ���,�˺�������ͷ�ļ�
	void CGAllActorCpp();//Ϊÿ��actor����һ���ಢ����ͬһ���ļ���,�˺�������.cpp�ļ�
	void CGMain();//�������������̵߳ĺ���
/**********************************************/
	void CGMakefile();//Ϊlinux�µĴ�������Makefile�ļ�
/**********************************************/
	~X86CodeGenerate(){delete pEdgeInfo;}
private:
	StageAssignment *pSa;
	MetisPartiton *Mp;
	SchedulerSSG *sssg_;	
	TemplateClass *Tc;
	std::vector<FlatNode *> flatNodes_;
	std::vector<FlatNode *> vTemplateNode_; //chenwenbin 20140724 �洢����ģ����
	std::vector<std::string> vTemplateName_; //chenwenbin ��¼ÿ��ģ���������
	std::map<FlatNode *,std::string> mapFlatnode2Template_; //chenwenbin ���flatnode��Ӧ��ģ����
	ActorEdgeInfo* pEdgeInfo;//��Ÿ����ߵ�������Ϣ��by lihe 2012-09-04
	int nCpucore_;
	int nActors_;
	int nTemplateNode_;
	string dir_;
	string _profile_Name;//��������profile�õģ�����profile�ļ��� 20121127 zww
	//int buffer_size_;				//lihe buffer_size����Ҫ
	bool extractDecl, isInParam;
	FlatNode* curactor;
	std::map<FlatNode *, int> mapFlatNode2Place; // ��Ÿ� FlatNode ��Ӧ�� place���
	stringstream declList, declInitList, stateInit;
	stringstream strScheduler, parameterBuf, thisBuf;
	stringstream ExternTypeBuf; //Ƕ��ʽ�еĽӿ�����
	stringstream globalvarbuf,temp_declInitList;//globalvarbuf����ȫ�ֱ���������
	stringstream declInitList_temp;
	string OutputPath;
	string InputPath;
	FlatNode *readerActor,*writerActor;//��ʶ��д�ļ�������actor
	map<operatorNode *, std::string> mapOperator2ClassName; // ��Ÿ� class ��Ӧ�� composite
	multimap<FlatNode*,string> mapActor2InEdge;		//actor��Ӧ����ߵ�����
	multimap<FlatNode*,string> mapActor2OutEdge;	//actor��Ӧ����ߵ�����
	map<int,set<int> > mapNum2Stage;//��������ŵ�stage�Ķ�Ӧ��ϵ
	vector<string> staticNameInit;	//���actor��state��var��Ա�ĳ�ʼ�����������actor��Ӧ��Դ�ļ���
	vector<string> ptrname;//���ڴ��ÿ��actor�ж�̬���ɵ���������ƣ��Ա��ں����delete����
	vector<string> nDeclDim;//���ڴ��δ�����ȫ������ά��
/*********************************************/
	void CGFileReaderActor(stringstream &buf);
	void CGFileWriterActor(stringstream &buf);
	void CGFrta();
	void SPL2X86_Node(Node *, int);
	void SPL2X86_List(List *, int);
	// -----------------------------------------------
	void SPL2X86_Const(Node *node, ConstNode *u, int offset);
	void SPL2X86_Id(Node *node, idNode *u, int offset);
	void SPL2X86_Binop(Node *node, binopNode *u, int offset);
	void SPL2X86_Unary(Node *node, unaryNode *u, int offset);
	void SPL2X86_Cast(Node *node, castNode *u, int offset);
	void SPL2X86_Comma(Node *node, commaNode *u, int offset);
	void SPL2X86_Ternary(Node *node, ternaryNode *u, int offset);
	void SPL2X86_Array(Node *node, arrayNode *u, int offset);
	void SPL2X86_Call(Node *node, callNode *u, int offset);
	void SPL2X86_Initializer(Node *node, initializerNode *u, int offset);
	void SPL2X86_ImplicitCast(Node *node, implicitcastNode *u, int offset);
	void SPL2X86_Label(Node *node, labelNode *u, int offset);
	void SPL2X86_Switch(Node *node, SwitchNode *u, int offset);
	void SPL2X86_Case(Node *node, CaseNode *u, int offset);
	void SPL2X86_Default(Node *node, DefaultNode *u, int offset);
	void SPL2X86_If(Node *node, IfNode *u, int offset);
	void SPL2X86_IfElse(Node *node, IfElseNode *u, int offset);
	void SPL2X86_While(Node *node, WhileNode *u, int offset);
	void SPL2X86_Do(Node *node, DoNode *u, int offset);
	void SPL2X86_For(Node *node, ForNode *u, int offset);
	void SPL2X86_Goto(Node *node, GotoNode *u, int offset);
	void SPL2X86_Continue(Node *node, ContinueNode *u, int offset);
	void SPL2X86_Break(Node *node, BreakNode *u, int offset);
	void SPL2X86_Return(Node *node, ReturnNode *u, int offset);
	void SPL2X86_Block(Node *node, BlockNode *u, int offset);
	void SPL2X86_Prim(Node *node, primNode *u, int offset);
	void SPL2X86_Tdef(Node *node, tdefNode *u, int offset);
	void SPL2X86_Ptr(Node *node, ptrNode *u, int offset);
	void SPL2X86_Adcl(Node *node, adclNode *u, int offset);
	void SPL2X86_Fdcl(Node *node, fdclNode *u, int offset);
	void SPL2X86_Sdcl(Node *node, sdclNode *u, int offset);
	void SPL2X86_Udcl(Node *node, udclNode *u, int offset);
	void SPL2X86_Edcl(Node *node, edclNode *u, int offset);
	void SPL2X86_Decl(Node *node, declNode *u, int offset);
	void SPL2X86_Attrib(Node *node, attribNode *u, int offset);
	void SPL2X86_Proc(Node *node, procNode *u, int offset);
	void SPL2X86_Text(Node *node, textNode *u, int offset);
	/*****--------------Define For SPL----------******/
	void SPL2X86_STRdcl(Node *node, strdclNode *u, int offset);
	void SPL2X86_Comdcl(Node *node, comDeclNode *u, int offset) { }
	void SPL2X86_Composite(Node *node, compositeNode *u, int offset) { }
	void SPL2X86_ComInOut(Node *node, comInOutNode *u, int offset) { }
	void SPL2X86_ComBody(Node *node, comBodyNode *u, int offset) { }
	void SPL2X86_Param(Node *node, paramNode *u, int offset){ }
	void SPL2X86_OperBody(Node *node, operBodyNode *u, int offset) { }
	void SPL2X86_Operdcl(Node *node, operDeclNode *u, int offset) { }
	void SPL2X86_Operator_(Node *node, operatorNode *u, int offset) { }
	void SPL2X86_Window(Node *node, windowNode *u, int offset) { }
	void SPL2X86_Sliding(Node *node, slidingNode *u, int offset) { }
	void SPL2X86_Tumbling(Node *node, tumblingNode *u, int offset) { }
	/*****--------------New For SPL----------*****/
	void SPL2X86_CompositeCall(Node *node, comCallNode *u, int offset) { }
	void SPL2X86_Pipeline(Node *node, PipelineNode *u, int offset) { }
	void SPL2X86_SplitJoin(Node *node, SplitJoinNode *u, int offset) { }
	void SPL2X86_Split(Node *node, splitNode *u, int offset) { }
	void SPL2X86_Join(Node *node, joinNode *u, int offset) { }
	void SPL2X86_RoundRobin(Node *node, roundrobinNode *u, int offset) { }
	void SPL2X86_Duplicate(Node *node, duplicateNode *u, int offset) { }
	/*****************���ķ��������***********/
	void SPL2X86_Add(Node *node, addNode *u, int offset) { }
	void SPL2X86_Itco(Node *node, itcoNode *u, int offset);
	
};
extern "C"{
	extern Bool Win;
	extern Bool Linux;
	extern Bool TRACE;
	extern Bool CHECKEACHACTORTIME;
	extern Bool MakeProfile;
 	extern GLOBAL const char *output_file;
 	extern Bool CallModelEmbed; 
 	extern GLOBAL const char *infileName;
 	extern GLOBAL const char *outfileName ;
 	extern GLOBAL Bool NoCheckBuffer;
 	extern GLOBAL Bool AmplifySchedule;
 	extern GLOBAL int AmplifyFactor;
 	extern GLOBAL Bool CALRATIO;
	extern GLOBAL Bool PrintResult;
 };
#ifndef WIN32
	#define MAXPathSize 100
	extern GLOBAL char COStreamPath[MAXPathSize];
#endif
#endif