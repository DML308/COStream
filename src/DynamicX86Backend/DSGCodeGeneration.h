#ifndef _DSGCODEGENERATION_H_
#define _DSGCODEGENERATION_H_

#include "../CodeGeneration.h"
#include "X86DynamicCodeGeneration.h"
#include <string>

GLOBAL void DSGCodeGeneration(char*currentDir, DividedStaticGraph*dsg,string substring,vector<StageAssignment*> pSAList,vector<MetisPartiton*>mplist);

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
};
#endif