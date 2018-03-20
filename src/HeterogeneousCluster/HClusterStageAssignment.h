/*
* �칹��Ⱥ�׶θ�ֵ
* 2016/06/06 yrr
*/

#ifndef _HCLUSTER_STAGE_ASSIGNMENT_H
#define  _HCLUSTER_STAGE_ASSIGNMENT_H

#include "HClusterPartition.h"
#include "../schedulerSSG.h"
#include "../flatNode.h"


class HClusterStageAssignment {
public:
	/*���캯��*/
	HClusterStageAssignment(HClusterPartition *hcp, SchedulerSSG *sssg);

	/*�����칹��Ⱥ�������񻮷ֽ��н׶θ�ֵ
	* (1) ���ڷ������ڵ㣬�׶θ�ֵ
	*/
	void createStageAssignment();
	void printStageResult();

	map<FlatNode*, int> createFlatNode2StageForCPU(map<FlatNode*, int> &flatNode2CoreNum, int hClusterNum);
	map<FlatNode*, int> createFlatNode2StageForGPU(map<FlatNode*, int> &flatNode2GpuNum, map<FlatNode*, int> &statefulNodes, int hClusterNum);
	//��SDF�ڵ㼯����������
	vector<FlatNode*> createTopoLogicalOrder(vector<FlatNode*> &flatNodes);

	//�õ�ָ���ڵ�Ľ׶κ�
	int getStageNum(FlatNode *node);

	//�õ�ָ����Ⱥ�ڵ�����׶κ�
	int getMaxStageNum(int hClusterNum);

	//�õ�ָ���ڵ�ļ�Ⱥ���
	int getHClusterNum(FlatNode *node);

	//�õ�ָ����Ⱥ�ڵ�Ľ׶θ�ֵӳ��
	map<FlatNode *, int> getFlatNode2StageOfHCluster(int hClusterNum);
	multimap<int, FlatNode*> getStage2FlatNodesOfHCluster(int hClusterNum);
	vector<FlatNode *> getFlatNodesOfHClusterAndStage(int hClusterNum, int stage);

	//��Ⱥ�ڵ��ŵ��׶κŵ�actor��ӳ��
	map<int, multimap<int, FlatNode *>> getHCluster2Stage2FlatNode() {
		return m_hCluster2StageNum2FlatNode;
	}
	/*��Ⱥ�ڵ��ŵ�actor���׶κŵ�ӳ��*/
	map<int, map<FlatNode *, int>> getHCluster2FlatNode2Stage()
	{
		return m_hCluster2FlatNode2StageNum;
	}

	/*actor����Ⱥ�ڵ㵽�׶κŵ�ӳ��*/
	map<FlatNode *, pair<int, int>> getFlatNode2HCluster2Stage() {
		return m_flatNode2hCluster2StageNum;
	}

protected:
	HClusterPartition *m_hcp;
	SchedulerSSG *m_sssg;
	/*��Ⱥ�ڵ��ŵ��׶κŵ�actor��ӳ��*/
	map<int,multimap<int, FlatNode *>> m_hCluster2StageNum2FlatNode;
	/*��Ⱥ�ڵ��ŵ�actor���׶κŵ�ӳ��*/
	map<int,map<FlatNode *, int>> m_hCluster2FlatNode2StageNum;
	/*actor����Ⱥ�ڵ㵽�׶κŵ�ӳ��*/
	map<FlatNode *, pair<int, int>> m_flatNode2hCluster2StageNum;
public:
	vector<FlatNode *> m_flatNodes;	//SDF�ڵ㼯��
	vector<FlatNode *> m_topoFlatNodes;	//�����������SDF�ڵ㼯��

private:
	vector<FlatNode *> m_actorSet;//���ڷ���flatNode���ϵ�
};
#endif