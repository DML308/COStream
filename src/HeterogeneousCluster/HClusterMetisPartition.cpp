#include "HClusterMetisPartition.h"

HClusterMetisPartition::HClusterMetisPartition(int _nparts, SchedulerSSG * sssg, vector <FlatNode*> _flatNodes) :
	m_nParts(_nparts), m_sssg(sssg),m_flatNodes(_flatNodes)
{
	/*设置每个节点的稳态执行次数*/
	for (vector<FlatNode *>::iterator iter = m_flatNodes.begin(); iter != m_flatNodes.end(); ++iter)
	{
		m_flatNode2SteadyCounts.insert(make_pair(*iter, m_sssg->GetSteadyCount(*iter)));
	}//for

	/*设置每个节点的工作量*/
	set<FlatNode *> flatNodes(m_flatNodes.begin(), m_flatNodes.end());
	for (set<FlatNode*>::iterator iter = flatNodes.begin(); iter != flatNodes.end(); ++iter)
	{
		m_flatNode2Workload.insert(make_pair(*iter, m_sssg->GetSteadyWork(*iter)));
	}//for

	 //初始化metis划分用到的成员变量
	for (int i = 0; i < METIS_NOPTIONS; i++)
		options[i] = 0;
	options[METIS_OPTION_PTYPE] = METIS_PTYPE_KWAY;//
	options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;//Specifies the type of objective
	options[METIS_OPTION_CTYPE] = METIS_CTYPE_SHEM;//Specifies the matching scheme to be used during coarsening.
	options[METIS_OPTION_IPTYPE] = METIS_IPTYPE_METISRB;//Determines the algorithm used during initial partitioning.
	options[METIS_OPTION_RTYPE] = METIS_RTYPE_GREEDY;//Determines the algorithm used for refinement.
	options[METIS_OPTION_DBGLVL] = 0;//Specifies the amount of progress/debugging information will be printed during the execution of the algorithms.
	options[METIS_OPTION_NITER] = 10;//Specifies the number of iterations for the refinement algorithms at each stage of the uncoarsening process.Default is 10.
	options[METIS_OPTION_NCUTS] = 1;//Specifies the number of different partitionings that it will compute. The final partitioning is the one that achieves the best edgecut or communication volume. Default is 1.
	options[METIS_OPTION_SEED] = -1;//Specifies the seed for the random number generator
	options[METIS_OPTION_MINCONN] = 0;//Specifies that the partitioning routines should try to minimize the maximum degree of the subdomain graph,	i.e., the graph in which each partition is a node, and edges connect subdomains with a shared interface.
	options[METIS_OPTION_CONTIG] = 0;//Specifies that the partitioning routines should try to produce partitions that are contiguous. Note that if the input graph is not connected this option is ignored.
	options[METIS_OPTION_UFACTOR] = 30;//Specifies the maximum allowed load imbalance among the partitions.
	options[METIS_OPTION_NUMBERING] = 0;//C-style numbering is assumed that starts from 0.
	options[METIS_OPTION_NOOUTPUT] = 0;
	options[METIS_OPTION_BALANCE] = 0;
	tpwgts = NULL;
	ubvec = NULL;
	mncon = 1;
}


/*线程级划分方法*/
map<int, vector<FlatNode*>> HClusterMetisPartition::threadPartition()
{
	if (m_flatNodes.empty())
	{
		return map<int, vector<FlatNode*>>();
	}

	initPartitionInfo();
	metisPartition();	
	extractPartitionResult();
	return m_partNum2FlatNodes;
}


