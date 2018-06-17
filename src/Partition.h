#ifndef _PARTITION_H_
#define _PARTITION_H_

#include "schedulerSSG.h"
#include "ClusterPartition.h"

extern "C"
{
	extern Bool X10DistributedBackEnd;/*代码生成选择“推数据”通信模型x10的后端*/
	extern int CpuCoreNum;
	extern int NThreads;/*表示选择的threads个数*/
};

class Partition
{
public:
	//所有调度方法均会使用到的成员变量
	std::map<FlatNode *,int>FlatNode2PartitionNum;//节点到划分编号的映射
	std::multimap<int,FlatNode *>PartitonNum2FlatNode;//划分编号到节点的映射
	std::vector<FlatNode*>PartitonNumSet;//用于记录返回的节点集合 
	int mnparts;//划分的份数，依赖于指定的places个数


public:
	Partition();
	int getParts();//返回划分个数mnparts
	void setCpuCoreNum(int,SchedulerSSG*);//设置place数目（即进程数目）
	int findID(SchedulerSSG *sssg,FlatNode *flatnode);//根据flatnode找到其下标号 如source_0中的0
	std::vector<FlatNode *>findNodeSetInPartition(int partitionNum);//根据编号num查找其中的节点，将节点集合返回给PartitonNumSet(编号->节点)
	int findPartitionNumForFlatNode(FlatNode *flatnode);//根据节点返回其所在划分区的编号(节点->编号)
	int Adjust(SchedulerSSG *sssg, int mode);//调整划分结果避免死锁
	void FinalAdjust(SchedulerSSG *sssg);//每个async只能允许有一个连通子图
	bool IsDeadLock(const std::vector<FlatNode *> &,const std::vector<FlatNode *> &, const int);//检查两节点集合是否存在相互等待
	void AddDownFlatnodes(std::vector<FlatNode *> &,const std::vector<FlatNode *> &);//找到同一集合中节点的所有下游节点
	void AddUpFlatnodes(std::vector<FlatNode *> &,const std::vector<FlatNode *> &);//找到同一集合中节点的所有上游节点
	FlatNode *FindTopFlatnode(const std::vector<FlatNode *> &);//找到集合中拓扑排序的顶端节点
	void AddFlatnodes(std::vector<FlatNode *> &,const std::vector<FlatNode *> &);//在vector结尾添加flatnodes
	int FindFlatnode(const std::vector<FlatNode *> &, const FlatNode *);
	double SumOfWork(const std::vector<FlatNode *> &);//计算节点的工作量之和
	int UnLock(const std::vector<FlatNode *> &, const std::vector<FlatNode *> &, const int);//解锁，直接死锁
	//以下是划分成员方法，具体实现由子类实现
	virtual void SssgPartition(SchedulerSSG *sssg ,int level)=0;//划分成员方法，具体实现由子类实现
	virtual ~Partition(){}
	inline std::map<FlatNode *,int>GetFlatNode2PartitionNum(void)
	{
		return FlatNode2PartitionNum;
	}


protected:
	std::multimap<FlatNode *, FlatNode *> currentSrc2Dest, currentDest2Src;
	std::vector<FlatNode *> currentSrcUp, currentSrcDown, currentDestUp, currentDestDown; 

};
GLOBAL void DumpStreamGraph(SchedulerSSG *ssg,Partition *,const char *fileName, ClusterPartition *);
GLOBAL void ComputeSpeedup(SchedulerSSG *sssg,Partition *mp,std::string,const char *fileName,std::string);
#endif
