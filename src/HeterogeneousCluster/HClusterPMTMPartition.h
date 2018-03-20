/*
* �칹��Ⱥ���ַ���һ��PMTM��Process Metis Thread Metis Partition������Metis���֣��߳�Metis����
* 2016/08/27 yrr
*/
#pragma once
#ifndef _HCLUSTER_PMTM_PARTITION_H
#define _HCLUSTER_PMTM_PARTITION_H

#include "HClusterBackend.h"
#include "HClusterPartition.h"
#include "HClusterMetisPartition.h"


/*PGTM���ַ����̳��Ի���HClusterPartition*/
class HClusterPMTMPartition :public HClusterPartition {
public:
	/*���캯��*/
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

		/*���ö������ֽ������*/
		m_coreNum2FlatNodes = vector<map<int, vector<FlatNode *>>>(m_nClusters, map<int, vector<FlatNode *>>());
		m_coreNum2FlatNodes.resize(m_nClusters);

		m_flatNodes2CoreNum = vector<map<FlatNode*, int>>(m_nClusters, map<FlatNode*, int>());
		m_flatNodes2CoreNum.resize(m_nClusters);

		/*���GPU�˵���״̬�ڵ�����⴦���ǻ�ϼܹ��洢��*/
		m_statelessNode2GpuNum = vector<map<FlatNode*, int>>(m_nClusters, map<FlatNode*, int>());
		m_gpuNum2StatelessNodes = vector<map<int, vector<FlatNode*>>>(m_nClusters, map<int, vector<FlatNode*>>());
	}

	/*����������*/
	virtual ~HClusterPMTMPartition() {}

	/*ʵ�ʻ��ֲ���*/
	void SssgHClusterPartition();

	/*����ָ��actor����ִ̬�д���*/
	int getSteadyCount(FlatNode *flatNode)
	{
		return m_sssg->GetSteadyCount(flatNode);
	}
	/*����������񻮷ֺ����ͨ����*/
	void computeAllComm();
private:
	/*Metis���ֳ�Ա����*/
	HClusterMetisPartition *m_hmp;
	/*Metis���ֳ�Ա����(����Ϊ��Ⱥ�ڵ���)*/
	vector<pair<HClusterMetisPartition*,HClusterMetisPartition*> > m_hmps;
	/*��������*/
	void processPartition();
	void threadPartition();
};
#endif
