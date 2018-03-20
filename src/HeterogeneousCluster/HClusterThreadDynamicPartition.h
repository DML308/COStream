/*
* 异构集群线程级任务划分ThreadDynamic
* yrr 2017-01-10
*/
#pragma 
#ifndef _HCLUSTER_THREAD_DYNAMIC_PARTITION_H_
#define _HCLUSTER_THREAD_DYNAMIC_PARTITION_H_

#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <iterator>
#include <utility>
#include <algorithm>
#include "../metis.h"

#include "../schedulerSSG.h"
#include "../flatNode.h"
#include "../ActorStateDetector.h"
#include "HClusterProcessGroupPartition.h"

using namespace std;

/*计算单元actor集合类*/
class ActorSet {
public:
	ActorSet(set<FlatNode*>, SchedulerSSG *);
	~ActorSet() {}

	/*对当前算子集合的入，出集合增加删除操作*/
	void pushInSet(ActorSet *);
	void removeInSet(ActorSet *);
	void pushOutSet(ActorSet *);
	void removeOutSet(ActorSet *);
	/*向当前集合中插入或删除算子*/
	void pushActor(FlatNode*);
	void removeActor(FlatNode*);
	vector<ActorSet *> getInSets();
	vector<ActorSet *> getOutSets();
	/*计算当前算子集合对外总通信量*/
	long computeAllComm();
	long computeTheComm(ActorSet *);
	/*融合两个算子集合*/
	ActorSet *unionSet(ActorSet *);
public:
	/*该集合对应的负载和通信量*/
	long m_workload, m_comm;
	/*上下通信集合*/
	map<ActorSet*, long > m_inSet2Comm, m_outSet2Comm;
	/*当前算子集合含有的节点*/
	set<FlatNode *> m_flatNodes;
private:
	SchedulerSSG *m_sssg;

};

/*线程级动态划分算法类定义*/
class HClusterThreadDynamicPartition {
public:
	HClusterThreadDynamicPartition() {}
	HClusterThreadDynamicPartition(int _nparts, SchedulerSSG * sssg, vector <FlatNode*> _flatNodes) :
		m_nParts(_nparts),m_acturalParts(0), m_sssg(sssg), m_flatNodes(_flatNodes) {
		
		/*计算目标actor集合的总负载*/
		m_allWorkload = 0;
		m_allComm = 0;
		for (vector<FlatNode*>::iterator iter = m_flatNodes.begin(); iter != m_flatNodes.end(); ++iter)
		{
			m_allWorkload += (m_sssg->GetSteadyWork(*iter) * m_sssg->GetSteadyCount(*iter));
		}//for
		m_avgWorkload = m_allWorkload / m_nParts;

		/*构造actor与set之间的初始关系*/
		map<FlatNode *, ActorSet*> tmpActor2Set;
		for (vector<FlatNode*>::iterator iter = m_flatNodes.begin(); iter != m_flatNodes.end(); ++iter)
		{
			set<FlatNode*> tmpNodes;
			tmpNodes.insert(*iter);

			ActorSet *tmpActorSet = new ActorSet(tmpNodes, m_sssg);
			m_sets.insert(tmpActorSet);
			tmpActor2Set.insert(make_pair(*iter, tmpActorSet));
		}//for

		for (set<ActorSet*>::iterator iter = m_sets.begin(); iter != m_sets.end(); ++iter)
		{
			set<ActorSet*> tmpInSets, tmpOutSets;
			set<FlatNode*> tmpFlatNodes = (*iter)->m_flatNodes;
			for (set < FlatNode*>::iterator iter2 = tmpFlatNodes.begin(); iter2 != tmpFlatNodes.end(); ++iter2)
			{
				vector<FlatNode*> inNodes = (*iter2)->inFlatNodes;
				vector<FlatNode*> outNodes = (*iter2)->outFlatNodes;

				for (vector<FlatNode*>::iterator inIter = inNodes.begin(); inIter != inNodes.end(); ++inIter)
				{
					if (tmpActor2Set.find(*inIter) != tmpActor2Set.end())
					{
						tmpInSets.insert(tmpActor2Set.find(*inIter)->second);
					}
				}
				for (vector<FlatNode*>::iterator outIter = outNodes.begin(); outIter != outNodes.end(); ++outIter)
				{
					if (tmpActor2Set.find(*outIter) != tmpActor2Set.end())
					{
						tmpOutSets.insert(tmpActor2Set.find(*outIter)->second);
					}
				}
			}//for

			for (set<ActorSet*>::iterator inIter = tmpInSets.begin(); inIter != tmpInSets.end(); ++inIter)
			{
				(*iter)->pushInSet(*inIter);
			}//for

			for (set<ActorSet *>::iterator outIter = tmpOutSets.begin(); outIter != tmpOutSets.end(); ++outIter)
			{
				(*iter)->pushOutSet(*outIter);
			}
		}//for

		/*初始化算子集合间通信量矩阵*/
		m_commofTwoSets = vector<vector<int>>(m_sets.size(), vector<int>(m_sets.size(), 0));
		m_acturalParts = m_sets.size();
	}
	~HClusterThreadDynamicPartition() {}

