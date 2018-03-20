/*
 * 异构集群二级任务划分头文件 -- 所有划分方法的基类 2016/04/29 yrr
 */

#pragma once
#ifndef H_CLUSTER_PARTITION_H
#define H_CLUSTER_PARTITION_H

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <iterator>
#include <utility>
#include <algorithm>

#include "../schedulerSSG.h"
#include "../staticStreamGraph.h"
#include "../flatNode.h"
#include "../ActorStateDetector.h"

using namespace std;

class HClusterPartition {
public:
	//显示设置默认构造函数
	HClusterPartition() {}
	/*构造函数的实现，默认集群只有一个机器节点*/
	HClusterPartition(int _nClusters, map<int, std::pair<int, int>> hCluster2CpuAndGpu, SchedulerSSG *sssg)
		:m_nClusters(_nClusters), m_hCluster2CpuAndGpu(hCluster2CpuAndGpu), m_allWorkLoad(0),m_sssg(sssg) 
	{
		vector<FlatNode*>flatNodes = m_sssg->flatNodes;
		this->m_allWorkLoad = computeWorkload(flatNodes);
		this->m_avgWorkLoad = computeWorkload(flatNodes) / m_nClusters;//计算理论上的平均负载

		m_statelessNodes = vector<vector<FlatNode*>>(m_nClusters, vector<FlatNode*>());
		m_statefulNodes = vector<vector<FlatNode*>>(m_nClusters, vector<FlatNode*>());
	}

	/*作为基类，设置虚析构函数*/
	virtual ~HClusterPartition() {}

	/*判断当前服务器节点是否含有GPU*/
	bool isGPUClusterNode(int hClusterIdx) {
		return m_hCluster2CpuAndGpu[hClusterIdx].second > 0 ? true : false;
	}

	//得到异构集群节点个数
	inline int getHClusters()	const { return m_nClusters; } 

	/*稳态调度次数，由具体的划分子类实现*/
	virtual int getSteadyCount(FlatNode *flatNode) = 0;
	/*具体划分算法，由子类实现*/
	virtual void SssgHClusterPartition() = 0;

	/*打印集群进程级划分结果*/
	void printPGResult();
	/*打印集群线程级划分结果*/
	void printTPResult();

	/*返回划分映射结果*/
	map<int, vector<FlatNode *>> getHClusterNum2FlatNodes() { return m_hClusterNum2FlatNodes; }
	map<FlatNode *, int> getFlatNode2hClusterNum() { return m_flatNode2hClusterNum; }

	long computeWorkload(std::vector<FlatNode *>);//计算vector中所有flatNode的工作量的总和
	/*得到工作整体和平均负载*/
	inline double getAllWorkLoad() { return m_allWorkLoad; }
	inline double getAvgWorkLoad() { return m_avgWorkLoad; }

	/*得到指定服务器节点的负载*/
	inline double getWeightOfHCluster(int hClusterNum) 
	{ 
		assert(!m_hCluster2Weight.empty()); 
		if (m_hCluster2Weight.empty())
			return 0;
		return m_hCluster2Weight[hClusterNum]; 
	}

	/*得到指定服务器节点上指定核上的负载*/
	inline double getWeightOfCoreOfHCluster(int hClusterNum, int coreNum) 
	{ 
		if (m_hCluster2Weight.empty() || m_hCluster2Core2Weight.empty())
			return 0;
		return m_hCluster2Core2Weight[hClusterNum][coreNum]; 
	}

	inline double getWeightOfGpuOfHCluster(int hClusterNum, int gpuNum)
	{
		if (m_hCluster2Weight.empty() || m_hCluster2Gpu2Weight.empty())
			return 0;
		return m_hCluster2Gpu2Weight[hClusterNum][gpuNum];
	}


	inline void setHCluster2Weight(map<int, double> hCluster2Weight) { this->m_hCluster2Weight = hCluster2Weight; }
	inline map<int, double> getHCluster2Weight() { return m_hCluster2Weight; }
	
