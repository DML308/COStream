#include "HClusterPGTMPartition.h"

/*任务划分模块，分为2步： 进程级任务划分和线程级任务划分*/
void HClusterPGTMPartition::SssgHClusterPartition()
{
	/*进入进程级划分*/
	processPartition();		

	cout << endl << "PGTM二级任务划分完成：" << endl;
	/*打印进程级划分结果*/
	printPGResult();

	/*进入线程级划分*/
	threadPartition();

	/*打印线程级划分结果*/
	printTPResult();
}

/*进程级划分*/
void HClusterPGTMPartition::processPartition()
{
	map<int, vector<FlatNode*>> hCluster2FlatNodes;
	map<int, double> hCluster2Weight;

	if (this->m_nClusters == 1)
	{
		hCluster2FlatNodes[0] = this->m_sssg->flatNodes;
	}
	else {
		hCluster2FlatNodes = this->m_hpg->processPartition();
	}

	/*将划分结果填充本结构*/
	m_hClusterNum2FlatNodes = map<int, vector<FlatNode*>>(hCluster2FlatNodes.begin(), hCluster2FlatNodes.end());	
	for (map<int, vector<FlatNode*>>::iterator iter = m_hClusterNum2FlatNodes.begin(); iter != m_hClusterNum2FlatNodes.end(); ++iter)
	{
		m_hCluster2Weight.insert(make_pair(iter->first, computeWorkload(iter->second)));
		vector<FlatNode*>::iterator iter2 = (iter->second).begin();
		for (; iter2 != (iter->second).end(); ++iter2)
		{
			m_flatNode2hClusterNum[*iter2] = iter->first;
		}//for
	}//for
}

/*线程级划分*/
void HClusterPGTMPartition::threadPartition()
{
	/*对每个集群节点的进程级划分结果*/
	for (int i = 0; i < m_nClusters; ++i)
	{
		int nCore = 0, nGpu = 0;
		if (isGPUClusterNode(i))
		{
			nGpu = this->m_hCluster2CpuAndGpu[i].second;
			nCore = this->m_hCluster2CpuAndGpu[i].first;
			classifyFlatNodes(i);

			m_hmps[i].first = new HClusterMetisPartition(nCore, m_sssg, m_statefulNodes[i]);

			/*进入CPU端线程级划分*/
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

			//设置二级划分权重
			map<int, double> tmpCore2Weight;
			for (map<int, vector<FlatNode*>>::iterator iter1 = tmpCoreNum2FlatNodes.begin(); iter1 != tmpCoreNum2FlatNodes.end(); ++iter1)
			{
				tmpCore2Weight.insert(make_pair(iter1->first, computeWorkload(iter1->second)));
			}//for
			m_hCluster2Core2Weight.insert(make_pair(i, tmpCore2Weight));

			/*GPU端二级划分*/
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

			//设置二级划分权重
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

			/*进入线程级划分*/
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

			//设置二级划分权重
			map<int, double> tmpCore2Weight;
			for (map<int, vector<FlatNode*>>::iterator iter1 = tmpCoreNum2FlatNodes.begin(); iter1 != tmpCoreNum2FlatNodes.end(); ++iter1)
			{
				tmpCore2Weight.insert(make_pair(iter1->first, computeWorkload(iter1->second)));
			}//for
			m_hCluster2Core2Weight.insert(make_pair(i, tmpCore2Weight));
		}//else
	}//for
}