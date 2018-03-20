#include "DynamicX86Backend.h"

GLOBAL void dynamicX86Backend(string str)
{
	vector<StageAssignment*>pSAList;
	vector<MetisPartiton*>mplist;
	/*（1） 划分SSG图阶段*/
	PhaseName = "DivideSSG";
	if (Errors == 0 && DivideSSG)
	{
		DSG = new DividedStaticGraph(SSG);
	}

	//处理切分子图窗口
	PhaseName = "ReviseSSG";
	if (Errors == 0 && DivideSSG)
	{
		ReviseDSGFunction(DSG);
	}

	/*（2）工作量估计*/
	if (Errors == 0 && WorkEstimate)
	{
		GenerateWorkEst(DSG, WorkEstimateByDataFlow);
	}

	// （3）对平面图进行初始化调度和稳态调度
	PhaseName = "schedulerSSG";
	if (Errors == 0 && SchedulingFlatSSG)
	{
		SchedulingSSG(DSG);
	}

	// （4）用XML文本的形式描述SDF图
	PhaseName = "SSG2Graph";
	if (Errors == 0 && SSG2Graph)
	{
		//DumpStreamGraph(DSG, NULL, "flatgraph.dot", NULL);
	}//if

	// （5）对节点进行调度划分
	PhaseName = "Partition";
	if (Errors == 0 && DynamicX86Backend && DivideSSG)
	{
		DSGNewMetisPartiton(mplist, DSG);

		DSGsetCpuCoreNum(CpuCoreNum, DSG, mplist);
		DSGSssgPartition(DSG, 1, mplist);
	}
	
	//（6）调整
	PhaseName = "Combine";
	if (Errors == 0 && DivideSSG == TRUE && Compress)	//只有打开DSG开关和压缩开关才会运行
	{
		DSGStageCombine(DSG, mplist);
		mplist = adjustPartitionList(DSG, mplist);
	}

	//（7）打印划分结果图
	if (Errors == 0 && DynamicX86Backend && DivideSSG)
	{
		//DumpStreamGraph(DSG, mp, "PartitionGraph.dot", NULL);
	}//if

	 /*（8）打印理论加速比*/
	if (Errors == 0 && Speedup && DynamicX86Backend)
	{
		ComputeSpeedup(DSG, NULL, str, "workEstimate.txt", "RRS");
	}

	/*阶段赋值*/
	if (Errors == 0 && DynamicX86Backend)
	{
		DSGNewStageAssignment(pSAList);
		DSGactorTopologicalorder(DSG, pSAList);
		DSGactorStageMap(pSAList, mplist);
	}//if

	/*代码生成*/
	PhaseName = "CodeGeneration";
	if (Errors == 0 && GenerateDestCode && DynamicX86Backend)
	{
#ifdef WIN32
		char *tmp = new char[1000];
		getcwd(tmp, 1000);
		ActorEdgeInfo actorEdgeInfo(DSG);		//用于调试信息
		DSGCodeGeneration(tmp, DSG, str, pSAList, mplist);
		//CodeGeneration(tmp,)
		tmp = NULL;
#else
		DIR *dir = NULL;
		DIR *dir1 = NULL;
		char *tmp;

		if (output_file == NULL)
			tmp = "./CppFiles/"; //若未选择输出目录，则默认为当前目录下的CppFiles
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
			if (posbuf == -1)//只是指定了目标文件的名称
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
		cout << "调试信息1" << endl;
		CodeGeneration(tmp, SSSG, substring, pSA, mp, &actorEdgeInfo);
		//delete tmp;
		delete pSA;
		//tmp = NULL;
#endif	
	}

	system("pause");
	//后端standalone++++++++++++++++++++++++++++++++++++++++++++++++++++++++end	

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
