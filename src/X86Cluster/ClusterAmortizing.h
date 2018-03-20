#ifndef _CLUSTER_AMORTIZING_H_
#define _CLUSTER_AMORTIZING_H_

#include "ClusterPartition.h"
#include "ClusterHorizontalFission.h"
#include "ClusterBasics.h"
#include "../schedulerSSG.h"
#include <iostream>
#include <map>
#include <vector>
#include <list>
using namespace std;

class ClusterAmortize
{
public:
	ClusterAmortize(ClusterPartition *cp, SchedulerSSG* sssg,int nclusters, int nplaces);//���캯��
	void AmortizingClusters();//��̯����Ⱥ��̯��
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
	float PrepassReplication(map<FlatNode *,int>_flatNode2core, int _amortizeFactor);//�ڼ�Ⱥ�ĵ����ڵ��������Ʒ����㷨,���ص�ֵ�ǵ�ǰ������ɺ�cluster����ˮƽ���Ѻ��ƽ�����ӣ����ո��㷨ִ�еĽ�����յ���ˮƽ�����е����ģ�飩
	void horizontialFisssionInCluster(map<FlatNode *,int>_flatNode2core,map<FlatNode *, int>_flatNode2localsteadycount,int _amortizingFactor,int _ncluster);//������ʵ�ʵķ���

	int GetLocalSteadyCount(FlatNode *flatNode);
	int GetSteadyCount(FlatNode *flatNode);//ȡȫ����̬�µ�ִ�д���
	
	map<FlatNode *,int> GetflatNodes2core(int _Ncluster);//ȡһ�������е����е�flatNode�ڵ�


private:
	std::map<int, std::map<FlatNode *, int > > cluster2FlatNode2Core;//cluster�ı��,flatNode,core֮���ӳ��
	std::map<int, std::map<FlatNode *, int > > _amortizeCluster2FlatNode2Core;//���̯����ɺ�Ļ���

	std::map<FlatNode *,int> flatNode2steadycountCluster;//flatNode��cluster�оֲ���̬��ִ�д���
	std::map<FlatNode *,int> flatNode2amortizationSteadycount;//����̯����ɺ�ڵ��ִ�д���
	SchedulerSSG *ca_sssg;//������ɺ��sssgͼ
	int mnClusters;//��Ⱥ���ܹ��Ľڵ���
	int mnCores;//��Ⱥ��һ���ڵ�ĺ˵���Ŀ
};

GLOBAL void CPAmortizating(ClusterPartition *cp,SchedulerSSG *sssg,int nclusters, int nplaces);//�Ի��ֵĽ����̯��



#endif