#ifndef GPUSETMULTINUM_H
#define GPUSETMULTINUM_H

#include "HAFLPartition.h"
#include "ActorEdgeInfo.h"
#include "ActorStageAssignment.h"
class GPUSetMultiNum
{
public:
	GPUSetMultiNum(SchedulerSSG *,StageAssignment*);
	~GPUSetMultiNum(){};
	void GetCpuAndGpuNeededBuffer(SchedulerSSG *sssg,HAFLPartition * haflp);
	void SetMultiNum(SchedulerSSG *sssg,HAFLPartition * maflp);

	ActorEdgeInfo* pEdgeInfo;//��Ÿ����ߵ�������Ϣ
	StageAssignment *pSa;
	
private:
	double CpuTotalBufferSize;
	double GpuTotalBufferSize;
	double CpuNeededBuffer;
	double GpuNeededBuffer;
	double MinMultiNum;
	int nvtxs;    //���嶥�����
	std::vector<FlatNode *> flatNodes_;
//public:
//	std::map<FlatNode *, int> MultiNum2FlatNode;
};
extern "C"
{
	extern GLOBAL int MultiNum;
	extern GLOBAL int GpuNum;
};
#endif
