#include "HClusterPMTMPartition.h"

/*���񻮷�ģ�飬��Ϊ2���� ���̼����񻮷ֺ��̼߳����񻮷�*/
void HClusterPMTMPartition::SssgHClusterPartition()
{
	/*������̼�����*/
	processPartition();
	
	cout << endl<< "PMTM�������񻮷���ɣ�" << endl;
	/*��ӡ���̼����ֽ��*/
	printPGResult();

	/*�����̼߳�����*/
	threadPartition();

	computeAllComm();

	/*��ӡ�̼߳����ֽ��*/
	//printTPResult();
}

void HClusterPMTMPartition::computeAllComm()
{
	assert(this->m_coreNum2FlatNodes.size() <= m_nClusters);
	for (int i = 0; i < m_nClusters; ++i)
	{
		/*��ϼܹ��Ķ������ֽ��*/
		if (isGPUClusterNode(i))
		{
			map<int, vector<FlatNode*>> tmp = m_coreNum2FlatNodes[i];

			for (map<int, vector<FlatNode *>>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter)
			{
				set<FlatNode*> sNodes(iter->second.begin(), iter->second.end());
				for (set<FlatNode*>::iterator sIter = sNodes.begin(); sIter != sNodes.end(); ++sIter)
				{
					//��ִ̬�д���
					int sc = m_sssg->GetSteadyCount(*sIter);

					for (int i = 0; i < (*sIter)->nIn; ++i)
					{
						if (sNodes.count((*sIter)->inFlatNodes[i]) == 0)
						{
							m_allComm += (sc * (*sIter)->inPopWeights[i]);
						}//if
					}//for

					for (int j = 0; j < (*sIter)->nOut; ++j)
					{
						if (sNodes.count((*sIter)->outFlatNodes[j]) == 0)
						{
							m_allComm += (sc * (*sIter)->outPushWeights[j]);
						}//if
					}//for
				}//for
			}//for

			map<int, vector<FlatNode *>> tmp2 = m_gpuNum2StatelessNodes[i];
			for (map<int, vector<FlatNode *>>::iterator iter = tmp2.begin(); iter != tmp2.end(); ++iter)
			{
				set<FlatNode*> sNodes(iter->second.begin(), iter->second.end());
				for (set<FlatNode*>::iterator sIter = sNodes.begin(); sIter != sNodes.end(); ++sIter)
				{
					//��ִ̬�д���
					int sc = m_sssg->GetSteadyCount(*sIter);

					for (int i = 0; i < (*sIter)->nIn; ++i)
					{
						if (sNodes.count((*sIter)->inFlatNodes[i]) == 0)
						{
							m_allComm += (sc * (*sIter)->inPopWeights[i]);
						}//if
					}//for

					for (int j = 0; j < (*sIter)->nOut; ++j)
					{
						if (sNodes.count((*sIter)->outFlatNodes[j]) == 0)
						{
							m_allComm += (sc * (*sIter)->outPushWeights[j]);
						}//if
					}//for
				}//for
			}//for
		}//if
		else {
			map<int, vector<FlatNode*>> tmp = m_coreNum2FlatNodes[i];

			for (map<int, vector<FlatNode *>>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter)
			{
				set<FlatNode*> sNodes(iter->second.begin(), iter->second.end());
				for (set<FlatNode*>::iterator sIter = sNodes.begin(); sIter != sNodes.end(); ++sIter)
				{
					//��ִ̬�д���
					int sc = m_sssg->GetSteadyCount(*sIter);

					for (int i = 0; i < (*sIter)->nIn; ++i)
					{
						if (sNodes.count((*sIter)->inFlatNodes[i]) == 0)
						{
							m_allComm += (sc * (*sIter)->inPopWeights[i]);
						}//if
					}//for

					for (int j = 0; j < (*sIter)->nOut; ++j)
					{
						if (sNodes.count((*sIter)->outFlatNodes[j]) == 0)
						{
							m_allComm += (sc * (*sIter)->outPushWeights[j]);
						}//if
					}//for
				}//for
			}//for
		}//else
	}//for
}

//���̼����񻮷�
void HClusterPMTMPartition::processPartition()
{
	map<int, vector<FlatNode*>> hCluster2FlatNodes;
	if (this->m_nClusters == 1)
	{
		hCluster2FlatNodes[0] = this->m_sssg->flatNodes;
	}//if
	else {
		hCluster2FlatNodes = this->m_hmp->threadPartition();	
	}//else

	/*�����ֽ����䱾�ṹ*/
	m_hClusterNum2FlatNodes = map<int, vector<FlatNode*>>(hCluster2FlatNodes.begin(), hCluster2FlatNodes.end());
	map<int, vector<FlatNode*>>::iterator iter = m_hClusterNum2FlatNodes.begin();
	for (; iter != m_hClusterNum2FlatNodes.end(); ++iter)
	{
		m_hCluster2Weight.insert(make_pair(iter->first, computeWorkload(iter->second)));
		vector<FlatNode*>::iterator iter2 = (iter->second).begin();
		for (; iter2 != (iter->second).end(); ++iter2)
		{
			m_flatNode2hClusterNum[*iter2] = iter->first;
		}//for
	}//for
}

