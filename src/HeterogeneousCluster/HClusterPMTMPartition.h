/*
* 异构集群划分方法一：PMTM（Process Metis Thread Metis Partition）进程Metis划分，线程Metis划分
* 2016/08/27 yrr
*/
#pragma once
#ifndef _HCLUSTER_PMTM_PARTITION_H
#define _HCLUSTER_PMTM_PARTITION_H

#include "HClusterBackend.h"
#include "HClusterPartition.h"
#include "HClusterMetisPartition.h"


/*PGTM划分方法继承自基类HClusterPartition*/
class HClusterPMTMPartition :public HClusterPartition {
public:
	/*构造函数*/
	HClusterPMTMPartition(int _nClusters, map<int, std::pair<int, int>> hCluster2CpuAndGpu, SchedulerSSG *sssg) :
		HClusterPartition(_nClusters, hCluster2CpuAndGpu,sssg)
	{
		m_allComm = 0;
		vector<FlatNode*> flatNodes = sssg->flatNodes;
		m_hmp = new HClusterMetisPartition(_nClusters, sssg,flatNodes);
		for (int i = 0; i < _nClusters; ++i)
		{
			HClusterMetisPartition *hmp1,*hmp2 = NULL;
			m_hmps.push_back(make_pair(hmp1,hmp2));
		}

		/*设置二级划分结果容量*/
		m_coreNum2FlatNodes = vector<map<int, vector<FlatNode *>>>(m_nClusters, map<int, vector<FlatNode *>>());
		m_coreNum2FlatNodes.resize(m_nClusters);

		m_flatNodes2CoreNum = vector<map<FlatNode*, int>>(m_nClusters, map<FlatNode*, int>());
		m_flatNodes2CoreNum.resize(m_nClusters);

		/*针对GPU端的无状态节点的特殊处理，非混合架构存储空*/
		m_statelessNode2GpuNum = vector<map<FlatNode*, int>>(m_nClusters, map<FlatNode*, int>());
		m_gpuNum2StatelessNodes = vector<map<int, vector<FlatNode*>>>(m_nClusters, map<int, vector<FlatNode*>>());
	}

	/*虚析构函数*/
	virtual ~HClusterPMTMPartition() {}

	/*实际划分操作*/
	void SssgHClusterPartition();

	/*返回指定actor的稳态执行次数*/
	int getSteadyCount(FlatNode *flatNode)
	{
		return m_sssg->GetSteadyCount(flatNode);
	}
	/*计算二级任务划分后的总通信量*/
	void computeAllComm();
private:
	/*Metis划分成员对象*/
	HClusterMetisPartition *m_hmp;
	/*Metis划分成员对象(个数为集群节点数)*/
	vector<pair<HClusterMetisPartition*,HClusterMetisPartition*> > m_hmps;
	/*二级划分*/
	void processPartition();
	void threadPartition();
};
#endif
