#include "HClusterMetisPartition.h"

HClusterMetisPartition::HClusterMetisPartition(int _nparts, SchedulerSSG * sssg, vector <FlatNode*> _flatNodes) :
	m_nParts(_nparts), m_sssg(sssg),m_flatNodes(_flatNodes)
{
	/*����ÿ���ڵ����ִ̬�д���*/
	for (vector<FlatNode *>::iterator iter = m_flatNodes.begin(); iter != m_flatNodes.end(); ++iter)
	{
		m_flatNode2SteadyCounts.insert(make_pair(*iter, m_sssg->GetSteadyCount(*iter)));
	}//for

	/*����ÿ���ڵ�Ĺ�����*/
	set<FlatNode *> flatNodes(m_flatNodes.begin(), m_flatNodes.end());
	for (set<FlatNode*>::iterator iter = flatNodes.begin(); iter != flatNodes.end(); ++iter)
	{
		m_flatNode2Workload.insert(make_pair(*iter, m_sssg->GetSteadyWork(*iter)));
	}//for

	 //��ʼ��metis�����õ��ĳ�Ա����
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


/*�̼߳����ַ���*/
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


/*��ʼ��������Ϣ*/
void HClusterMetisPartition::initPartitionInfo()
{
	vector<FlatNode *> tmpFlatNodes = m_flatNodes;
	set<FlatNode *> tmpSet(tmpFlatNodes.begin(), tmpFlatNodes.end());
	nvtxs = tmpFlatNodes.size();
	/* metis ������ȣ���ϱ�ͨ���Լ����ؾ���  */
	std::vector<int>xadj(nvtxs + 1, 0);//��̬����xadj���顪����ʾadjncy�д�xadj[i]�±꿪ʼ��xadj[i+1]�±������i�������¶˽ڵ�
	std::vector<int> vwgt(nvtxs); //�ڵ�Ĺ�����
	std::vector<int>vsize(nvtxs, 0);//�ڵ��ܵ�ͨ����
	int edgenum = 0;//ͼ�ı���,������ѡ�ô�ͼ�ı���Ҳ���ԣ���Ӱ����
	edgenum = m_sssg->GetMapEdge2DownFlatNode().size();//ͼ�ı���
	std::vector<int>adjncy(edgenum * 2);
	std::vector<long>adjwgt(edgenum * 2);//���ڴ洢�ߵ�Ȩ��
	int k = 0;//k���ڼ�¼flatnode�����ڽڵ���
	std::map<FlatNode *, int>::iterator iter;
	typedef std::multimap<int, FlatNode *>::iterator iter1;
	for (int i = 0; i < nvtxs; i++)
	{
		FlatNode *node = tmpFlatNodes[i];
		int flag = 0;//��֤sumֻ��һ��
		int sum = 0;
		sum += xadj[i];
		int nOut = 0;
		for (int j = 0; j < node->nOut; j++)
		{
			if (tmpSet.count(node->outFlatNodes[j]))
				++nOut; //���ҳɹ�
		}
		if (nOut != 0)
		{
			flag = 1;
			xadj[i + 1] = sum + nOut;
			for (int j = 0; j < node->nOut; j++)
			{
				if (!tmpSet.count(node->outFlatNodes[j]))
					continue;//������ڵ㲻��place��
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
				++nIn; //���ҳɹ�
		}
		if (nIn != 0)
		{
			if (flag == 0) xadj[i + 1] = sum + nIn;
			else xadj[i + 1] += nIn;
			for (int j = 0; j < node->nIn; j++)
			{
				if (!tmpSet.count(node->inFlatNodes[j]))
					continue;//������ڵ㲻��place��
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

	mxadj = &_xadj[0]; //�������,���������ڽӱ������еķ�Χ
	madjncy = &_adjncy[0]; // �����, adjncy: adjacency
	madjwgt = NULL;//�ߵ�Ȩ��
	mvsize = &_vsize[0];//���ڵ��ͨ����(�ڵ㷢�͵�������)
	mvwgt = &_vwgt[0];//���ڵ�Ĺ�����
}

bool HClusterMetisPartition::metisPartition()
{
	//����metis����,���յĻ��ֽ��������mpart�У��±���flatNode����ţ�ֵ�Ǻ˵ı��
	//����ֵΪTRUE��ʾͼ�����֣����򻮷�ʧ��
	std::vector<int>part(nvtxs);
	int *tmp_part = &part[0];
	mpart.clear();
	if (m_nParts == 1)//���ֻ��һ���ڵ��һ��thread��������
	{
		for (int i = 0; i < nvtxs; i++) {
			part[i] = 0;
		}
		mpart = part;//���ڵ�����Ӧ�Ļ��ֱ��
		return TRUE;
	}//if

	if (nvtxs == 1)//ֻ��һ������
	{
		part[0] = 0;
		mpart = part;//���ڵ�����Ӧ�Ļ��ֱ��
		return TRUE;
	}//if

	if (METIS_OK == METIS_PartGraphKway(&nvtxs, &mncon, mxadj, madjncy, mvwgt, mvsize, madjwgt, &m_nParts, tpwgts, ubvec, options, &objval, tmp_part))
	{
		//��part[i]��һ���жϣ����ֵȨ��ͬ�����һ���µķ����㷨(metis�����е�flatNode���ŵ�һ��������)
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
		mpart = part;//���ڵ�����Ӧ�Ļ��ֱ��
		return TRUE;
	}//if
	else 
	{ 
		std::cout << "Warning: METIS_PartGraphKway failed!\n"; return FALSE; 
	}//else
}

/*��ȡ���ֽ��*/
void HClusterMetisPartition::extractPartitionResult()
{
	//��part����ȡ���ֽ��
	assert(mpart.size() != 0);
	m_flatNode2PartNum.clear();
	m_partNum2FlatNodes.clear();
	for (int i = 0; i < nvtxs; i++)
	{
		m_flatNode2PartNum.insert(std::make_pair(m_flatNodes[i], mpart[i]));//�����ڵ㵽���ֱ�ŵ�ӳ��
		m_partNum2FlatNodes[mpart[i]].push_back(m_flatNodes[i]);
	}//for

	//Ҫ�Ի��ֵĽ�����е�������֤���С�ĺ���һ����actor������Ŵ�ĺ�����һ����
	std::map<int, vector<FlatNode *> > tmpCore2FlatNodes;
	for (int i = 0; i < m_nParts; i++)
	{
		if (!m_partNum2FlatNodes[i].empty())
			tmpCore2FlatNodes.insert(make_pair(i, m_partNum2FlatNodes[i]));
	}//for

	/*ʵ�ʻ������*/
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
//	/*����ǶԺ���GPU�Ļ�ϼܹ����֣����н������*/
//	vector<FlatNode*>::iterator iter = m_flatNodes.begin();
//	for (; iter != m_flatNodes.end(); ++iter)
//	{
//		if (!(!DetectiveActorState(*iter) && (UporDownStatelessNode(*iter) != 3)))
//			m_statefulNodes.push_back(*iter);
//	}//for
//
//	/*�޳���״̬�ڵ�*/
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
//		/*���޳��󣬸�GPU�ڵ㼯���п���Ϊ��*/
//		tmpCoreNum2FlatNodes.insert(make_pair(iter->first, tmpFlatNodes));
//
//	}//for
//	m_coreNum2FlatNodes.clear();
//	m_coreNum2FlatNodes = tmpCoreNum2FlatNodes;
//
//	//Ҫ�Ի��ֵĽ�����е�������֤���С��GPU��һ����actor������Ŵ��GPU����һ����
//	std::map<int, vector<FlatNode *> > tmpCore2FlatNodes;
//	for (int i = 0; i < m_nParts; i++)
//	{
//		if (!m_coreNum2FlatNodes[i].empty())
//			tmpCore2FlatNodes.insert(make_pair(i, m_coreNum2FlatNodes[i]));
//	}//for
//
//	 /*ʵ�ʻ������*/
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


/*����SDFͼ�ڵ�ı��*/
int HClusterMetisPartition::getFlatNodeId(FlatNode * faltNode, vector<FlatNode*> flatNodes)
{
	//����flatnode�ҵ�����vector�еı�ţ�һ�����ֱ�ʶ��
	for (int i = 0; i<flatNodes.size(); i++)
		if (faltNode == flatNodes[i])
			return i;
}
