/*
 * �칹��Ⱥ��˴������ɽ׶�
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
/*�칹��Ⱥ��˴���������*/
class HClusterCodeGenerator
{
	typedef enum {//��Ҫ������ʾһ���ڵ��������߱�ʾ��id����ߵĴ����ұ߱�ʾ��id�ұߵĴ�������int a[10],left��a��right��[10]
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
		/*��֤���ֵ��ȣ��׶θ�ֵ��������ȷ��*/
		assert(m_hsa != NULL && m_hcp != NULL && m_sssg != NULL && !m_curDir.empty());
		assert(m_hsa->getFlatNode2HCluster2Stage().size() == m_hcp->getFlatNode2hClusterNum().size());
		assert(sssg->GetFlatNodes().size() == m_hcp->getFlatNode2hClusterNum().size());

		//�õ���ǰSDFͼ�е�actor����
		m_flatNodes = sssg->GetFlatNodes();	

		//��ʼ������Ҫ����kernel�Ľڵ㼯��
		m_kernelNodes = vector<FlatNode*>();

		m_globalVarName = vector<string>();

		m_nActors = m_flatNodes.size();

		m_globalList = NULL;
		m_extractStateDecl = false;

		//��ʼ��
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

	/*����������ں���*/
	void HClusterCodeGeneration(const string &str);

	/*�õ�ָ���������ڵ��ϣ��̸߳���*/
	int getCpuThreadNumOnHCluster(int hClusterNum);
	int getGpuThreadNumOnHCluster(int hClusterNum);

	int getNetCommEdgeSize(FlatNode *pflatNode, FlatNode *cflatNode, int pushValue, Bool type);//ȡpflatNode�������ߣ���cflatNode�������ߣ���Ӧ�Ļ������Ĵ�С��pushValue�������ߵ�push����,type��ʾ�������뻹�������
	/*=================================================================*/
	/*���ɸ���actor����*/
	void CGActors();
	void CGActor(FlatNode *actor, OperatorType ot);//����ָ��actor����
	void CGActor_GPU(FlatNode *actor, OperatorType ot);//����ָ��actor����
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
	void CGFlush(stringstream &buf, FlatNode *actor);//���������ߵ����һ��flush����

	void CGAllKernel();
	void CGMachineFile();
	/*=================================================================*/
	void CGGlobalVar(); // ����ȫ�ֱ���
	void CGGlobalVarextern();

	void CGGlobalStreamHeader();//����ȫ�ֱ��������ߵ���Ϣ
	void CGGlobalStreamCpp();
	void CGThreads();//�������е��߳��ļ�
	void CGThread(int, int ,int ,int,stringstream&);//���ݼ�Ⱥ��ţ����ֱ�ţ����������߳�
	void CGProcess(int cluster, stringstream&);// �������еĽ���
	void CGProcesses();//������������
	void CGMain();//�������������̵߳ĺ���
	/*=======================================================================*/
	
	void createStream2StructTypeMap();//�������ߵ�����֮���map
	void collectGlobalInfo();//�ռ������ȫ����Ϣ���������ṹ���Լ������ȣ�
	//�ж��ռ�ÿ��actor�ж�Ӧ��kernel�����Ƿ���Ҫȫ�ֱ���
	bool judgeKernelParameters(FlatNode *node, string varName);
protected:
	int findID(FlatNode *flatnode);//ȡflatNode��name�ĺ�˵ı��
	//string GetPrimDataType(Node *from);
	//string GetArrayDataType(Node *node);
	string NodeDeclToString(Node *);//��һ�������ڵ�ת����һ���ַ���
	string GetMPIDataType(Node *);//����prim����ȷ����Ӧ��MPI�е�����
	void CommitMPIDataType(stringstream&buf);//��MPI�ײ�ϵͳ�ύ����
	int GetIORadio(FlatNode *flatNode, int edgeNo, Bool type);//ȡedge�߶�Ӧ�������������,type��ʾ�������뻹�������
	/***********************************************/
	void OutputCRSpace(stringstream &, int);
	void outputToFile(std::string, std::string);
	void CGDeclList(stringstream &, DeclKind, List *, int, FlatNode *);//����decl list
	void CGTextNode(stringstream &, Node *, int);
	void CGNode(stringstream &, Node *, int);//��Ҫ����������ڵ㣨������decl�ڵ㣩
	void CGDecl(stringstream &, DeclKind, Node *, int, FlatNode *);
	void CGExpr(stringstream &, Node *, int);//Ϊһ��ı��ʽ���ɴ���
	void CGInnerExpr(stringstream &, Node *, int, Context, int);
	int Precedence(Node *node, Bool *left_assoc);//ȡ����������ȼ�
	void CGStatementList(stringstream &, List *, int);//�����������list
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

	//�ڵ㼯��
	vector<FlatNode *> m_flatNodes;
	vector<FlatNode *> m_kernelNodes;
	int m_nActors;
	FlatNode *m_curActor;
	/*�������ɵ�ǰĿ¼*/
	string m_curDir;

	map<string, string> m_stream2StructType; // SDFͼ��ÿ�����������������֮���ӳ��(�ߵ����ƣ�struct1)
	map<string, string> m_struct2StructFields; //�ṹ�����ͽṹ������ɵ��ַ���֮���map
	map<string, List *> m_struct2FieldList; //�ṹ������ṹ���Ա
	multimap<FlatNode *, string> m_actor2InEdge; //actor��Ӧ�������
	multimap<FlatNode *, string> m_actor2OutEdge; //actor��Ӧ�������

	List *m_globalList; //���ڼ�¼ȫ�ֶ���ı����������Լ�����
	bool m_extractStateDecl;//������ȡcomposite��param�в����Ķ��壬���ս���Щ�������뵽composite�еĸ���operator���γɵ���ĳ�Ա��

	stringstream m_strScheduler, m_parameterBuf, m_thisBuf;//��¼ÿһ��actor����Ϣ
	stringstream m_stateDeclStream;//��д���ļ�ʱ����actor��state�еı���(������������ɺ�state�еı�����Ҫ��Դ��3�����棺param��var��state)
	stringstream m_stateInitStream;//��¼state�б����ĳ�ʼ����
	stringstream m_destructorStream;//��¼��������Ҫ�ͷŵ�����
	stringstream m_array_staticMemberStream;//��¼actor�ڲ��ľ�̬��Ա���Լ������ԱΪ���ʼ��׼����
	stringstream m_globalVarStream;//���ڱ���ȫ�ֱ�����global��

	/*������ÿ��actor����kernel�����ı�������ӳ���ϵ*/
	vector<string> m_globalVarName;
	/*�洢����ȫ�ֱ�������һ��string�����Ǳ�������
	  �ڶ���map�У�������Ϊ��ά������string�洢��ά����
      Ϊһά������ڶ���stringΪ0��Ϊ����������string��Ϊ0
	*/
	vector<map<string, map<string, string>>> m_globalVarInfo;
	/*�洢ȫ�ֱ�����Ӧ������*/
	vector<string> m_globalVarType;

	//��������
	multimap<int, map<int, set<int>>> m_hCluster2CoreNum2Stage;//��Ⱥ�ڵ㣬��������ŵ�stage�Ķ�Ӧ��ϵ
	multimap<int, map<int, set<int>>> m_hCluster2GpuNum2Stage;
};


#endif