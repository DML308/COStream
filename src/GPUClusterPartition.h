#ifndef _GPUCLUSTER_PARTITION_H
#define _GPUCLUSTER_PARTITION_H

#include "Partition.h"
#include "ActorStateDetector.h"

class GPUClusterPartition:public Partition
{
public:
	int FindOutpushNum(FlatNode *node1,FlatNode *node2);  //得到node2是node1的第几个输出节点
	int GetChangedValue(FlatNode *node,SchedulerSSG *sssg,int partvalue,int marks); //调整边，得到调整后的边通信量
	bool JudgeNextBorder(FlatNode *node,int marks); //得到是否相邻marks的边界点
	int UporDownStatelessNode(FlatNode *node);   //父节点，子节点是否全为stateful节点
	void AlterBorder(FlatNode *node,int marks);   //修正边值
	void SssgPartition(SchedulerSSG *sssg,int level);//对GPU上的节点进行划分
	inline std::vector<FlatNode *> GetGPUNodes(void)
	{ return GPUNodes;}
private:
	std::vector<FlatNode *> GPUNodes;
	int TotalComputing;    //所有GPU节点的计算量
	int PartEdgeValue;   //通信量
	int nvtxs;    //定义顶点个数
	int AvgComputing;
};
#endif