#include "DynamicX86Backend.h"

GLOBAL void dynamicX86Backend(string str)
{
	vector<StageAssignment*>pSAList;
	vector<MetisPartiton*>mplist;
	/*��1�� ����SSGͼ�׶�*/
	PhaseName = "DivideSSG";
	if (Errors == 0 && DivideSSG)
	{
		DSG = new DividedStaticGraph(SSG);
	}

	//�����з���ͼ����
	PhaseName = "ReviseSSG";
	if (Errors == 0 && DivideSSG)
	{
		ReviseDSGFunction(DSG);
	}

	/*��2������������*/
	if (Errors == 0 && WorkEstimate)
	{
		GenerateWorkEst(DSG, WorkEstimateByDataFlow);
	}

	// ��3����ƽ��ͼ���г�ʼ�����Ⱥ���̬����
	PhaseName = "schedulerSSG";
	if (Errors == 0 && SchedulingFlatSSG)
	{
		SchedulingSSG(DSG);
	}

	// ��4����XML�ı�����ʽ����SDFͼ
	PhaseName = "SSG2Graph";
	if (Errors == 0 && SSG2Graph)
	{
		//DumpStreamGraph(DSG, NULL, "flatgraph.dot", NULL);
	}//if

	// ��5���Խڵ���е��Ȼ���
	PhaseName = "Partition";
	if (Errors == 0 && DynamicX86Backend && DivideSSG)
	{
		DSGNewMetisPartiton(mplist, DSG);

		DSGsetCpuCoreNum(CpuCoreNum, DSG, mplist);
		DSGSssgPartition(DSG, 1, mplist);
	}
	
	//��6������
	PhaseName = "Combine";
	if (Errors == 0 && DivideSSG == TRUE && Compress)	//ֻ�д�DSG���غ�ѹ�����زŻ�����
	{
		DSGStageCombine(DSG, mplist);
		mplist = adjustPartitionList(DSG, mplist);
	}

	//��7����ӡ���ֽ��ͼ
	if (Errors == 0 && DynamicX86Backend && DivideSSG)
	{
		//DumpStreamGraph(DSG, mp, "PartitionGraph.dot", NULL);
	}//if

	 /*��8����ӡ���ۼ��ٱ�*/
	if (Errors == 0 && Speedup && DynamicX86Backend)
	{
		ComputeSpeedup(DSG, NULL, str, "workEstimate.txt", "RRS");
	}

	/*�׶θ�ֵ*/
	if (Errors == 0 && DynamicX86Backend)
	{
		DSGNewStageAssignment(pSAList);
		DSGactorTopologicalorder(DSG, pSAList);
		DSGactorStageMap(pSAList, mplist);
	}//if

	/*��������*/
	PhaseName = "CodeGeneration";
	if (Errors == 0 && GenerateDestCode && DynamicX86Backend)
	{
#ifdef WIN32
		char *tmp = new char[1000];
		getcwd(tmp, 1000);
		ActorEdgeInfo actorEdgeInfo(DSG);		//���ڵ�����Ϣ
		DSGCodeGeneration(tmp, DSG, str, pSAList, mplist);
		//CodeGeneration(tmp,)
		tmp = NULL;
#else
		DIR *dir = NULL;
		DIR *dir1 = NULL;
		char *tmp;

		if (output_file == NULL)
			tmp = "./CppFiles/"; //��δѡ�����Ŀ¼����Ĭ��Ϊ��ǰĿ¼�µ�CppFiles
		else
		{
			char *buf;

			if (output_file[strlen(output_file) - 1] != '/')
			{
				buf = new char[strlen(output_file) + 2];
				strcpy(buf, output_file);
				buf[strlen(output_file)] = '/';
			}
			else
			{
				buf = new char[strlen(output_file) + 1];
				strcpy(buf, output_file);
			}
			string tmpbuf = buf;
			cout << "tmpbuf =" << tmpbuf << endl;
			posbuf = tmpbuf.find_last_of('/');
			cout << "posbuf =" << posbuf << endl;
			if (posbuf == -1)//ֻ��ָ����Ŀ���ļ�������
			{
				tmp = "./";
			}
			else
			{
				mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
				dir1 = opendir(buf);
				tmp = buf;

			}
		}
		cout << "������Ϣ1" << endl;
		CodeGeneration(tmp, SSSG, substring, pSA, mp, &actorEdgeInfo);
		//delete tmp;
		delete pSA;
		//tmp = NULL;
#endif	
	}

	system("pause");
	//���standalone++++++++++++++++++++++++++++++++++++++++++++++++++++++++end	

	if (Errors > 0) {
		fprintf(stderr, "\nCompilation Failed: %d error%s, %d warning%s\n",
			Errors, PLURAL(Errors),
			Warnings, PLURAL(Warnings));
	}
	else if (Warnings > 0) {
		fprintf(stderr, "\nCompilation Successful (%d warning%s)\n",
			Warnings,
			PLURAL(Warnings));
	}
}
