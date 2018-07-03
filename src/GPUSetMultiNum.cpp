#include "GPUSetMultiNum.h"
using namespace std;
GPUSetMultiNum::GPUSetMultiNum(SchedulerSSG *sssg,StageAssignment *PSA)
{
	CpuTotalBufferSize = 23866253312-500*1024*1024; //单位字节
	GpuTotalBufferSize = 3145728000-500*1024*1024;  //3071M
	CpuNeededBuffer = 0;
	GpuNeededBuffer = 0;
	pEdgeInfo = new ActorEdgeInfo(sssg);
	pSa = PSA;
	nvtxs=sssg->GetFlatNodes().size();
	for (int i = 0; i < nvtxs; i++)
	{
		flatNodes_.push_back(sssg->GetFlatNodes()[i]);
	}
}

void GPUSetMultiNum::GetCpuAndGpuNeededBuffer(SchedulerSSG *sssg,HAFLPartition *haflp)
{
	vector<int>::iterator iter;
	vector<FlatNode *>::iterator iter_1,iter_2;
	for (iter_1=flatNodes_.begin();iter_1!=flatNodes_.end();++iter_1)//遍历所有结点
	{
		iter=(*iter_1)->outPushWeights.begin();
		for (iter_2=(*iter_1)->outFlatNodes.begin();iter_2!=(*iter_1)->outFlatNodes.end();iter_2++)
		{
			int stageminus = pSa->FindStage(*iter_2)-pSa->FindStage(*iter_1);
			if((*iter_1)->GPUPart != (*iter_2)->GPUPart)
			{
				if(pEdgeInfo->getEdgeInfo((*iter_1),(*iter_2)).typeName == "int_x")
				{
					CpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*stageminus*GpuNum)*(*iter)*sizeof(int);
					GpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*stageminus)*(*iter)*sizeof(int);
				}
				else if (pEdgeInfo->getEdgeInfo((*iter_1),(*iter_2)).typeName == "double_x")
				{
					CpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*stageminus*GpuNum)*(*iter)*sizeof(double);
					GpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*stageminus)*(*iter)*sizeof(double);
				}
			}
			else if ((*iter_2)->GPUPart >= GpuNum && (*iter_1)->GPUPart >= GpuNum)
			{
				if(pEdgeInfo->getEdgeInfo((*iter_1),(*iter_2)).typeName == "int_x")
					CpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*(stageminus+1)*GpuNum)*(*iter)*sizeof(int);
				else if (pEdgeInfo->getEdgeInfo((*iter_1),(*iter_2)).typeName == "double_x")
					CpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*(stageminus+1)*GpuNum)*(*iter)*sizeof(double);
			}
			else if((*iter_2)->GPUPart < GpuNum && (*iter_1)->GPUPart < GpuNum)
			{
				if(pEdgeInfo->getEdgeInfo((*iter_1),(*iter_2)).typeName == "int_x")
					GpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*(stageminus+1))*(*iter)*sizeof(int);
				else if (pEdgeInfo->getEdgeInfo((*iter_1),(*iter_2)).typeName == "double_x")
					GpuNeededBuffer += (sssg->GetInitCount(*iter_1)+sssg->GetSteadyCount(*iter_1)*(stageminus+1))*(*iter)*sizeof(double);
			}
			iter++;
		}
	}
}

void GPUSetMultiNum::SetMultiNum(SchedulerSSG *sssg,HAFLPartition *haflp)
{
	GetCpuAndGpuNeededBuffer(sssg,haflp);
	double MaxMultiNum;  //扩大倍数上限
	if (CpuNeededBuffer != 0)
	{
		MaxMultiNum = CpuTotalBufferSize / CpuNeededBuffer;
		if(MaxMultiNum > GpuTotalBufferSize / GpuNeededBuffer)
			MaxMultiNum = GpuTotalBufferSize / GpuNeededBuffer;
	}
	else
		MaxMultiNum = GpuTotalBufferSize / GpuNeededBuffer;
	int TempMultiNum = (int)MaxMultiNum / 32 * 32*GpuNum;
	
	if (TempMultiNum < MultiNum)
		MultiNum = TempMultiNum;
	//MultiNum = 1280;
}