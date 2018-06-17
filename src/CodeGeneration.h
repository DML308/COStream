#ifndef _CODEGENERATION_H
#define _CODEGENERATION_H

#include "X10CodeGenerate.h"
#include "X86CodeGenerate.h"
#include "GPUCodeGenerate.h"
#include "ActorStageAssignment.h"
#include "MetisPartiton.h"
#define Buffer_Size 100
GLOBAL void CodeGeneration(char *currentDir, SchedulerSSG *sssg,string substring,StageAssignment *psa,MetisPartiton *Mp,HAFLPartition *haflp,TemplateClass *tc,DNBPartition*);

extern "C"
{
	extern Bool X10Backend;/*代码生成选择x10的后端*/
	extern Bool X86Backend;/*代码生成选择x86的后端*/
	extern Bool GPUBackend;/*代码生成选择GPU的后端*/
	extern int CpuCoreNum;/*表示选择的places个数*/
	extern int GpuNum;
	extern Bool Win;
	extern Bool Linux;
	extern int MultiNum;
};
#endif