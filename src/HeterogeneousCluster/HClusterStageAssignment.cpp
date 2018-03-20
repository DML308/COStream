#include "HClusterStageAssignment.h"

HClusterStageAssignment::HClusterStageAssignment(HClusterPartition * hcp, SchedulerSSG * sssg):m_hcp(hcp),m_sssg(sssg)
{
}

/*�����칹��Ⱥ�Ľ׶θ�ֵ*/
void HClusterStageAssignment::createStageAssignment()
{
	/*�õ���Ⱥ�ڵ�����񻮷ֽ��*/
	map<int, vector<FlatNode *>> tmpHClusterNum2FlatNodes = this->m_hcp->getHClusterNum2FlatNodes();
	//�õ���Ⱥ�ڵ����
	int hClusterNum = m_hcp->getHClusters();
	//��֤���ֽ������ȷ��
	assert(hClusterNum == tmpHClusterNum2FlatNodes.size());

	for (int i = 0; i < hClusterNum; ++i)
	{
		multimap<int, FlatNode *> tmpStage2FlatNodes;
		map<FlatNode*, int> tmpFlatNode2Stage;

		//��CPU������
		if (!m_hcp->isGPUClusterNode(i))
		{
			//��մ洢
			tmpStage2FlatNodes.clear();
			tmpFlatNode2Stage.clear();

			/*�õ���ͼactor�ڵ���CPU�˱�ŵ�ӳ��*/
			map<FlatNode*, int> tmpFlatNode2CoreNum = m_hcp->getFlatNode2HClusterCore(i);

			//�õ�actor�ڵ㵽�׶κŵ�ӳ��
			tmpFlatNode2Stage = createFlatNode2StageForCPU(tmpFlatNode2CoreNum, i);
			this->m_hCluster2FlatNode2StageNum.insert(make_pair(i, tmpFlatNode2Stage));

			//�õ��׶κŵ�actor�ڵ��ӳ��
			map<FlatNode*, int>::iterator iter = tmpFlatNode2Stage.begin();
			while (iter != tmpFlatNode2Stage.end())
			{
				tmpStage2FlatNodes.insert(make_pair(iter->second, iter->first));
				++iter;
			}//while

			this->m_hCluster2StageNum2FlatNode.insert(make_pair(i, tmpStage2FlatNodes));

			//�õ��ڵ㵽��Ⱥ��ŵ��׶κŵ�ӳ��
			for (map<int, map<FlatNode*, int>>::iterator iter1 = m_hCluster2FlatNode2StageNum.begin(); iter1 != m_hCluster2FlatNode2StageNum.end(); ++iter1)
			{
				for (map<FlatNode*, int>::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2)
				{
					m_flatNode2hCluster2StageNum.insert(make_pair(iter2->first, make_pair(iter1->first, iter2->second)));
				}//for
			}//for

		}//if
		 //GPU��ϼܹ��������ڵ�׶θ�ֵ
		else {
			tmpStage2FlatNodes.clear();
			tmpFlatNode2Stage.clear();

			/*�õ���ͼactor�ڵ����ŵ�ӳ��*/
			map<FlatNode*, int> tmpFlatNode2GPUNum = m_hcp->getStatelessNode2GpuNum(i);
			map<FlatNode*, int> tmpFlatNode2CoreNum = m_hcp->getFlatNode2HClusterCore(i);


			//�õ�actor�ڵ㵽�׶κŵ�ӳ��
			tmpFlatNode2Stage = createFlatNode2StageForGPU(tmpFlatNode2GPUNum, tmpFlatNode2CoreNum, i);
			this->m_hCluster2FlatNode2StageNum.insert(make_pair(i, tmpFlatNode2Stage));

			//�õ��׶κŵ�actor�ڵ��ӳ��
			map<FlatNode*, int>::iterator iter = tmpFlatNode2Stage.begin();
			while (iter != tmpFlatNode2Stage.end())
			{
				tmpStage2FlatNodes.insert(make_pair(iter->second, iter->first));
				++iter;
			}//while

			this->m_hCluster2StageNum2FlatNode.insert(make_pair(i, tmpStage2FlatNodes));

			//�õ��ڵ㵽��Ⱥ��ŵ��׶κŵ�ӳ��
			for (map<int, map<FlatNode*, int>>::iterator iter1 = m_hCluster2FlatNode2StageNum.begin(); iter1 != m_hCluster2FlatNode2StageNum.end(); ++iter1)
			{
				for (map<FlatNode*, int>::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2)
				{
					m_flatNode2hCluster2StageNum.insert(make_pair(iter2->first, make_pair(iter1->first, iter2->second)));
				}//for
			}//for	
		}//else	
	}//for

	 //��ӡ�׶θ�ֵ���
#if 0
	printStageResult();
#endif
}

