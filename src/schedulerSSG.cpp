#include "schedulerSSG.h"
#include <list>
#include <cmath>

using namespace std;

int SchedulerSSG::GetInitCount(FlatNode *node)
{
	std::map<FlatNode *, int> ::iterator pos;
	pos = mapInitCount2FlatNode.find(node);
	assert(pos!= mapInitCount2FlatNode.end());
	return pos->second;
}

int SchedulerSSG::GetSteadyCount(FlatNode *node)
{
	std::map<FlatNode *, int> ::iterator pos;
	pos = mapSteadyCount2FlatNode.find(node);
	assert(pos!=mapSteadyCount2FlatNode.end());
	return pos->second;
}

SchedulerSSG::SchedulerSSG(StaticStreamGraph *ssg)
{
	comName = ssg->GetName();
	flatNodes = ssg->GetFlatNodes();
	/*vTemplateNode = ssg->GetTemplateNode();
	vTemplateName = ssg->GetTemplateName();
	mapFlatnode2Template = ssg->GetFlatnode2Template();*/
	mapEdge2UpFlatNode = ssg->GetMapEdge2UpFlatNode();
	mapEdge2DownFlatNode = ssg->GetMapEdge2DownFlatNode();
	topLevel = ssg->GetTopLevel();
	mapSteadyWork2FlatNode = ssg->GetSteadyWorkMap();
	mapInitWork2FlatNode = ssg->GetInitWorkMap();
}

bool SchedulerSSG::SteadyScheduling()
{
	list<FlatNode *> flatNodeList;
	std::map<FlatNode *,int>::iterator pos;
	// 默认第一个节点是源，也就是说peek和pop均为0,在图的表示上暂不允许有多个源，但可以有多个peek = pop = 0节点
	FlatNode *up = topLevel, *down = NULL;
	int nPush = 0, nPop = 0, nLcm = 0;
	int x, y, i, j;

	// 现在考虑的是只有一个输入口的情况
	while (1)
	{
		// 稳态调度系列初始系数为1
		mapSteadyCount2FlatNode.insert(make_pair(up, 1));
		// 遍历该节点的所有输出节点（广度优先遍历）
		for (i = 0; i < up->nOut; ++i)
		{
			nPush = up->outPushWeights[i]; // 上端节点的push值
			//if(!nPush)continue;
			down = up->outFlatNodes[i]; // 找到下端节点

			for (j = 0; down->inFlatNodes[j] != up; j++); // 下端节点找到与上端节点对应的标号
			nPop = down->inPopWeights[j]; // 下端节点取出对应的pop值

			// 检查该节点是否已进行稳态调度，每条只进行一次稳态调度
			pos = mapSteadyCount2FlatNode.find(down);
			// 该节点未进行稳态调度
			if (pos == mapSteadyCount2FlatNode.end())
			{
				// 得到上端节点的稳态调度系数
				pos = mapSteadyCount2FlatNode.find(up);
				x = pos->second;
				nPush *= x; // 为什么是x*nPush呢？理解稳态调度的概念--节点在流水线稳定运行中执行的最少次数
				if(nPush != 0)
				{
					// nPush, nPop的最小公倍数;
					nLcm = lcm(nPush, nPop);
					int temp = nLcm/nPush;
					if( temp != 1) // 加一个判断，提高效率，乘1是不必要的
					{
						// 根据计算规则得来的
						for (pos = mapSteadyCount2FlatNode.begin(); pos != mapSteadyCount2FlatNode.end(); ++pos)
							pos->second *= temp;
					}
					mapSteadyCount2FlatNode.insert(make_pair(down, nLcm/nPop));
				}
				else // 对push(0)作处理 lxx.2012.02.22
				{
					assert(nPop == 0);
					// 取 1 值 lxx.2012.02.22
					mapSteadyCount2FlatNode.insert(make_pair(down, 1)); 
				}
				// 将down加入listNode是为了对down的输出节点进行调度
				flatNodeList.push_back(down);
			}
			else //该节点已进行稳态调度，检查SDF图是否存在稳态调度系列，一般不存在的话表明程序有误
			{
				y = pos->second;
				pos = mapSteadyCount2FlatNode.find(up);
				x = pos->second;

				//nPop == 0 说明在进行join 0 操作
				if((nPop != 0 ) && (nPush * x) != (nPop * y))
				{
					cout<<"不存在稳态调度..."<<endl;
					system("pause");
					exit(1); // 表示不存在稳态调度 
				}
			}
		}
		if(flatNodeList.size() == 0) break; // 链表为空，说明所有节点已调度完毕
		up = flatNodeList.front();
		flatNodeList.pop_front();
	}

	return true;
}

