#include "HClusterStageAssignment.h"

HClusterStageAssignment::HClusterStageAssignment(HClusterPartition * hcp, SchedulerSSG * sssg):m_hcp(hcp),m_sssg(sssg)
{
}

/*处理异构集群的阶段赋值*/
void HClusterStageAssignment::createStageAssignment()
{
	/*得到集群节点的任务划分结果*/
	map<int, vector<FlatNode *>> tmpHClusterNum2FlatNodes = this->m_hcp->getHClusterNum2FlatNodes();
	//得到集群节点个数
	int hClusterNum = m_hcp->getHClusters();
	//验证划分结果的正确性
	assert(hClusterNum == tmpHClusterNum2FlatNodes.size());

	for (int i = 0; i < hClusterNum; ++i)
	{
		multimap<int, FlatNode *> tmpStage2FlatNodes;
		map<FlatNode*, int> tmpFlatNode2Stage;

		//纯CPU服务器
		if (!m_hcp->isGPUClusterNode(i))
		{
			//清空存储
			tmpStage2FlatNodes.clear();
			tmpFlatNode2Stage.clear();

			/*得到子图actor节点与CPU核编号的映射*/
			map<FlatNode*, int> tmpFlatNode2CoreNum = m_hcp->getFlatNode2HClusterCore(i);

			//得到actor节点到阶段号的映射
			tmpFlatNode2Stage = createFlatNode2StageForCPU(tmpFlatNode2CoreNum, i);
			this->m_hCluster2FlatNode2StageNum.insert(make_pair(i, tmpFlatNode2Stage));

			//得到阶段号到actor节点的映射
			map<FlatNode*, int>::iterator iter = tmpFlatNode2Stage.begin();
			while (iter != tmpFlatNode2Stage.end())
			{
				tmpStage2FlatNodes.insert(make_pair(iter->second, iter->first));
				++iter;
			}//while

			this->m_hCluster2StageNum2FlatNode.insert(make_pair(i, tmpStage2FlatNodes));

			//得到节点到集群编号到阶段号的映射
			for (map<int, map<FlatNode*, int>>::iterator iter1 = m_hCluster2FlatNode2StageNum.begin(); iter1 != m_hCluster2FlatNode2StageNum.end(); ++iter1)
			{
				for (map<FlatNode*, int>::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2)
				{
					m_flatNode2hCluster2StageNum.insert(make_pair(iter2->first, make_pair(iter1->first, iter2->second)));
				}//for
			}//for

		}//if
		 //GPU混合架构服务器节点阶段赋值
		else {
			tmpStage2FlatNodes.clear();
			tmpFlatNode2Stage.clear();

			/*得到子图actor节点与编号的映射*/
			map<FlatNode*, int> tmpFlatNode2GPUNum = m_hcp->getStatelessNode2GpuNum(i);
			map<FlatNode*, int> tmpFlatNode2CoreNum = m_hcp->getFlatNode2HClusterCore(i);


			//得到actor节点到阶段号的映射
			tmpFlatNode2Stage = createFlatNode2StageForGPU(tmpFlatNode2GPUNum, tmpFlatNode2CoreNum, i);
			this->m_hCluster2FlatNode2StageNum.insert(make_pair(i, tmpFlatNode2Stage));

			//得到阶段号到actor节点的映射
			map<FlatNode*, int>::iterator iter = tmpFlatNode2Stage.begin();
			while (iter != tmpFlatNode2Stage.end())
			{
				tmpStage2FlatNodes.insert(make_pair(iter->second, iter->first));
				++iter;
			}//while

			this->m_hCluster2StageNum2FlatNode.insert(make_pair(i, tmpStage2FlatNodes));

			//得到节点到集群编号到阶段号的映射
			for (map<int, map<FlatNode*, int>>::iterator iter1 = m_hCluster2FlatNode2StageNum.begin(); iter1 != m_hCluster2FlatNode2StageNum.end(); ++iter1)
			{
				for (map<FlatNode*, int>::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2)
				{
					m_flatNode2hCluster2StageNum.insert(make_pair(iter2->first, make_pair(iter1->first, iter2->second)));
				}//for
			}//for	
		}//else	
	}//for

	 //打印阶段赋值结果
#if 0
	printStageResult();
#endif
}