void HClusterStageAssignment::printStageResult()
{
	assert(m_hCluster2FlatNode2StageNum.size() == m_hcp->getHClusters());
	for (map<int, map<FlatNode*, int>>::iterator iter1 = m_hCluster2FlatNode2StageNum.begin(); iter1 != m_hCluster2FlatNode2StageNum.end(); ++iter1)
	{
		cout << "######��Ⱥ�ڵ㡶" << iter1->first << "��######actor�׶θ�ֵ�����" << endl;
		for (map<FlatNode*, int>::iterator iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2)
		{
			cout << "��" << iter2->first->name << " , " << iter2->second << " ��" << endl;
		}//for
		cout << "=======================================" << endl;

	}//for
	cout << endl;
}

map<FlatNode*, int> HClusterStageAssignment::createFlatNode2StageForCPU(map<FlatNode*, int> &flatNode2CoreNum, int hClusterNum)
{
	map<FlatNode*, int> flatNode2Stage;
	
	//�õ���ǰ��ͼ������actor����
	vector<FlatNode *> tmpFlatNodes = m_hcp->getSubFlatNodes(hClusterNum);

	//�Խڵ㼯�Ͻ�����������
	vector<FlatNode*> topoFlatNodes = createTopoLogicalOrder(tmpFlatNodes);

#if 0
	cout << "############������������֤################" << endl;
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

			//�����ǰ���νڵ��ڵ�ǰ��ͼ��
			if (inTag)
			{
				map<FlatNode *, int>::iterator iter3 = flatNode2Stage.find(*iter2);

				if (iter3 != flatNode2Stage.end() && iter3->second > maxStage)
				{
					maxStage = iter3->second;
				}//if

				//�鿴��ǰ���ӽڵ��Ӧ�Ķ������ֺ�
				map <FlatNode *, int>::iterator parent = flatNode2CoreNum.find(*iter2);
				assert(parent != flatNode2CoreNum.end());

				map<FlatNode*, int >::iterator child = flatNode2CoreNum.find(*iter1);
				assert(child != flatNode2CoreNum.end());

				/*�������һ�����ֺ��ϣ����ñ�־*/
				if (parent->second != child->second)
				{
					flag = true;
				}//if
			}//if
		}//for

		//��ǰ�ڵ�Ľ׶κţ�Ϊ�������νڵ����׶κ�����1
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
	cout << "#############��ӡ�׶θ�ֵ���############" << endl;
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

	/*�õ���ǰ�������ϵ�����actor�ڵ�*/
	vector<FlatNode *> tmpFlatNodes = m_hcp->getSubFlatNodes(hClusterNum);
	assert(tmpFlatNodes.size() == (flatNode2GpuNum.size() + statefulNode2CoreNum.size()));
	//�Խڵ㼯�Ͻ�����������
	vector<FlatNode*> topoFlatNodes = createTopoLogicalOrder(tmpFlatNodes);
#if 0
	cout << "############������������֤################" << endl;
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

			//��ͬһ̨��������
			if (inTag)
			{
				//�������ֵ���ϼܹ���������GPU��
				if (m_hcp->isNodeInGPU(hClusterNum, *iter1) && m_hcp->isNodeInGPU(hClusterNum, *iter2))
				{
					//�鿴��ǰ���ӽڵ��Ӧ�Ķ������ֺ�
					map <FlatNode *, int>::iterator parent = flatNode2GpuNum.find(*iter2);
					assert(parent != flatNode2GpuNum.end());

					map<FlatNode*, int >::iterator child = flatNode2GpuNum.find(*iter1);
					assert(child != flatNode2GpuNum.end());

					/*�ж��Ƿ���ͬһ��GPU���ý׶κ�*/
					if (parent->second != child->second)
					{
						flag1 = true;
					}//if
				}//if
				//��ͬһ̨�������ϣ��Ҷ������ֵ�CPU��
				else if (m_hcp->isNodeInCPU(hClusterNum, *iter1) && m_hcp->isNodeInCPU(hClusterNum, *iter2))
				{
					map<FlatNode *, int>::iterator iter3 = flatNode2Stage.find(*iter2);

					if (iter3 != flatNode2Stage.end() && iter3->second > maxStage)
					{
						maxStage = iter3->second;
					}//if

					 //�鿴��ǰ���ӽڵ��Ӧ�Ķ������ֺ�
					map <FlatNode *, int>::iterator parent = statefulNode2CoreNum.find(*iter2);
					assert(parent != statefulNode2CoreNum.end());

					map<FlatNode*, int >::iterator child = statefulNode2CoreNum.find(*iter1);
					assert(child != statefulNode2CoreNum.end());

					/*�ж��Ƿ���һ�����ֺˣ����ý׶κ�*/
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

		//���µ�ǰ���׶κ�
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
	/*���˽��*/
	vector<FlatNode *> tmpTopoNodes;
	/*����ڵ����*/
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
		// ȡ��Ҫ��ջ�Ľڵ�
		FlatNode *tmpFlatNode = flatNodeStack.back();
		tmpTopoNodes.push_back(tmpFlatNode);
		flatNodeStack.pop_back();
		//������group���ڽӵ����ȼ�1
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

