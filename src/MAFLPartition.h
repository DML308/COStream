#ifndef _GPU_SIMPLE_PARTITION_H
#define _GPU_SIMPLE_PARTITION_H

#include "Partition.h"
#include "ActorStateDetector.h"

class MAFLPartition:public Partition
{
public:
	int UporDownStatelessNode(FlatNode *node);   //���ڵ㣬�ӽڵ��Ƿ�ȫΪstateful�ڵ�
	void SssgPartition(SchedulerSSG *sssg,int level);
	std::vector<FlatNode *> FindNodeInOneDevice(int);
	int Index2Device(int); //index��GPU�豸��ӳ��
	void SetGpuNum(SchedulerSSG *sssg);  //����Ӧ������Ҫ�����GPU����
	void SetMultiNum2FlatNode();
private:
	std::vector<FlatNode *> GPUNodes;
	std::vector<FlatNode *> CPUNodes;
	int nvtxs;    //���嶥�����
	std::vector<FlatNode *> flatNodes_;
public:
	std::map<FlatNode *, int> MultiNum2FlatNode;
	
};
extern "C"
{
	extern GLOBAL int MultiNum;
	extern GLOBAL int GpuNum;
};
#endif