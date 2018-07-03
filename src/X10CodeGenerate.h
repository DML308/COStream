#ifndef _CODEGENERATE_H_
#define _CODEGENERATE_H_

#include "schedulerSSG.h"
#include "X10LibCopy.h"

//生成多个文件，包括接口，actor类，主控程序
class X10CodeGenerate
{
private:
	SchedulerSSG *sssg_;	
	std::vector<FlatNode *> flatNodes_;
	std::string comName_;//x10的class名
	int nCpucore_;
	int nRepeatCount_;
	int nActors_;
	int nPerThreads_;
	std::string dir_;
	int buffer_size_;
	int currentNum_;
	std::map<operatorNode *, std::string> mapOperator2ClassName; // 存放各 class 对应的 composite
	stringstream strScheduler, parameterBuf, thisBuf;
	bool extractDecl, isInParam;
	stringstream declList, declInitList, stateInit;
	std::vector<string> classInstances;
	stringstream runBuf, edgeBuf;
	std::map<FlatNode *, int> mapFlatNode2Place; // 存放各 FlatNode 对应的 place序号
public:
	X10CodeGenerate(SchedulerSSG *, int, int, const char *);

public:
	void SimpleScheduler();

public:
	void CGinterface(); // 生成主接口程序
	void CGclassMembers(Node *strdcl, stringstream &buf); // 生成 stream 的数据成员

