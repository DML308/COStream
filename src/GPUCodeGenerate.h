#ifndef _GPUCODEGENERATE_H_
#define _GPUCODEGENERATE_H_

#include "schedulerSSG.h"
#include "ActorStageAssignment.h"
#include "MetisPartiton.h"
#include "GPULibCopy.h"
#include <iostream>
//#include <io.h>
#include <cstdio>
#include <cstdlib>
//#include <direct.h>
#include <cstring>
#include <map>
#include <set>
#include <stack>
#include "GPUClusterPartition.h"
#include "HAFLPartition.h"
#include "ActorEdgeInfo.h"
#include "TemplateClass.h"
#include "DNBPartition.h"
//#include "process.h"
using namespace std;
/***************************************************/
/*********************GPU(OpenCL)代码生成*******************/
/***************************************************/

//生成多个文件
class GPUCodeGenerate
{
public:
	/******************构造函数******************/
	GPUCodeGenerate(SchedulerSSG *, int, int, const char *,StageAssignment *,HAFLPartition*,TemplateClass*,string,DNBPartition*);
	/********************************************/
	void OutputToFile(std::string, std::string);//输出到文件
	string GetPrimDataType(Node *);
	string GetNodeDataType(Node *);
	string GetArrayDataType(Node *);
	string GetArrayDim(Node *);
	string GetDataInitVal(string );
	void GetArrayDataInitVal(Node *, stringstream &);
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
	void CGdataGetforGPU(FlatNode *actor,stringstream &buf);
	void CGdataSendforGPU(FlatNode *actor,stringstream &buf);
	void CGexternbuffer(FlatNode *actor,stringstream &buf);
	void CGrun(stringstream &buf, string initFun); // 生成run方法
	void CGrunInitScheduleWork(FlatNode *actor,stringstream &buf);
	void CGrunInitScheduleWorkforGPU(FlatNode *actor,stringstream &buf);//
	void CGrunSteadyScheduleWork(FlatNode *actor,stringstream &buf);
	void CGrunSteadyScheduleWorkforGPU(FlatNode *actor,stringstream &buf);//
	void CGrecv(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGsend(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGflush(stringstream &, string);// 生成flush方法
	void CGinitWork(stringstream &, string);
	void CGinitPeek(stringstream &, string);
	void CGinitPush(stringstream &, string);
	void CGpopToken(FlatNode *,stringstream &, string);
	void CGpushToken(FlatNode *,stringstream &, string);
	void CGpopTokenForGPU(FlatNode *,stringstream &, string);//用于生成GPU上actor的pop函数
	void CGpushTokenForGPU(FlatNode *,stringstream &, string);//用于生成GPU上actor的push函数
	void CGpushToken(stringstream &, string);
	void CGdeclList(FlatNode *actor, OperatorType ot, stringstream &);
	void CGinitVarAndState(FlatNode *actor, OperatorType ot, stringstream &);
	void CGlogicInit(FlatNode *actor, OperatorType ot, stringstream &, stringstream &);
	void CGthis(FlatNode *actor, OperatorType ot, stringstream &,string);
	void CGEdgeParam(FlatNode *actor,stringstream &);
	/********************************************/
	void CGGlobalvar(); // 生成全局变量
	void CGAllKernel();//生成OpenCL kernel文件
	void CGGlobalvarextern();
	void CGglobalHeader();//生成全局变量——边的信息
	void CGglobalCpp();
	void CGAllActor();//生成所有actor的类
	void CGThreads();//生成所有线程文件
	void CGThread(int,stringstream&);
	void CGdatatag(FlatNode *,stringstream &);//为分配在GPU上的Actor生成读写标志
	void CGCreateBuffer(FlatNode *,stringstream &);//cwb为actor创建GPU空间
	void CGdatataginit(FlatNode *,stringstream &);//在构造函数中对上述标志位进行初始化
	void InitBufferSize();//往Edge2Buffersize中填数据
	void SetMultiNum();//自动设置扩大因子




	void CGAllActorHeader();//为每个actor生成一个类并放在同一个文件中,此函数生成头文件
	void CGAllActorCpp();//为每个actor生成一个类并放在同一个文件中,此函数生成.cpp文件
	void CGMain();//生成启动各个线程的函数
	void CGTestFile();//测试文件，对于最后的工程来说没有意义
	/**********************************************/
	void CGMakefile();//为linux下的代码生成Makefile文件
	/**********************************************/
	string FindNumofNode(FlatNode *);//返回结点的序号
	int ReturnBufferSize(FlatNode *,FlatNode *);//返回两个结点之间的缓冲区大小
	int ReturnPushWeight(FlatNode *actorA,FlatNode *actorB);//返回A push给B的数据量
	int ReturnPeekWeight(FlatNode *actorA,FlatNode *actorB);//返回B从A处peek的数据量
	string ReturnNameOfEdge(FlatNode *actorA,FlatNode *actorB);//返回AB之间结构体包含的变量，如x
	string ReturnTypeOfEdge(FlatNode *actorA,FlatNode *actorB);//返回AB之间结构体包含的变量类型，如double
	string ReturnNameOfEdgestr(string namestr);
	string ReturnTypeOfEdgestr(string typestr);
	string ReturnNameOfTheEdge(string searchstring);//返回边的名称
	int ReturnNumBiger(int size);//返回比size大的最小的数，该数是2的幂级数
	bool DataDependence(FlatNode *,FlatNode *); //cwb判断两个actor间数据依赖关系，peek>pop
	bool ExistDependence(FlatNode *); //cwb判断该actor中存在peek>pop
private:
	string BenchmarkName;
	StageAssignment *pSa;
	HAFLPartition *Haflp;
	//MetisPartiton *pPp;
	//GPUClusterPartition *pPp;
	SchedulerSSG *sssg_;
	TemplateClass *Tc;
	DNBPartition *Dnbp;
	std::vector<FlatNode *> flatNodes_;
	std::vector<FlatNode *> vTemplateNode_; //chenwenbin 20140724 存储所有模板结点
	std::vector<std::string> vTemplateName_; //chenwenbin 记录每个模板类的名字
	std::map<FlatNode *,std::string> mapFlatnode2Template_; //chenwenbin 存放flatnode对应的模板类
	int nTemplateNode_;  //cwb
	ActorEdgeInfo* pEdgeInfo;//存放各个边的类型信息
	map<string,string> mapOperator2EdgeName;
	int nGpu_;
	int nActors_;
	string dir_;
	int buffer_size_;
	bool extractDecl, isInParam;
	FlatNode* curactor;
	std::map<FlatNode *, int> mapFlatNode2Place; // 存放各 FlatNode 对应的 place序号
	stringstream declList, declInitList, stateInit;
	stringstream edgexy;
	stringstream strScheduler, parameterBuf, thisBuf;
	stringstream globalvarbuf,temp_declInitList;
	stringstream declInitList_temp;
	string OutputPath;
	string InputPath;
	map<operatorNode *, std::string> mapOperator2ClassName; // 存放各 class 对应的 composite
	multimap<FlatNode*,string> mapActor2InEdge;
	multimap<FlatNode*,string> mapActor2OutEdge;
	map<int,set<int> > mapNum2Stage;//处理器编号到stage的对应关系
	multimap<string,string> mapedge2xy;//存放各边的类型
	multimap<string,string> mapedge2xy2;//mapedge2xy的副本
	map<map<FlatNode *,FlatNode *>,map<string,string> > edge2nameandtype;//存储边的类型,前面两个FlatNode表示结点，第一个string表示该边对应的结构体包含变量的类型如double，第二个string表示其名称如x
	map<string,string>nameandtypeofedge;//边的名称以及类型,目前只支持单个类型，不支持数组和结构体
	map<string,map<string,string> > edgestrnameandtype;//
	map<string,string>edge2name;//存储边与其名称的对应关系
	map<string,string>allnameandtype;//用于存储所有的边的名称和类型，主要作用是生成globalheader中的自定义结构体
	//map<string,string> mapStruct2Struct;//存放每个operate的struct的映射
	vector<string> staticname2value;
	vector<string> ptrname;//用于存放每个actor中动态生成的数组的名称，以便于后面的delete操作
	vector<string> ptrtype;//用于存放与上面vector对应参数的类型
	multimap<string,string>ptrdim;//用于保存与上面对应的数组的位数
	multimap<FlatNode *,map<string,string> > actor2ptr;//用于存放所有的actor中动态生成的数组参数信息
	vector<string> kernelparam;//用于存放传给每个actor的kernel的参数，此参数不包括边以及读写标志，目前仅仅包括每个actor的logic state部分的参数
	vector<string> kernelparamtype;//与上面的数组对应，记录了参数的类型
	map<FlatNode *,vector<string> >Actor2KernelPar;//用于存放传给每个actor与其kernel的参数映射关系
	map<FlatNode *,vector<string> >Actor2KernelParType;//用于存放传给每个actor与其kernel的参数类型映射关系
	map<FlatNode*,vector<string> >OutbufferOfActor;//用于存放每个actor输出buffer
	map<FlatNode*,vector<string> >InbufferOfActor;//用于存放每个actor中为输入actor在CPU上时创建的输入buffer
	map<map<FlatNode*,FlatNode*>,string>BufferNameOfEdge;//用于存放结点之间边的名称
	map<int,int>thread2queue;//将线程号映射到队列号
	string structdatatype;//结构体中数据(x,y)的类型
	multimap<FlatNode *,map<string,map<string,string> > >alllocalvariable;//用于存储所有actor动态生成的局部数组
	multimap<FlatNode *,map<string,map<string,string> > >allstaticvariable;//用于存储所有actor的静态变量
	vector<map<string,map<string,string> > >staticvariable,staticvariablefinal;//用于存储actor的静态变量，第一个string参数是变量名，第二个map中：若参数为二维数组则string存储其维数，为一维数组则第二个string为0，为常数则两个string都为0
	vector<string>staticvartype,staticvartypefinal;//用于存储与上面map对应的参数的类型
	multimap<FlatNode *,vector<string> >allstatictype;
	vector<map<string,map<string,string> > >globalvariable,globalvariablefinal;//同上
	vector<string>globalvartype,globalvartypefinal;//同上
	set<string>array2Dname,array2Dnamefinal;//将static和global中的二维数组名称存储起来，用于kernel的生成
	set<string>array1Dname,array1Dnamefinal;//
	set<string>arraylocal2Dname;//将局部二维动态数组存起来，用于kernel的生成
	map<string,int>Edge2Buffersize;//记录每条边对应buffer的大小
	map<FlatNode *,int>Node2QueueID;//记录结点到GPU队列的映射关系
	int RetrunBufferSizeOfGpu(int num);//返回编号为num的gpu中buffer的总大小
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
	extern GLOBAL Bool TimeofEverycore;
	extern GLOBAL int NCpuThreads;
	extern GLOBAL Bool DNBPFlag;
};
#endif
