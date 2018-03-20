#include "GPUSetMultiNum.h"
using namespace std;
GPUSetMultiNum::GPUSetMultiNum(SchedulerSSG *sssg)
{
	CpuTotalBufferSize = 24348950528; //单位字节
	GpuTotalBufferSize = 3221225472;  //3G大小
	CpuNeededBuffer = 0;
	GpuNeededBuffer = 0;
	nvtxs=sssg->GetFlatNodes().size();
	for (int i = 0; i < nvtxs; i++)
	{
		flatNodes_.push_back(sssg->GetFlatNodes()[i]);
	}
}

void GPUSetMultiNum::GetCpuAndGpuNeededBuffer(SchedulerSSG *sssg,MAFLPartition *maflp)
{
	vector<int>::iterator iter;
	vector<FlatNode *>::iterator iter_1,iter_2;
	for (iter_1=flatNodes_.begin();iter_1!=flatNodes_.end();++iter_1)//遍历所有结点
	{
		iter=(*iter_1)->outPushWeights.begin();
		for (iter_2=(*iter_1)->outFlatNodes.begin();iter_2!=(*iter_1)->outFlatNodes.end();iter_2++)
		{
			if((*iter_1)->GPUPart != (*iter_2)->GPUPart)
			{
				CpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*(GpuNum+1))*(*iter);
				GpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*2)*(*iter);
			}
			else if ((*iter_1)->GPUPart == (*iter_2)->GPUPart && (*iter_1)->GPUPart == GpuNum)
			{
				CpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1))*(*iter);
			}
			else if((*iter_1)->GPUPart == (*iter_2)->GPUPart && (*iter_1)->GPUPart != GpuNum)
				GpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1))*(*iter);
			iter++;
		}
	}
	CpuNeededBuffer *= sizeof(double);
	GpuNeededBuffer *= sizeof(double);
}

void GPUSetMultiNum::SetMultiNum(SchedulerSSG *sssg,MAFLPartition *maflp)
{
	GetCpuAndGpuNeededBuffer(sssg,maflp);
	double MaxMultiNum;  //扩大倍数上限
	if (CpuNeededBuffer != 0)
	{
		MaxMultiNum = CpuTotalBufferSize / (CpuNeededBuffer*GpuNum);
		if(MaxMultiNum > GpuTotalBufferSize / GpuNeededBuffer)
			MaxMultiNum = GpuTotalBufferSize / GpuNeededBuffer;
	}
	else
		MaxMultiNum = GpuTotalBufferSize / GpuNeededBuffer;
	MultiNum = ((int)MaxMultiNum / 32-1) * 32*GpuNum;
}