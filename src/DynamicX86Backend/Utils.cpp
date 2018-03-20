#include "Utils.h"
using namespace std;
static stringstream bufnew;
static stringstream vertexBufnew;
static stringstream edgeBufnew;
static stringstream *vertexBufInPlacenew;
//146����ɫ�����ֲ��ִ��ڸ�����ɫ�������޷���ʾ
static string colornew[] = { "aliceblue", "antiquewhite", "yellowgreen", "aquamarine", "azure",
	"magenta", "maroon", "mediumaquamarine", "mediumblue", "mediumorchid",
	"mediumpurple", "mediumseagreen", "mediumslateblue", "mediumspringgreen", "mediumturquoise",
	"mediumvioletred", "midnightblue", "mintcream", "mistyrose", "moccasin",
	"navajowhite", "navy", "oldlace", "olive", "olivedrab",
	"orange", "orangered", "orchid", "palegoldenrod", "palegreen",
	"paleturquoise", "palevioletred", "papayawhip", "peachpuff", "peru",
	"pink", "plum", "powderblue", "purple", "red",
	"rosybrown", "royalblue", "saddlebrown", "salmon", "sandybrown",
	"seagreen", "seashell", "sienna", "silver", "skyblue",
	"slateblue", "slategray", "slategrey", "snow", "springgreen",
	"steelblue", "tan", "teal", "thistle", "tomato",
	"deeppink", "deepskyblue", "dimgray", "dimgrey", "dodgerblue",
	"firebrick", "floralwhite", "forestgreen", "fuchsia", "gainsboro",
	"ghostwhite", "gold", "goldenrod", "gray", "grey",
	"green", "greenyellow", "honeydew", "hotpink", "indianred",
	"indigo", "ivory", "khaki", "lavender", "lavenderblush",
	"lawngreen", "lemonchiffon", "lightblue", "lightcoral", "lightcyan",
	"lightgoldenrodyellow", "lightgray", "lightgreen", "lightgrey", "lightpink",
	"lightsalmon", "lightseagreen", "lightskyblue", "lightslategray", "lightslategrey",
	"lightsteelblue", "lightyellow", "lime&nbsp", "limegreen", "linen",
	"beige", "bisque", "yellow", "blanchedalmond", "blue",
	"blueviolet", "brown", "burlywood", "cadetblue", "chartreuse",
	"chocolate", "coral", "cornflowerblue", "cornsilk", "crimson",
	"cyan", "darkblue", "darkcyan", "darkgoldenrod", "darkgray",
	"darkgreen", "darkgrey", "darkkhaki", "darkmagenta", "darkolivegreen",
	"darkorange", "darkorchid", "darkred", "darksalmon", "darkseagreen",
	"darkslateblue", "darkslategray", "darkslategrey", "darkturquoise", "darkviolet",
	"turquoise", "violet", "wheat", "white", "whitesmoke" };

void toBuildOutPutStringNew(FlatNode *node)
{

}


GLOBAL void DumpStreamGraph(DividedStaticGraph*dsg, Partition*mp, const char*filename, ClusterPartition*cp)
{
	/*bufnew.str("");
	bufnew << "digraph Flattend{ \n";
	
	bufnew << "}\n";
	ofstream fw;
	fw.open(filename);
	fw << bufnew.str();
	fw.close();*/
}
GLOBAL void ComputeSpeedup(DividedStaticGraph *sssg, Partition *mp, string sourceFileName, const char *fileName, string pSelected)
{

}
GLOBAL void DSGNewStageAssignment(vector<StageAssignment* >&list)
{
	int size = DSG->scheStaticChildGraph.size();
	for (int i = 0; i < size; i++)
	{
		StageAssignment*psa = new StageAssignment();
		list.push_back(psa);
	}
}

GLOBAL void DSGactorTopologicalorder(DividedStaticGraph*dsg, vector<StageAssignment*>&list)
{
	for (int i = 0; i < dsg->scheStaticChildGraph.size(); i++)
	{
		list[i]->actorTopologicalorderDSG(dsg->scheStaticChildGraph[i]->GetFlatNodes());
	}
}

GLOBAL void DSGactorStageMap(vector<StageAssignment*>list, vector<MetisPartiton*>list2)
{
	for (int i = 0; i < list2.size(); i++)
	{
		list[i]->actorStageMap(list2[i]->GetFlatNode2PartitionNum());
	}
}

