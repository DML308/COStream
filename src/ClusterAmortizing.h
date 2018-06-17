#ifndef _CLUSTER_AMORTIZING_H_
#define _CLUSTER_AMORTIZING_H_

#include "ClusterPartition.h"
#include "ClusterHorizontalFission.h"
#include "ClusterBasics.h"
#include "schedulerSSG.h"
#include <iostream>
#include <map>
#include <vector>
#include <list>
using namespace std;

class ClusterAmortize
{
public:
	ClusterAmortize(ClusterPartition *cp, SchedulerSSG* sssg,int nclusters, int nplaces);//构造函数
	void AmortizingClusters();//在摊销集群间摊销
	map<FlatNode *,int> GetAmortizeSteadyCount()
	{
		return flatNode2amortizationSteadycount;
	}
	map<int, std::map<FlatNode *, int > > GetAmoritizePartition()
	{
		return _amortizeCluster2FlatNode2Core;
	}
	~ClusterAmortize(){};

private:
	float PrepassReplication(map<FlatNode *,int>_flatNode2core, int _amortizeFactor);//在集群的单个节点上做复制分裂算法,返回的值是当前划分完成后cluster经过水平分裂后的平衡因子（按照该算法执行的结果最终调用水平分裂中的相关模块）
	void horizontialFisssionInCluster(map<FlatNode *,int>_flatNode2core,map<FlatNode *, int>_flatNode2localsteadycount,int _amortizingFactor,int _ncluster);//用于做实际的分裂

	int GetLocalSteadyCount(FlatNode *flatNode);
	int GetSteadyCount(FlatNode *flatNode);//取全局稳态下的执行次数
	
	map<FlatNode *,int> GetflatNodes2core(int _Ncluster);//取一个划分中的所有的flatNode节点


private:
	std::map<int, std::map<FlatNode *, int > > cluster2FlatNode2Core;//cluster的编号,flatNode,core之间的映射
	std::map<int, std::map<FlatNode *, int > > _amortizeCluster2FlatNode2Core;//存放摊销完成后的划分

	std::map<FlatNode *,int> flatNode2steadycountCluster;//flatNode在cluster中局部稳态的执行次数
	std::map<FlatNode *,int> flatNode2amortizationSteadycount;//保存摊销完成后节点的执行次数
	SchedulerSSG *ca_sssg;//划分完成后的sssg图
	int mnClusters;//集群中总共的节点数
	int mnCores;//集群中一个节点的核的数目
};

GLOBAL void CPAmortizating(ClusterPartition *cp,SchedulerSSG *sssg,int nclusters, int nplaces);//对划分的结果做摊销



#endif