	/*线程级划分函数*/
	map<int, vector<FlatNode*>> threadPartition();

	/*划分实现步骤:(1)流图粗化，(2)凸子图及边界调整，(3)反粗化及映射关系建立*/
	void coarseningPhase();
	void partitionAdjustment();
	void uncoarseningPhase();
	vector<ActorSet*> topoSort(vector<ActorSet*>);
	void consructCommMatrix();
	bool judgeSetUnion(int first, int second, int size);
	void movingFlatNodeInterSet(FlatNode*, ActorSet*, ActorSet*);//将flatNode从一个集合移动到另一个集合
	Bool refiningPhase_IsAdjustableFlatNode(FlatNode *flatNode, ActorSet* objGroup, int gain);
	void refiningPhase_UpdatePriQueue(FlatNode *, std::map<std::pair<FlatNode *, ActorSet*>, int > &, std::multimap<int, std::pair<FlatNode *, ActorSet*> > &);
	void refiningPhase_RemoveOldGain(FlatNode *flatNode, std::map<std::pair<FlatNode *, ActorSet*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, ActorSet* > > &gain2FlatNode2AdjPT);
	void insertNewFlatNodeGain(FlatNode *flatNode, ActorSet *curGroup, std::map<std::pair<FlatNode *, ActorSet*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, ActorSet*> > &gain2FlatNode2AdjPT);

	/*计算要融合的两个算子集合可得到的收益*/
	double computeGain(int i, int j)
	{
		double comm = 2 * m_commofTwoSets[i][j];
		double gain = (comm * 360) / (m_setWorkload[i] + m_setWorkload[j]);
		return gain;
	}

	int getFlatNodeId(FlatNode *faltNode, vector<FlatNode *> flatNodes) 
	{
		//根据flatnode找到其在vector中的编号（一个数字标识）
		for (int i = 0; i < flatNodes.size(); i++)
			if (faltNode == flatNodes[i])
				return i;
	}

	/*判断actor是否在当前划分服务器*/
	bool judgeInCurClusterNode(FlatNode *node)
	{
		if (find(m_flatNodes.begin(), m_flatNodes.end(), node) != m_flatNodes.end())
			return true;
		else
			return false;
	}
public:
		/*当前服务器划分后总通信量*/
		long m_allComm;
private:
	int m_nParts;
	int m_acturalParts;
	SchedulerSSG *m_sssg;
	vector<FlatNode *> m_flatNodes;

	/*要划分节点的总负载与平均负载*/
	long m_allWorkload;
	long m_avgWorkload;

	
	/*算子集合类，以及与节点两者的相互映射关系*/
	set<ActorSet *> m_sets;
	map<FlatNode *, ActorSet *> m_actor2Set;

	/*算子集合的负载与通信量*/
	vector<long> m_setWorkload, m_setComm;
	vector<vector<int>> m_commofTwoSets;

	/*划分映射结果*/
	map<int, vector<FlatNode*>> m_partNum2FlatNodes;
	map<FlatNode *, int> m_flatNode2PartNum;

	/*融合后的划分编号与算子集合之间的映射*/
	map<int, ActorSet*> m_partNum2ActorSets;
	map<ActorSet*, int> m_actorSet2PartNum;

	/*辅助*/
	vector<int> m_setIdx2ActualIdx;
	map<ActorSet*, int> m_set2Idx;
	map<int, ActorSet*> m_idx2Set;
};



#endif
