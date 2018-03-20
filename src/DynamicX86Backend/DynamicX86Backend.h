/*
*	异构集群后端头文件 2016/04/28 yrr
*/
#ifndef _DYNAMIC_X86_BACKEND_H
#define _DYNAMIC_X86_BACKEND_H
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <iterator>
#include <utility>
#include <algorithm>

#include "../staticStreamGraph.h"
#include "../schedulerSSG.h"
#include "../Partition.h"

#include "DividedStaticGraph.h"
#include "DSGCodeGeneration.h"
#include "X86DynamicCodeGeneration.h"
#include "Utils.h"

using namespace std;


//异构集群后端入口函数,参数为目标代码路径
GLOBAL void dynamicX86Backend(string str);

/*重用main.h内部分全局变量*/
extern "C"
{
	extern Bool Win;
	extern Bool Linux;
	extern GLOBAL  const char *PhaseName;
	extern GLOBAL  StaticStreamGraph *SSG; // New COStream
	extern GLOBAL  SchedulerSSG *SSSG; // New CoStream
	extern GLOBAL  Bool SSG2Graph; // 打印SSG的dot图开关 
	extern GLOBAL  Bool SchedulingFlatSSG; // 对平面图进行初始化调度 和稳态调度开关*/
	extern GLOBAL Bool WorkEstimate; // 工作量估计——静态工作量估计
	extern GLOBAL Bool WorkEstimateByDataFlow;	//使用数据流估计稳态工作量
	extern GLOBAL Bool Speedup;/*开关：打印加速比情况*/
	extern GLOBAL Bool GenerateDestCode; // 将SDF图转换成目标代码											
	extern GLOBAL int CpuCoreNum;/*默认初始化为1一台机器中核的数目*/
	/******************************设置X86动态数据流后端配置 yangshengzhe*******************/
	
	extern GLOBAL Bool DynamicX86Backend;
	extern GLOBAL Bool DivideSSG; //划分静态子图开关
	extern GLOBAL Bool Compress;	//压缩算法开关
	extern GLOBAL DividedStaticGraph* DSG;
};

#endif