void HClusterStageAssignment::printStageResult()
{
	assert(m_hCluster2FlatNode2StageNum.size() == m_hcp->getHClusters());
	for (map<int, map<FlatNode*, int>>::iterator iter1 = m_hCluster2FlatNode2StageNum.begin(); iter1 != m_hCluster2FlatNode2StageNum.end(); ++iter1)
	{
		cout << "######集群节点《" << iter1->first << "》######actor阶段赋值结果：" << endl;
		for (map<FlatNode*, int>::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2)
		{
			cout << "《" << iter2->first->name << " , " << iter2->second << " 》" << endl;
		}//for
		cout << "=======================================" << endl;

	}//for
	cout << endl;
}

map<FlatNode*, int> HClusterStageAssignment::createFlatNode2StageForCPU(map<FlatNode*, int> &flatNode2CoreNum, int hClusterNum)
{
	map<FlatNode*, int> flatNode2Stage;
	
	//得到当前子图的所有actor集合
	vector<FlatNode *> tmpFlatNodes = m_hcp->getSubFlatNodes(hClusterNum);

	//对节点集合进行拓扑排序
	vector<FlatNode*> topoFlatNodes = createTopoLogicalOrder(tmpFlatNodes);

#if 0
	cout << "############拓扑排序结果验证################" << endl;
	for (vector<FlatNode *>::iterator iter = topoFlatNodes.begin(); iter != topoFlatNodes.end(); ++iter)
	{
		cout << (*iter)->name << endl;
	}//for
	cout << "############## end #####################" << endl;
#endif

	int maxStage = 0, tmpStage = 0;
	bool inTag,flag;

	for (vector<FlatNode *>::iterator iter1 = topoFlatNodes.begin(); iter1 != topoFlatNodes.end(); ++iter1)
	{
		maxStage = 0;
		flag = false;
		for (vector<FlatNode *>::iterator iter2 = (*iter1)->inFlatNodes.begin(); iter2 != (*iter1)->inFlatNodes.end(); ++iter2)
		{
			inTag = false;
			if (find(tmpFlatNodes.begin(), tmpFlatNodes.end(), *iter2) != tmpFlatNodes.end())
				inTag = true;

			//如果当前上游节点在当前子图中
			if (inTag)
			{
				map<FlatNode *, int>::iterator iter3 = flatNode2Stage.find(*iter2);

				if (iter3 != flatNode2Stage.end() && iter3->second > maxStage)
				{
					maxStage = iter3->second;
				}//if

				//查看当前父子节点对应的二级划分号
				map <FlatNode *, int>::iterator parent = flatNode2CoreNum.find(*iter2);
				assert(parent != flatNode2CoreNum.end());

				map<FlatNode*, int >::iterator child = flatNode2CoreNum.find(*iter1);
				assert(child != flatNode2CoreNum.end());

				/*如果不在一个划分核上，设置标志*/
				if (parent->second != child->second)
				{
					flag = true;
				}//if
			}//if
		}//for

		//当前节点的阶段号，为所有上游节点最大阶段号增加1
		if (flag)
		{
			tmpStage = maxStage + 1;
		}//if
		else {
			tmpStage = maxStage;
		}

		flatNode2Stage.insert(make_pair(*iter1, tmpStage));
	}//for

	#if 0
	cout << "#############打印阶段赋值结果############" << endl;
	for (map<FlatNode *, int>::iterator iter = flatNode2Stage.begin(); iter != flatNode2Stage.end(); ++iter)
	{
		cout << "<" << iter->first->name << " , " << iter->second << ">" << endl;
	}//for
	cout << "############### end #############" << endl;
	#endif
	return flatNode2Stage;
}