	void CGactors(); // 生成各个actor程序
	void CGactor(FlatNode *actor, OperatorType ot);//生成指定actor程序
	void CGrun(stringstream &buf, string initFun); // 生成run方法
	void CGrecv(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGsend(FlatNode *, OperatorType , string, stringstream & ,stringstream & ,stringstream & ,stringstream & ,stringstream &); 
	void CGflush(stringstream &, string);// 生成flush方法
	void CGinitWork(stringstream &, string);
	void CGinitPeek(stringstream &, string);
	void CGinitPush(stringstream &, string);
	void CGpopToken(stringstream &, string);
	void CGpushToken(stringstream &, string);
	void CGwork(FlatNode *actor, OperatorType ot, stringstream &);
	void CGdeclList(FlatNode *actor, OperatorType ot, stringstream &);
	void CGinitVarAndState(FlatNode *actor, OperatorType ot, stringstream &);
	void CGlogicInit(FlatNode *actor, OperatorType ot, stringstream &, stringstream &);
	void CGthis(FlatNode *actor, OperatorType ot, stringstream &);

	void CGmain(); // 生成主控程序
	void CGmainRun();
	void CGmainScheduleData(stringstream &buf);
	void CGmainActor(FlatNode *, OperatorType , int);//生成指定actor实例调用

public:
	void OutputToFile(std::string, std::string);//输出到文件
	string GetPrimDataType(Node *);
	string GetNodeDataType(Node *);
	string GetArrayDataType(Node *);
	string GetPtrDataType(Node *);
	string GetArrayDim(Node *);
	string GetDataInitVal(string );
	void GetArrayDataInitVal(Node *, stringstream &);
	string GetOpType(OpType op);
	void ExtractDeclVariables(Node *);
	void RecursiveAdclInit(List *init);
	void AdclInit(Node * from,int offset);

public:
	/*****--------------Other----------*****/	
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

private:
	void SPL2X10_Node(Node *, int);
	void SPL2X10_List(List *, int);
	// -----------------------------------------------
	void SPL2X10_Const(Node *node, ConstNode *u, int offset);
	void SPL2X10_Id(Node *node, idNode *u, int offset);
	void SPL2X10_Binop(Node *node, binopNode *u, int offset);
	void SPL2X10_Unary(Node *node, unaryNode *u, int offset);
	void SPL2X10_Cast(Node *node, castNode *u, int offset);
	void SPL2X10_Comma(Node *node, commaNode *u, int offset);
	void SPL2X10_Ternary(Node *node, ternaryNode *u, int offset);
	void SPL2X10_Array(Node *node, arrayNode *u, int offset);
	void SPL2X10_Call(Node *node, callNode *u, int offset);
	void SPL2X10_Initializer(Node *node, initializerNode *u, int offset);
	void SPL2X10_ImplicitCast(Node *node, implicitcastNode *u, int offset);
	void SPL2X10_Label(Node *node, labelNode *u, int offset);
	void SPL2X10_Switch(Node *node, SwitchNode *u, int offset);
	void SPL2X10_Case(Node *node, CaseNode *u, int offset);
	void SPL2X10_Default(Node *node, DefaultNode *u, int offset);
	void SPL2X10_If(Node *node, IfNode *u, int offset);
	void SPL2X10_IfElse(Node *node, IfElseNode *u, int offset);
	void SPL2X10_While(Node *node, WhileNode *u, int offset);
	void SPL2X10_Do(Node *node, DoNode *u, int offset);
	void SPL2X10_For(Node *node, ForNode *u, int offset);
	void SPL2X10_Goto(Node *node, GotoNode *u, int offset);
	void SPL2X10_Continue(Node *node, ContinueNode *u, int offset);
	void SPL2X10_Break(Node *node, BreakNode *u, int offset);
	void SPL2X10_Return(Node *node, ReturnNode *u, int offset);
	void SPL2X10_Block(Node *node, BlockNode *u, int offset);
	void SPL2X10_Prim(Node *node, primNode *u, int offset);
	void SPL2X10_Tdef(Node *node, tdefNode *u, int offset);
	void SPL2X10_Ptr(Node *node, ptrNode *u, int offset);
	void SPL2X10_Adcl(Node *node, adclNode *u, int offset);
	void SPL2X10_Fdcl(Node *node, fdclNode *u, int offset);
	void SPL2X10_Sdcl(Node *node, sdclNode *u, int offset);
	void SPL2X10_Udcl(Node *node, udclNode *u, int offset);
	void SPL2X10_Edcl(Node *node, edclNode *u, int offset);
	void SPL2X10_Decl(Node *node, declNode *u, int offset);
	void SPL2X10_Attrib(Node *node, attribNode *u, int offset);
	void SPL2X10_Proc(Node *node, procNode *u, int offset);
	void SPL2X10_Text(Node *node, textNode *u, int offset);
	/*****--------------Define For SPL----------******/
	void SPL2X10_STRdcl(Node *node, strdclNode *u, int offset);
	void SPL2X10_Comdcl(Node *node, comDeclNode *u, int offset) { }
	void SPL2X10_Composite(Node *node, compositeNode *u, int offset) { }
	void SPL2X10_ComInOut(Node *node, comInOutNode *u, int offset) { }
	void SPL2X10_ComBody(Node *node, comBodyNode *u, int offset) { }
	void SPL2X10_Param(Node *node, paramNode *u, int offset){ }
	void SPL2X10_OperBody(Node *node, operBodyNode *u, int offset) { }
	void SPL2X10_Operdcl(Node *node, operDeclNode *u, int offset) { }
	void SPL2X10_Operator_(Node *node, operatorNode *u, int offset) { }
	void SPL2X10_Window(Node *node, windowNode *u, int offset) { }
	void SPL2X10_Sliding(Node *node, slidingNode *u, int offset) { }
	void SPL2X10_Tumbling(Node *node, tumblingNode *u, int offset) { }
	/*****--------------New For SPL----------*****/
	void SPL2X10_CompositeCall(Node *node, comCallNode *u, int offset) { }
	void SPL2X10_Pipeline(Node *node, PipelineNode *u, int offset) { }
	void SPL2X10_SplitJoin(Node *node, SplitJoinNode *u, int offset) { }
	void SPL2X10_Split(Node *node, splitNode *u, int offset) { }
	void SPL2X10_Join(Node *node, joinNode *u, int offset) { }
	void SPL2X10_RoundRobin(Node *node, roundrobinNode *u, int offset) { }
	void SPL2X10_Duplicate(Node *node, duplicateNode *u, int offset) { }
	void SPL2X10_Add(Node *node,addNode *u,int offset) { }
};

#endif
