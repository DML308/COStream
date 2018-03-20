/*
 * 异构集群后端代码生成阶段
 * 2016/10/24 yrr
 */

#ifndef _HCLUSTERCODEGENERATOR_H_
#define _HCLUSTERCODEGENERATOR_H_

#include "HClusterPartition.h"
#include "HClusterStageAssignment.h"
#include "HClusterLibCopy.h"
#include "../schedulerSSG.h"
#include "../staticStreamGraph.h"
#include "../flatNode.h"
#include "../ActorEdgeInfo.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <stack>

using namespace std;

extern List *Program;
/*异构集群后端代码生成类*/
class HClusterCodeGenerator
{
	typedef enum {//主要用来标示一个节点的类型左边标示在id的左边的串，右边标示在id右边的串，例如int a[10],left是a，right是[10]
		Left,
		Right
	} Context;

	typedef enum {
		TopDecl,   /* toplevel decls: procs, global variables, typedefs, SUE defs */
		BlockDecl,      /* local decls at beginning of a block */
		SUFieldDecl,    /* structure/union field decls */
		EnumConstDecl,  /* enumerated constant decls */
		FormalDecl      /* formal args to a procedure */
	} DeclKind;
public:
	HClusterCodeGenerator(SchedulerSSG *sssg, HClusterPartition *hcp, HClusterStageAssignment * hsa, const char* curDir) :
		m_sssg(sssg), m_hcp(hcp), m_hsa(hsa),m_curDir(curDir)
	{
		/*验证划分调度，阶段赋值操作的正确性*/
		assert(m_hsa != NULL && m_hcp != NULL && m_sssg != NULL && !m_curDir.empty());
		assert(m_hsa->getFlatNode2HCluster2Stage().size() == m_hcp->getFlatNode2hClusterNum().size());
		assert(sssg->GetFlatNodes().size() == m_hcp->getFlatNode2hClusterNum().size());

		//得到当前SDF图中的actor集合
		m_flatNodes = sssg->GetFlatNodes();	

		//初始化所有要生成kernel的节点集合
		m_kernelNodes = vector<FlatNode*>();

		m_globalVarName = vector<string>();

		m_nActors = m_flatNodes.size();

		m_globalList = NULL;
		m_extractStateDecl = false;

		//初始化
		map<int, set<int>> tmpPartNum2Stage;
		for (int i = 0; i < m_hcp->getHClusters(); ++i)
		{
			
			int nGpu = 0, nCore = 0;
			if (m_hcp->isGPUClusterNode(i))
			{
				nGpu = getGpuThreadNumOnHCluster(i);
			}//if
			else {
				nGpu = 0;
			}//else
			nCore = getCpuThreadNumOnHCluster(i);
					
			vector<FlatNode*> tmpFlatNodes;
			set<int> tmpStage;
			tmpPartNum2Stage.clear();
			for (int j = 0; j < nCore; ++j)
			{
				tmpStage.clear();
				tmpFlatNodes = m_hcp->getHClusterCore2FlatNodes(i)[j];

				for (vector<FlatNode*>::iterator iter = tmpFlatNodes.begin(); iter != tmpFlatNodes.end(); ++iter)
				{
					tmpStage.insert(m_hsa->getStageNum(*iter));
				}//for
				tmpPartNum2Stage.insert(make_pair(j, tmpStage));
			}//for		
			m_hCluster2CoreNum2Stage.insert(make_pair(i, tmpPartNum2Stage));

			tmpPartNum2Stage.clear();
			if (nGpu != 0)
			{
				for (int k = 0; k < nGpu; ++k)
				{
					tmpStage.clear();
					tmpFlatNodes = m_hcp->getHClusterGpu2StatelessNodes(i)[k];

					for (vector<FlatNode*>::iterator iter = tmpFlatNodes.begin(); iter != tmpFlatNodes.end(); ++iter)
					{
						tmpStage.insert(m_hsa->getStageNum(*iter));
					}//for
					tmpPartNum2Stage.insert(make_pair(k, tmpStage));
				}//for
			}//if

			m_hCluster2GpuNum2Stage.insert(make_pair(i, tmpPartNum2Stage));
		}//for
	}

	~HClusterCodeGenerator(){}

	/*代码生成入口函数*/
	void HClusterCodeGeneration(const string &str);

	/*得到指定服务器节点上，线程个数*/
	int getCpuThreadNumOnHCluster(int hClusterNum);
	int getGpuThreadNumOnHCluster(int hClusterNum);

	int getNetCommEdgeSize(FlatNode *pflatNode, FlatNode *cflatNode, int pushValue, Bool type);//取pflatNode（生产者）到cflatNode（消费者）对应的缓冲区的大小，pushValue是生产者的push码率,type表示的是输入还是输出边
	/*=================================================================*/
	/*生成各个actor程序*/
	void CGActors();
	void CGActor(FlatNode *actor, OperatorType ot);//生成指定actor程序
	void CGActor_GPU(FlatNode *actor, OperatorType ot);//生成指定actor程序
	void CGEdgeParam(FlatNode *actor, stringstream &);
	void CGActorMemberVar(FlatNode *actor, OperatorType ot, stringstream &);
	void CGLogicStateInit(FlatNode *actor, OperatorType ot, stringstream &);
	void CGLogicInit(FlatNode *actor, OperatorType ot, stringstream &);
	void CGPopToken(FlatNode *, stringstream &, string);
	void CGPushToken(FlatNode *, stringstream &, string);
	void CGThis(FlatNode *actor, OperatorType ot, stringstream &);
	void CGDestructor(FlatNode *actor, OperatorType ot, stringstream &);
	void CGWork(FlatNode *actor, OperatorType ot, stringstream &);
	void CGWork_GPU(FlatNode *actor, OperatorType ot, stringstream &);
	void CGRunInitScheduleWork(stringstream &buf, FlatNode *actor);
	void CGRunSteadyScheduleWork(stringstream &buf, FlatNode *actor);
	void CGFlush(stringstream &buf, FlatNode *actor);//创建生产者的最后一次flush操作

