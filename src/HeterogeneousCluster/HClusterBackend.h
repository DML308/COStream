/*
 *	�칹��Ⱥ���ͷ�ļ� 2016/04/28 yrr
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


//�칹��Ⱥ�����ں���,����ΪĿ�����·��
GLOBAL void hClusterBanckend(string str);

/*����main.cpp�ڲ���ȫ�ֱ���*/
extern "C"
{
	extern Bool Win;
	extern Bool Linux;
	extern GLOBAL  const char *PhaseName;
	extern GLOBAL  StaticStreamGraph *SSG; // New COStream
	extern GLOBAL  SchedulerSSG *SSSG; // New CoStream
	extern GLOBAL  Bool SSG2Graph; // ��ӡSSG��dotͼ���� 
	extern GLOBAL  Bool SchedulingFlatSSG; // ��ƽ��ͼ���г�ʼ������ ����̬���ȿ���*/

	/*�����칹��Ⱥ����*********************************/
	GLOBAL extern int hClusterNum;
	/*����Ⱥ��ţ���CPU������GPU��������*/
	GLOBAL extern std::map<int, std::pair<int, int>> hCluster2CpuAndGpu;

	/*���ַ�ʽ*/
	extern GLOBAL Bool PGTMPartition;
	extern GLOBAL Bool PMTMPartition;
	extern GLOBAL Bool PGTDPartition;
	extern GLOBAL Bool MemoryOptimiZation;
	extern GLOBAL Bool RUNTIME;
	extern GLOBAL Bool CurGpuFlag;
};

#endif