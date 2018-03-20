/*
异构集群平台下Metis划分算法实现类 -- 可复用
*/

#pragma once
#ifndef HCLUSTER_METIS_PARTITION_H
#define HCLUSTER_METSI_PARTITION_H

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

using namespace std;

class HClusterMetisPartition {
public:
	HClusterMetisPartition(){}
	HClusterMetisPartition(int _nparts, SchedulerSSG *sssg, vector <FlatNode*> _flatNodes);
	~HClusterMetisPartition(){}

	/*线程级划分函数*/
	map<int, vector<FlatNode*>> threadPartition();

	void initPartitionInfo();
	bool metisPartition();
	void extractPartitionResult();
	/*void statefulNodesAdjustment();
	void statefulNodesPartition();*/
	int getFlatNodeId(FlatNode *faltNode, vector<FlatNode *> flatNodes);

private:
	int m_nParts;
	SchedulerSSG *m_sssg;
	vector<FlatNode *> m_flatNodes;


	/*划分映射结果*/
	map<int, vector<FlatNode*>> m_partNum2FlatNodes;
	map<FlatNode *, int> m_flatNode2PartNum;

	/*要划分节点的信息*/
	map<FlatNode*, long> m_flatNode2Workload;
	map<FlatNode *, int> m_flatNode2SteadyCounts;
	
	///*有状态节点单独提取出来*/
	//vector<FlatNode*> m_statefulNodes;
	//map<int, vector<FlatNode*>> m_partNum2StatefulNodes;
	//map<FlatNode*, int> m_statefulNode2PartNum;

	//===============================================================
	//为使用metis以及分裂所用的数据成员
	int *mxadj;//定义指针指向xadj数组
	int *madjncy;//定义指针指向adjncy数组
	int *mobjval;//定义指针指向objval
	std::vector<int> mpart;//定义指针指向part数组 ——存放的是metis最终的划分结果：下标是flatNode编号，值是所在核的编号
	int *mvwgt;//定义指针指向vwgt数组，后者存储每个顶点的权值
	int *madjwgt;//定义指针指向adjwgt数组
	int nvtxs;//定义顶点个数
	float *tpwgts;
	int *mvsize;//定义指针指向vsize数组
	float *ubvec;
	int mncon;
	int objval;
	int options[40];//参数数组
	std::vector<int>_xadj;//动态定义xadj数组——表示adjncy中从xadj[i]下标开始到xadj[i+1]下标结束是i结点的上下端节点
	std::vector<int>_vwgt; //节点的工作量
	std::vector<int>_vsize;//节点总的通信量
	std::vector<int>_adjncy;
};

#endif // !HCLUSTER_METIS_PARTITION_H
