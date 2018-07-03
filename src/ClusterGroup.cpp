#include "ClusterGroup.h"
using namespace std;


ClusterGroup::ClusterGroup(FlatNode *flatNode)//构造函数
{//只有一个节点构造group，一个group中就含有一个flatNode节点。(根据SDF图构造初始flatNode)
	srcFlatNode.push_back(flatNode);//源
	snkFlatNode.push_back(flatNode);//终
	flatNodes.push_back(flatNode);//结点集合
	flatNode2SteadyCount.insert(make_pair(flatNode,1));//稳态
	for(int i=0;i<flatNode->inFlatNodes.size();i++)
			if(flatNode->inPopWeights[i])precedenceFlatNode.push_back(flatNode->inFlatNodes[i]);//插前驱flatNode (有数据传输)
	for(int i=0;i<flatNode->outFlatNodes.size();i++)
			if(flatNode->outPushWeights[i])successorFlatNode.push_back(flatNode->outFlatNodes[i]);//插后继flatNode(有数据传输)
	groupSize = 1;
	workload = SSG->GetSteadyWork(flatNode);
	float tmpCost=0;
	for(int i = 0;i < flatNode->outPushWeights.size();i++)
	{
		tmpCost +=BOOTTIME + (flatNode->outPushWeights[i])/BANDWITH; 
	}
	for(int i=0;i<flatNode->inPopWeights.size();i++)
	{
		tmpCost +=BOOTTIME + (flatNode->inPopWeights[i])/BANDWITH;
	}
	commCost = tmpCost;
	lock = FALSE;
}

