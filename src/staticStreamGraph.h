#ifndef _STATIC_STREAM_GRAPH_H_
#define _STATIC_STREAM_GRAPH_H_

#include "flatNode.h"
#include <set>

extern "C"
{
	extern int NThreads;
};

class StaticStreamGraph
{
protected:
	std::string comName;
	FlatNode *topLevel; // SDFͼ����ʼ�ڵ㣬����ֻ��һ������Ϊ0�Ľڵ�

	std::vector<FlatNode *> tmpFlatNodes;// SDFͼĳplace�ϻ�ĳthread�ϵĽڵ㼯��

public://������������protected�޸ĳ�public���Ա��ܶ�������޸�
	
	std::vector<FlatNode *> flatNodes;// SDFͼ���нڵ㼯��
	std::map<Node *, FlatNode *>  mapEdge2UpFlatNode; // ������������϶˰�
	std::multimap<Node *, FlatNode *>  mapEdge2DownFlatNode;//������������¶˰�
	
	std::map<FlatNode *, int> mapSteadyWork2FlatNode; // ��Ÿ���operator��workestimate����̬���������ƣ�
	std::map<FlatNode *, int> mapInitWork2FlatNode; // ��Ÿ���operator��workestimate����̬��

public:
	StaticStreamGraph()	{	}
	void SetName(const char *);
	inline void AddFlatNode(FlatNode *flatNode)
	{
		flatNodes.push_back(flatNode);
	}
	inline void SetTopLevel()
	{
		//assert (flatNodes[0] != NULL && flatNodes[0]->nIn == 0); 
		topLevel = flatNodes[0];
	}
	void PrintFlatNodes(); // ��ӡSDFͼ
	void GenerateFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite); // ����SDFͼ����ʼ��flatNodes, mapEdge2FlatNode
	
	FlatNode *GenerateFlatNodes(operatorNode *u);//zww 20120917 ���
	FlatNode * InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,FlatNode *oldFlatNode);
	FlatNode * InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,std::set<FlatNode *> oldFlatNodeSet);//zww 20121011 ���

	void SetFlatNodesWeights(); // �����ڵ�����Ȩ��
	void SetFlatNodesWeights(FlatNode *flatNode); // ���أ�Ϊ��ʹÿ��FlatNode�ܹ���������Ȩ�أ������ڵ�����Ȩ��   zww-20120312
	void ResetFlatNodeNames(); // ����ÿ��flatnode��nameֵ
	void ResetFlatNodeVisitTimes(); // ����ÿ��flastnode��visttimesֵ
	inline FlatNode *GetTopLevel(void)
	{ return topLevel;}
	void AddSteadyWork(FlatNode *,int); // �����̬���ȹ�����
	void AddInitWork(FlatNode *,int); // ��ų�̬���ȹ�����
	int GetSteadyWork(FlatNode *n);
	int GetInitWork(FlatNode *n);
	std::vector<FlatNode *> GetFlatNodes(int place_id); //lxx.2012.03.28
	std::vector<FlatNode *> GetFlatNodes(int place_id, int thread_id); //lxx.2012.03.28
	void GetPreName();   //cwb��ȡ��actor��������ǰ������
	inline std::string GetName(){ return comName;}

	inline std::vector<FlatNode *> GetFlatNodes(void)
	{ return flatNodes;}
	inline std::map<Node *,FlatNode *>  GetMapEdge2UpFlatNode(void)
	{ return mapEdge2UpFlatNode;}
	inline std::multimap<Node *,FlatNode *>  GetMapEdge2DownFlatNode(void)
	{ return mapEdge2DownFlatNode;}
	inline std::map<FlatNode *, int> GetSteadyWorkMap(void)
	{ return mapSteadyWork2FlatNode;}
	inline std::map<FlatNode *, int> GetInitWorkMap(void)
	{ return mapInitWork2FlatNode;}
};

GLOBAL extern StaticStreamGraph *SSG;
GLOBAL StaticStreamGraph *AST2FlatStaticStreamGraph(Node *mainComposite);
GLOBAL void GenerateWorkEst(StaticStreamGraph *ssg,bool WorkEstimateByDataFlow);
GLOBAL bool CheckSplitUncertainty(StaticStreamGraph*ssg);


GLOBAL bool CheckSplitUncertaintybeifen(StaticStreamGraph *ssg);
GLOBAL bool hasUncertainty(StaticStreamGraph*ssg);
#endif