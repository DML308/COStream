#include "DividedStaticGraph.h"

int DividedStaticGraph::getCount()
{
	return graphCount;
}

void DividedStaticGraph::setCount(int count)
{
	graphCount = count;
}

int DividedStaticGraph::getlastGraph()
{
	return lastGraph;
}
bool DividedStaticGraph::isLastGraph(int testNum)
{
	if (testNum == lastGraph) return true;
	else return false;
}

DividedStaticGraph::DividedStaticGraph(StaticStreamGraph*ssg)
{
	std::vector<FlatNode*> tmp = ssg->flatNodes;
	
	//查找uncertainty边.划分生成多个ssg图
	vector<FlatNode*> tmplist;
	
	for (int i = 0; i < tmp.size(); ++i)
	{
		List*windowlist = tmp[i]->contents->body->u.operBody.window;
		ListMarker window_maker;
		Node *windowNode = NULL;
		IterateList(&window_maker, windowlist);
		bool hasuncertainty;
		tmplist.push_back(tmp[i]);
		while (NextOnList(&window_maker, (GenericREF)&windowNode))
		{
			NodeType nodetype = windowNode->u.window.wtype->typ;
			if (nodetype == Uncertainty)
			{
				//生成一个子SSG图，tmplist清空
				StaticStreamGraph*childgraph = new StaticStreamGraph();
				childgraph->flatNodes = tmplist;
				//添加边绑定
				map<Node*, FlatNode*>::iterator iter1 = ssg->mapEdge2UpFlatNode.begin();
				map<Node*, FlatNode*>::iterator iter2 = ssg->mapEdge2DownFlatNode.begin();
				//算子与上游边绑定
				while (iter1 != ssg->mapEdge2UpFlatNode.end())
				{
					for (vector<FlatNode*>::iterator iter3 = childgraph->flatNodes.begin(); iter3 != childgraph->flatNodes.end(); iter3++)
					{
						if ((*iter3) == iter1->second)
						{
							childgraph->mapEdge2UpFlatNode.insert(make_pair(iter1->first, iter1->second));
						}
					}
					iter1++;
				}
				//算子与下游边绑定
				while (iter2 != ssg->mapEdge2DownFlatNode.end())
				{
					for (vector<FlatNode*>::iterator iter4 = childgraph->flatNodes.begin(); iter4 != childgraph->flatNodes.end(); iter4++)
					{
						if ((*iter4) == iter2->second)
						{
							childgraph->mapEdge2DownFlatNode.insert(make_pair(iter2->first, iter2->second));
						}
					}
					iter2++;
				}
				tmplist.clear();
				childgraph->SetTopLevel();
				staticChildGraph.push_back(childgraph);
			}
				
		}
		
	}
	//生成一个子SSG图，tmplist清空
	StaticStreamGraph*childgraph = new StaticStreamGraph();
	childgraph->flatNodes = tmplist;
	//添加边绑定
	map<Node*, FlatNode*>::iterator iter1 = ssg->mapEdge2UpFlatNode.begin();
	map<Node*, FlatNode*>::iterator iter2 = ssg->mapEdge2DownFlatNode.begin();
	//算子与上游边绑定
	while (iter1 != ssg->mapEdge2UpFlatNode.end())
	{
		for (vector<FlatNode*>::iterator iter3 = childgraph->flatNodes.begin(); iter3 != childgraph->flatNodes.end(); iter3++)
		{
			if ((*iter3) == iter1->second)
			{
				childgraph->mapEdge2UpFlatNode.insert(make_pair(iter1->first, iter1->second));
			}
		}
		iter1++;
	}
	//算子与下游边绑定
	while (iter2 != ssg->mapEdge2DownFlatNode.end())
	{
		for (vector<FlatNode*>::iterator iter4 = childgraph->flatNodes.begin(); iter4 != childgraph->flatNodes.end(); iter4++)
		{
			if ((*iter4) == iter2->second)
			{
				childgraph->mapEdge2DownFlatNode.insert(make_pair(iter2->first, iter2->second));
			}
		}
		iter2++;
	}
	childgraph->SetTopLevel();
	staticChildGraph.push_back(childgraph);
	//生成所有SSG图后,完成算子与子图的映射
	for (int i = 0;i<staticChildGraph.size();i++)
	{
		for (vector<FlatNode*>::iterator iterSSG = staticChildGraph[i]->flatNodes.begin(); iterSSG != staticChildGraph[i]->flatNodes.end(); iterSSG++)
		{
			flatNode2graphindex.insert(make_pair((*iterSSG), i));
		}
	}


	//
	lastGraph = staticChildGraph.size() - 1;
	graphCount = staticChildGraph.size();

	//填充combinelist
	for (int i = 0; i < staticChildGraph.size(); ++i)
	{
		combineList.push_back(NOTJUDGE);
	}

	allocatePosition();
	
}
std::vector<FlatNode*> DividedStaticGraph::getAllFlatNodes()
{
	std::vector<FlatNode*> result;
	for (int i = 0; i < scheStaticChildGraph.size();i++)
	{
		std::vector<FlatNode*>tmp = scheStaticChildGraph[i]->GetFlatNodes();
		for (int j = 0; j < tmp.size(); j++)
			result.push_back(tmp[j]);
	}
	return result;
}
void DividedStaticGraph::updateSsg2Core(SchedulerSSG*graph, int coreNum)
{
	ssg2coreNum.push_back(std::pair<SchedulerSSG*, int>(graph, coreNum));//子图与使用核数的映射
}
void DividedStaticGraph::allocatePosition()
{
	//分配算子位置到flatNode2position
	for (vector<StaticStreamGraph*>::iterator iter1 = staticChildGraph.begin(); iter1 != staticChildGraph.end(); iter1++)
	{
		int size = (*iter1)->flatNodes.size();
		if (size == 1)
		{
			//这个子图只有一个算子。那么它既是FIRST也是LAST，就使用both
			flatNode2position.insert(make_pair((*iter1)->flatNodes[0],BOTH));
		}
		else
		{
			for (int i = 0; i < size; ++i)
			{
				if (i == 0)
				{
					flatNode2position.insert(make_pair((*iter1)->flatNodes[i], FIRST));
				}
				else if (i == (size - 1))
				{
					flatNode2position.insert(make_pair((*iter1)->flatNodes[i], LAST));
				}
				else
					flatNode2position.insert(make_pair((*iter1)->flatNodes[i], MIDDLE));
			}
		}
		
	}
}
ActorPosition DividedStaticGraph::getActorPosition(FlatNode*node)
{
	//获取一个算子的位置
	ActorPosition position;
	position = flatNode2position.find(node)->second;
	return position;
}
bool DividedStaticGraph::isSink(FlatNode* node)
{
	int sizeOfLastGraph = staticChildGraph[staticChildGraph.size() - 1]->flatNodes.size();
	if (node == staticChildGraph[staticChildGraph.size() - 1]->flatNodes[sizeOfLastGraph - 1])
		return true;
	else return false;
}