/*初始化划分信息*/
void HClusterMetisPartition::initPartitionInfo()
{
	vector<FlatNode *> tmpFlatNodes = m_flatNodes;
	set<FlatNode *> tmpSet(tmpFlatNodes.begin(), tmpFlatNodes.end());
	nvtxs = tmpFlatNodes.size();
	/* metis 任务调度，结合边通信以及负载均衡  */
	std::vector<int>xadj(nvtxs + 1, 0);//动态定义xadj数组——表示adjncy中从xadj[i]下标开始到xadj[i+1]下标结束是i结点的上下端节点
	std::vector<int> vwgt(nvtxs); //节点的工作量
	std::vector<int>vsize(nvtxs, 0);//节点总的通信量
	int edgenum = 0;//图的边数,在这里选用大图的边数也可以，不影响结果
	edgenum = m_sssg->GetMapEdge2DownFlatNode().size();//图的边数
	std::vector<int>adjncy(edgenum * 2);
	std::vector<long>adjwgt(edgenum * 2);//用于存储边的权重
	int k = 0;//k用于记录flatnode的相邻节点数
	std::map<FlatNode *, int>::iterator iter;
	typedef std::multimap<int, FlatNode *>::iterator iter1;
	for (int i = 0; i < nvtxs; i++)
	{
		FlatNode *node = tmpFlatNodes[i];
		int flag = 0;//保证sum只加一次
		int sum = 0;
		sum += xadj[i];
		int nOut = 0;
		for (int j = 0; j < node->nOut; j++)
		{
			if (tmpSet.count(node->outFlatNodes[j]))
				++nOut; //查找成功
		}
		if (nOut != 0)
		{
			flag = 1;
			xadj[i + 1] = sum + nOut;
			for (int j = 0; j < node->nOut; j++)
			{
				if (!tmpSet.count(node->outFlatNodes[j]))
					continue;//该输出节点不在place中
				adjncy[k] = getFlatNodeId(node->outFlatNodes[j], tmpFlatNodes);
				adjwgt[k] = 1;

				vsize[i] = 1;
				k++;
			}
		}
		int nIn = 0;
		for (int j = 0; j < node->nIn; j++)
		{
			if (tmpSet.count(node->inFlatNodes[j]))
				++nIn; //查找成功
		}
		if (nIn != 0)
		{
			if (flag == 0) xadj[i + 1] = sum + nIn;
			else xadj[i + 1] += nIn;
			for (int j = 0; j < node->nIn; j++)
			{
				if (!tmpSet.count(node->inFlatNodes[j]))
					continue;//该输出节点不在place中
				adjncy[k] = getFlatNodeId(node->inFlatNodes[j], tmpFlatNodes);
				adjwgt[k] = 1;
				vsize[i] = 1;
				k++;
			}
		}
		iter = m_flatNode2SteadyCounts.find(node);
		vwgt[i] = m_flatNode2Workload.find(node)->second * iter->second;
	}
	_xadj = xadj;
	_vwgt = vwgt;
	_vsize = vsize;
	_adjncy = adjncy;

	mxadj = &_xadj[0]; //顶点相关,顶点编号在邻接边数组中的范围
	madjncy = &_adjncy[0]; // 边相关, adjncy: adjacency
	madjwgt = NULL;//边的权重
	mvsize = &_vsize[0];//各节点的通信量(节点发送的数据量)
	mvwgt = &_vwgt[0];//各节点的工作量
}

bool HClusterMetisPartition::metisPartition()
{
	//采用metis划分,最终的划分结果保存在mpart中，下标是flatNode的序号，值是核的编号
	//返回值为TRUE表示图被划分，否则划分失败
	std::vector<int>part(nvtxs);
	int *tmp_part = &part[0];
	mpart.clear();
	if (m_nParts == 1)//如果只有一个节点或一个thread则不作划分
	{
		for (int i = 0; i < nvtxs; i++) {
			part[i] = 0;
		}
		mpart = part;//各节点所对应的划分编号
		return TRUE;
	}//if

	if (nvtxs == 1)//只有一个顶点
	{
		part[0] = 0;
		mpart = part;//各节点所对应的划分编号
		return TRUE;
	}//if

	if (METIS_OK == METIS_PartGraphKway(&nvtxs, &mncon, mxadj, madjncy, mvwgt, mvsize, madjwgt, &m_nParts, tpwgts, ubvec, options, &objval, tmp_part))
	{
		//对part[i]做一个判断，如果值权相同则调用一个新的分配算法(metis将所有的flatNode都放到一个核中了)
		int partTag = 0;
		for (int i = 0; i < nvtxs - 1; i++)
		{
			if (part[i] != part[i + 1]) { partTag = 1; break; }
		}
		if (!partTag)
		{
			std::cout << "metisPartirion error,applying RRSPartition...\n";
			int perParts = (int)ceil((float)nvtxs / m_nParts);
			for (int i = 0, total = nvtxs; i < m_nParts; i++)
				for (int j = 0; j < perParts && total>0; j++, total--)
					part[perParts*i + j] = i;
		}
		mpart = part;//各节点所对应的划分编号
		return TRUE;
	}//if
	else 
	{ 
		std::cout << "Warning: METIS_PartGraphKway failed!\n"; return FALSE; 
	}//else
}