GLOBAL void DSGNewMetisPartiton(vector<MetisPartiton*>&list,DividedStaticGraph*)
{
	for (int i = 0; i < DSG->scheStaticChildGraph.size(); i++)
	{
		MetisPartiton*mp = new MetisPartiton(0, 0);
		list.push_back(mp);
	}
}
GLOBAL void DSGsetCpuCoreNum(int nplaces, DividedStaticGraph*dsg, vector<MetisPartiton*>&list)
{
	for (int i = 0; i < list.size(); ++i)
	{
		list[i]->setCpuCoreNum(nplaces, dsg->scheStaticChildGraph[i]);
		//���ͼ�нڵ���ĿС�ں�������ȡ�ڵ���Ŀ
	}

}
GLOBAL void DSGSssgPartition(DividedStaticGraph*dsg, int level, vector<MetisPartiton*>&list)
{
	assert(level == 1);
	for (int i = 0; i < list.size(); ++i)
	{
		list[i]->SssgPartition(dsg->scheStaticChildGraph[i], level);
	}

}

GLOBAL void DSGStageCombine(DividedStaticGraph*dsg,vector<MetisPartiton*>mplist)
{
	//ͳ��������ͼ��ʹ�ú���
	for(int i = 0;i<mplist.size();i++)
	{
		int coreNum = getMaxStageNum(mplist[i]);
		dsg->maxUseCoreNumList.push_back(coreNum);
	}
	//combineList����ȥ��ʹ����ȫ���˵���ͼ
	for(int i = 0;i<dsg->staticChildGraph.size();i++)
	{
		if(dsg->maxUseCoreNumList[i]==CpuCoreNum)
		{
			dsg->combineList[i] = CombineState::FULLCORE;
			dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], CpuCoreNum);
		}
	}
	//��ѭ��һ�ζԲ�ΪFULLCODE����ͼ���кϲ��ж�
	for(int i = 0;i<dsg->staticChildGraph.size();++i)
	{
		if(dsg->combineList[i]!=CombineState::FULLCORE)
		{
			if (dsg->combineList[i] == CombineState::MERGETWO || dsg->combineList[i] == CombineState::MERGEUPTWO || dsg->combineList[i] == CombineState::MERGEDOWNTWO)//����Ҫ����//��ʾ����һ����ͼ�������ͼ���кϲ����Ѿ�����һ����ͼ�ĺϲ��ж���������ͼ��״̬�������޸�
				//������ϲ�������������ñ���ͼ״̬ΪNOTMERGE��������һ����ͼ״̬�����޸ģ���Ϊ��һ����ͼ�����ܺ�����һ����ͼ�ϲ�
				continue;
			else
			{
				if(dsg->combineList[i]==CombineState::NOTMERGE)
					SyntaxError("Syntax error:state NOTMERGE with not partition");
				else
				{
					
					if (i == dsg->staticChildGraph.size() - 1)//���һ��
					{
						dsg->combineList[i] = CombineState::NOTMERGE;
						
						//ʹ�ú����������ˣ����ǵ������û�п��Ժϲ���
					}
					else
					{
						if (dsg->combineList[i + 1] == CombineState::FULLCORE)//��һ����8��
						{
							dsg->combineList[i] = CombineState::NOTMERGE;
							dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], CpuCoreNum);
						}
						else
						{
							int temp = DSGStageCombineJudge(dsg->scheStaticChildGraph[i], mplist[i], dsg->scheStaticChildGraph[i + 1], mplist[i + 1]);
								if(temp>0)
								{
										
									if (temp == 1)
									{
										dsg->combineList[i] = CombineState::MERGEONE;
										dsg->combineList[i + 1] = CombineState::MERGETWO;
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], getMaxStageNum(mplist[i]));
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i + 1], getMaxStageNum(mplist[i + 1]));
									}
									else if (temp == 2)	//Aѹ��
									{
										dsg->combineList[i] = CombineState::MERGEUPONE;
										dsg->combineList[i + 1] = CombineState::MERGEUPTWO;
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], CpuCoreNum-getMaxStageNum(mplist[i+1]));
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i + 1], getMaxStageNum(mplist[i + 1]));
									}
									else
									{//Bѹ��
										dsg->combineList[i] = CombineState::MERGEDOWNONE;
										dsg->combineList[i + 1] = CombineState::MERGEDOWNTWO;
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], getMaxStageNum(mplist[i]));
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i + 1],CpuCoreNum-getMaxStageNum(mplist[i]));
									}
								}
								else
								{
									dsg->combineList[i] = CombineState::NOTMERGE;//���ʺϺ�������ͼѹ��
									dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], CpuCoreNum);
								}
						}
					}
				}
			}
		}
		/*else //ΪCombineState����FULLCORE
		{
			dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], 8);
		}*/
	}


}

