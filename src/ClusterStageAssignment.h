#ifndef _CLUSTER_STAGEAS_ASSIGNMENT_H_
#define _CLUSTER_STAGEAS_ASSIGNMENT_H_

#include "ClusterPartition.h"

using namespace std;

class ClusterStageAssignment
{
public:
	ClusterStageAssignment(ClusterPartition *);//构造函数

	void CreateStageAssignment();//根据二级划分进行集群间的阶段复制

	multimap<int,FlatNode *> GetCluster2Stage2Actor(int clusterNum);//取集群上一个节点的阶段复制的结果
	map<FlatNode*, int> GetCluster2Actor2Stage(int clusterNum);

	map<int,multimap<int ,FlatNode *> > GetAllCluster2Stage2Actor();//返回所有的结果
	map<int,map<FlatNode * ,int> > GetAllCluster2Actor2Stage();
	
	vector<FlatNode*> GetActors(int clusterNum,int stage);//参数是集群节点的编号，阶段号

	vector<FlatNode *> CreateTopoLogicalOrder(vector<FlatNode *>original);//创建拓扑序列

	void CreateFlatNode2StageMap(map<FlatNode *,int>FlatNode2Core);////参数是FlatNode与core之间的映射
	
	int GetStageNum(FlatNode*);//根据结点寻找其所在的阶段号
	int GetMaxStageNum(int clusterNum);//返回一台机器节点上的最大的阶段编号

protected:
	vector<FlatNode *>actorTopoOrder;//用于存储actor的拓扑排序
	vector<FlatNode *>actorSet;//用于返回flatNode集合的
	map<int ,multimap<int,FlatNode *> > cluster2Stage2Actor;//集群节点的编号，阶段号，actor之间的映射
	map<int ,map<FlatNode*, int> > cluster2Actor2Stage;//集群节点的编号，actor，阶段号之间的映射
	map<FlatNode *,pair<int, int> > actor2Cluster2stage;//actor ,集群节点的编号,阶段号之间的映射
	ClusterPartition *cpartition;//集群二路划分的结果
private:
	map<FlatNode *,int>actor2Stage;//用于临时存储阶段赋值的结果
	multimap<int,FlatNode*>stage2Actor;//用于临时存储阶段赋值的结果
};
#endif