	void CGAllKernel();
	void CGMachineFile();
	/*=================================================================*/
	void CGGlobalVar(); // 生成全局变量
	void CGGlobalVarextern();

	void CGGlobalStreamHeader();//生成全局变量——边的信息
	void CGGlobalStreamCpp();
	void CGThreads();//生成所有的线程文件
	void CGThread(int, int ,int ,int,stringstream&);//根据集群编号，划分编号，创建单个线程
	void CGProcess(int cluster, stringstream&);// 创建所有的进程
	void CGProcesses();//创建单个进程
	void CGMain();//生成启动各个线程的函数
	/*=======================================================================*/
	
	void createStream2StructTypeMap();//构造边与边的类型之间的map
	void collectGlobalInfo();//收集程序的全局信息（变量，结构，以及函数等）
	//判断收集每个actor中对应的kernel参数是否需要全局变量
	bool judgeKernelParameters(FlatNode *node, string varName);
protected:
	int findID(FlatNode *flatnode);//取flatNode的name的后端的编号
	//string GetPrimDataType(Node *from);
	//string GetArrayDataType(Node *node);
	string NodeDeclToString(Node *);//将一个声明节点转换成一个字符串
	string GetMPIDataType(Node *);//根据prim类型确定对应在MPI中的类型
	void CommitMPIDataType(stringstream&buf);//向MPI底层系统提交类型
	int GetIORadio(FlatNode *flatNode, int edgeNo, Bool type);//取edge边对应的输入输出速率,type表示的是输入还是输出边
	/***********************************************/
	void OutputCRSpace(stringstream &, int);
	void outputToFile(std::string, std::string);
	void CGDeclList(stringstream &, DeclKind, List *, int, FlatNode *);//处理decl list
	void CGTextNode(stringstream &, Node *, int);
	void CGNode(stringstream &, Node *, int);//主要处理的是语句节点（不包括decl节点）
	void CGDecl(stringstream &, DeclKind, Node *, int, FlatNode *);
	void CGExpr(stringstream &, Node *, int);//为一般的表达式生成代码
	void CGInnerExpr(stringstream &, Node *, int, Context, int);
	int Precedence(Node *node, Bool *left_assoc);//取运算符的优先级
	void CGStatementList(stringstream &, List *, int);//处理所有语句list
	void CGConst(stringstream &, Node *, int);
	int OutputChar(stringstream &, char);
	void CGBinop(stringstream &, Node *, int, int);
	string GetOpType(OpType op);
	void CGUnary(stringstream &, Node *, int, int);
	void CGType(stringstream &, Node *, int);
	void CGPartialType(stringstream &, Context, Node *, TypeQual, int);
	void CGSUE(stringstream &, SUEtype *, Bool, int);
	Bool IsSourceExpression(Node *);
	void CGCommaList(stringstream &, List *, int);
	void CGArray(stringstream &, Node *, int, int);
	void CGDimensions(stringstream &, List *, int);
	void CGCall(stringstream &, Node *, int, int);
	void CGArrayPtr(stringstream &out, Node *, int);
	void CGAttribs(stringstream &out, List *attribs, int);
private:
	HClusterPartition *m_hcp;
	HClusterStageAssignment *m_hsa;
	SchedulerSSG *m_sssg;

	//节点集合
	vector<FlatNode *> m_flatNodes;
	vector<FlatNode *> m_kernelNodes;
	int m_nActors;
	FlatNode *m_curActor;
	/*代码生成当前目录*/
	string m_curDir;

	map<string, string> m_stream2StructType; // SDF图中每条边与边上数据类型之间的映射(边的名称，struct1)
	map<string, string> m_struct2StructFields; //结构体名和结构体域组成的字符串之间的map
	map<string, List *> m_struct2FieldList; //结构体名与结构体成员
	multimap<FlatNode *, string> m_actor2InEdge; //actor对应的输入边
	multimap<FlatNode *, string> m_actor2OutEdge; //actor对应的输出边

	List *m_globalList; //用于记录全局定义的变量，类型以及函数
	bool m_extractStateDecl;//用于提取composite的param中参数的定义，最终将这些参数放入到composite中的各个operator所形成的类的成员中

	stringstream m_strScheduler, m_parameterBuf, m_thisBuf;//记录每一个actor内信息
	stringstream m_stateDeclStream;//在写入文件时保存actor的state中的变量(经常量传播完成后state中的变量主要来源于3个方面：param，var，state)
	stringstream m_stateInitStream;//记录state中变量的初始化的
	stringstream m_destructorStream;//记录析构函数要释放的内容
	stringstream m_array_staticMemberStream;//记录actor内部的静态成员，以及数组成员为其初始化准备的
	stringstream m_globalVarStream;//用于保存全局变量（global）

	/*声明与每个actor与其kernel函数的变量参数映射关系*/
	vector<string> m_globalVarName;
	/*存储所有全局变量，第一个string参数是变量名，
	  第二个map中：若参数为二维数组则string存储其维数，
      为一维数组则第二个string为0，为常数则两个string都为0
	*/
	vector<map<string, map<string, string>>> m_globalVarInfo;
	/*存储全局变量对应的类型*/
	vector<string> m_globalVarType;

	//服务器上
	multimap<int, map<int, set<int>>> m_hCluster2CoreNum2Stage;//集群节点，处理器编号到stage的对应关系
	multimap<int, map<int, set<int>>> m_hCluster2GpuNum2Stage;
};


#endif