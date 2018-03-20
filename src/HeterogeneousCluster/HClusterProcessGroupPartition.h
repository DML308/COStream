/*
异构集群的进程级组划分算法 2016/12/27 yrr
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

//平衡因子:进程级的初始划分中使用
#define MINBALANCEFACTOR 0.8
#define MAXBALANCEFACTOR 1.2
//平衡因子:线程级的水平分裂中用到
#define BALANCE_FACTOE 1.2

using namespace std;

//设置粗化时group数量的下限因子—最终粗化节点的数目为group的数目 * 32(其中32是经验值)
const int COARSEN_FACTOR = 32;

//单个数据通信的开销（主频是2.4的实验值350-380之间，主频是2.5的实验值1800-200之间）
const int OVERHEAD_PER_COMMUNICATION_DATA = 360;
const double EXP = 0.0000000001;

class FlatNodeGroup {
private:
	SchedulerSSG *m_sssg;
	set<FlatNode *> m_flatNodes;

	long m_workload;//当前group的负载
	int m_totalData;//当前group与其他group的通信数据总量

	std::map<FlatNodeGroup *, int> m_upGroup2Weight;//当前group与上端group的通信数据量之间的map
	std::map<FlatNodeGroup *, int> m_downGroup2Weight;//当前group与下端group的通信数据量之间的map
public:
	FlatNodeGroup(set<FlatNode*>, SchedulerSSG *);
	int computeTotalCommunicateData();
	inline long getWorkload();
	int getTotalCommunicateData()
	{
		return this->m_totalData;
	}
	int getCurCommunicateData(FlatNodeGroup *);

	/*取当前组的actor集合*/
	inline set<FlatNode*> getFlatNodes() {
		return m_flatNodes;
	}


	std::map<FlatNodeGroup*, int> getUpGroup2Weight()//取当前group的所有upGroup2Weight
	{
		return this->m_upGroup2Weight;
	}
	std::map<FlatNodeGroup*, int> getDownGroup2Weight()//取当前group的所有downGroup2Weight
	{
		return this->m_downGroup2Weight;
	}

	FlatNodeGroup *fusingGroup(FlatNodeGroup *);//当前的group与另一个group融合
	void insertupGroup(FlatNodeGroup *);//向当前的group的上端节点添加group
	void insertdownGroup(FlatNodeGroup*);//向当前group的下端节点添加group
	void deleteupGroup(FlatNodeGroup *objGroup)//在当前group的upGroup中删除指定的group，成功删除返回真
	{
		std::map<FlatNodeGroup *, int>::const_iterator const_iter = m_upGroup2Weight.find(objGroup);
		if (const_iter != m_upGroup2Weight.end())
		{
			m_totalData -= const_iter->second;
			m_upGroup2Weight.erase(objGroup);
		}

	}
	void deletedownGroup(FlatNodeGroup *objGroup)//在当前group的downGroup中删除指定的group，成功删除返回真
	{
		std::map<FlatNodeGroup *, int>::const_iterator const_iter = m_downGroup2Weight.find(objGroup);
		if (const_iter != m_downGroup2Weight.end())
		{
			m_totalData -= const_iter->second;
			m_downGroup2Weight.erase(objGroup);
		}
	}
	std::vector<FlatNodeGroup* > getUpGroup();//取当前group的所有upGroup
	std::vector<FlatNodeGroup* > getDownGroup();//取当前group的所有downGroup	
	void insertFlatNode(FlatNode *);//向FlatNodeGroup中添加一个flatNode节点,同时还要维护group间的连接关系
	void deleteFlatNode(FlatNode *);//从FlatNodeGroup删除一个flatNode节点,同时还要维护group间的连接关系
	
	~FlatNodeGroup() {}
};