map<FlatNode*, int> HClusterStageAssignment::createFlatNode2StageForGPU(map<FlatNode*, int> &flatNode2GpuNum, map<FlatNode*, int> &statefulNode2CoreNum, int hClusterNum)
{
	assert(m_hcp->isGPUClusterNode(hClusterNum));
	map<FlatNode*, int> flatNode2Stage;

	/*得到当前服务器上的所有actor节点*/
	vector<FlatNode *> tmpFlatNodes = m_hcp->getSubFlatNodes(hClusterNum);
	assert(tmpFlatNodes.size() == (flatNode2GpuNum.size() + statefulNode2CoreNum.size()));
	//对节点集合进行拓扑排序
	vector<FlatNode*> topoFlatNodes = createTopoLogicalOrder(tmpFlatNodes);
#if 0
	cout << "############拓扑排序结果验证################" << endl;
	for (vector<FlatNode *>::iterator iter = topoFlatNodes.begin(); iter != topoFlatNodes.end(); ++iter)
	{
		cout << (*iter)->name << endl;
	}//for
	cout << "############## end #####################" << endl;
#endif

	int maxStage = 0, tmpStage = 0;
	bool inTag=false,flag1,flag2;
	for (vector<FlatNode *>::iterator iter1 = topoFlatNodes.begin(); iter1 != topoFlatNodes.end(); ++iter1)
	{
		maxStage = 0;
		flag1 = false;
		flag2 = false;
		for (vector<FlatNode *>::iterator iter2 = (*iter1)->inFlatNodes.begin(); iter2 != (*iter1)->inFlatNodes.end(); ++iter2)
		{
			inTag = false;
			if (find(tmpFlatNodes.begin(), tmpFlatNodes.end(), *iter2) != tmpFlatNodes.end())
				inTag = true;

			map<FlatNode *, int>::iterator iter3 = flatNode2Stage.find(*iter2);

			if (iter3 != flatNode2Stage.end() && iter3->second > maxStage)
			{
				maxStage = iter3->second;
			}//if

			//在同一台服务器上
			if (inTag)
			{
				//都被划分到混合架构服务器的GPU端
				if (m_hcp->isNodeInGPU(hClusterNum, *iter1) && m_hcp->isNodeInGPU(hClusterNum, *iter2))
				{
					//查看当前父子节点对应的二级划分号
					map <FlatNode *, int>::iterator parent = flatNode2GpuNum.find(*iter2);
					assert(parent != flatNode2GpuNum.end());

					map<FlatNode*, int >::iterator child = flatNode2GpuNum.find(*iter1);
					assert(child != flatNode2GpuNum.end());

					/*判断是否在同一个GPU设置阶段号*/
					if (parent->second != child->second)
					{
						flag1 = true;
					}//if
				}//if
				//在同一台服务器上，且都被划分到CPU端
				else if (m_hcp->isNodeInCPU(hClusterNum, *iter1) && m_hcp->isNodeInCPU(hClusterNum, *iter2))
				{
					map<FlatNode *, int>::iterator iter3 = flatNode2Stage.find(*iter2);

					if (iter3 != flatNode2Stage.end() && iter3->second > maxStage)
					{
						maxStage = iter3->second;
					}//if

					 //查看当前父子节点对应的二级划分号
					map <FlatNode *, int>::iterator parent = statefulNode2CoreNum.find(*iter2);
					assert(parent != statefulNode2CoreNum.end());

					map<FlatNode*, int >::iterator child = statefulNode2CoreNum.find(*iter1);
					assert(child != statefulNode2CoreNum.end());

					/*判断是否在一个划分核，设置阶段号*/
					if (parent->second != child->second)
					{
						flag1 = true;
					}//if
				}//elif
				else {
					flag2 = true;
				}
			}//if
		}//for

		//更新当前最大阶段号
		if (flag1 || flag2)
		{
			tmpStage = maxStage + 1;
		}
		else if (flag2) {
			tmpStage = maxStage + 2;
		}
		else {
			tmpStage = maxStage;
		}
		flatNode2Stage.insert(make_pair(*iter1, tmpStage));
	}//for
	return flatNode2Stage;
}

