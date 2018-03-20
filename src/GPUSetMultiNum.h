#include "MAFLPartition.h"

class GPUSetMultiNum
{
public:
	GPUSetMultiNum(SchedulerSSG *sssg);
	~GPUSetMultiNum(){};
	void GetCpuAndGpuNeededBuffer(SchedulerSSG *sssg,MAFLPartition * maflp);
	void SetMultiNum(SchedulerSSG *sssg,MAFLPartition * maflp);
	
private:
	double CpuTotalBufferSize;
	double GpuTotalBufferSize;
	double CpuNeededBuffer;
	double GpuNeededBuffer;
	double MinMultiNum;
	int nvtxs;    //定义顶点个数
	std::vector<FlatNode *> flatNodes_;
//public:
//	std::map<FlatNode *, int> MultiNum2FlatNode;
};
extern "C"
{
	extern GLOBAL int MultiNum;
	extern GLOBAL int GpuNum;
};