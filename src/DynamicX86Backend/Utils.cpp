#include "Utils.h"
using namespace std;
static stringstream bufnew;
static stringstream vertexBufnew;
static stringstream edgeBufnew;
static stringstream *vertexBufInPlacenew;
//146种颜色，划分部分大于给定颜色种类则无法表示
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
		//如果图中节点数目小于核数，就取节点数目
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
	//统计所有子图的使用核数
	for(int i = 0;i<mplist.size();i++)
	{
		int coreNum = getMaxStageNum(mplist[i]);
		dsg->maxUseCoreNumList.push_back(coreNum);
	}
	//combineList中先去除使用完全部核的子图
	for(int i = 0;i<dsg->staticChildGraph.size();i++)
	{
		if(dsg->maxUseCoreNumList[i]==CpuCoreNum)
		{
			dsg->combineList[i] = CombineState::FULLCORE;
			dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], CpuCoreNum);
		}
	}
	//再循环一次对不为FULLCODE的子图进行合并判断
	for(int i = 0;i<dsg->staticChildGraph.size();++i)
	{
		if(dsg->combineList[i]!=CombineState::FULLCORE)
		{
			if (dsg->combineList[i] == CombineState::MERGETWO || dsg->combineList[i] == CombineState::MERGEUPTWO || dsg->combineList[i] == CombineState::MERGEDOWNTWO)//？需要检验//表示其上一个子图和这个子图进行合并，已经在上一个子图的合并中对这两个子图的状态进行了修改
				//而如果合并开销大则会设置本子图状态为NOTMERGE，不对下一个子图状态进行修改，因为下一个子图还可能和再下一个子图合并
				continue;
			else
			{
				if(dsg->combineList[i]==CombineState::NOTMERGE)
					SyntaxError("Syntax error:state NOTMERGE with not partition");
				else
				{
					
					if (i == dsg->staticChildGraph.size() - 1)//最后一个
					{
						dsg->combineList[i] = CombineState::NOTMERGE;
						
						//使用核数不是满核，但是到了最后，没有可以合并的
					}
					else
					{
						if (dsg->combineList[i + 1] == CombineState::FULLCORE)//下一个是8核
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
									else if (temp == 2)	//A压缩
									{
										dsg->combineList[i] = CombineState::MERGEUPONE;
										dsg->combineList[i + 1] = CombineState::MERGEUPTWO;
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], CpuCoreNum-getMaxStageNum(mplist[i+1]));
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i + 1], getMaxStageNum(mplist[i + 1]));
									}
									else
									{//B压缩
										dsg->combineList[i] = CombineState::MERGEDOWNONE;
										dsg->combineList[i + 1] = CombineState::MERGEDOWNTWO;
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], getMaxStageNum(mplist[i]));
										dsg->updateSsg2Core(dsg->scheStaticChildGraph[i + 1],CpuCoreNum-getMaxStageNum(mplist[i]));
									}
								}
								else
								{
									dsg->combineList[i] = CombineState::NOTMERGE;//不适合和下游子图压缩
									dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], CpuCoreNum);
								}
						}
					}
				}
			}
		}
		/*else //为CombineState：：FULLCORE
		{
			dsg->updateSsg2Core(dsg->scheStaticChildGraph[i], 8);
		}*/
	}


}

