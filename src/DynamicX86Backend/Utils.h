#ifndef _UTILS_H_
#define _UTILS_H_
#include "../flatNode.h"
#include <string>
using namespace std;
#include "../Partition.h"
#include "../MetisPartiton.h"
#include "../ActorStageAssignment.h"
#include "DividedStaticGraph.h"

//����ʵ��ͼ
GLOBAL void DumpStreamGraph(DividedStaticGraph*dsg, Partition*u, const char *filename, ClusterPartition*);
//������ٱ�
GLOBAL void ComputeSpeedup(DividedStaticGraph *sssg, Partition *mp, string sourceFileName, const char *fileName, string pSelected);

//�½׶λ���
GLOBAL void DSGNewStageAssignment(vector<StageAssignment* >&list);
GLOBAL void DSGactorTopologicalorder(DividedStaticGraph*,vector<StageAssignment*>&list);
GLOBAL void DSGactorStageMap(vector<StageAssignment*>list,vector<MetisPartiton*>list2);

//��metis����
GLOBAL void DSGNewMetisPartiton(vector<MetisPartiton*>&,DividedStaticGraph*);
GLOBAL void DSGsetCpuCoreNum(int, DividedStaticGraph*, vector<MetisPartiton*>&);
GLOBAL void DSGSssgPartition(DividedStaticGraph*, int, vector<MetisPartiton*>&);

//�׶λ������㷨
GLOBAL int DSGStageCombineJudge(SchedulerSSG*graph1,MetisPartiton*mp1,SchedulerSSG*graph2,MetisPartiton*mp2);
GLOBAL void DSGStageCombine(DividedStaticGraph*dsg,vector<MetisPartiton*>mplisttmp);
GLOBAL int getMaxStageNum(MetisPartiton*mp);
GLOBAL int getMaxWorkLoad(SchedulerSSG*graph, MetisPartiton*mp);
GLOBAL vector<MetisPartiton*> adjustPartitionList(DividedStaticGraph*dsg,vector<MetisPartiton*> mplist);

#endif