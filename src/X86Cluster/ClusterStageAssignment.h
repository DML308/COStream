#ifndef _CLUSTER_STAGEAS_ASSIGNMENT_H_
#define _CLUSTER_STAGEAS_ASSIGNMENT_H_

#include "ClusterPartition.h"

using namespace std;

class ClusterStageAssignment
{
public:
	ClusterStageAssignment(ClusterPartition *);//���캯��

	void CreateStageAssignment();//���ݶ������ֽ��м�Ⱥ��Ľ׶θ���

	multimap<int,FlatNode *> GetCluster2Stage2Actor(int clusterNum);//ȡ��Ⱥ��һ���ڵ�Ľ׶θ��ƵĽ��
	map<FlatNode*, int> GetCluster2Actor2Stage(int clusterNum);

	map<int,multimap<int ,FlatNode *> > GetAllCluster2Stage2Actor();//�������еĽ��
	map<int,map<FlatNode * ,int> > GetAllCluster2Actor2Stage();
	
	vector<FlatNode*> GetActors(int clusterNum,int stage);//�����Ǽ�Ⱥ�ڵ�ı�ţ��׶κ�

	vector<FlatNode *> CreateTopoLogicalOrder(vector<FlatNode *>original);//������������

	void CreateFlatNode2StageMap(map<FlatNode *,int>FlatNode2Core);////������FlatNode��core֮���ӳ��
	
	int GetStageNum(FlatNode*);//���ݽ��Ѱ�������ڵĽ׶κ�
	int GetMaxStageNum(int clusterNum);//����һ̨�����ڵ��ϵ����Ľ׶α��

protected:
	vector<FlatNode *>actorTopoOrder;//���ڴ洢actor����������
	vector<FlatNode *>actorSet;//���ڷ���flatNode���ϵ�
	map<int ,multimap<int,FlatNode *> > cluster2Stage2Actor;//��Ⱥ�ڵ�ı�ţ��׶κţ�actor֮���ӳ��
	map<int ,map<FlatNode*, int> > cluster2Actor2Stage;//��Ⱥ�ڵ�ı�ţ�actor���׶κ�֮���ӳ��
	map<FlatNode *,pair<int, int> > actor2Cluster2stage;//actor ,��Ⱥ�ڵ�ı��,�׶κ�֮���ӳ��
	ClusterPartition *cpartition;//��Ⱥ��·���ֵĽ��
private:
	map<FlatNode *,int>actor2Stage;//������ʱ�洢�׶θ�ֵ�Ľ��
	multimap<int,FlatNode*>stage2Actor;//������ʱ�洢�׶θ�ֵ�Ľ��
};
#endif