	inline void setHCluster2Core2Weight(map<int,map<int, double>> hCluster2Core2Weight) { this->m_hCluster2Core2Weight = hCluster2Core2Weight; }
	inline void setHCluster2Gpu2Weight(map<int, map<int, double>> hCluster2Gpu2Weight) { this->m_hCluster2Gpu2Weight = hCluster2Gpu2Weight; }
	inline map<int, map<int, double>> getHCluster2Core2Weight() { return m_hCluster2Core2Weight; }
	inline map<int, map<int, double>> getHCluster2Gpu2Weight() { return m_hCluster2Gpu2Weight; }

	/*返回指定集群节点对应的划分子图节点*/
	vector<FlatNode *> getSubFlatNodes(int hClusterNum);
	
	/*得到指定节点的服务器编号*/
	int getHClusterNumOfNode(FlatNode *flatNode);

	/*求指定SDF图节点对应的集群机器编号和CPU核编号*/
	pair<int,int> getHClusterCoreNum(FlatNode *flatNode);
	pair<int, int> getHClusterGpuNum(FlatNode *flatNode);


	/*给定集群节点编号，返回该服务器的二级划分映射*/
	map<int, vector<FlatNode*>> getHClusterCore2FlatNodes(int clusterId)
	{
		assert(clusterId >= 0 && clusterId < m_nClusters);
		return m_coreNum2FlatNodes[clusterId];
	}
	map<FlatNode*, int> getFlatNode2HClusterCore(int clusterId)
	{
		assert(clusterId >= 0 && clusterId < m_nClusters);
		return m_flatNodes2CoreNum[clusterId];
	}

	vector<FlatNode *> getStatelessNodes(int hClusterId)
	{
		assert(hClusterId >= 0 && hClusterId < m_nClusters);
		return m_statelessNodes[hClusterId];
	}

	map<int, vector<FlatNode *>> getHClusterGpu2StatelessNodes(int hClusterId)
	{
		assert(hClusterId >= 0 && hClusterId < m_nClusters);
		return m_gpuNum2StatelessNodes[hClusterId];
	}

	map<FlatNode*, int> getStatelessNode2GpuNum(int hClusterId)
	{
		assert(hClusterId >= 0 && hClusterId < m_nClusters);
		return m_statelessNode2GpuNum[hClusterId];
	}

	/*判断node节点划分到混合架构服务器的GPU端还是CPU端*/
	bool isNodeInGPU(int hClusterNum, FlatNode *node)
	{
		/*首先保证当前服务器为混合架构，然后该节点一级划分分配至GPU混合架构服务器*/
		assert(isGPUClusterNode(hClusterNum) && isInGPUClusterNode(node));
		if (find(m_statelessNodes[hClusterNum].begin(), m_statelessNodes[hClusterNum].end(),node) != m_statelessNodes[hClusterNum].end() && m_statelessNode2GpuNum[hClusterNum].find(node) != m_statelessNode2GpuNum[hClusterNum].end())
			return true;
		return false;
	}

	bool isNodeInCPU(int hClusterNum, FlatNode *node)
	{
		assert(isGPUClusterNode(hClusterNum));
		return !isNodeInGPU(hClusterNum, node);
	}

	/*判断该节点在集群的的GPU服务器还是CPU服务器*/
	bool isInGPUClusterNode(FlatNode *node) {
		bool ret = false;
		for (int i = 0; i < m_nClusters; ++i)
		{
			if (isGPUClusterNode(i))
			{
				if (find(m_hClusterNum2FlatNodes[i].begin(), m_hClusterNum2FlatNodes[i].end(), node) != m_hClusterNum2FlatNodes[i].end())
				{
					ret = true;
					return ret;
				}
				else {
					continue;
				}
			}//if
		}//for
		return ret;
	}