bool DividedStaticGraph::isSource(FlatNode*node)
{
	
	if (node == staticChildGraph[0]->flatNodes[0])
		return true;
	else return false;
}

int DividedStaticGraph::getSteadyCount(FlatNode*node)
{
	int index = getGraphIndexByFlatNode(node);

	SchedulerSSG *graph = scheStaticChildGraph[index];

	return graph->GetSteadyCount(node);
}
int DividedStaticGraph::getNextGraphFirstNodePop(FlatNode*node)
{
	int index = getGraphIndexByFlatNode(node);

	assert(index != (staticChildGraph.size() - 1));

	int nextindex = index + 1;

	SchedulerSSG* graph = scheStaticChildGraph[nextindex];

	int steadycount = graph->GetSteadyCount(graph->flatNodes[0]);

	//int steadywork = graph->GetSteadyWork(graph->flatNodes[0]);
	int steadyWindow = graph->flatNodes[0]->inPeekWeights[0];

	return steadyWindow*steadycount;
}

int DividedStaticGraph::getThisNodePop(FlatNode*node)
{
	//获取一个子图的首算子的数据需求量。注意子图首算子必须只有一个输入边
	int steadycount = getSteadyCount(node);

	assert((node->inPopWeights.size()) == 1);

	int inneed = node->inPopWeights[0];

	return inneed*steadycount;
}
int DividedStaticGraph::getGraphIndexByFlatNode(FlatNode* node)
{
	for (int i = 0; i < staticChildGraph.size(); i++)
	{
		for (int j = 0; j < staticChildGraph[i]->flatNodes.size(); j++)
		{
			if (staticChildGraph[i]->flatNodes[j] == node)
				return i;
		}
	}
	return -1;
}

GLOBAL void SchedulingSSG(DividedStaticGraph*DSG)
{
	for(vector<StaticStreamGraph*>::iterator iter1 = DSG->staticChildGraph.begin(); iter1 != DSG->staticChildGraph.end(); iter1++)
	{
		//每个ssg图进行初稳态调度
		SchedulerSSG* tmpsssg = new SchedulerSSG(*iter1);
		if (tmpsssg->SteadyScheduling2())
		{
			tmpsssg->InitScheduling2();
		}
		else
		{
			fprintf(stdout, "程序不存在稳态调度，无法进行代码生成！\n");
			system("pause");
			exit(1);
		}
		DSG->scheStaticChildGraph.push_back(tmpsssg);
	}
}

GLOBAL void ReviseDSGFunction(DividedStaticGraph*DSG)
{
	//不需要修改
}
GLOBAL void GenerateWorkEst(DividedStaticGraph*dsg, bool WorkEstimateByDataFlow)
{
	for (vector<StaticStreamGraph*>::iterator iter1 = DSG->staticChildGraph.begin(); iter1 != DSG->staticChildGraph.end(); iter1++)
	{
		GenerateWorkEst(*iter1, WorkEstimateByDataFlow);
	}
}


