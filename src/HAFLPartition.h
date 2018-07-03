#ifndef _GPU_SIMPLE_PARTITION_H
#define _GPU_SIMPLE_PARTITION_H

#include "Partition.h"
#include "ActorStateDetector.h"

class HAFLPartition:public Partition
{
public:
	int UporDownStatelessNode(FlatNode *node);   //父节点，子节点是否全为stateful节点
	void SssgPartition(SchedulerSSG *sssg,int level);
	std::vector<FlatNode *> FindNodeInOneDevice(int);
	int Index2Device(int); //index到GPU设备的映射
	void SetGpuNum(SchedulerSSG *sssg);  //自适应设置需要分配的GPU数量
	void SetMultiNum2FlatNode();
public:
	std::vector<FlatNode *> GPUNodes;
	std::vector<FlatNode *> CPUNodes;
	std::map<FlatNode *, int> MultiNum2FlatNode;
	int nvtxs;    //定义顶点个数
	std::vector<FlatNode *> flatNodes_;
	
	
};
extern "C"
{
	extern GLOBAL int MultiNum;
	extern GLOBAL int GpuNum;
};
#endif