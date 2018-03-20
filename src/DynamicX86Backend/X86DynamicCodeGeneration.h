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
	
	stringstream globalvarbuf, temp_declInitList;//globalvarbuf包含全局变量的声明
	
	bool extractDecl, isInParam;
	stringstream strScheduler, parameterBuf, thisBuf;
	stringstream ExternTypeBuf; //嵌入式中的接口类型
	FlatNode* curactor;		//当前正在解析/生成的actor
	//源代码中是否存在C函数调用定义标志
	string dir_;//生成文件路径
	int flag;
	//记录declInitList初始长度，用来恢复函数定义前的内容
	ActorEdgeInfo* pEdgeInfo;//存放各个边的类型信息，by lihe 2012-09-04
	std::vector<FlatNode *> flatNodes_;//所有节点
	int declInitLen;
	stringstream declInitList_temp;
	vector<string> nDeclDim;//用于存放未定义的全局数组维度
	vector<string> staticNameInit;	//存放actor内state和var成员的初始化，被输出到actor对应的源文件中
	map<operatorNode *, std::string> mapOperator2ClassName; // 存放各 class 对应的 composite名称

	
	int nCpucore_;		//cpu核数
	int nActors_;		//所有flatNode的数目
	vector<map<int, set<int> > > mapNum2StageList;
	multimap<FlatNode*, string> mapActor2InEdge;
	multimap<FlatNode*, string> mapActor2OutEdge;

	map<int, set<int> > mapNum2Stage;//?没初始化/处理器编号到stage的对应关系
	//需要一个子图对应的动态输出输入边
	map<int, string> graphIndex2InName;
	map<int, string> graphIndex2OutName;



	FlatNode *readerActor, *writerActor;//标识读写文件操作的actor
	//记录C函数声明的个数，用来正确规范函数声明代码格式
	int n_fdcl;
	vector<StageAssignment*> psalist;		//各子图阶段划分的存储
	vector<MetisPartiton*> mplist;			//各子图metis划分的存储
	DividedStaticGraph* dsg_;				//整体的SDF图，内含各子图的SSSG图
	int threadNum;							//线程数量


	//需要一个子图实际使用核数的映射
	map<int, int> graphIndex2coreNum;
public:
	DynamicX86CodeGenerate(DividedStaticGraph*dsg, int, const char*, vector<StageAssignment*>, vector<MetisPartiton*>);
	//工具函数
	int getThreadIndex2SSSGindex(int index);		//根据线程函数的index获得对应SSSG图的index

	bool isMainThreadInGraph(int index);			//判断是否是主线程

	int getindexinGraph(int index);			//根据线程函数的index获得该线程在子图中的编号
	
	string getGraphInName(int graphindex);		//获得一个子图的输入边名称

	string getGraphOutName(int graphindex);		//获得一个子图的输出边名称

	void setGraphindex2coreNum();

	void DYGlobalVar();	//生成全局变量定义文件
	void DYGlobalVarExtern();	//生成全局变量声明文件
	void DYGlobalHeader();	//生成stream流类型和全局数据流缓存区声明
	void DYGlobalCpp();		//生成流缓冲区定义
	
	void DYGlobalActors(); //生成以类表示的计算单元actor
	void DYMain();		//生成启动线程的main文件
	void DYAllActorHeader();		//生成一个包含所有actor头文件的头文件
	void CGMakefile();	//为linux下代码生成makefile文件
	void DYactors();		//生成actors
	void DYactor(FlatNode*actor, OperatorType ot);		//生成每一个actor
	void DYwork(FlatNode*actor, OperatorType ot, stringstream &);		//生成work函数
	void DYrunSteadyScheduleWork(FlatNode*actor, stringstream &buf);		//生成稳态调度函数
	void DYrunInitScheduleWork(FlatNode*actor, stringstream &buf);		//生成初态调度函数
	void DYrun(stringstream&buf, string initFun);
	void DYinitWork(stringstream&);			//初始化函数init和initvar的函数		
	void DYinitPeek(stringstream&, string);		//peek函数，不实用

	void DYpopToken(FlatNode*, stringstream&);
	void DYpushToken(FlatNode*, stringstream&);
	void DYdeclList(FlatNode*actor, OperatorType ot, stringstream&);
	void DYinitVarAndState(FlatNode*actor, OperatorType ot, stringstream&);
	void DYlogicInit(FlatNode *actor, OperatorType ot, stringstream &);
	void DYEdgeParam(FlatNode *actor, stringstream &);
	void DYthis(FlatNode *actor, OperatorType ot, stringstream &);

	void DYThreads();
	void DYThread(int index, stringstream&buf);	//线程生成代码
	void DYThreadFaker(int index, stringstream&buf);	//线程生成代码
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

	//数据获取函数
	//string GetPrimDataType(Node *);
	//string GetArrayDataType(Node *node);
	string GetArrayDim(Node *from);
	
	string GetOpType(OpType op);


private:
	void COSX86_Node(Node*,int);
	void COSX86_List(List*,int);
	//使用X86的AST分析函数
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
	/*****************新文法新增结点***********/
	void COSX86_Add(Node *node, addNode *u, int offset) { }
private:
	stringstream declList, declInitList, stateInit;
};

#endif // !_X86DYNAMIC_H_
