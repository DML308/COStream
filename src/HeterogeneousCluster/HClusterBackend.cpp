#include "HClusterBackend.h"

GLOBAL void hClusterBanckend(string str)
{
	/*（1）对平面图进行初态调度和稳态调度*/
	PhaseName = "scheduleSSG";
	if (0 == Errors && SchedulingFlatSSG)
	{
		SSSG = SchedulingSSG(SSG);
	}//if

	/*（2）用xml文本的形式描述SDF图*/
	if (0 == Errors && SSG2Graph)
	{
		DumpStreamGraph(SSSG, NULL, "hClusterSSSG.dot", NULL);
	}//if

	/*（3）集群间的二级划分*/
	HClusterPartition *hCP = NULL;
	//进程级划分与线程级划分都采用Metis算法
	if (0 == Errors && PMTMPartition)
	{
		hCP = new HClusterPMTMPartition(hClusterNum, hCluster2CpuAndGpu, SSSG);
		hCP->SssgHClusterPartition();
#if 1
		/*二级任务划分性能评价：输出总通信量*/
		cout << endl << "PMTMPartition 总通信量为:\t" << hCP->m_allComm << endl;
#endif
	}
	//进程级划分采用组划分，线程级划分采用Metis算法
	else if (0 == Errors && PGTMPartition) {
		hCP = new HClusterPGTMPartition(hClusterNum, hCluster2CpuAndGpu, SSSG);
		hCP->SssgHClusterPartition();
#if 1
		/*二级任务划分性能评价：输出总通信量*/
		cout << endl << "PGTMPartition 总通信量为:\t" << hCP->m_allComm << endl;
#endif
	}//elif
	//进程级划分采用组划分，线程级划分采用..
	else if (0 == Errors && PGTDPartition)
	{	    
		hCP = new HClusterPGTDPartition(hClusterNum, hCluster2CpuAndGpu, SSSG);
		hCP->SssgHClusterPartition();	
#if 1
		/*二级任务划分性能评价：输出总通信量*/
		cout << endl << "PGTDPartition 总通信量为:\t" << hCP->m_allComm << endl;
#endif
	}//elif

	/*（4）阶段赋值*/
	HClusterStageAssignment *hCStage = new HClusterStageAssignment(hCP, SSSG);
	hCStage->createStageAssignment();

	/*（5）异步流水目标代码生成*/
	cout << "异步流水线代码生成..." << endl;
	char *curDir = new char[1000];
	getcwd(curDir, 1000);
	HClusterCodeGenerator *hcg = new HClusterCodeGenerator(SSSG, hCP, hCStage,curDir);
	hcg->HClusterCodeGeneration(str);
}