/*
*	�칹��Ⱥ���ͷ�ļ� 2016/04/28 yrr
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


//�칹��Ⱥ�����ں���,����ΪĿ�����·��
GLOBAL void dynamicX86Backend(string str);

/*����main.h�ڲ���ȫ�ֱ���*/
extern "C"
{
	extern Bool Win;
	extern Bool Linux;
	extern GLOBAL  const char *PhaseName;
	extern GLOBAL  StaticStreamGraph *SSG; // New COStream
	extern GLOBAL  SchedulerSSG *SSSG; // New CoStream
	extern GLOBAL  Bool SSG2Graph; // ��ӡSSG��dotͼ���� 
	extern GLOBAL  Bool SchedulingFlatSSG; // ��ƽ��ͼ���г�ʼ������ ����̬���ȿ���*/
	extern GLOBAL Bool WorkEstimate; // ���������ơ�����̬����������
	extern GLOBAL Bool WorkEstimateByDataFlow;	//ʹ��������������̬������
	extern GLOBAL Bool Speedup;/*���أ���ӡ���ٱ����*/
	extern GLOBAL Bool GenerateDestCode; // ��SDFͼת����Ŀ�����											
	extern GLOBAL int CpuCoreNum;/*Ĭ�ϳ�ʼ��Ϊ1һ̨�����к˵���Ŀ*/
	/******************************����X86��̬������������� yangshengzhe*******************/
	
	extern GLOBAL Bool DynamicX86Backend;
	extern GLOBAL Bool DivideSSG; //���־�̬��ͼ����
	extern GLOBAL Bool Compress;	//ѹ���㷨����
	extern GLOBAL DividedStaticGraph* DSG;
};

#endif