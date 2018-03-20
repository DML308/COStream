/*
 *	异构集群后端头文件 2016/04/28 yrr
 */
#ifndef _HCLUSTER_BACKEND_H
#define _HCLUSTER_BACKEND_H
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
#include "HClusterPartition.h"
#include "HClusterPGTMPartition.h"
#include "HClusterPMTMPartition.h"
#include "HClusterPGTDPartition.h"
#include "HClusterStageAssignment.h"
#include "HClusterCodeGenerator.h"
using namespace std;


//异构集群后端入口函数,参数为目标代码路径
GLOBAL void hClusterBanckend(string str);

/*重用main.cpp内部分全局变量*/
extern "C"
{
	extern Bool Win;
	extern Bool Linux;
	extern GLOBAL  const char *PhaseName;
	extern GLOBAL  StaticStreamGraph *SSG; // New COStream
	extern GLOBAL  SchedulerSSG *SSSG; // New CoStream
	extern GLOBAL  Bool SSG2Graph; // 打印SSG的dot图开关 
	extern GLOBAL  Bool SchedulingFlatSSG; // 对平面图进行初始化调度 和稳态调度开关*/

	/*设置异构集群配置*********************************/
	GLOBAL extern int hClusterNum;
	/*《集群编号，《CPU核数，GPU个数》》*/
	GLOBAL extern std::map<int, std::pair<int, int>> hCluster2CpuAndGpu;

	/*划分方式*/
	extern GLOBAL Bool PGTMPartition;
	extern GLOBAL Bool PMTMPartition;
	extern GLOBAL Bool PGTDPartition;
	extern GLOBAL Bool MemoryOptimiZation;
	extern GLOBAL Bool RUNTIME;
	extern GLOBAL Bool CurGpuFlag;
};

#endif