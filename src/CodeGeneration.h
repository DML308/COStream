#ifndef _CODEGENERATION_H
#define _CODEGENERATION_H

#include "X10CodeGenerate.h"
#include "X86CodeGenerate.h"
#include "GPUCodeGenerate.h"
#include "ActorStageAssignment.h"
#include "MetisPartiton.h"
#define Buffer_Size 100
GLOBAL void CodeGeneration(char *currentDir, SchedulerSSG *sssg, string substring, StageAssignment *psa, Partition *Mp, MAFLPartition *maflp);

extern "C"
{
	extern Bool X10Backend;/*��������ѡ��x10�ĺ��*/
	extern Bool X86Backend;/*��������ѡ��x86�ĺ��*/
	extern Bool GPUBackend;/*��������ѡ��GPU�ĺ��*/
	extern int CpuCoreNum;/*��ʾѡ���places����*/
	extern int GpuNum;
	extern Bool Win;
	extern Bool Linux;
	extern int MultiNum;
	extern GLOBAL Bool realnCpucore;
};
#endif