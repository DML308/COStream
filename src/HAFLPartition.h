#ifndef _GPU_SIMPLE_PARTITION_H
#define _GPU_SIMPLE_PARTITION_H

#include "Partition.h"
#include "ActorStateDetector.h"

class HAFLPartition:public Partition
{
public:
	int UporDownStatelessNode(FlatNode *node);   //���ڵ㣬�ӽڵ��Ƿ�ȫΪstateful�ڵ�
	void SssgPartition(SchedulerSSG *sssg,int level);
	std::vector<FlatNode *> FindNodeInOneDevice(int);
	int Index2Device(int); //index��GPU�豸��ӳ��
	void SetGpuNum(SchedulerSSG *sssg);  //����Ӧ������Ҫ�����GPU����
	void SetMultiNum2FlatNode();
public:
	std::vector<FlatNode *> GPUNodes;
	std::vector<FlatNode *> CPUNodes;
	std::map<FlatNode *, int> MultiNum2FlatNode;
	int nvtxs;    //���嶥�����
	std::vector<FlatNode *> flatNodes_;
	
	
};
extern "C"
{
	extern GLOBAL int MultiNum;
	extern GLOBAL int GpuNum;
};
#endif