vector<FlatNode*> HClusterStageAssignment::createTopoLogicalOrder(vector<FlatNode*> &flatNodes)
{
	/*拓扑结果*/
	vector<FlatNode *> tmpTopoNodes;
	/*保存节点入度*/
	vector<int> inDegree;
	vector<FlatNode*> flatNodeStack;
	int size = flatNodes.size();
	for (int i = 0; i < size; ++i)
	{
		int inCount = 0;
		for (int j = 0; j < flatNodes[i]->nIn; ++j)
		{
			if (find(flatNodes.begin(),flatNodes.end(),flatNodes[i]->inFlatNodes[j]) != flatNodes.end())
				++inCount;
		}//for
		inDegree.push_back(inCount);
	}

	for (int i = 0; i < size; ++i)
	{
		if (inDegree[i] == 0)
			flatNodeStack.push_back(flatNodes[i]);
	}//for

	while (!flatNodeStack.empty())
	{
		// 取将要出栈的节点
		FlatNode *tmpFlatNode = flatNodeStack.back();
		tmpTopoNodes.push_back(tmpFlatNode);
		flatNodeStack.pop_back();
		//对所有group的邻接点的入度减1
		for (int i = 0; i != tmpFlatNode->nOut; ++i)
		{
			for (int j = 0; j < size; ++j)
			{
				if (flatNodes[j] == tmpFlatNode->outFlatNodes[i])
					if (!(--inDegree[j]))
						flatNodeStack.push_back(flatNodes[j]);
			}//for
		}//for
	}//while

	assert(tmpTopoNodes.size() == flatNodes.size());

	return tmpTopoNodes;
}

int HClusterStageAssignment::getStageNum(FlatNode * node)
{
	map<FlatNode *, pair<int, int>>::iterator iter = this->m_flatNode2hCluster2StageNum.find(node);
	assert(iter != m_flatNode2hCluster2StageNum.end());
	if (iter == m_flatNode2hCluster2StageNum.end())
		return -1;
	return iter->second.second;
}

int HClusterStageAssignment::getMaxStageNum(int hClusterNum)
{
	multimap<int, FlatNode *> tmpStage2Actor = getStage2FlatNodesOfHCluster(hClusterNum);
	multimap<int, FlatNode*>::reverse_iterator iter = tmpStage2Actor.rbegin();
	return iter->first + 1;
}

int HClusterStageAssignment::getHClusterNum(FlatNode * node)
{
	map<FlatNode *, pair<int, int>>::iterator iter = this->m_flatNode2hCluster2StageNum.find(node);
	assert(iter != m_flatNode2hCluster2StageNum.end());
	if (iter == m_flatNode2hCluster2StageNum.end())
		return -1;
	return iter->second.first;
}

map<FlatNode*, int> HClusterStageAssignment::getFlatNode2StageOfHCluster(int hClusterNum)
{
	map<int, map<FlatNode *, int> >::iterator iter= m_hCluster2FlatNode2StageNum.find(hClusterNum);
	assert(iter != m_hCluster2FlatNode2StageNum.end());

	return iter->second;
}

multimap<int, FlatNode*> HClusterStageAssignment::getStage2FlatNodesOfHCluster(int hClusterNum)
{
	map<int, multimap<int, FlatNode *>>::iterator iter = m_hCluster2StageNum2FlatNode.find(hClusterNum);
	assert(iter != m_hCluster2StageNum2FlatNode.end());
	return iter->second;
}

vector<FlatNode*> HClusterStageAssignment::getFlatNodesOfHClusterAndStage(int hClusterNum, int stage)
{
	vector<FlatNode *> swapVec;
	m_actorSet.swap(swapVec);
	multimap<int, FlatNode*> tmpStage2Actor = getStage2FlatNodesOfHCluster(hClusterNum);

	typedef multimap<int, FlatNode*>::iterator Num2NodeIter;
	pair<Num2NodeIter, Num2NodeIter>range = tmpStage2Actor.equal_range(stage);
	for (Num2NodeIter iter = range.first; iter != range.second; ++iter)
	{
		m_actorSet.push_back(iter->second);
	}
	return createTopoLogicalOrder(m_actorSet);
}

