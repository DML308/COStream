#ifndef _GPUCLUSTER_PARTITION_H
#define _GPUCLUSTER_PARTITION_H

#include "Partition.h"
#include "ActorStateDetector.h"

class GPUClusterPartition:public Partition
{
public:
	int FindOutpushNum(FlatNode *node1,FlatNode *node2);  //�õ�node2��node1�ĵڼ�������ڵ�
	int GetChangedValue(FlatNode *node,SchedulerSSG *sssg,int partvalue,int marks); //�����ߣ��õ�������ı�ͨ����
	bool JudgeNextBorder(FlatNode *node,int marks); //�õ��Ƿ�����marks�ı߽��
	//int UporDownStatelessNode(FlatNode *node);   //���ڵ㣬�ӽڵ��Ƿ�ȫΪstateful�ڵ�
	void AlterBorder(FlatNode *node,int marks);   //������ֵ
	void SssgPartition(SchedulerSSG *sssg,int level);//��GPU�ϵĽڵ���л���
	inline std::vector<FlatNode *> GetGPUNodes(void)
	{ return GPUNodes;}
private:
	std::vector<FlatNode *> GPUNodes;
	int TotalComputing;    //����GPU�ڵ�ļ�����
	int PartEdgeValue;   //ͨ����
	int nvtxs;    //���嶥�����
	int AvgComputing;
};
#endif