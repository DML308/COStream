/*
* 异构集群划分方法三：PGTD（Process Group Thread Dynamic Partition）进程Metis划分，线程Metis划分
* 2017/01/10 yrr
*/
#pragma once
#ifndef _HCLUSTER_PGTD_PARTITION_H_
#define _HCLUSTER_PGTD_PARTITION_H_
#include "HClusterBackend.h"
#include "HClusterPartition.h"
#include "HClusterProcessGroupPartition.h"
#include "HClusterThreadDynamicPartition.h"

/*PGTD二级任务划分算法类定义*/
class HClusterPGTDPartition :public HClusterPartition {
public:
	/*构造函数*/
	HClusterPGTDPartition(int _nClusters, map<int, std::pair<int, int>> hCluster2CpuAndGpu, SchedulerSSG *sssg) :
		HClusterPartition(_nClusters, hCluster2CpuAndGpu, sssg)
	{
		m_allComm = 0;
		m_hpg = new HClusterProcessGroupPartition(sssg, _nClusters);
		for (int i = 0; i < _nClusters; ++i)
		{
			HClusterThreadDynamicPartition *htd1,*htd2 = NULL;
			m_htds.push_back(make_pair(htd1,htd2));
		}

		/*设置二级划分结果容量*/
		m_coreNum2FlatNodes = vector<map<int, vector<FlatNode *>>>(m_nClusters, map<int, vector<FlatNode *>>());
		m_coreNum2FlatNodes.resize(m_nClusters);

		m_flatNodes2CoreNum = vector<map<FlatNode*, int>>(m_nClusters, map<FlatNode*, int>());
		m_flatNodes2CoreNum.resize(m_nClusters);


		/*针对Gpu端无状态节点的特殊处理，非混合架构存储空*/
		m_statelessNode2GpuNum = vector<map<FlatNode*, int>>(m_nClusters, map<FlatNode*, int>());
		m_gpuNum2StatelessNodes = vector<map<int, vector<FlatNode*>>>(m_nClusters, map<int, vector<FlatNode*>>());
	}

	/*虚析构函数*/
	virtual ~HClusterPGTDPartition() {}
	/*实际划分操作*/
	void SssgHClusterPartition();

	/*返回指定actor的稳态执行次数*/
	int getSteadyCount(FlatNode *flatNode)
	{
		return m_sssg->GetSteadyCount(flatNode);
	}

private:
	/*组划分成员对象*/
	HClusterProcessGroupPartition *m_hpg;

	/*动态划分成员对象(个数为集群节点数)*/
	vector<pair<HClusterThreadDynamicPartition*,HClusterThreadDynamicPartition*> > m_htds;

	/*二级划分*/
	void processPartition();
	void threadPartition();

};


#endif