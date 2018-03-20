/*
* 异构集群二级任务划分基类实现文件 -- 2016/04/29 yrr
*/
#include "HClusterPartition.h"

void HClusterPartition::printPGResult()
{
	std::cout << "集群所有节点的总权重为：" << this->getAllWorkLoad() << endl;
	std::cout << "集群所有节点的平均权重为：" << this->getAvgWorkLoad() << endl;
	std::cout << "================================================================================" << endl;
	for (auto iter = m_hClusterNum2FlatNodes.begin(); iter != m_hClusterNum2FlatNodes.end(); ++iter)
	{
		std::cout << "集群节点 " << iter->first << " 权重为：" << this->getWeightOfHCluster(iter->first) << endl << endl;
		std::cout << "集群节点：" << iter->first << " 包含的节点有：" << endl;
		int idx = 0;
		for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
		{
			std::cout << "{" << iter->first << ", " << (*iter2)->name << "}" << "\t";
			++idx;
			if (idx % 5 == 0)
				std::cout << endl;
		}//for

		std::cout << endl << endl;
	}//for
	std::cout << "===============================================================================" << endl;
}

void HClusterPartition::printTPResult()
{
	assert(this->m_coreNum2FlatNodes.size() <= m_nClusters);
	for (int i = 0; i < m_nClusters; ++i)
	{
		/*混合架构的二级划分结果*/
		if (isGPUClusterNode(i))
		{
			map<int, vector<FlatNode*>> tmp = m_coreNum2FlatNodes[i];

			for (map<int, vector<FlatNode *>>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter)
			{
				std::cout << "\n集群节点：" << i << "\t核：" << iter->first << " 包含的节点有：" << endl;

				int idx = 0;
				for (vector<FlatNode*>::iterator fIter = iter->second.begin(); fIter != iter->second.end(); ++fIter)
				{
					std::cout << "{" << iter->first << ", " << (*fIter)->name << "}" << "\t";
					++idx;
					if (idx % 5 == 0)
						std::cout << endl;
				}//for
				cout << endl;
			}//for

			map<int, vector<FlatNode *>> tmp2 = m_gpuNum2StatelessNodes[i];
			for (map<int, vector<FlatNode *>>::iterator iter = tmp2.begin(); iter != tmp2.end(); ++iter)
			{
				std::cout << "\n集群节点：" << i << "\tGpu：" << iter->first << " 包含的节点有：" << endl;

				int idx = 0;
				for (vector<FlatNode*>::iterator fIter = iter->second.begin(); fIter != iter->second.end(); ++fIter)
				{
					std::cout << "{" << iter->first << ", " << (*fIter)->name << "}" << "\t";
					++idx;
					if (idx % 5 == 0)
						std::cout << endl;
				}//for
				cout << endl;
			}//for
		}//if
		else {
			map<int, vector<FlatNode*>> tmp = m_coreNum2FlatNodes[i];

			for (map<int, vector<FlatNode *>>::iterator iter = tmp.begin(); iter != tmp.end(); ++iter)
			{
				std::cout << "\n集群节点：" << i << "\t核：" << iter->first << " 包含的节点有：" << endl;

				int idx = 0;
				for (vector<FlatNode*>::iterator fIter = iter->second.begin(); fIter != iter->second.end(); ++fIter)
				{
					std::cout << "{" << iter->first << ", " << (*fIter)->name << "}" << "\t";
					++idx;
					if (idx % 5 == 0)
						std::cout << endl;
				}//for
				cout << endl;
			}//for
		}//else
	}//for
	std::cout << "\n===============================================================================" << endl;
}


long HClusterPartition::computeWorkload(std::vector<FlatNode*>flatNodes)
{
	if (flatNodes.empty()) return 0;
	long workload = 0;
	for (int i = 0; i != flatNodes.size(); i++)
	{
		workload += m_sssg->GetSteadyWork(flatNodes[i]) * m_sssg->GetSteadyCount(flatNodes[i]);
	}
	return workload;
}

/*返回指定集群节点对应的划分子图节点*/
vector<FlatNode *> HClusterPartition::getSubFlatNodes(int hClusterNum)
{
	map<int, vector<FlatNode*>>::iterator iter = m_hClusterNum2FlatNodes.find(hClusterNum);
	assert(iter != m_hClusterNum2FlatNodes.end());
	return iter->second;
}

int HClusterPartition::getHClusterNumOfNode(FlatNode * flatNode)
{
	map<FlatNode*, int>::iterator iter1 = m_flatNode2hClusterNum.find(flatNode);
	assert(iter1 != m_flatNode2hClusterNum.end());
	return iter1->second;
}

pair<int, int> HClusterPartition::getHClusterCoreNum(FlatNode * flatNode)
{
	map<FlatNode*, int>::iterator iter1 = m_flatNode2hClusterNum.find(flatNode);
	assert(iter1 != m_flatNode2hClusterNum.end());
	map<FlatNode*, int>::iterator iter2 = m_flatNodes2CoreNum[iter1->second].find(flatNode);
	assert(iter2 != m_flatNodes2CoreNum[iter1->second].end());

	return pair<int, int>(iter1->second,iter2->second);
}

pair<int, int> HClusterPartition::getHClusterGpuNum(FlatNode * flatNode)
{
	map<FlatNode*, int>::iterator iter1 = m_flatNode2hClusterNum.find(flatNode);
	assert(iter1 != m_flatNode2hClusterNum.end() && isGPUClusterNode(iter1->second));
	map<FlatNode*, int>::iterator iter2 = m_statelessNode2GpuNum[iter1->second].find(flatNode);
	assert(iter2 != m_statelessNode2GpuNum[iter1->second].end());

	return pair<int, int>(iter1->second, iter2->second);
}