GLOBAL int DSGStageCombineJudge(SchedulerSSG*graph1,MetisPartiton*mp1,SchedulerSSG*graph2,MetisPartiton*mp2)	//����0��ʾ���ܺϲ�������1��ʾ��ѹ���ϲ���2��ʾAѹ���ϲ���3��ʾBѹ���ϲ�
{
	std::vector<std::pair<FlatNode*, std::pair<int, int> > > flatNodeWordLoadStageList1;	//��ͼ1��ÿ��flatNode����̬�µĸ�node�Ĺ���������node���ڵĽ׶�
	std::vector<std::pair<FlatNode*, std::pair<int, int> > > flatNodeWordLoadStageList2;
	//�ֱ��������ͼ�����������ʱ���
	for (int i = 0; i < graph1->GetFlatNodes().size(); ++i)
	{
		FlatNode* tmpNode = graph1->GetFlatNodes()[i];
		int steadyCount = graph1->GetSteadyCount(graph1->GetFlatNodes()[i]);
		int workLoad = graph1->GetSteadyWorkMap().find(graph1->GetFlatNodes()[i])->second;
		int tmpStage = mp1->FlatNode2PartitionNum.find(tmpNode)->second;
		std::pair<int, int> iter(workLoad*steadyCount, tmpStage);
		std::pair<FlatNode*, std::pair<int, int> > tmpiter(tmpNode, iter);
		flatNodeWordLoadStageList1.push_back(tmpiter);
	}
	for (int i = 0; i < graph2->GetFlatNodes().size(); ++i)
	{
		FlatNode*tmpNode = graph2->GetFlatNodes()[i];
		int steadyCount = graph2->GetSteadyCount(graph2->GetFlatNodes()[i]);
		int workLoad = graph2->GetSteadyWorkMap().find(graph2->GetFlatNodes()[i])->second;
		int tmpStage = mp2->FlatNode2PartitionNum.find(tmpNode)->second;
		std::pair<int, int> iter(workLoad*steadyCount, tmpStage);
		std::pair<FlatNode*, std::pair<int, int> > tmpiter(tmpNode, iter);
		flatNodeWordLoadStageList2.push_back(tmpiter);
	}
	int coreNum1 = getMaxStageNum(mp1);
	int coreNum2 = getMaxStageNum(mp2);
	//������Ҫ�����ͼ1�����workload����ͼ2�����workLoad
	int maxLoad1 = getMaxWorkLoad(graph1,mp1);
	int maxLoad2 = getMaxWorkLoad(graph2,mp2);

	if (coreNum1 + coreNum2 <= CpuCoreNum)
		return 1;	//��ѹ��
	else
	{
		//ѹ����ͼA
		int leaveA = CpuCoreNum - coreNum2;
		bool compressA = false;
		float compressARatio = 0;
		float compressBRatio = 0;
		//���¼�����ͼA�������
		MetisPartiton*tmpmp1 = new MetisPartiton(0, 0);
		tmpmp1->setCpuCoreNum(leaveA, graph1);
		tmpmp1->SssgPartition(graph1, 1);
		int maxloadChanged = getMaxWorkLoad(graph1, tmpmp1);
		if (maxloadChanged > maxLoad1 + maxLoad2)	//û��Ҫ�ϲ�
			compressA=false;
		else
		{
			compressA = true;
			compressARatio = maxloadChanged / (maxLoad1 + maxLoad2);	//���ֵԽС��ʾѹ��Խ�ɹ�
		}
		//ѹ����ͼB
		int leaveB = CpuCoreNum - coreNum1;
		bool compressB = false;
		//���¼�����ͼB�������
		MetisPartiton*tmpmp2 = new MetisPartiton(0, 0);
		tmpmp2->setCpuCoreNum(leaveB, graph2);
		tmpmp2->SssgPartition(graph2, 1);
		maxloadChanged = getMaxWorkLoad(graph2, tmpmp2);
		if (maxloadChanged > maxLoad1 + maxLoad2)	//û��Ҫ�ϲ�
			compressA = false;
		else
		{
			compressB = true;
			compressBRatio = maxloadChanged / (maxLoad1 + maxLoad2);
		}
			


		if (compressA == true&&compressB == true)//�ȶ�ʱ��
		{
			//AB����ѹ��
			if (compressA < compressB)
			{
				
				return 2;
			}
			else
			{
				return 3;
			}
		}
		else if (compressA == true && compressB == false)
		{
			return 2;
		}
		else if (compressB == true && compressA == false)
		{
			return 3;
		}
		else
		{
			return 0;
		}
	}

	return 0;

}
GLOBAL int getMaxStageNum(MetisPartiton*mp)
//���һ����ͼ�Ļ�����
{
	std::map<FlatNode*,int> tmp = mp->FlatNode2PartitionNum;
	int maxReturnNum = 0;
	map<FlatNode*,int>::iterator iter1;
	for(iter1=tmp.begin();iter1!=tmp.end();iter1++)
	{
		if(iter1->second>maxReturnNum)
			maxReturnNum = iter1->second;
	}
	return maxReturnNum+1;//��Ϊ���ִ�0��ʼ����Ҫ��1
}