ClusterGroup::ClusterGroup(ClusterGroup* group1,ClusterGroup* group2)//构造函数
{//将两个相邻的group聚合成一个
	//flatNode 节点的集合
	lock = FALSE;
	for(int i = 0; i!= group1->flatNodes.size(); i++)
	{
		flatNodes.push_back(group1->flatNodes[i]);
	}
	for(int i = 0; i!= group2->flatNodes.size(); i++)
	{
		flatNodes.push_back(group2->flatNodes[i]);
	}
	 // 前驱flatNode
	Bool flag = TRUE;
	for (int i = 0;i != group1->precedenceFlatNode.size(); i++)
	{
		flag = TRUE;
		for (int j = 0;j != group2->flatNodes.size();j++)
		{
			if(group1->precedenceFlatNode[i]== group2->flatNodes[j]){flag = FALSE;break;}//如果group1的前驱在group2中，则该前驱不能作为新的group的前驱
		}
		if(flag)  this->AddPrecedenceFlatNode(group1->precedenceFlatNode[i]);
	}
	for (int i = 0;i != group2->precedenceFlatNode.size(); i++)
	{//要排除在group1和group1的前驱中
		flag = TRUE;
		for (int j = 0;j != group1->flatNodes.size();j++)
		{
			for(int k = 0 ;k!= group1->precedenceFlatNode.size();k++)
			if(group2->precedenceFlatNode[i]== group1->flatNodes[j] ) {flag = FALSE;break;}	
		}
		if(flag) this->AddPrecedenceFlatNode(group2->precedenceFlatNode[i]);
	}
	//后继flatNode
	for (int i = 0;i != group1->successorFlatNode.size(); i++)
	{
		flag = TRUE;
		for (int j = 0;j != group2->flatNodes.size();j++)
		{
			if(group1->successorFlatNode[i]== group2->flatNodes[j]) {flag = FALSE;break;} 
		}
		if(flag) this->AddSuccessorFlatNode(group1->successorFlatNode[i]);
	}
	for (int i = 0;i != group2->successorFlatNode.size(); i++)
	{
		flag = TRUE;
		for (int j = 0;j != group1->flatNodes.size();j++)
		{
			if(group2->successorFlatNode[i]== group1->flatNodes[j]) {flag = FALSE;break;} 
		}
		if(flag) this->AddSuccessorFlatNode(group2->successorFlatNode[i]);
	}
	//向src 和snk中添加元素(有出去的边就是终点，有进来的边就是起点)
	for (int i = 0; i != this->flatNodes.size(); i++)
	{
		int num = 0;
		flag = TRUE;
		int zeroEdge = 0;
		for (int j = 0; j != this->flatNodes[i]->inFlatNodes.size();j++)
		{
			if(this->flatNodes[i]->inPopWeights[j] == 0) {zeroEdge++;continue;}////20121204添加（处理有边连接但没有数据传输）

			for (int kk = 0; kk!= this->srcFlatNode.size();kk++)
			{
				if(this->flatNodes[i]== this->srcFlatNode[kk] ){flag = FALSE; break;}//已经在src中就不插入
			}
			if (flag)
			{
				for(int k = 0; k != this->flatNodes.size(); k++)
				{
					if(this->flatNodes[i]->inFlatNodes[j] == this->flatNodes[k] ) {num++; break;}
				}	
			}					
		}
		if(num + zeroEdge !=this->flatNodes[i]->inFlatNodes.size()) this->srcFlatNode.push_back(this->flatNodes[i]);
		if(this->flatNodes[i]->inFlatNodes.size() == 0) this->srcFlatNode.push_back(this->flatNodes[i]);//处理source节点
		num = 0;
		flag = TRUE;
		zeroEdge = 0;
		for (int j = 0; j != this->flatNodes[i]->outFlatNodes.size();j++)
		{
			if(this->flatNodes[i]->outPushWeights[j] == 0){ zeroEdge++;continue;}////20121204添加（处理有边连接但没有数据传输）
			for (int kk = 0; kk!= this->snkFlatNode.size();kk++)
			{
				if(this->flatNodes[i]== this->snkFlatNode[kk] ){flag = FALSE; break;}//已经在src中就不插入
			}
			if (flag)
			{
				for(int k = 0; k != this->flatNodes.size(); k++)
				{
					if(this->flatNodes[i]->outFlatNodes[j] == this->flatNodes[k] ) {num++; break;}
				}	
			}
		}
		if(num + zeroEdge !=this->flatNodes[i]->outFlatNodes.size()) this->snkFlatNode.push_back(this->flatNodes[i]);
		if(this->flatNodes[i]->outFlatNodes.size() == 0) this->snkFlatNode.push_back(this->flatNodes[i]);
	}
	// group1，group2 作为子group添加到新的group中
	subClusterGroups.push_back(group1);
	subClusterGroups.push_back(group2);
	//调度
	flatNode2SteadyCount = SteadySchedulingGroup(flatNodes);
	//计算通信开销
	std::vector<int> sendDataSize;//对应的是终
	std::vector<int> recvDataSize;//对应的是源
	float tmpCost = 0;
	std::map<FlatNode *,int>::iterator pos;
	for (int i = 0; i != srcFlatNode.size();i++)
	{
		flag = FALSE;
		pos = flatNode2SteadyCount.find(srcFlatNode[i]);
		assert(pos != flatNode2SteadyCount.end());
		for (int j = 0;j != srcFlatNode[i]->inFlatNodes.size(); j++)
		{
			for (int k = 0; k != this->flatNodes.size(); k++)
			{
				if(srcFlatNode[i]->inFlatNodes[j] == this->flatNodes[k]){flag = TRUE;break;}
			}
			if(!flag)
			{
				recvDataSize.push_back(pos->second * srcFlatNode[i]->inPopWeights[j]);
				tmpCost += BOOTTIME + ((float)(pos->second * srcFlatNode[i]->inPopWeights[j]))/BANDWITH;
			}
		}		
	}
	for (int i = 0; i != snkFlatNode.size();i++)
	{	
		flag = FALSE;
		pos = flatNode2SteadyCount.find(snkFlatNode[i]);
		assert(pos != flatNode2SteadyCount.end());
		for (int j = 0;j != snkFlatNode[i]->outFlatNodes.size(); j++)
		{
			for (int k = 0; k != this->flatNodes.size(); k++)
			{
				if(snkFlatNode[i]->outFlatNodes[j] == this->flatNodes[k]) {flag=TRUE;break;}
			}
			if( !flag )
			{
				recvDataSize.push_back(pos->second * snkFlatNode[i]->outPushWeights[j]);
				tmpCost += BOOTTIME + ((float)(pos->second * snkFlatNode[i]->outPushWeights[j]))/BANDWITH;
			}
		}		
	}
	commCost = tmpCost;
	//确定group的工作量
	int tmpWork = 0;
	for(int i = 0 ;i != this->flatNodes.size(); i++)
	{	
		tmpWork += SSG->GetSteadyWork(this->flatNodes[i]) * flatNode2SteadyCount.find(this->flatNodes[i])->second;
	}
	workload = tmpWork;
	groupSize = group1->groupSize + group2->groupSize;
}


