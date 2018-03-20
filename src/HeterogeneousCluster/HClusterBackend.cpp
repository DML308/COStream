#include "HClusterBackend.h"

GLOBAL void hClusterBanckend(string str)
{
	/*��1����ƽ��ͼ���г�̬���Ⱥ���̬����*/
	PhaseName = "scheduleSSG";
	if (0 == Errors && SchedulingFlatSSG)
	{
		SSSG = SchedulingSSG(SSG);
	}//if

	/*��2����xml�ı�����ʽ����SDFͼ*/
	if (0 == Errors && SSG2Graph)
	{
		DumpStreamGraph(SSSG, NULL, "hClusterSSSG.dot", NULL);
	}//if

	/*��3����Ⱥ��Ķ�������*/
	HClusterPartition *hCP = NULL;
	//���̼��������̼߳����ֶ�����Metis�㷨
	if (0 == Errors && PMTMPartition)
	{
		hCP = new HClusterPMTMPartition(hClusterNum, hCluster2CpuAndGpu, SSSG);
		hCP->SssgHClusterPartition();
#if 1
		/*�������񻮷��������ۣ������ͨ����*/
		cout << endl << "PMTMPartition ��ͨ����Ϊ:\t" << hCP->m_allComm << endl;
#endif
	}
	//���̼����ֲ����黮�֣��̼߳����ֲ���Metis�㷨
	else if (0 == Errors && PGTMPartition) {
		hCP = new HClusterPGTMPartition(hClusterNum, hCluster2CpuAndGpu, SSSG);
		hCP->SssgHClusterPartition();
#if 1
		/*�������񻮷��������ۣ������ͨ����*/
		cout << endl << "PGTMPartition ��ͨ����Ϊ:\t" << hCP->m_allComm << endl;
#endif
	}//elif
	//���̼����ֲ����黮�֣��̼߳����ֲ���..
	else if (0 == Errors && PGTDPartition)
	{	    
		hCP = new HClusterPGTDPartition(hClusterNum, hCluster2CpuAndGpu, SSSG);
		hCP->SssgHClusterPartition();	
#if 1
		/*�������񻮷��������ۣ������ͨ����*/
		cout << endl << "PGTDPartition ��ͨ����Ϊ:\t" << hCP->m_allComm << endl;
#endif
	}//elif

	/*��4���׶θ�ֵ*/
	HClusterStageAssignment *hCStage = new HClusterStageAssignment(hCP, SSSG);
	hCStage->createStageAssignment();

	/*��5���첽��ˮĿ���������*/
	cout << "�첽��ˮ�ߴ�������..." << endl;
	char *curDir = new char[1000];
	getcwd(curDir, 1000);
	HClusterCodeGenerator *hcg = new HClusterCodeGenerator(SSSG, hCP, hCStage,curDir);
	hcg->HClusterCodeGeneration(str);
}