bool SchedulerSSG::InitScheduling()
{
	list<FlatNode *> flatNodeList;
	std::map<FlatNode *,int>::iterator pos;
	FlatNode *down = NULL, *up = NULL;
	int nPush = 0, nPop = 0, nPeek = 0;
	int x, y, i, j, n, num = 0;

	//现在考虑的是只有一个输出口的情况
	//找到sink节点
	for (i = 0; i < flatNodes.size(); ++i)
	{
		if(!flatNodes[i]->nOut)
		{
			down = flatNodes[i];
			num ++;
		}
	}

	if(num > 1)
	{
		fprintf(stdout, "FATAL ERROR: 程序存在多个输出口！\n");
		system("pause");
		exit(1);
	}

	if(num == 0)
	{
		fprintf(stdout, "FATAL ERROR: 程序无输出口！\n");
		system("pause");
		exit(1);
	}

	//每个节点的初始化调度次数初始值为0
	mapInitCount2FlatNode.insert(make_pair(down, 0));
	while (1)
	{
		//遍历该节点的所有输入节点
		for (i = 0; i < down->nIn; i++)
		{
			//找到下端节点的peek、pop值
			nPeek = down->inPeekWeights[i];
			nPop  = down->inPopWeights[i];
			up = down->inFlatNodes[i];

			//找到对应上端节点的pop值
			for (j = 0; up->outFlatNodes[j] != down; j++);
			nPush = up->outPushWeights[j];

			pos = mapInitCount2FlatNode.find(down);
			//下端节点已有的初始化调度次数
			x = pos->second;

			//下端节点运行一次需要的额外数据量
			y = nPeek - nPop;
			if(y <= 0 || nPeek <= nPush)
				y = 0;
			if(nPush != 0)	
				n = ceil((x * nPop + y)/float(nPush));
			else
				n = 0;
			pos = mapInitCount2FlatNode.find(up);
			if (pos == mapInitCount2FlatNode.end())//zww：20120322，为了找没有输出的节点而修改
			{
				mapInitCount2FlatNode.insert(make_pair(up, n));
				flatNodeList.push_back(up);
			}
			else
			{
				if(pos->second < n) 
				{
					pos->second = n;
					//该节点的初始化调度次数已改变，必须重新加入队列对其上端节点进行调度
					flatNodeList.push_back(up);
				}
			}
		}
		if(flatNodeList.size() == 0) break;//链表为空，说明所有节点已调度完毕
		down = flatNodeList.front();
		flatNodeList.pop_front();
	}
	return true;
}


//求a,b的最大公约数
int SchedulerSSG::gcd(int a, int b)
{
	int r = 0;
	if(a < b)
	{
		r = a;
		a = b; 
		b = r;
	}
	assert(b);
	while(a % b)
	{
		assert(b);
		r = a % b;
		a = b;
		b = r;
	}

	return b;
}


//求a,b的最小公倍数
int SchedulerSSG::lcm(int a, int b)
{
	int product = a * b;

	return product/gcd(a,b);
}