std::map<FlatNode *,int> ClusterGroup::GetSteadyCountMap()
{
	return flatNode2SteadyCount;
}
std::vector<FlatNode *> ClusterGroup::GetSrcFlatNode()
{//取源
	return srcFlatNode;
}

std::vector<FlatNode *> ClusterGroup::GetSnkFlatNode()
{//取终
	return snkFlatNode;
}

std::vector<FlatNode *> ClusterGroup::GetFlatNodes()
{
	return flatNodes;
}
std::vector<FlatNode *> ClusterGroup::GetPrecedenceFlatNode()
{
	return precedenceFlatNode;
}

void ClusterGroup::AddPrecedenceFlatNode(FlatNode *flatNode)
{
	Bool flag = TRUE;
	for(int i=0; i != precedenceFlatNode.size(); i++)
	{
		if(precedenceFlatNode[i] == flatNode){flag = FALSE; break;}
	}
	if(flag) precedenceFlatNode.push_back(flatNode);
}

void ClusterGroup::DeletePrecedenceFlatNode(FlatNode * flatNode)
{
	std::vector<FlatNode *>::iterator pos;
	for(std::vector<FlatNode*>::iterator iter = precedenceFlatNode.begin();iter	!= precedenceFlatNode.end();iter++)
	{
		if (*iter = flatNode)
		{
			pos = iter;
			break;
		}
	}
	assert(pos!=precedenceFlatNode.end());
	precedenceFlatNode.erase(pos);
}

std::vector<FlatNode *> ClusterGroup::GetSuccessorFlatNode()
{
	return successorFlatNode;
}

void ClusterGroup::AddSuccessorFlatNode(FlatNode *flatNode)
{
	Bool flag = TRUE;
	for(int i=0; i != successorFlatNode.size(); i++)
	{
		if(successorFlatNode[i] == flatNode){flag = FALSE; break;}
	}
	if(flag) successorFlatNode.push_back(flatNode);
}
void ClusterGroup::DeleteSuccessorFlatNode(FlatNode * flatNode)
{
	std::vector<FlatNode *>::iterator pos;
	for(std::vector<FlatNode*>::iterator iter = successorFlatNode.begin();iter	!= successorFlatNode.end();iter++)
	{
		if (*iter = flatNode)
		{
			pos = iter;
			break;
		}
	}
	assert(pos!=successorFlatNode.end());
	successorFlatNode.erase(pos);
}

std::vector<ClusterGroup *> ClusterGroup::GetClusterGroups()
{
	return subClusterGroups;
}

std::vector<ClusterGroup *> ClusterGroup::GetPrecedenceClusterGroup()
{
	return  precedenceClusterGroup;
}
void ClusterGroup::AddPrecedenceClusterGroup(ClusterGroup *group)
{//后继中group不重复
	Bool flag = TRUE;
	for(int i=0; i != precedenceClusterGroup.size(); i++)
	{
		if(precedenceClusterGroup[i] == group){flag = FALSE; break;}
	}
	if(flag) precedenceClusterGroup.push_back(group);
}

std::vector<ClusterGroup *> ClusterGroup::GetSuccessorClusterGroup()
{
	return  successorClusterGroup;
}