//�̼߳����񻮷�
void HClusterPMTMPartition::threadPartition()
{
	/*��ÿ����Ⱥ�ڵ�Ľ��̼����ֽ��*/
	for (int i = 0; i < m_nClusters; ++i)
	{
		int nCore = 0, nGpu = 0;
		if (isGPUClusterNode(i))
		{
			nGpu = this->m_hCluster2CpuAndGpu[i].second;
			nCore = this->m_hCluster2CpuAndGpu[i].first;
			classifyFlatNodes(i);
		
			m_hmps[i].first = new HClusterMetisPartition(nCore, m_sssg, m_statefulNodes[i]);
		
			/*����CPU���̼߳�����*/
			map<FlatNode *, int> tmpFlatNode2CoreNum;
			map<int, vector<FlatNode*>> tmpCoreNum2FlatNodes = m_hmps[i].first->threadPartition();
			this->m_coreNum2FlatNodes[i] = tmpCoreNum2FlatNodes;
			for (map<int, vector<FlatNode*>>::iterator iter = tmpCoreNum2FlatNodes.begin(); iter != tmpCoreNum2FlatNodes.end(); ++iter)
			{
				for (vector<FlatNode*>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
				{
					tmpFlatNode2CoreNum.insert(make_pair(*iter2, iter->first));
				}
			}
			this->m_flatNodes2CoreNum[i] = tmpFlatNode2CoreNum;


			//���ö�������Ȩ��
			map<int, double> tmpCore2Weight;
			for (map<int, vector<FlatNode*>>::iterator iter1 = tmpCoreNum2FlatNodes.begin(); iter1 != tmpCoreNum2FlatNodes.end(); ++iter1)
			{
				tmpCore2Weight.insert(make_pair(iter1->first, computeWorkload(iter1->second)));
			}//for
			m_hCluster2Core2Weight.insert(make_pair(i, tmpCore2Weight));
		

			/*GPU�˶�������*/
			m_hmps[i].second = new HClusterMetisPartition(nGpu, m_sssg, m_statelessNodes[i]);
			map<FlatNode *, int> tmpFlatNode2GpuNum;
			map<int, vector<FlatNode*>> tmpGpuNum2FlatNodes = m_hmps[i].second->threadPartition();
			this->m_gpuNum2StatelessNodes[i] = tmpGpuNum2FlatNodes;
			for (map<int, vector<FlatNode*>>::iterator iter = tmpGpuNum2FlatNodes.begin(); iter != tmpGpuNum2FlatNodes.end(); ++iter)
			{
				for (vector<FlatNode*>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
				{
					tmpFlatNode2GpuNum.insert(make_pair(*iter2, iter->first));
				}
			}
			this->m_statelessNode2GpuNum[i] = tmpFlatNode2GpuNum;


			//���ö�������Ȩ��
			map<int, double> tmpGpu2Weight;
			for (map<int, vector<FlatNode*>>::iterator iter1 = tmpGpuNum2FlatNodes.begin(); iter1 != tmpGpuNum2FlatNodes.end(); ++iter1)
			{
				tmpGpu2Weight.insert(make_pair(iter1->first, computeWorkload(iter1->second)));
			}//for
			m_hCluster2Gpu2Weight.insert(make_pair(i, tmpGpu2Weight));
		}//if
		else {
			nCore = this->m_hCluster2CpuAndGpu[i].first;
			m_hmps[i].first = new HClusterMetisPartition(nCore, m_sssg, m_hClusterNum2FlatNodes[i]);
			m_hmps[i].second = NULL;

			/*�����̼߳�����*/
			map<FlatNode *, int> tmpFlatNode2CoreNum;
			map<int, vector<FlatNode*>> tmpCoreNum2FlatNodes = m_hmps[i].first->threadPartition();
			this->m_coreNum2FlatNodes[i] = tmpCoreNum2FlatNodes;
			for (map<int, vector<FlatNode*>>::iterator iter = tmpCoreNum2FlatNodes.begin(); iter != tmpCoreNum2FlatNodes.end(); ++iter)
			{
				for (vector<FlatNode*>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
				{
					tmpFlatNode2CoreNum.insert(make_pair(*iter2, iter->first));
				}
			}
			this->m_flatNodes2CoreNum[i] = tmpFlatNode2CoreNum;


			//���ö�������Ȩ��
			map<int, double> tmpCore2Weight;
			for (map<int, vector<FlatNode*>>::iterator iter1 = tmpCoreNum2FlatNodes.begin(); iter1 != tmpCoreNum2FlatNodes.end(); ++iter1)
			{
				tmpCore2Weight.insert(make_pair(iter1->first, computeWorkload(iter1->second)));
			}//for
			m_hCluster2Core2Weight.insert(make_pair(i, tmpCore2Weight));

		}//else

	}//for
}
