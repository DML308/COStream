/*
* �칹��Ⱥ���ַ�������PGTD��Process Group Thread Dynamic Partition������Metis���֣��߳�Metis����
* 2017/01/10 yrr
*/
#pragma once
#ifndef _HCLUSTER_PGTD_PARTITION_H_
#define _HCLUSTER_PGTD_PARTITION_H_
#include "HClusterBackend.h"
#include "HClusterPartition.h"
#include "HClusterProcessGroupPartition.h"
#include "HClusterThreadDynamicPartition.h"

/*PGTD�������񻮷��㷨�ඨ��*/
class HClusterPGTDPartition :public HClusterPartition {
public:
	/*���캯��*/
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

		/*���ö������ֽ������*/
		m_coreNum2FlatNodes = vector<map<int, vector<FlatNode *>>>(m_nClusters, map<int, vector<FlatNode *>>());
		m_coreNum2FlatNodes.resize(m_nClusters);

		m_flatNodes2CoreNum = vector<map<FlatNode*, int>>(m_nClusters, map<FlatNode*, int>());
		m_flatNodes2CoreNum.resize(m_nClusters);


		/*���Gpu����״̬�ڵ�����⴦���ǻ�ϼܹ��洢��*/
		m_statelessNode2GpuNum = vector<map<FlatNode*, int>>(m_nClusters, map<FlatNode*, int>());
		m_gpuNum2StatelessNodes = vector<map<int, vector<FlatNode*>>>(m_nClusters, map<int, vector<FlatNode*>>());
	}

	/*����������*/
	virtual ~HClusterPGTDPartition() {}
	/*ʵ�ʻ��ֲ���*/
	void SssgHClusterPartition();

	/*����ָ��actor����ִ̬�д���*/
	int getSteadyCount(FlatNode *flatNode)
	{
		return m_sssg->GetSteadyCount(flatNode);
	}

private:
	/*�黮�ֳ�Ա����*/
	HClusterProcessGroupPartition *m_hpg;

	/*��̬���ֳ�Ա����(����Ϊ��Ⱥ�ڵ���)*/
	vector<pair<HClusterThreadDynamicPartition*,HClusterThreadDynamicPartition*> > m_htds;

	/*��������*/
	void processPartition();
	void threadPartition();

};


#endif