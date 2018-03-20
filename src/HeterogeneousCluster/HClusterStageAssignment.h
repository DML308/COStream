/*
* 异构集群阶段赋值
* 2016/06/06 yrr
*/

#ifndef _HCLUSTER_STAGE_ASSIGNMENT_H
#define  _HCLUSTER_STAGE_ASSIGNMENT_H

#include "HClusterPartition.h"
#include "../schedulerSSG.h"
#include "../flatNode.h"


class HClusterStageAssignment {
public:
	/*构造函数*/
	HClusterStageAssignment(HClusterPartition *hcp, SchedulerSSG *sssg);

	/*根据异构集群二级任务划分进行阶段赋值
	* (1) 对于服务器节点，阶段赋值
	*/
	void createStageAssignment();
	void printStageResult();

	map<FlatNode*, int> createFlatNode2StageForCPU(map<FlatNode*, int> &flatNode2CoreNum, int hClusterNum);
	map<FlatNode*, int> createFlatNode2StageForGPU(map<FlatNode*, int> &flatNode2GpuNum, map<FlatNode*, int> &statefulNodes, int hClusterNum);
	//对SDF节点集合拓扑排序
	vector<FlatNode*> createTopoLogicalOrder(vector<FlatNode*> &flatNodes);

	//得到指定节点的阶段号
	int getStageNum(FlatNode *node);

	//得到指定集群节点的最大阶段号
	int getMaxStageNum(int hClusterNum);

	//得到指定节点的集群编号
	int getHClusterNum(FlatNode *node);

	//得到指定集群节点的阶段赋值映射
	map<FlatNode *, int> getFlatNode2StageOfHCluster(int hClusterNum);
	multimap<int, FlatNode*> getStage2FlatNodesOfHCluster(int hClusterNum);
	vector<FlatNode *> getFlatNodesOfHClusterAndStage(int hClusterNum, int stage);

	//集群节点编号到阶段号到actor的映射
	map<int, multimap<int, FlatNode *>> getHCluster2Stage2FlatNode() {
		return m_hCluster2StageNum2FlatNode;
	}
	/*集群节点编号到actor到阶段号的映射*/
	map<int, map<FlatNode *, int>> getHCluster2FlatNode2Stage()
	{
		return m_hCluster2FlatNode2StageNum;
	}

	/*actor到集群节点到阶段号的映射*/
	map<FlatNode *, pair<int, int>> getFlatNode2HCluster2Stage() {
		return m_flatNode2hCluster2StageNum;
	}

protected:
	HClusterPartition *m_hcp;
	SchedulerSSG *m_sssg;
	/*集群节点编号到阶段号到actor的映射*/
	map<int,multimap<int, FlatNode *>> m_hCluster2StageNum2FlatNode;
	/*集群节点编号到actor到阶段号的映射*/
	map<int,map<FlatNode *, int>> m_hCluster2FlatNode2StageNum;
	/*actor到集群节点到阶段号的映射*/
	map<FlatNode *, pair<int, int>> m_flatNode2hCluster2StageNum;
public:
	vector<FlatNode *> m_flatNodes;	//SDF节点集合
	vector<FlatNode *> m_topoFlatNodes;	//经拓扑排序的SDF节点集合

private:
	vector<FlatNode *> m_actorSet;//用于返回flatNode集合的
};
#endif