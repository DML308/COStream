/*
�칹��Ⱥ�Ľ��̼��黮���㷨 2016/12/27 yrr
*/
#pragma once
#ifndef _HCLUSTER_PROCESS_GROUP_PARTITION_H
#define _HCLUSTER_PROCESS_GROUP_PARTITION_H

#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <iterator>
#include <utility>
#include <algorithm>

#include "../schedulerSSG.h"
#include "../flatNode.h"

//ƽ������:���̼��ĳ�ʼ������ʹ��
#define MINBALANCEFACTOR 0.8
#define MAXBALANCEFACTOR 1.2
//ƽ������:�̼߳���ˮƽ�������õ�
#define BALANCE_FACTOE 1.2

using namespace std;

//���ôֻ�ʱgroup�������������ӡ����մֻ��ڵ����ĿΪgroup����Ŀ * 32(����32�Ǿ���ֵ)
const int COARSEN_FACTOR = 32;

//��������ͨ�ŵĿ�������Ƶ��2.4��ʵ��ֵ350-380֮�䣬��Ƶ��2.5��ʵ��ֵ1800-200֮�䣩
const int OVERHEAD_PER_COMMUNICATION_DATA = 360;
const double EXP = 0.0000000001;

class FlatNodeGroup {
private:
	SchedulerSSG *m_sssg;
	set<FlatNode *> m_flatNodes;

	long m_workload;//��ǰgroup�ĸ���
	int m_totalData;//��ǰgroup������group��ͨ����������

	std::map<FlatNodeGroup *, int> m_upGroup2Weight;//��ǰgroup���϶�group��ͨ��������֮���map
	std::map<FlatNodeGroup *, int> m_downGroup2Weight;//��ǰgroup���¶�group��ͨ��������֮���map
public:
	FlatNodeGroup(set<FlatNode*>, SchedulerSSG *);
	int computeTotalCommunicateData();
	inline long getWorkload();
	int getTotalCommunicateData()
	{
		return this->m_totalData;
	}
	int getCurCommunicateData(FlatNodeGroup *);

	/*ȡ��ǰ���actor����*/
	inline set<FlatNode*> getFlatNodes() {
		return m_flatNodes;
	}


	std::map<FlatNodeGroup*, int> getUpGroup2Weight()//ȡ��ǰgroup������upGroup2Weight
	{
		return this->m_upGroup2Weight;
	}
	std::map<FlatNodeGroup*, int> getDownGroup2Weight()//ȡ��ǰgroup������downGroup2Weight
	{
		return this->m_downGroup2Weight;
	}

	FlatNodeGroup *fusingGroup(FlatNodeGroup *);//��ǰ��group����һ��group�ں�
	void insertupGroup(FlatNodeGroup *);//��ǰ��group���϶˽ڵ����group
	void insertdownGroup(FlatNodeGroup*);//��ǰgroup���¶˽ڵ����group
	void deleteupGroup(FlatNodeGroup *objGroup)//�ڵ�ǰgroup��upGroup��ɾ��ָ����group���ɹ�ɾ��������
	{
		std::map<FlatNodeGroup *, int>::const_iterator const_iter = m_upGroup2Weight.find(objGroup);
		if (const_iter != m_upGroup2Weight.end())
		{
			m_totalData -= const_iter->second;
			m_upGroup2Weight.erase(objGroup);
		}

	}
	void deletedownGroup(FlatNodeGroup *objGroup)//�ڵ�ǰgroup��downGroup��ɾ��ָ����group���ɹ�ɾ��������
	{
		std::map<FlatNodeGroup *, int>::const_iterator const_iter = m_downGroup2Weight.find(objGroup);
		if (const_iter != m_downGroup2Weight.end())
		{
			m_totalData -= const_iter->second;
			m_downGroup2Weight.erase(objGroup);
		}
	}
	std::vector<FlatNodeGroup* > getUpGroup();//ȡ��ǰgroup������upGroup
	std::vector<FlatNodeGroup* > getDownGroup();//ȡ��ǰgroup������downGroup	
	void insertFlatNode(FlatNode *);//��FlatNodeGroup�����һ��flatNode�ڵ�,ͬʱ��Ҫά��group������ӹ�ϵ
	void deleteFlatNode(FlatNode *);//��FlatNodeGroupɾ��һ��flatNode�ڵ�,ͬʱ��Ҫά��group������ӹ�ϵ
	
	~FlatNodeGroup() {}
};