	/*为当前服务器上的节点进行分类*/
	bool classifyFlatNodes(int hClusterNum)
	{
		m_statelessNodes[hClusterNum].clear();
		m_statefulNodes[hClusterNum].clear();
		if (m_hClusterNum2FlatNodes[hClusterNum].empty())
			return false;
		if (!isGPUClusterNode(hClusterNum))
		{
			m_statefulNodes[hClusterNum] = m_hClusterNum2FlatNodes[hClusterNum];
			m_statelessNodes[hClusterNum] = vector<FlatNode*>();
			return true;
		}//if
		else {
			for (vector<FlatNode *>::iterator iter = m_hClusterNum2FlatNodes[hClusterNum].begin(); iter != m_hClusterNum2FlatNodes[hClusterNum].end(); ++iter)
			{
				/*若该节点本身无状态且其（入和出节点不都是有状态）则划分到GPU端*/
				if ((!DetectiveActorState(*iter)) && (UporDownStatelessNode(*iter) != 3))
				{
					m_statelessNodes[hClusterNum].push_back(*iter);
				}//if
				else {
					m_statefulNodes[hClusterNum].push_back(*iter);
				}//else
			}//for			
			/*m_statelessNodes[hClusterNum] = m_hClusterNum2FlatNodes[hClusterNum];
			m_statefulNodes[hClusterNum] = vector<FlatNode*>();
			assert((m_statefulNodes[hClusterNum].size() + m_statelessNodes[hClusterNum].size()) == m_hClusterNum2FlatNodes[hClusterNum].size());	
			*/
		}//else
		
		return true;
	}

public:
	/*二级任务划分后的总通信量*/
	long m_allComm;

protected:
	/*集群中机器节点个数*/
	int m_nClusters;
	//SDF图对象
	SchedulerSSG *m_sssg;

	/*集群中机器节点CPU和GPU配置*/
	map<int, pair<int, int>> m_hCluster2CpuAndGpu;

protected:
	/*一级划分映射《集群节点编号，子图节点集合》*/
	map<int, vector<FlatNode *>> m_hClusterNum2FlatNodes;
	/*一级划分映射《SDF图节点，集群节点编号》*/
	map<FlatNode *, int> m_flatNode2hClusterNum;

	/*一级划分映射《集群节点编号，子图工作权值》*/
	map<int, double> m_hCluster2Weight;
	/*二级划分映射《集群节点编号，《二级划分编号，划分工作量》》*/
	map<int, map<int, double>> m_hCluster2Core2Weight;
	map<int, map<int, double>> m_hCluster2Gpu2Weight;

	/*二级划分映射*/
	vector<map<int, vector<FlatNode *>>> m_coreNum2FlatNodes;
	vector<map<FlatNode *, int>> m_flatNodes2CoreNum;

	/*增加一个存储结构，专门处理混合架构服务器上划分到GPU端的节点*/
	vector<vector<FlatNode *>> m_statelessNodes,m_statefulNodes;
	/*增加一个二级映射结构，用于保存混合架构服务器上无状态节点的二级划分结果*/
	vector<map<int, vector<FlatNode *>>> m_gpuNum2StatelessNodes;
	vector<map<FlatNode*, int>> m_statelessNode2GpuNum;

	/*SDF图工作负载总和*/
	double m_allWorkLoad;
	double m_avgWorkLoad;
};

/*重用main.cpp内部分全局变量*/
extern "C"
{
	extern Bool Win;
	extern Bool Linux;
	extern GLOBAL  const char *PhaseName;
	extern GLOBAL  StaticStreamGraph *SSG; // New COStream
	extern GLOBAL  SchedulerSSG *SSSG; // New CoStream
	extern GLOBAL  Bool SSG2Graph; // 打印SSG的dot图开关 
	extern GLOBAL  Bool SchedulingFlatSSG; // 对平面图进行初始化调度 和稳态调度开关*/

	/*设置异构集群配置*********************************/
	GLOBAL extern int hClusterNum;
	/*《集群编号，《CPU核数，GPU个数》》*/
	GLOBAL extern std::map<int, std::pair<int, int>> hCluster2CpuAndGpu;

	/*划分方式*/
	extern GLOBAL Bool PGTMPartition;
	extern GLOBAL Bool PMTMPartition;
	extern GLOBAL Bool PGTDPartition;

	extern GLOBAL Bool RUNTIME;
};


#endif