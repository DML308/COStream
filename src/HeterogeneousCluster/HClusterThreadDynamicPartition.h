/*
* �칹��Ⱥ�̼߳����񻮷�ThreadDynamic
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

/*���㵥Ԫactor������*/
class ActorSet {
public:
	ActorSet(set<FlatNode*>, SchedulerSSG *);
	~ActorSet() {}

	/*�Ե�ǰ���Ӽ��ϵ��룬����������ɾ������*/
	void pushInSet(ActorSet *);
	void removeInSet(ActorSet *);
	void pushOutSet(ActorSet *);
	void removeOutSet(ActorSet *);
	/*��ǰ�����в����ɾ������*/
	void pushActor(FlatNode*);
	void removeActor(FlatNode*);
	vector<ActorSet *> getInSets();
	vector<ActorSet *> getOutSets();
	/*���㵱ǰ���Ӽ��϶�����ͨ����*/
	long computeAllComm();
	long computeTheComm(ActorSet *);
	/*�ں��������Ӽ���*/
	ActorSet *unionSet(ActorSet *);
public:
	/*�ü��϶�Ӧ�ĸ��غ�ͨ����*/
	long m_workload, m_comm;
	/*����ͨ�ż���*/
	map<ActorSet*, long > m_inSet2Comm, m_outSet2Comm;
	/*��ǰ���Ӽ��Ϻ��еĽڵ�*/
	set<FlatNode *> m_flatNodes;
private:
	SchedulerSSG *m_sssg;

};

/*�̼߳���̬�����㷨�ඨ��*/
class HClusterThreadDynamicPartition {
public:
	HClusterThreadDynamicPartition() {}
	HClusterThreadDynamicPartition(int _nparts, SchedulerSSG * sssg, vector <FlatNode*> _flatNodes) :
		m_nParts(_nparts),m_acturalParts(0), m_sssg(sssg), m_flatNodes(_flatNodes) {
		
		/*����Ŀ��actor���ϵ��ܸ���*/
		m_allWorkload = 0;
		m_allComm = 0;
		for (vector<FlatNode*>::iterator iter = m_flatNodes.begin(); iter != m_flatNodes.end(); ++iter)
		{
			m_allWorkload += (m_sssg->GetSteadyWork(*iter) * m_sssg->GetSteadyCount(*iter));
		}//for
		m_avgWorkload = m_allWorkload / m_nParts;

		/*����actor��set֮��ĳ�ʼ��ϵ*/
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

		/*��ʼ�����Ӽ��ϼ�ͨ��������*/
		m_commofTwoSets = vector<vector<int>>(m_sets.size(), vector<int>(m_sets.size(), 0));
		m_acturalParts = m_sets.size();
	}
	~HClusterThreadDynamicPartition() {}

	/*�̼߳����ֺ���*/
	map<int, vector<FlatNode*>> threadPartition();

	/*����ʵ�ֲ���:(1)��ͼ�ֻ���(2)͹��ͼ���߽������(3)���ֻ���ӳ���ϵ����*/
	void coarseningPhase();
	void partitionAdjustment();
	void uncoarseningPhase();
	vector<ActorSet*> topoSort(vector<ActorSet*>);
	void consructCommMatrix();
	bool judgeSetUnion(int first, int second, int size);
	void movingFlatNodeInterSet(FlatNode*, ActorSet*, ActorSet*);//��flatNode��һ�������ƶ�����һ������
	Bool refiningPhase_IsAdjustableFlatNode(FlatNode *flatNode, ActorSet* objGroup, int gain);
	void refiningPhase_UpdatePriQueue(FlatNode *, std::map<std::pair<FlatNode *, ActorSet*>, int > &, std::multimap<int, std::pair<FlatNode *, ActorSet*> > &);
	void refiningPhase_RemoveOldGain(FlatNode *flatNode, std::map<std::pair<FlatNode *, ActorSet*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, ActorSet* > > &gain2FlatNode2AdjPT);
	void insertNewFlatNodeGain(FlatNode *flatNode, ActorSet *curGroup, std::map<std::pair<FlatNode *, ActorSet*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, ActorSet*> > &gain2FlatNode2AdjPT);

	/*����Ҫ�ںϵ��������Ӽ��Ͽɵõ�������*/
	double computeGain(int i, int j)
	{
		double comm = 2 * m_commofTwoSets[i][j];
		double gain = (comm * 360) / (m_setWorkload[i] + m_setWorkload[j]);
		return gain;
	}

	int getFlatNodeId(FlatNode *faltNode, vector<FlatNode *> flatNodes) 
	{
		//����flatnode�ҵ�����vector�еı�ţ�һ�����ֱ�ʶ��
		for (int i = 0; i < flatNodes.size(); i++)
			if (faltNode == flatNodes[i])
				return i;
	}

	/*�ж�actor�Ƿ��ڵ�ǰ���ַ�����*/
	bool judgeInCurClusterNode(FlatNode *node)
	{
		if (find(m_flatNodes.begin(), m_flatNodes.end(), node) != m_flatNodes.end())
			return true;
		else
			return false;
	}
public:
		/*��ǰ���������ֺ���ͨ����*/
		long m_allComm;
private:
	int m_nParts;
	int m_acturalParts;
	SchedulerSSG *m_sssg;
	vector<FlatNode *> m_flatNodes;

	/*Ҫ���ֽڵ���ܸ�����ƽ������*/
	long m_allWorkload;
	long m_avgWorkload;

	
	/*���Ӽ����࣬�Լ���ڵ����ߵ��໥ӳ���ϵ*/
	set<ActorSet *> m_sets;
	map<FlatNode *, ActorSet *> m_actor2Set;

	/*���Ӽ��ϵĸ�����ͨ����*/
	vector<long> m_setWorkload, m_setComm;
	vector<vector<int>> m_commofTwoSets;

	/*����ӳ����*/
	map<int, vector<FlatNode*>> m_partNum2FlatNodes;
	map<FlatNode *, int> m_flatNode2PartNum;

	/*�ںϺ�Ļ��ֱ�������Ӽ���֮���ӳ��*/
	map<int, ActorSet*> m_partNum2ActorSets;
	map<ActorSet*, int> m_actorSet2PartNum;

	/*����*/
	vector<int> m_setIdx2ActualIdx;
	map<ActorSet*, int> m_set2Idx;
	map<int, ActorSet*> m_idx2Set;
};



#endif
