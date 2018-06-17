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
						/*********************X86代码生成*******************/
						/***************************************************/

//生成多个文件
class X86CodeGenerate
{
public:
	/******************构造函数******************/
	X86CodeGenerate(SchedulerSSG *, int, const char *,StageAssignment *,MetisPartiton *,TemplateClass*);
	/********************************************/
	void OutputToFile(std::string, std::string);//输出到文件
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
	void CGactors(); // 生成各个actor程序
	void CGactor(FlatNode *actor,string name, OperatorType ot);//生成指定actor程序
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
	void CGrun(stringstream &buf, string initFun); // 生成run方法
	void CGrunInitScheduleWork(FlatNode *actor,stringstream &buf);
	void CGrunSteadyScheduleWork(FlatNode *actor,stringstream &buf);
	void CGrecv(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGsend(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGflush(stringstream &, string);// 生成flush方法
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
	void CGGlobalvar(); // 生成全局变量
	void CGGlobalvarextern();
	void CGExternType(); //嵌入式中生成接口类型
	void CGglobalHeader();//生成全局变量——边的信息
	void CGglobalCpp();
	void CGAllActor();//生成所有actor的类
	void CGThreads();//生成所有线程文件
	void CGThread(int,stringstream&);
	void CGAllActorHeader();//为每个actor生成一个类并放在同一个文件中,此函数生成头文件
	void CGAllActorCpp();//为每个actor生成一个类并放在同一个文件中,此函数生成.cpp文件
	void CGMain();//生成启动各个线程的函数
/**********************************************/
	void CGMakefile();//为linux下的代码生成Makefile文件
/**********************************************/
	~X86CodeGenerate(){delete pEdgeInfo;}
private:
	StageAssignment *pSa;
	MetisPartiton *Mp;
	SchedulerSSG *sssg_;	
	TemplateClass *Tc;
	std::vector<FlatNode *> flatNodes_;
	std::vector<FlatNode *> vTemplateNode_; //chenwenbin 20140724 存储所有模板结点
	std::vector<std::string> vTemplateName_; //chenwenbin 记录每个模板类的名字
	std::map<FlatNode *,std::string> mapFlatnode2Template_; //chenwenbin 存放flatnode对应的模板类
	ActorEdgeInfo* pEdgeInfo;//存放各个边的类型信息，by lihe 2012-09-04
	int nCpucore_;
	int nActors_;
	int nTemplateNode_;
	string dir_;
	string _profile_Name;//用于生成profile用的，保存profile文件名 20121127 zww
	//int buffer_size_;				//lihe buffer_size不需要
	bool extractDecl, isInParam;
	FlatNode* curactor;
	std::map<FlatNode *, int> mapFlatNode2Place; // 存放各 FlatNode 对应的 place序号
	stringstream declList, declInitList, stateInit;
	stringstream strScheduler, parameterBuf, thisBuf;
	stringstream ExternTypeBuf; //嵌入式中的接口类型
	stringstream globalvarbuf,temp_declInitList;//globalvarbuf包含全局变量的声明
	stringstream declInitList_temp;
	string OutputPath;
	string InputPath;
	FlatNode *readerActor,*writerActor;//标识读写文件操作的actor
	map<operatorNode *, std::string> mapOperator2ClassName; // 存放各 class 对应的 composite
	multimap<FlatNode*,string> mapActor2InEdge;		//actor对应输入边的名称
	multimap<FlatNode*,string> mapActor2OutEdge;	//actor对应输出边的名称
	map<int,set<int> > mapNum2Stage;//处理器编号到stage的对应关系
	vector<string> staticNameInit;	//存放actor内state和var成员的初始化，被输出到actor对应的源文件中
	vector<string> ptrname;//用于存放每个actor中动态生成的数组的名称，以便于后面的delete操作
	vector<string> nDeclDim;//用于存放未定义的全局数组维度
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
	/*****************新文法新增结点***********/
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