GLOBAL int getMaxWorkLoad(SchedulerSSG*graph, MetisPartiton*mp)
{
	//���ĳһ��̬��ͼ�е�ĳһ�׶ε������
	std::vector<FlatNode*> nodelist = graph->GetFlatNodes();
	std::map<FlatNode*, int> wordLoadList = graph->GetSteadyWorkMap();
	std::vector<std::pair<FlatNode*, int> >steadyWordLoadList;
	for (int i = 0; i < nodelist.size(); i++)
	{
		//ѭ������node���㵥��Node����̬����
		int wordLoad = wordLoadList.find(nodelist[i])->second;
		int steadyCount = graph->GetSteadyCount(nodelist[i]);
		int totalWork = wordLoad*steadyCount;
		steadyWordLoadList.push_back(std::pair<FlatNode*, int>(nodelist[i], totalWork));
	}
	//ͳ�ƽ׶ε������
	int maxLoad = 0;
	int tmpLoad = steadyWordLoadList[0].second;
	int tmpStageNum = mp->FlatNode2PartitionNum.find(steadyWordLoadList[0].first)->second;
	for (int i = 1; i < steadyWordLoadList.size(); ++i)
	{
		int StageNum = mp->FlatNode2PartitionNum.find(steadyWordLoadList[i].first)->second;
		if (tmpStageNum == StageNum)	//ͬһ�׶�
		{
			tmpLoad += steadyWordLoadList[i].second;
		}
		else//��ͬ�׶�
		{
			tmpStageNum = StageNum;
			if (tmpLoad >= maxLoad)
			{
				maxLoad = tmpLoad;
			}
			tmpLoad = 0;
		}
	}
	return maxLoad;
}
GLOBAL vector<MetisPartiton*> adjustPartitionList(DividedStaticGraph*dsg, vector<MetisPartiton*> mplist)		//���������б�
{
	for (int i = 0; i < mplist.size(); ++i)
	{
		if (dsg->combineList[i] == CombineState::FULLCORE || dsg->combineList[i] == CombineState::MERGEONE || dsg->combineList[i] == CombineState::MERGETWO)		//���ϲ�
		{
			continue;
		}
	
		else if (dsg->combineList[i] == CombineState::MERGEUPONE || dsg->combineList[i] == CombineState::MERGEUPTWO)	//Aѹ��
		{

			MetisPartiton*mpNew = new MetisPartiton(0, 0);
			int coreUsage = CpuCoreNum-getMaxStageNum(mplist[i + 1]);
			mpNew->setCpuCoreNum(coreUsage, dsg->scheStaticChildGraph[i]);
			mpNew->SssgPartition(dsg->scheStaticChildGraph[i], 1);
			mplist[i] = mpNew;
			i++;
		}
		else if (dsg->combineList[i] == CombineState::MERGEDOWNONE || dsg->combineList[i] == CombineState::MERGEDOWNTWO)	//Bѹ��
		{
			MetisPartiton*mpNew = new MetisPartiton(0, 0);
			int coreUsage = CpuCoreNum - getMaxStageNum(mplist[i]);
			mpNew->setCpuCoreNum(coreUsage, dsg->scheStaticChildGraph[i+1]);
			mpNew->SssgPartition(dsg->scheStaticChildGraph[i + 1], 1);
			mplist[i+1] = mpNew;
			i++;
		}
		else
		{
			continue;
		}
	}

	return mplist;
}