/*���̵��黮���㷨*/
class HClusterProcessGroupPartition {
public:
	HClusterProcessGroupPartition(SchedulerSSG *_sssg, int _nClusters);
	~HClusterProcessGroupPartition();
	map<int, vector<FlatNode*>> processPartition();

private:
	void constructGroups();//����sssg�����ʼ��group 
	long computeWorkload(std::vector<FlatNode *>);//����vector������flatNode�Ĺ��������ܺ�
	void preProcess();//���齨�ĳ�ʼ��group����Ԥ����
	void coarseGrainedPartition();//�ֻ��׶Ρ������ѡ��group��һ���ǽ�ͨ�ű�����group����һ����һ���ǰ�����ͨ�űȣ�������������һ��group����һ��
	void initialPartition();//��ʼ���֡������ֻ����ͼ���ֳ�ncluster�ݣ�һ���Ǽ����ֻ�����һ����ʹ�ö��2·�����㷨��
	void fineGrainedPartition();//����ɳ�ʼ���ֺ��ͼ���е��������ȱ�����ʼ������ɺ��ÿһ��cluster�����е�group������ÿ��group�����ͨ������ڵ�ͨ�ŵĲ����ڵ�ĵ�ͨ�Ų�ֹһ��ѡ����һ���������뵽һ�����ȶ�����
						 //Ȼ������ȶ�����ѡȡgroup�����ƶ�������������ж�Ҫ��Ҫ�ƶ�����������Ҫ����2������ͨ�еļ��٣�����ƽ���Ӱ�죩
	void adjustBoundaryFlatNodeWorkload();
	void updateFlatNodeWorkloadSSG(std::vector<FlatNode *> flatNodes);

	/*********************************************************/
	
	std::vector<FlatNodeGroup *>  topoSortGroup(std::vector<FlatNodeGroup *>);//�ж�group��ͼ�Ƿ�����������
	Bool topoSortGroup(int group1, int group2, int nsize);//�����ڽӾ�������������
	void updateGroupGraph(std::map<FlatNodeGroup *, std::set<FlatNodeGroup *> >Preprocess);//����Ԥ����Ľ���޸�GroupGraph
	void updateAdjMatrixInfo(int group1, int group2, int **adjMatrix, int size);//����ÿ����group�Ľ�������ڽӾ���
	void updataGroupGraphByCoarsening(int original_size);//���ݴֻ��Ľ���޸�GroupGraph
	void refiningPhase_CollectBoundFlatNode(std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> > &gain2FlatNode2AdjPT);
	Bool refiningPhase_IsAdjustableFlatNode(FlatNode *flatNode, FlatNodeGroup* objGroup, int gain);
	void refiningPhase_UpdatePriQueue(FlatNode *flatNode, std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> > &gain2FlatNode2AdjPT);
	void refiningPhase_RemoveOldGain(FlatNode *flatNode, std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, FlatNodeGroup* > > &gain2FlatNode2AdjPT);
	void movingFlatNodeInterGroup(FlatNode*, FlatNodeGroup*, FlatNodeGroup*);//��flatNode��һ��group�ƶ�����һ��group
	
	
	void insertNewFlatNodeGain(FlatNode *flatNode, FlatNodeGroup *curGroup, std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> > &gain2FlatNode2AdjPT);
	void constructAdjMatrixInfo();//���ݳ�ʼ��group��ϵ�����ڽӾ����Լ���Ҫ�õ��ļ�¼��ص���Ϣ�Ľṹ
	float computeGain(int group1No, int group2No);//����group1��group2�ı�ż��������ںϲ���������

private:
	SchedulerSSG *m_sssg;
	int m_nClusters;
	long m_avgWorkload;
	set<FlatNodeGroup *> m_groups;
	map<FlatNode *, FlatNodeGroup *> m_flatNode2group;//��¼flatNode��group֮���map
	std::map<FlatNodeGroup *, int> m_group2groupNo;//group�����Ӧ�ı��
	std::map<int, FlatNodeGroup *> m_groupNo2group;//group��Ŷ�Ӧ��group
	std::vector<int> m_groupNo2actualNo;//group��ʼʱ�ı�����ڱ������Ƕ�Ӧ��ʵ�ʵı�ţ�key-value�����±��Ǿɵ�group��ţ�ֵ���µ�group���
	int **m_groupAdjMatrix;//��¼group֮����ڽӹ�ϵ�������е�ʵ��ֵ��ʾgroup֮��ͨ������Ϊ0��ʾ����֮��û��ֱ��������ϵ����Ԥ������ɺ�Ż��õ�
	int **m_groupAdjMatrix_copy;//_groupAdjMatrix�ĸ����������������޸��õ�
	std::vector<long> m_groupWorkload;//��¼group�ĸ���
	std::vector<int> m_groupCommunication;//��¼����group�Ķ����ͨ����
	std::vector<FlatNodeGroup *> m_cluster2group;//��Ⱥ����뻮��֮���map
												//���������ṹ������ɳ�ʼ���ֺ�ʹ�õ�
	std::multimap<int, FlatNode *> m_cluster2flatNode;//cluster��flatNode֮���map�����ڼ�¼flatNode����һ��cluster�ڵ��ϵ�
	std::map<FlatNode *, int> m_flatNode2cluster;//flatNode ��cluster֮���map
};
#endif