/*提取划分结果*/
void HClusterMetisPartition::extractPartitionResult()
{
	//从part中提取划分结果
	assert(mpart.size() != 0);
	m_flatNode2PartNum.clear();
	m_partNum2FlatNodes.clear();
	for (int i = 0; i < nvtxs; i++)
	{
		m_flatNode2PartNum.insert(std::make_pair(m_flatNodes[i], mpart[i]));//建立节点到划分编号的映射
		m_partNum2FlatNodes[mpart[i]].push_back(m_flatNodes[i]);
	}//for

	//要对划分的结果进行调整，保证编号小的核上一定有actor，而编号大的核上则不一定有
	std::map<int, vector<FlatNode *> > tmpCore2FlatNodes;
	for (int i = 0; i < m_nParts; i++)
	{
		if (!m_partNum2FlatNodes[i].empty())
			tmpCore2FlatNodes.insert(make_pair(i, m_partNum2FlatNodes[i]));
	}//for

	/*实际划分情况*/
	if (tmpCore2FlatNodes.size() == m_nParts)
		return;
	else {
		int curPart = 0;
		m_flatNode2PartNum.clear();
		m_partNum2FlatNodes.clear();
		for (map<int, vector<FlatNode*>>::iterator iter = tmpCore2FlatNodes.begin(); iter != tmpCore2FlatNodes.end(); ++iter)
		{
			vector<FlatNode *> flatNodes(iter->second.begin(), iter->second.end());
			m_partNum2FlatNodes.insert(make_pair(curPart, flatNodes));

			for (vector<FlatNode*>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
			{
				m_flatNode2PartNum.insert(make_pair(*iter2, curPart));
			}//for
			++curPart;
		}//for
	}//else
}


//void HClusterMetisPartition::statefulNodesAdjustment()
//{
//	/*如果是对含有GPU的混合架构划分，进行结果调整*/
//	vector<FlatNode*>::iterator iter = m_flatNodes.begin();
//	for (; iter != m_flatNodes.end(); ++iter)
//	{
//		if (!(!DetectiveActorState(*iter) && (UporDownStatelessNode(*iter) != 3)))
//			m_statefulNodes.push_back(*iter);
//	}//for
//
//	/*剔除有状态节点*/
//	map<int, vector<FlatNode*>> tmpCoreNum2FlatNodes;
//	for (map<int,vector<FlatNode *>>::iterator iter = m_coreNum2FlatNodes.begin(); iter != m_coreNum2FlatNodes.end(); ++iter)
//	{
//		vector<FlatNode *> tmpFlatNodes;
//		for (vector<FlatNode *>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
//		{
//			if (!DetectiveActorState(*iter2))
//			{
//				tmpFlatNodes.push_back(*iter2);
//			}//if
//		}//for
//
//		/*被剔除后，该GPU节点集合有可能为空*/
//		tmpCoreNum2FlatNodes.insert(make_pair(iter->first, tmpFlatNodes));
//
//	}//for
//	m_coreNum2FlatNodes.clear();
//	m_coreNum2FlatNodes = tmpCoreNum2FlatNodes;
//
//	//要对划分的结果进行调整，保证编号小的GPU上一定有actor，而编号大的GPU上则不一定有
//	std::map<int, vector<FlatNode *> > tmpCore2FlatNodes;
//	for (int i = 0; i < m_nParts; i++)
//	{
//		if (!m_coreNum2FlatNodes[i].empty())
//			tmpCore2FlatNodes.insert(make_pair(i, m_coreNum2FlatNodes[i]));
//	}//for
//
//	 /*实际划分情况*/
//	if (tmpCore2FlatNodes.size() == m_nParts)
//		return;
//	else {
//		int curPart = 0;
//		m_flatNode2CoreNum.clear();
//		m_coreNum2FlatNodes.clear();
//		for (map<int, vector<FlatNode*>>::iterator iter = tmpCore2FlatNodes.begin(); iter != tmpCore2FlatNodes.end(); ++iter)
//		{
//			vector<FlatNode *> flatNodes(iter->second.begin(), iter->second.end());
//			m_coreNum2FlatNodes.insert(make_pair(curPart, flatNodes));
//
//			for (vector<FlatNode*>::iterator iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
//			{
//				m_flatNode2CoreNum.insert(make_pair(*iter2, curPart));
//			}//for
//			++curPart;
//		}//for
//	}//else
//}


/*查找SDF图节点的编号*/
int HClusterMetisPartition::getFlatNodeId(FlatNode * faltNode, vector<FlatNode*> flatNodes)
{
	//根据flatnode找到其在vector中的编号（一个数字标识）
	for (int i = 0; i<flatNodes.size(); i++)
		if (faltNode == flatNodes[i])
			return i;
}
