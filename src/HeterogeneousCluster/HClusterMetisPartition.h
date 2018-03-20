/*
�칹��Ⱥƽ̨��Metis�����㷨ʵ���� -- �ɸ���
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

	/*�̼߳����ֺ���*/
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


	/*����ӳ����*/
	map<int, vector<FlatNode*>> m_partNum2FlatNodes;
	map<FlatNode *, int> m_flatNode2PartNum;

	/*Ҫ���ֽڵ����Ϣ*/
	map<FlatNode*, long> m_flatNode2Workload;
	map<FlatNode *, int> m_flatNode2SteadyCounts;
	
	///*��״̬�ڵ㵥����ȡ����*/
	//vector<FlatNode*> m_statefulNodes;
	//map<int, vector<FlatNode*>> m_partNum2StatefulNodes;
	//map<FlatNode*, int> m_statefulNode2PartNum;

	//===============================================================
	//Ϊʹ��metis�Լ��������õ����ݳ�Ա
	int *mxadj;//����ָ��ָ��xadj����
	int *madjncy;//����ָ��ָ��adjncy����
	int *mobjval;//����ָ��ָ��objval
	std::vector<int> mpart;//����ָ��ָ��part���� ������ŵ���metis���յĻ��ֽ�����±���flatNode��ţ�ֵ�����ں˵ı��
	int *mvwgt;//����ָ��ָ��vwgt���飬���ߴ洢ÿ�������Ȩֵ
	int *madjwgt;//����ָ��ָ��adjwgt����
	int nvtxs;//���嶥�����
	float *tpwgts;
	int *mvsize;//����ָ��ָ��vsize����
	float *ubvec;
	int mncon;
	int objval;
	int options[40];//��������
	std::vector<int>_xadj;//��̬����xadj���顪����ʾadjncy�д�xadj[i]�±꿪ʼ��xadj[i+1]�±������i�������¶˽ڵ�
	std::vector<int>_vwgt; //�ڵ�Ĺ�����
	std::vector<int>_vsize;//�ڵ��ܵ�ͨ����
	std::vector<int>_adjncy;
};

#endif // !HCLUSTER_METIS_PARTITION_H