/*进程的组划分算法*/
class HClusterProcessGroupPartition {
public:
	HClusterProcessGroupPartition(SchedulerSSG *_sssg, int _nClusters);
	~HClusterProcessGroupPartition();
	map<int, vector<FlatNode*>> processPartition();

private:
	void constructGroups();//根据sssg构造初始的group 
	long computeWorkload(std::vector<FlatNode *>);//计算vector中所有flatNode的工作量的总和
	void preProcess();//对组建的初始的group进行预处理
	void coarseGrainedPartition();//粗化阶段——如何选择group（一种是将通信边最大的group组在一起；另一中是按计算通信比，让其收益最大的一组group组在一起）
	void initialPartition();//初始划分——将粗化后的图划分成ncluster份（一种是继续粗化，另一种是使用多层2路划分算法）
	void fineGrainedPartition();//对完成初始划分后的图进行调整——先遍历初始划分完成后的每一个cluster中所有的group，计算每个group对外的通信与对内的通信的差（对外节点的的通信不止一个选最大的一个），插入到一个优先队列中
						 //然后从优先队列中选取group考察移动后带来的收益判定要不要移动。（收益主要考察2个方面通行的减少，负载平衡的影响）
	void adjustBoundaryFlatNodeWorkload();
	void updateFlatNodeWorkloadSSG(std::vector<FlatNode *> flatNodes);

	/*********************************************************/
	
	std::vector<FlatNodeGroup *>  topoSortGroup(std::vector<FlatNodeGroup *>);//判断group的图是否有拓扑排序
	Bool topoSortGroup(int group1, int group2, int nsize);//根据邻接矩阵做拓扑排序
	void updateGroupGraph(std::map<FlatNodeGroup *, std::set<FlatNodeGroup *> >Preprocess);//根据预处理的结果修改GroupGraph
	void updateAdjMatrixInfo(int group1, int group2, int **adjMatrix, int size);//根据每次组group的结果更新邻接矩阵
	void updataGroupGraphByCoarsening(int original_size);//根据粗化的结果修改GroupGraph
	void refiningPhase_CollectBoundFlatNode(std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> > &gain2FlatNode2AdjPT);
	Bool refiningPhase_IsAdjustableFlatNode(FlatNode *flatNode, FlatNodeGroup* objGroup, int gain);
	void refiningPhase_UpdatePriQueue(FlatNode *flatNode, std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> > &gain2FlatNode2AdjPT);
	void refiningPhase_RemoveOldGain(FlatNode *flatNode, std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, FlatNodeGroup* > > &gain2FlatNode2AdjPT);
	void movingFlatNodeInterGroup(FlatNode*, FlatNodeGroup*, FlatNodeGroup*);//将flatNode从一个group移动到另一个group
	
	
	void insertNewFlatNodeGain(FlatNode *flatNode, FlatNodeGroup *curGroup, std::map<std::pair<FlatNode *, FlatNodeGroup*>, int > &flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode *, FlatNodeGroup*> > &gain2FlatNode2AdjPT);
	void constructAdjMatrixInfo();//根据初始的group关系构造邻接矩阵，以及需要用到的记录相关的信息的结构
	float computeGain(int group1No, int group2No);//利用group1和group2的编号计算他们融合产生的收益

private:
	SchedulerSSG *m_sssg;
	int m_nClusters;
	long m_avgWorkload;
	set<FlatNodeGroup *> m_groups;
	map<FlatNode *, FlatNodeGroup *> m_flatNode2group;//记录flatNode与group之间的map
	std::map<FlatNodeGroup *, int> m_group2groupNo;//group与其对应的编号
	std::map<int, FlatNodeGroup *> m_groupNo2group;//group编号对应其group
	std::vector<int> m_groupNo2actualNo;//group初始时的编号与在被处理是对应的实际的编号（key-value），下标是旧的group编号，值是新的group编号
	int **m_groupAdjMatrix;//记录group之间的邻接关系，数组中的实际值表示group之间通信量，为0表示他们之间没有直接依赖关系——预处理完成后才会用到
	int **m_groupAdjMatrix_copy;//_groupAdjMatrix的副本——在拓扑是修改用的
	std::vector<long> m_groupWorkload;//记录group的负载
	std::vector<int> m_groupCommunication;//记录各个group的对外的通行量
	std::vector<FlatNodeGroup *> m_cluster2group;//集群编号与划分之间的map
												//下面两个结构是在完成初始划分后使用的
	std::multimap<int, FlatNode *> m_cluster2flatNode;//cluster与flatNode之间的map，用于记录flatNode在哪一个cluster节点上的
	std::map<FlatNode *, int> m_flatNode2cluster;//flatNode 与cluster之间的map
};
#endif