std::map<FlatNode *, int> SchedulerSSG::SteadySchedulingGroup(std::vector<FlatNode *>flatNodeVec)
{//构造一个局部的稳态


	list<FlatNode *> flatNodeList;
	std::map<FlatNode *,int>::iterator pos;
	std::map<FlatNode *, int> flatNode2SteadyCount;
	assert(flatNodeVec.size() > 0);
	map<FlatNode * ,Bool> flatNodesTag;//由于标示flatNodeVec中的节点是否被调度
	for (int indexNode = 0; indexNode != flatNodeVec.size(); indexNode++)
	{
		flatNodesTag.insert(make_pair(flatNodeVec[indexNode],FALSE));
	}
	// 默认第一个节点是源，也就是说peek和pop均为0,在图的表示上暂不允许有多个源，但可以有多个peek = pop = 0节点
	FlatNode *up = flatNodeVec[0], *down = NULL, *parent = NULL;
	int nPush = 0, nPop = 0, nLcm = 0;
	int x, y, i, j;
	Bool flag = FALSE;//只有当flatNode在flatNodeVec中才进行调度
	while (!flatNodesTag.empty())
	{	
		up = flatNodesTag.begin()->first;
		while(1)
		{		
			// 稳态调度系列初始系数为1
			flatNode2SteadyCount.insert(make_pair(up, 1));
			flatNodesTag.erase(up);
			// 遍历该节点的所有输出节点up对child节点进行的调度，该循环执行完，对up的parent节点进行调度
			/*调度up的child节点*/
			for (i = 0; i < up->nOut; ++i)
			{
				flag = FALSE;
				for (int k = 0; k != flatNodeVec.size(); ++k)
				{
					if(up->outFlatNodes[i] == flatNodeVec[k]) 
					{
						flag = TRUE;break;
					}
				}
				if(flag)
				{
					nPush = up->outPushWeights[i]; // 上端节点的push值
					down = up->outFlatNodes[i]; // 找到下端节点

					for (j = 0; down->inFlatNodes[j] != up; j++); // 下端节点找到与上端节点对应的标号
					nPop = down->inPopWeights[j]; // 下端节点取出对应的pop值

					// 检查该节点是否已进行稳态调度，每条只进行一次稳态调度
					pos = flatNode2SteadyCount.find(down);
					// 该节点未进行稳态调度
					if (pos == flatNode2SteadyCount.end())
					{
						// 得到上端节点的稳态调度系数
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;
						nPush *= x; // 为什么是x*nPush呢？理解稳态调度的概念--节点在流水线稳定运行中执行的最少次数
						if(nPush != 0)
						{
							// nPush, nPop的最小公倍数;
							nLcm = lcm(nPush, nPop);
							int temp = nLcm/nPush;
							if( temp != 1) // 加一个判断，提高效率，乘1是不必要的
							{
								// 根据计算规则得来的
								for (pos = flatNode2SteadyCount.begin(); pos != flatNode2SteadyCount.end(); ++pos)
									pos->second *= temp;
							}

							flatNode2SteadyCount.insert(make_pair(down, nLcm/nPop));
							flatNodesTag.erase(down);
							// 将down加入listNode是为了对down的输出节点进行调度
							flatNodeList.push_back(down);
						}

					}
					else //该节点已进行稳态调度，检查SDF图是否存在稳态调度系列，一般不存在的话表明程序有误
					{
						y = pos->second;
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;

						//nPop == 0 说明在进行join 0 操作
						if((nPop != 0 ) && (nPush * x) != (nPop * y)) 
						{
							cout<<"不存在稳态调度1"<<endl;
							system("pause");
							exit(1); // 表示不存在稳态调度
						}
					}
				}
			}
			/*调度up的parent节点*/
			for (i = 0; i < up->nIn; ++i)
			{
				flag = FALSE;
				for (int k = 0; k != flatNodeVec.size(); ++k)
				{//判断parent节点在不在flatNodeVec中
					if(up->inFlatNodes[i] == flatNodeVec[k]) 
					{
						flag = TRUE;break;
					}
				}

				if(flag)
				{
					nPop = up->inPopWeights[i]; // 当前节点的pop值
					parent = up->inFlatNodes[i]; // 找到当前节点的父节点

					for (j = 0; parent->outFlatNodes[j] != up; j++); // up节点在parent的输出节点中对应的标号
					nPush = parent->outPushWeights[j]; // parent节点取出对应的push值

					// 检查该节点是否已进行稳态调度，每条只进行一次稳态调度
					pos = flatNode2SteadyCount.find(parent);
					// 该节点未进行稳态调度
					if (pos == flatNode2SteadyCount.end())
					{
						// 得到上端节点的稳态调度系数
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;
						nPop *= x; // 为什么是x*nPush呢？理解稳态调度的概念--节点在流水线稳定运行中执行的最少次数
						if(nPop != 0)
						{
							// nPush, nPop的最小公倍数;
							nLcm = lcm(nPush, nPop);
							int temp = nLcm/nPop;
							if( temp != 1) // 加一个判断，提高效率，乘1是不必要的
							{
								// 根据计算规则得来的
								for (pos = flatNode2SteadyCount.begin(); pos != flatNode2SteadyCount.end(); ++pos)
									pos->second *= temp;
							}

							flatNode2SteadyCount.insert(make_pair(parent, nLcm/nPush));
							flatNodesTag.erase(parent);
							// 将down,parent加入listNode是为了对down的输出节点进行调度
							flatNodeList.push_back(parent);
						}

					}
					else //该节点已进行稳态调度，检查SDF图是否存在稳态调度系列，一般不存在的话表明程序有误
					{
						y = pos->second;
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;

						//nPop == 0 说明在进行join 0 操作
						if((nPop != 0 ) && (nPop * x) != (nPush * y))
						{
							cout<<"不存在稳态调度2"<<endl;
							system("pause");
							exit(1); // 表示不存在稳态调度
						}
					}
				}

			}
			if(flatNodeList.size() == 0) break; // 链表为空，说明所有节点已调度完毕
			up = flatNodeList.front();
			flatNodeList.pop_front();

		}
	}

	return flatNode2SteadyCount;
}