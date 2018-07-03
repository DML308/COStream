#ifndef DNBPARTITION_H
#define DNBPARTITION_H

#include "HAFLPartition.h"
#include "ActorEdgeInfo.h"
using namespace std;
class DNBPartition:public HAFLPartition{
private:
	double CPUTotalWork;  //cpu端所有actor工作总量
	double GPUTotalWork;  //GPU端所有actor的工作总量
	double CpuGpuTotalComm;   //cpu与GPU间的数据传输量
	double Cpu2GpuVelocity;  //cpu与GPU传输速率
	double CpuFrequency;  //cpu计算频率
	double GpuFrequency;  //gpu计算频率
	int PENumber;    //GPU  每个CU中PE个数
	int CUNumber;   //GPU  CU个数
	int MaxCputhreadNum; //可用作cputhread的最大个数
public:
	//multimap<FlatNode*,int> Actor2Cputhread;  //actor与其CPU线程的映射
	multimap<int,pair<FlatNode*,string> > Cputhread2Actor;
	map<string,pair<int,int> > actorName2count;
	//multimap<int,pair<FlatNode*,pair<int,int>>> Cputhread2Actor;
	vector<double> CputhreadWorkValue; //存储每个CPU线程的工作量
	vector<FlatNode*> statelessNodes; //cpu中的stateless节点
	vector<FlatNode*> FissionNodes;
	multimap<FlatNode*,pair<int,int> > FissionNodes2count;
	map<FlatNode*,bool> actor2fission; //分裂过的节点用1表示，未分裂用0表示 
	SchedulerSSG *Sssg;
	ActorEdgeInfo* pEdgeInfo;//存放各个边的类型信息
public:
	DNBPartition(HAFLPartition *,SchedulerSSG*);
	void DiscreteNodePartition();
	int minThreadnumofWorkvalue(); //workvalue最小的thread号

};
extern "C"
{
	extern GLOBAL int NCpuThreads;
	extern GLOBAL int GpuNum;
};
#endif