std::vector<ClusterGroup *> ClusterGroup::GetBoundaryClusterGroup()
{
	std::vector<ClusterGroup *> vecswap;
	Bool flag = FALSE;
	boundaryGroupSet.swap(vecswap);//清空原来的内容，并释放内存
	for(int i = 0; i != subClusterGroups.size(); i++)
	{
		flag = FALSE;
		std::vector<ClusterGroup *> tmpPreGroup = subClusterGroups[i]->GetPrecedenceClusterGroup();
		for (int j = 0; j != tmpPreGroup.size(); j++)
		{
			int k = 0;
			for(k = 0; k != subClusterGroups.size();k++)
			{
				if(tmpPreGroup[j] == subClusterGroups[k]) break;
			}
			if(k == subClusterGroups.size()) {flag = TRUE;break;}//有前驱不在group内
		}
		boundaryGroupSet.push_back(successorClusterGroup[i]);
		if(flag) continue;

		std::vector<ClusterGroup *> tmpSucGroup = subClusterGroups[i]->GetSuccessorClusterGroup();
		for (int j = 0; j != tmpSucGroup.size(); j++)
		{
			int k = 0;
			for(k = 0; k != subClusterGroups.size();k++)
			{
				if(tmpPreGroup[j] == subClusterGroups[k]) break;
			}
			if(k == subClusterGroups.size()) {flag = TRUE;break;}//有前驱不在group内
		}
		boundaryGroupSet.push_back(successorClusterGroup[i]);
	}	
	return boundaryGroupSet;

}

void ClusterGroup::AddSuccessorClusterGroup(ClusterGroup *group)
{//后继中group不重复
	Bool flag = TRUE;
	for(int i=0; i != successorClusterGroup.size(); i++)
	{
		if(successorClusterGroup[i] == group){flag = FALSE; break;}
	}
	if(flag) successorClusterGroup.push_back(group);
}

Bool ClusterGroup::DeleteSuccessorClusterGroup(ClusterGroup *group)
{//后继中group不重复
	std::vector<ClusterGroup *>::iterator iter;
	for(iter=successorClusterGroup.begin(); iter != successorClusterGroup.end(); iter++)
	{
		if(*iter == group){break;}
	}
	if(iter != successorClusterGroup.end()) 
	{
		successorClusterGroup.erase(iter);
		return TRUE;
	}
	else return FALSE;
}

void ClusterGroup::SetSuccessorClusterGroup(std::vector<ClusterGroup *> original)
{
	std::vector<ClusterGroup *> tmp;
	successorClusterGroup.swap(tmp);
	for(int i = 0; i!= original.size();i++)
	{
		successorClusterGroup.push_back((original[i]));
	}
}

void ClusterGroup::SetPrecedenceClusterGroup(std::vector<ClusterGroup *> original)
{
	std::vector<ClusterGroup *> tmp;
	precedenceClusterGroup.swap(tmp);
	for(int i = 0; i!= original.size();i++)
	{
		precedenceClusterGroup.push_back((original[i]));
	}
}

Bool ClusterGroup::DeletePrecedenceClusterGroup(ClusterGroup *group)
{//后继中group不重复
	std::vector<ClusterGroup *>::iterator iter;
	for(iter=precedenceClusterGroup.begin(); iter != precedenceClusterGroup.end(); iter++)
	{
		if(*iter == group){break;}
	}
	if(iter != precedenceClusterGroup.end()) 
	{
		precedenceClusterGroup.erase(iter);
		return TRUE;
	}
	else return FALSE;
}

float ClusterGroup::GetCommunicationCost()
{
	return commCost;
}
void ClusterGroup::SetCommunicationCost(float cost)
{
	commCost = cost;
}

int ClusterGroup::GetWorkload()
{
	return workload;
}
void ClusterGroup::SetWorkload(int weight)
{
	workload = weight;
}

float ClusterGroup::GetCompCommRadio()
{
	return ((float)workload) / commCost;
}

//求a,b的最大公约数
int gcd(int a, int b)
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
int lcm(int a, int b)
{
	int product = a * b;

	return product/gcd(a,b);
}

GLOBAL std::map<FlatNode *, int> SteadySchedulingGroup(std::vector<FlatNode *>flatNodeVec)
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




