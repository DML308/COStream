/*
 * �칹��Ⱥ�������񻮷�ͷ�ļ� -- ���л��ַ����Ļ��� 2016/04/29 yrr
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
	//��ʾ����Ĭ�Ϲ��캯��
	HClusterPartition() {}
	/*���캯����ʵ�֣�Ĭ�ϼ�Ⱥֻ��һ�������ڵ�*/
	HClusterPartition(int _nClusters, map<int, std::pair<int, int>> hCluster2CpuAndGpu, SchedulerSSG *sssg)
		:m_nClusters(_nClusters), m_hCluster2CpuAndGpu(hCluster2CpuAndGpu), m_allWorkLoad(0),m_sssg(sssg) 
	{
		vector<FlatNode*>flatNodes = m_sssg->flatNodes;
		this->m_allWorkLoad = computeWorkload(flatNodes);
		this->m_avgWorkLoad = computeWorkload(flatNodes) / m_nClusters;//���������ϵ�ƽ������

		m_statelessNodes = vector<vector<FlatNode*>>(m_nClusters, vector<FlatNode*>());
		m_statefulNodes = vector<vector<FlatNode*>>(m_nClusters, vector<FlatNode*>());
	}

	/*��Ϊ���࣬��������������*/
	virtual ~HClusterPartition() {}

	/*�жϵ�ǰ�������ڵ��Ƿ���GPU*/
	bool isGPUClusterNode(int hClusterIdx) {
		return m_hCluster2CpuAndGpu[hClusterIdx].second > 0 ? true : false;
	}

	//�õ��칹��Ⱥ�ڵ����
	inline int getHClusters()	const { return m_nClusters; } 

	/*��̬���ȴ������ɾ���Ļ�������ʵ��*/
	virtual int getSteadyCount(FlatNode *flatNode) = 0;
	/*���廮���㷨��������ʵ��*/
	virtual void SssgHClusterPartition() = 0;

	/*��ӡ��Ⱥ���̼����ֽ��*/
	void printPGResult();
	/*��ӡ��Ⱥ�̼߳����ֽ��*/
	void printTPResult();

	/*���ػ���ӳ����*/
	map<int, vector<FlatNode *>> getHClusterNum2FlatNodes() { return m_hClusterNum2FlatNodes; }
	map<FlatNode *, int> getFlatNode2hClusterNum() { return m_flatNode2hClusterNum; }

	long computeWorkload(std::vector<FlatNode *>);//����vector������flatNode�Ĺ��������ܺ�
	/*�õ����������ƽ������*/
	inline double getAllWorkLoad() { return m_allWorkLoad; }
	inline double getAvgWorkLoad() { return m_avgWorkLoad; }

	/*�õ�ָ���������ڵ�ĸ���*/
	inline double getWeightOfHCluster(int hClusterNum) 
	{ 
		assert(!m_hCluster2Weight.empty()); 
		if (m_hCluster2Weight.empty())
			return 0;
		return m_hCluster2Weight[hClusterNum]; 
	}

	/*�õ�ָ���������ڵ���ָ�����ϵĸ���*/
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

	/*����ָ����Ⱥ�ڵ��Ӧ�Ļ�����ͼ�ڵ�*/
	vector<FlatNode *> getSubFlatNodes(int hClusterNum);
	
	/*�õ�ָ���ڵ�ķ��������*/
	int getHClusterNumOfNode(FlatNode *flatNode);

	/*��ָ��SDFͼ�ڵ��Ӧ�ļ�Ⱥ������ź�CPU�˱��*/
	pair<int,int> getHClusterCoreNum(FlatNode *flatNode);
	pair<int, int> getHClusterGpuNum(FlatNode *flatNode);


	/*������Ⱥ�ڵ��ţ����ظ÷������Ķ�������ӳ��*/
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

	/*�ж�node�ڵ㻮�ֵ���ϼܹ���������GPU�˻���CPU��*/
	bool isNodeInGPU(int hClusterNum, FlatNode *node)
	{
		/*���ȱ�֤��ǰ������Ϊ��ϼܹ���Ȼ��ýڵ�һ�����ַ�����GPU��ϼܹ�������*/
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

	/*�жϸýڵ��ڼ�Ⱥ�ĵ�GPU����������CPU������*/
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

	/*Ϊ��ǰ�������ϵĽڵ���з���*/
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
				/*���ýڵ㱾����״̬���䣨��ͳ��ڵ㲻������״̬���򻮷ֵ�GPU��*/
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
	/*�������񻮷ֺ����ͨ����*/
	long m_allComm;

protected:
	/*��Ⱥ�л����ڵ����*/
	int m_nClusters;
	//SDFͼ����
	SchedulerSSG *m_sssg;

	/*��Ⱥ�л����ڵ�CPU��GPU����*/
	map<int, pair<int, int>> m_hCluster2CpuAndGpu;

protected:
	/*һ������ӳ�䡶��Ⱥ�ڵ��ţ���ͼ�ڵ㼯�ϡ�*/
	map<int, vector<FlatNode *>> m_hClusterNum2FlatNodes;
	/*һ������ӳ�䡶SDFͼ�ڵ㣬��Ⱥ�ڵ��š�*/
	map<FlatNode *, int> m_flatNode2hClusterNum;

	/*һ������ӳ�䡶��Ⱥ�ڵ��ţ���ͼ����Ȩֵ��*/
	map<int, double> m_hCluster2Weight;
	/*��������ӳ�䡶��Ⱥ�ڵ��ţ����������ֱ�ţ����ֹ���������*/
	map<int, map<int, double>> m_hCluster2Core2Weight;
	map<int, map<int, double>> m_hCluster2Gpu2Weight;

	/*��������ӳ��*/
	vector<map<int, vector<FlatNode *>>> m_coreNum2FlatNodes;
	vector<map<FlatNode *, int>> m_flatNodes2CoreNum;

	/*����һ���洢�ṹ��ר�Ŵ����ϼܹ��������ϻ��ֵ�GPU�˵Ľڵ�*/
	vector<vector<FlatNode *>> m_statelessNodes,m_statefulNodes;
	/*����һ������ӳ��ṹ�����ڱ����ϼܹ�����������״̬�ڵ�Ķ������ֽ��*/
	vector<map<int, vector<FlatNode *>>> m_gpuNum2StatelessNodes;
	vector<map<FlatNode*, int>> m_statelessNode2GpuNum;

	/*SDFͼ���������ܺ�*/
	double m_allWorkLoad;
	double m_avgWorkLoad;
};

/*����main.cpp�ڲ���ȫ�ֱ���*/
extern "C"
{
	extern Bool Win;
	extern Bool Linux;
	extern GLOBAL  const char *PhaseName;
	extern GLOBAL  StaticStreamGraph *SSG; // New COStream
	extern GLOBAL  SchedulerSSG *SSSG; // New CoStream
	extern GLOBAL  Bool SSG2Graph; // ��ӡSSG��dotͼ���� 
	extern GLOBAL  Bool SchedulingFlatSSG; // ��ƽ��ͼ���г�ʼ������ ����̬���ȿ���*/

	/*�����칹��Ⱥ����*********************************/
	GLOBAL extern int hClusterNum;
	/*����Ⱥ��ţ���CPU������GPU��������*/
	GLOBAL extern std::map<int, std::pair<int, int>> hCluster2CpuAndGpu;

	/*���ַ�ʽ*/
	extern GLOBAL Bool PGTMPartition;
	extern GLOBAL Bool PMTMPartition;
	extern GLOBAL Bool PGTDPartition;

	extern GLOBAL Bool RUNTIME;
};


#endif