GLOBAL int DSGStageCombineJudge(SchedulerSSG*graph1,MetisPartiton*mp1,SchedulerSSG*graph2,MetisPartiton*mp2)	//返回0表示不能合并，返回1表示不压缩合并，2表示A压缩合并，3表示B压缩合并
{
	std::vector<std::pair<FlatNode*, std::pair<int, int> > > flatNodeWordLoadStageList1;	//子图1的每个flatNode，稳态下的该node的工作量，该node所在的阶段
	std::vector<std::pair<FlatNode*, std::pair<int, int> > > flatNodeWordLoadStageList2;
	//分别求得两子图的最大工作负载时间段
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
	//下面需要求出子图1的最大workload和子图2的最大workLoad
	int maxLoad1 = getMaxWorkLoad(graph1,mp1);
	int maxLoad2 = getMaxWorkLoad(graph2,mp2);

	if (coreNum1 + coreNum2 <= CpuCoreNum)
		return 1;	//不压缩
	else
	{
		//压缩子图A
		int leaveA = CpuCoreNum - coreNum2;
		bool compressA = false;
		float compressARatio = 0;
		float compressBRatio = 0;
		//重新计算子图A的最大负载
		MetisPartiton*tmpmp1 = new MetisPartiton(0, 0);
		tmpmp1->setCpuCoreNum(leaveA, graph1);
		tmpmp1->SssgPartition(graph1, 1);
		int maxloadChanged = getMaxWorkLoad(graph1, tmpmp1);
		if (maxloadChanged > maxLoad1 + maxLoad2)	//没必要合并
			compressA=false;
		else
		{
			compressA = true;
			compressARatio = maxloadChanged / (maxLoad1 + maxLoad2);	//这个值越小表示压缩越成功
		}
		//压缩子图B
		int leaveB = CpuCoreNum - coreNum1;
		bool compressB = false;
		//重新计算子图B的最大负载
		MetisPartiton*tmpmp2 = new MetisPartiton(0, 0);
		tmpmp2->setCpuCoreNum(leaveB, graph2);
		tmpmp2->SssgPartition(graph2, 1);
		maxloadChanged = getMaxWorkLoad(graph2, tmpmp2);
		if (maxloadChanged > maxLoad1 + maxLoad2)	//没必要合并
			compressA = false;
		else
		{
			compressB = true;
			compressBRatio = maxloadChanged / (maxLoad1 + maxLoad2);
		}
			


		if (compressA == true&&compressB == true)//比对时间
		{
			//AB均可压缩
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
//获得一个子图的划分数
{
	std::map<FlatNode*,int> tmp = mp->FlatNode2PartitionNum;
	int maxReturnNum = 0;
	map<FlatNode*,int>::iterator iter1;
	for(iter1=tmp.begin();iter1!=tmp.end();iter1++)
	{
		if(iter1->second>maxReturnNum)
			maxReturnNum = iter1->second;
	}
	return maxReturnNum+1;//因为划分从0开始所以要加1
}

GLOBAL int getMaxWorkLoad(SchedulerSSG*graph, MetisPartiton*mp)
{
	//获得某一静态子图中的某一阶段的最大负载
	std::vector<FlatNode*> nodelist = graph->GetFlatNodes();
	std::map<FlatNode*, int> wordLoadList = graph->GetSteadyWorkMap();
	std::vector<std::pair<FlatNode*, int> >steadyWordLoadList;
	for (int i = 0; i < nodelist.size(); i++)
	{
		//循环所有node计算单个Node的稳态负载
		int wordLoad = wordLoadList.find(nodelist[i])->second;
		int steadyCount = graph->GetSteadyCount(nodelist[i]);
		int totalWork = wordLoad*steadyCount;
		steadyWordLoadList.push_back(std::pair<FlatNode*, int>(nodelist[i], totalWork));
	}
	//统计阶段的最大负载
	int maxLoad = 0;
	int tmpLoad = steadyWordLoadList[0].second;
	int tmpStageNum = mp->FlatNode2PartitionNum.find(steadyWordLoadList[0].first)->second;
	for (int i = 1; i < steadyWordLoadList.size(); ++i)
	{
		int StageNum = mp->FlatNode2PartitionNum.find(steadyWordLoadList[i].first)->second;
		if (tmpStageNum == StageNum)	//同一阶段
		{
			tmpLoad += steadyWordLoadList[i].second;
		}
		else//不同阶段
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
GLOBAL vector<MetisPartiton*> adjustPartitionList(DividedStaticGraph*dsg, vector<MetisPartiton*> mplist)		//调整划分列表
{
	for (int i = 0; i < mplist.size(); ++i)
	{
		if (dsg->combineList[i] == CombineState::FULLCORE || dsg->combineList[i] == CombineState::MERGEONE || dsg->combineList[i] == CombineState::MERGETWO)		//不合并
		{
			continue;
		}
	
		else if (dsg->combineList[i] == CombineState::MERGEUPONE || dsg->combineList[i] == CombineState::MERGEUPTWO)	//A压缩
		{

			MetisPartiton*mpNew = new MetisPartiton(0, 0);
			int coreUsage = CpuCoreNum-getMaxStageNum(mplist[i + 1]);
			mpNew->setCpuCoreNum(coreUsage, dsg->scheStaticChildGraph[i]);
			mpNew->SssgPartition(dsg->scheStaticChildGraph[i], 1);
			mplist[i] = mpNew;
			i++;
		}
		else if (dsg->combineList[i] == CombineState::MERGEDOWNONE || dsg->combineList[i] == CombineState::MERGEDOWNTWO)	//B压缩
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