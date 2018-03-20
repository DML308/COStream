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
	
	//����uncertainty��.�������ɶ��ssgͼ
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
				//����һ����SSGͼ��tmplist���
				StaticStreamGraph*childgraph = new StaticStreamGraph();
				childgraph->flatNodes = tmplist;
				//��ӱ߰�
				map<Node*, FlatNode*>::iterator iter1 = ssg->mapEdge2UpFlatNode.begin();
				map<Node*, FlatNode*>::iterator iter2 = ssg->mapEdge2DownFlatNode.begin();
				//���������α߰�
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
				//���������α߰�
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
	//����һ����SSGͼ��tmplist���
	StaticStreamGraph*childgraph = new StaticStreamGraph();
	childgraph->flatNodes = tmplist;
	//��ӱ߰�
	map<Node*, FlatNode*>::iterator iter1 = ssg->mapEdge2UpFlatNode.begin();
	map<Node*, FlatNode*>::iterator iter2 = ssg->mapEdge2DownFlatNode.begin();
	//���������α߰�
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
	//���������α߰�
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
	//��������SSGͼ��,�����������ͼ��ӳ��
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

	//���combinelist
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
	ssg2coreNum.push_back(std::pair<SchedulerSSG*, int>(graph, coreNum));//��ͼ��ʹ�ú�����ӳ��
}
void DividedStaticGraph::allocatePosition()
{
	//��������λ�õ�flatNode2position
	for (vector<StaticStreamGraph*>::iterator iter1 = staticChildGraph.begin(); iter1 != staticChildGraph.end(); iter1++)
	{
		int size = (*iter1)->flatNodes.size();
		if (size == 1)
		{
			//�����ͼֻ��һ�����ӡ���ô������FIRSTҲ��LAST����ʹ��both
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
	//��ȡһ�����ӵ�λ��
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
	//��ȡһ����ͼ�������ӵ�������������ע����ͼ�����ӱ���ֻ��һ�������
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
		//ÿ��ssgͼ���г���̬����
		SchedulerSSG* tmpsssg = new SchedulerSSG(*iter1);
		if (tmpsssg->SteadyScheduling2())
		{
			tmpsssg->InitScheduling2();
		}
		else
		{
			fprintf(stdout, "���򲻴�����̬���ȣ��޷����д������ɣ�\n");
			system("pause");
			exit(1);
		}
		DSG->scheStaticChildGraph.push_back(tmpsssg);
	}
}

GLOBAL void ReviseDSGFunction(DividedStaticGraph*DSG)
{
	//����Ҫ�޸�
}
GLOBAL void GenerateWorkEst(DividedStaticGraph*dsg, bool WorkEstimateByDataFlow)
{
	for (vector<StaticStreamGraph*>::iterator iter1 = DSG->staticChildGraph.begin(); iter1 != DSG->staticChildGraph.end(); iter1++)
	{
		GenerateWorkEst(*iter1, WorkEstimateByDataFlow);
	}
}


