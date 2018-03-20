#ifndef _DSGCODEGENERATION_H_
#define _DSGCODEGENERATION_H_

#include "../CodeGeneration.h"
#include "X86DynamicCodeGeneration.h"
#include <string>

GLOBAL void DSGCodeGeneration(char*currentDir, DividedStaticGraph*dsg,string substring,vector<StageAssignment*> pSAList,vector<MetisPartiton*>mplist);

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