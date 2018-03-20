#include "HClusterThreadDynamicPartition.h"

/*�̼߳���̬��Ӧ�Ի����㷨*/
map<int, vector<FlatNode*>> HClusterThreadDynamicPartition::threadPartition()
{
	/*�п�*/
	if (m_flatNodes.empty() || m_nParts < 0)
		return map<int, vector<FlatNode*>>();
	/*������ͼ�ڵ��ں�*/
	coarseningPhase();
	
	/*͹��ͼ���߽����*/
	partitionAdjustment();
	
	/*������ͼ��SDF�ڵ�ӳ���ϵ����*/
	uncoarseningPhase();

	/*���¼���ͨ�ž���*/
	//consructCommMatrix();

	return m_partNum2FlatNodes;	
}

void HClusterThreadDynamicPartition::coarseningPhase()
{
	/*(1)�޸ļ��ϼ为�ء�ͨ��������*/
	map<ActorSet *, set<ActorSet *> > curSet2fusingSets;//��¼�ܹ����ֻ���һ���set(����ļ����а���set)
	vector<ActorSet *> tmpSets(m_sets.begin(), m_sets.end());
	vector<ActorSet*> topoSets = topoSort(tmpSets);
	/*�ѱ������*/
	set<ActorSet*> visited;
	assert(topoSets.size() == tmpSets.size());
	for (vector<ActorSet *>::iterator iter = topoSets.begin(); iter != topoSets.end(); ++iter)
	{
		ActorSet *curSet = *iter;
		if(visited.count(curSet))
			continue;

		/*�жϵ�ǰ���Ӽ����������ڼ���*/
		if(curSet->getOutSets().size() <= 1 && curSet->getInSets().size() <= 1)
			continue;

		//�����ǰ���ϵĸ����Ѿ��������ۻ��ֺ��ƽ�����أ�������
		int curWorkload = curSet->m_workload;
		if(curWorkload > m_avgWorkload)
			continue;
		
		/*���Դֻ������Ӽ���*/
		set<ActorSet*> potentialSets;
		vector<ActorSet*> curInSets = curSet->getInSets();
		potentialSets.clear();
		if (curInSets.size() > 1)
		{
			multimap<long, ActorSet*> potentialInSet;
			/*�ں��ϱ߼��Ͻڵ�*/
			for (vector<ActorSet*>::iterator inIter = curInSets.begin(); inIter != curInSets.end(); ++inIter)
			{
				if ((visited.count(*inIter) == 0) && ((*inIter)->getOutSets().size() <= 1) && ((*inIter)->getInSets().size() <= 1))
				{
					potentialInSet.insert(make_pair((*inIter)->m_workload, (*inIter)));
					curWorkload += (*inIter)->m_workload;
				}//if
			}//for

			if (curWorkload <= m_avgWorkload)
			{
				bool flag = false;
				/*�����ں��ϱ߽ڵ�����*/
				if (potentialInSet.size() == curInSets.size())
				{
					vector<ActorSet*> inInSets = potentialInSet.begin()->second->getInSets();
					ActorSet *inInSet = NULL;
					vector<ActorSet *> inInOutSets;
					if (!inInSets.empty())
					{
						inInSet = *(inInSets.begin());
						inInOutSets = inInSet->getOutSets();
					}
					/*�ж�inInOutSets��potentialInSet�Ƿ�һ��*/
					set<ActorSet*> _inInOutSets(inInOutSets.begin(), inInOutSets.end());
					for (multimap<long, ActorSet*>::iterator pIter = potentialInSet.begin(); pIter != potentialInSet.end(); ++pIter)
					{
						if (!_inInOutSets.count(pIter->second))
						{
							flag = true;
							break;
						}
					}//for
					
					if (!flag)
						potentialInSet.insert(make_pair(inInSet->m_workload, inInSet));
				
				}//if
			}//if
			else {
				/*�Ƴ����ش�Ľڵ�*/
				multimap<long, ActorSet*>::reverse_iterator rIter = potentialInSet.rbegin();
				while (!potentialInSet.empty())
				{
					curWorkload -= rIter->first;
					potentialInSet.erase(--rIter.base());
					if(curWorkload <= m_avgWorkload)
						break;

					rIter = potentialInSet.rbegin();
				}//while
			}//else

			//���Ͽ��ںϵ��϶˼���
			for (multimap<long, ActorSet*>::iterator uIter = potentialInSet.begin(); uIter != potentialInSet.end(); ++uIter)
			{
				potentialSets.insert(uIter->second);
			}//for
		}//if

		vector<ActorSet*> curOutSets = curSet->getOutSets();
		if (curOutSets.size() > 1)
		{
			multimap<long, ActorSet*> potentialOutSet;
			/*�ں��±߼��Ͻڵ�*/
			for (vector<ActorSet*>::iterator outIter = curOutSets.begin(); outIter != curOutSets.end(); ++outIter)
			{
				if ((visited.count(*outIter) == 0) && ((*outIter)->getOutSets().size() <= 1) && ((*outIter)->getInSets().size() <= 1))
				{
					potentialOutSet.insert(make_pair((*outIter)->m_workload, (*outIter)));
					curWorkload += (*outIter)->m_workload;
				}//if
			}//for

			if (curWorkload <= m_avgWorkload)
			{
				bool flag = false;
				/*�����ں��ϱ߽ڵ�����*/
				if (potentialOutSet.size() == curOutSets.size())
				{
					vector<ActorSet*> outOutSets = potentialOutSet.begin()->second->getOutSets();
					ActorSet *outOutSet = NULL;
					vector<ActorSet *> outOutInSets;
					if (!outOutSets.empty())
					{
						outOutSet = *(outOutSets.begin());
						outOutInSets = outOutSet->getInSets();
					}//if
					/*�ж�outOutInSets��potentialOutSet�Ƿ�һ��*/
					set<ActorSet*> _outOutInSets(outOutInSets.begin(), outOutInSets.end());
					for (multimap<long, ActorSet*>::iterator pIter = potentialOutSet.begin(); pIter != potentialOutSet.end(); ++pIter)
					{
						if (!_outOutInSets.count(pIter->second))
						{
							flag = true;
							break;
						}
					}//for

					if (!flag)
						potentialOutSet.insert(make_pair(outOutSet->m_workload, outOutSet));

				}//if
			}//if
			else {
				/*�Ƴ����ش�Ľڵ�*/
				multimap<long, ActorSet*>::reverse_iterator rIter = potentialOutSet.rbegin();
				while (!potentialOutSet.empty())
				{
					curWorkload -= rIter->first;
					potentialOutSet.erase(--rIter.base());
					if (curWorkload <= m_avgWorkload)
						break;

					rIter = potentialOutSet.rbegin();
				}//while
			}//else

			 //���Ͽ��ںϵ��϶˼���
			for (multimap<long, ActorSet*>::iterator uIter = potentialOutSet.begin(); uIter != potentialOutSet.end(); ++uIter)
			{
				potentialSets.insert(uIter->second);
			}//for
		}//if

		potentialSets.insert(curSet);
		visited.insert(potentialSets.begin(), potentialSets.end());
		curSet2fusingSets.insert(make_pair(curSet, potentialSets));
	}//for

	/*(2)���� ��ǰ�Ĵֻ�ͼ*/
	if (!curSet2fusingSets.empty())
	{
		for (map<ActorSet *, set<ActorSet *> >::iterator cIter = curSet2fusingSets.begin(); cIter != curSet2fusingSets.end(); ++cIter)
		{
			for (set<ActorSet*>::iterator fIter = (cIter->second).begin(); fIter != (cIter->second).end(); ++fIter)
			{
				if (*fIter != cIter->first)
				{
					/*��ǰ������ɾ�����ںϵ������Ӽ���*/
					m_sets.erase(*fIter);
					/*ִ���ںϲ���*/
					cIter->first->unionSet(*fIter);
				}//if
			}//for
		}//for
	}//if

	m_acturalParts = m_sets.size();	

	/*(3)�����ֻ�*/
	int boundary = m_nParts * 32, curSetNum = m_sets.size();
	if (curSetNum < boundary)
		return;

	//�������Ӽ���֮���ͨ�ž���
	consructCommMatrix();
	map<pair<int, int>, double> twoSetsOfGain;
	for (int i = 0; i < curSetNum; ++i)
	{
		for (int j = i; j < curSetNum; ++j)
		{
			if (m_commofTwoSets[i][j])
			{
				twoSetsOfGain.insert(make_pair(make_pair(i, j), computeGain(i,j)));
			}//if
		}//for
	}//for

	/*����ʱ���Ӽ�����Ŀ��������ֵʱ��ֹͣ�ֻ�*/
	int tmpSetNum = curSetNum;
	while (tmpSetNum > boundary)
	{
		map<double, pair<int, int>> gainOfTwoSets;
		for (map<pair<int, int>, double>::iterator tIter = twoSetsOfGain.begin(); tIter != twoSetsOfGain.end(); ++tIter)
		{
			gainOfTwoSets.insert(make_pair(tIter->second, tIter->first));
		}//for

		map<double, pair<int, int>>::reverse_iterator rgIter = gainOfTwoSets.rbegin();
		while (rgIter != gainOfTwoSets.rend())
		{
			if ((m_setWorkload[rgIter->second.first] + m_setWorkload[rgIter->second.second]) > m_avgWorkload)
			{
				++rgIter;
			}//if
			else if (judgeSetUnion(rgIter->second.first, rgIter->second.second, tmpSetNum)) {
				break;
			}//else
			else
				++rgIter;
		}//while

		/*�ҵ��������Դֻ������Ӽ���*/
		if (rgIter != gainOfTwoSets.rend())
		{
			for (int i = 0; i != m_setIdx2ActualIdx.size(); ++i)
			{
				if (m_setIdx2ActualIdx[i] == rgIter->second.second)
				{
					m_setIdx2ActualIdx[i] = rgIter->second.first;
				}
			}//for

			m_setWorkload[rgIter->second.first] += m_setWorkload[rgIter->second.second];
			m_setWorkload[rgIter->second.second] = 0;

			m_setComm[rgIter->second.first] += m_setComm[rgIter->second.second];
			m_setComm[rgIter->second.second] = 0;


			map<pair<int, int>, double>::iterator tIter = twoSetsOfGain.begin();
			while (tIter != twoSetsOfGain.end())
			{
				if (tIter->first.first == rgIter->second.first || tIter->first.first == rgIter->second.second ||
					tIter->first.second == rgIter->second.first || tIter->first.second == rgIter->second.second)
				{
					map<pair<int, int>, double>::iterator dIter = tIter;
					++tIter;
					twoSetsOfGain.erase(dIter);
				}
				else {
					++tIter;
				}//else
			}//while

			//���¼������ӵ�ͨ�ž���
			for (int i = 0; i < curSetNum; ++i)
			{
				if (m_commofTwoSets[rgIter->second.second][i] != -1)
				{
					m_commofTwoSets[rgIter->second.first][i] += m_commofTwoSets[rgIter->second.second][i];
				}//if

				if (m_commofTwoSets[i][rgIter->second.second] != -1)
				{
					m_commofTwoSets[i][rgIter->second.first] += m_commofTwoSets[i][rgIter->second.second];
				}//if
			}//for
			m_commofTwoSets[rgIter->second.first][rgIter->second.first] = 0;

			for (int i = 0; i < curSetNum; ++i)
			{
				m_commofTwoSets[rgIter->second.second][i] = -1;
				m_commofTwoSets[i][rgIter->second.second] = -1;
			}//for

			//����twoSetsOfGain
			for (int i = rgIter->second.first; i < curSetNum; ++i)
			{
				if(m_commofTwoSets[rgIter->second.first][i] == 0 || m_commofTwoSets[rgIter->second.first][i] == -1)
					continue;

				twoSetsOfGain.insert(make_pair(make_pair(rgIter->second.first, i), computeGain(rgIter->second.first, i)));
			}//for
			for (int i = 0; i < rgIter->second.first; ++i)
			{
				if (m_commofTwoSets[i][rgIter->second.first] == 0 || m_commofTwoSets[i][rgIter->second.first] == -1)
					continue;

				twoSetsOfGain.insert(make_pair(make_pair(i, rgIter->second.first), computeGain(i, rgIter->second.first)));
			}//for
		}//if
		else
			break;

		--tmpSetNum;
	}//while

	//(4)�ع���ǰ���Ӽ���
	map<int, set<int>> actualIdx2originalIdx;
	for (int i = 0; i < curSetNum; ++i)
	{
		set<int> tmpOriginalIdx;
		pair<map<int, set<int>>::iterator, bool> ret = actualIdx2originalIdx.insert(make_pair(m_setIdx2ActualIdx[i], tmpOriginalIdx));
		if (!ret.second)
			ret.first->second.insert(i);
	}//for

	for (map<int, set<int>>::iterator aIter = actualIdx2originalIdx.begin(); aIter != actualIdx2originalIdx.end(); ++aIter)
	{
		ActorSet *as = m_idx2Set.find(aIter->first)->second;
		for (set<int>::iterator sIter = aIter->second.begin(); sIter != aIter->second.end(); ++sIter)
		{
			ActorSet *ts = m_idx2Set.find(*sIter)->second;
			if (*sIter != aIter->first)
			{
				m_sets.erase(ts);
				as->unionSet(ts);
			}//if
		}//for
	}//for

	m_acturalParts = m_sets.size();
	vector<ActorSet *> finalSets(m_sets.begin(), m_sets.end()),tmpTopoSets = topoSort(finalSets);
	assert(m_sets.size() == tmpTopoSets.size());
}

/*����������¶Դֻ�ͼ��һ���ںϣ�����֤����ÿ��������ͼ����͹��ͼ*/
void HClusterThreadDynamicPartition::partitionAdjustment()
{
	vector<ActorSet *> curSets(m_sets.begin(), m_sets.end()), topoSets = topoSort(curSets);

	/*��¼���Ӽ����뻮�ֱ��֮���ӳ��*/
	map<ActorSet*, int> tmpSet2PartNum;
	int partIdx = 0;
	vector<ActorSet *> lastSets;
	vector<long> partsWorkload(m_nParts, 0);
	for (int i = 0; i != topoSets.size(); ++i)
	{
		/*�Ѿ�ӳ�䵽���һ������*/
		if (partIdx == m_nParts)
		{
			//����ʣ������Ӽ���
			lastSets.push_back(topoSets[i]);
			continue;
		}//if

		/*��ǰ���ֵĸ���*/
		long curPartWorkload = partsWorkload[partIdx] + topoSets[i]->m_workload;

		/*���Լ��뵱ǰ����*/
		if ((double)partsWorkload[partIdx] - (double)m_avgWorkload * 0.7 < 0.0000000001 || (double)curPartWorkload - (double)m_avgWorkload * 0.8 < 0.0000000001)
		{
			partsWorkload[partIdx] = curPartWorkload;
		}
		else if ((double)curPartWorkload - (double)m_avgWorkload * 1.2 < 0.0000000001)
		{
			//���شﵽ���ƣ�����ͨ�ſ���ȷ���Ƿ���뵱ǰ����
			map<ActorSet *, long> inComm = topoSets[i]->m_inSet2Comm;

			//�ж�topoSets[i]�뵱ǰ����partIdx�ڵ����Ӽ��ϵ�ͨ����
			long totalComm = 0;
			set<int> inEdges;
			for (map<ActorSet *, long>::iterator iter = inComm.begin(); iter != inComm.end(); ++iter)
			{
				map<ActorSet *, int>::iterator tIter = tmpSet2PartNum.find(iter->first);
				if (tIter != tmpSet2PartNum.end())
				{
					inEdges.insert(tIter->second);
				}//if

				/*��������ڵ�ǰ����partIdx��*/
				if (tIter != tmpSet2PartNum.end() && tIter->second == partIdx)
				{
					totalComm += tIter->first->m_comm;
				}//if

			}//for

			if ((inEdges.size() + topoSets[i]->m_outSet2Comm.size()) != 0 && (topoSets[i]->m_comm / (inEdges.size() + topoSets[i]->m_outSet2Comm.size()) < totalComm))
			{
				partsWorkload[partIdx] += topoSets[i]->m_workload;
			}//if
			else {
				++partIdx;
				//����һ���µĻ���
				if (partIdx != m_nParts)
				{
					partsWorkload[partIdx] = topoSets[i]->m_workload;
				}
				else {
					lastSets.push_back(topoSets[i]);
					continue;
				}//else
			}//else
		}//elif
		else {
			++partIdx;
			//����һ���µĻ���
			if (partIdx != m_nParts)
			{
				partsWorkload[partIdx] = topoSets[i]->m_workload;
			}
			else {
				lastSets.push_back(topoSets[i]);
				continue;
			}//else
		}//else

		tmpSet2PartNum.insert(make_pair(topoSets[i], partIdx));
	}//for

	//����ʣ������Ӽ��Ͻڵ�
	if (partIdx == m_nParts && !lastSets.empty())
	{
		for (vector<ActorSet*>::iterator lIter = lastSets.begin(); lIter != lastSets.end(); ++lIter)
		{
			partsWorkload[partIdx - 1] += (*lIter)->m_workload;
			tmpSet2PartNum.insert(make_pair(*lIter, partIdx - 1));
		}//for
	}//if
	else {
		/*����ʵ�ʷ���ΪpartIdx+1*/
		partIdx += 1;
	}//else
	

	assert(tmpSet2PartNum.size() == m_sets.size());
	assert(partIdx <= m_nParts);

	for (map<ActorSet*, int>::iterator tIter = tmpSet2PartNum.begin(); tIter != tmpSet2PartNum.end(); ++tIter)
	{
		set<FlatNode*> nodes = tIter->first->m_flatNodes;
		m_partNum2FlatNodes.insert(make_pair(tIter->second, vector<FlatNode*>(tIter->first->m_flatNodes.begin(), tIter->first->m_flatNodes.end())));
		for (set<FlatNode*>::iterator sIter = tIter->first->m_flatNodes.begin(); sIter != tIter->first->m_flatNodes.end(); ++sIter)
		{
			m_flatNode2PartNum.insert(make_pair(*sIter, tIter->second));
		}//for
	}//for

	/*���ֻ���ӳ�䵽��ͬ���ֵ����Ӽ����ں�*/
	multimap<int, ActorSet*> tmpPartNum2Sets;
	for (map<ActorSet*, int>::iterator tIter = tmpSet2PartNum.begin(); tIter != tmpSet2PartNum.end(); ++tIter)
	{
		tmpPartNum2Sets.insert(make_pair(tIter->second, tIter->first));
	}//for

	/*��󻮷ֹ�partIdx��*/
	m_sets.clear();
	m_actor2Set.clear();

	for (int i = 0; i < partIdx; ++i)
	{
		pair<multimap<int, ActorSet*>::iterator, multimap<int, ActorSet*>::iterator> range = tmpPartNum2Sets.equal_range(i);
		assert(range.first != tmpPartNum2Sets.end());

		ActorSet *as = range.first->second;
		++range.first;

		while (range.first != range.second)
		{
			as->unionSet(range.first->second);
			m_sets.erase(range.first->second);
			
			++range.first;
		}//while

		m_partNum2ActorSets.insert(make_pair(i, as));
		m_actorSet2PartNum.insert(make_pair(as, i));

		m_sets.insert(as);
		for (set<FlatNode*>::iterator iter = as->m_flatNodes.begin(); iter != as->m_flatNodes.end(); ++iter)
		{
			m_actor2Set.insert(make_pair(*iter, as));
		}//for
	}//for

	m_acturalParts = partIdx;
	assert(m_partNum2ActorSets.size() == partIdx && partIdx <= m_nParts);
	assert(m_sets.size() == m_partNum2ActorSets.size() && m_sets.size() == partIdx);
	assert(m_actor2Set.size() == m_flatNodes.size());
}

void HClusterThreadDynamicPartition::uncoarseningPhase()
{
	/*�����ڵ㣬�������ּ��ϡ������桷*/
	map<pair<FlatNode*, ActorSet*>, int > node2AdjPart2Gain;
	multimap<int, pair<FlatNode*, ActorSet*>> gain2Node2AdjPart;

	assert(m_flatNodes.size() == m_flatNode2PartNum.size());
	for (map<FlatNode*, ActorSet*>::iterator iter = m_actor2Set.begin(); iter != m_actor2Set.end(); ++iter)
	{
		int iD = 0;
		map<ActorSet*, int> adjPartition2EdgesNum;//���ڵ�����ڵĻ��֣��ڵ���û��������ߵ���Ŀ��
	
		for (int i = 0; i != iter->first->inFlatNodes.size(); ++i)
		{
			map<FlatNode*, ActorSet*>::iterator iIter = m_actor2Set.find(iter->first->inFlatNodes[i]);
			if (iIter != m_actor2Set.end())
			{
				if (iIter->second == iter->second)
					++iD;
				else {
					pair<map<ActorSet *, int>::iterator, bool> ret = adjPartition2EdgesNum.insert(std::make_pair(iIter->second, 1));
					if (!ret.second) ret.first->second += 1;
				}//else
			}//if
		}//for
	
		for (int i = 0; i != iter->first->outFlatNodes.size(); ++i)
		{
			map<FlatNode*, ActorSet*>::iterator oIter = m_actor2Set.find(iter->first->outFlatNodes[i]);
			if (oIter != m_actor2Set.end())
			{
				if (oIter->second == iter->second)
					++iD;
				else {
					pair<map<ActorSet *, int>::iterator, bool> ret = adjPartition2EdgesNum.insert(std::make_pair(oIter->second, 1));
					if (!ret.second) ret.first->second += 1;
				}//else
			}//if
		}//for
	
		for (map<ActorSet*, int>::iterator aIter = adjPartition2EdgesNum.begin(); aIter != adjPartition2EdgesNum.end(); ++aIter)
		{
			if(aIter->second == 0)
				continue;

			node2AdjPart2Gain.insert(make_pair(make_pair(iter->first, aIter->first), aIter->second - iD));
			gain2Node2AdjPart.insert(make_pair(aIter->second - iD, make_pair(iter->first, aIter->first)));
		}//for

	}//for

	assert(node2AdjPart2Gain.size() == gain2Node2AdjPart.size());

	while (!gain2Node2AdjPart.empty())
	{
		multimap<int, pair<FlatNode*, ActorSet*>>::reverse_iterator gIter = gain2Node2AdjPart.rbegin();
		FlatNode *adjNode = gIter->second.first;
		ActorSet *originalSet = m_actor2Set.find(adjNode)->second;
		if (refiningPhase_IsAdjustableFlatNode(adjNode, gIter->second.second, gIter->first))
		{
			refiningPhase_UpdatePriQueue(adjNode, node2AdjPart2Gain, gain2Node2AdjPart);
		}//if

		node2AdjPart2Gain.erase(gIter->second);
		gain2Node2AdjPart.erase(--gIter.base());
	}//while

	assert(m_sets.size() == m_acturalParts);
	map<FlatNode*, ActorSet*>::iterator iter = m_actor2Set.begin();
	/*��⵱ǰ���֣���������͹��ͼ����*/
	for (; iter != m_actor2Set.end(); ++iter)
	{
		if(iter->second->m_flatNodes.size() <= 1)
			continue;

		vector<FlatNode*> outNodes = iter->first->outFlatNodes;
		set<ActorSet*> objSets;
		int flag = 0;
		for (int i = 0; i != outNodes.size(); ++i)
		{
			if (judgeInCurClusterNode(outNodes[i]))
			{
				ActorSet *outSet = m_actor2Set.find(outNodes[i])->second;
				if (iter->second == outSet)
				{
					flag = 1;
					break;
				}//if
				else {
					objSets.insert(outSet);
				}//else
			}//if
			
		}//for

		if (flag)
		{
			objSets.clear();
			flag = 0;
			continue;
		}//if

		vector<FlatNode*> inNodes = iter->first->inFlatNodes;
		for (int i = 0; i != inNodes.size(); ++i)
		{
			if (judgeInCurClusterNode(inNodes[i]))
			{
				ActorSet *inSet = m_actor2Set.find(inNodes[i])->second;
				if (iter->second == inSet)
				{
					flag = 1;
					break;
				}
				else {
					objSets.insert(inSet);
				}//else
			}
			
		}//for
		if (flag)
		{
			objSets.clear();
			flag = 0;
			continue;
		}//if


		if (!objSets.empty())
		{
			set<ActorSet*>::iterator oIter = objSets.begin();
			ActorSet *objSet = *oIter;
			int objWorkload = objSet->m_workload;
			for (; oIter != objSets.end(); ++oIter)
			{
				if (objWorkload > (*oIter)->m_workload)
				{
					objWorkload = (*oIter)->m_workload;
				}//if
			}//for

			movingFlatNodeInterSet(iter->first, iter->second, objSet);
			objSets.clear();
		}//if	
	}//for

	/*����ӳ����*/
	m_flatNode2PartNum.clear();
	m_partNum2FlatNodes.clear();
	for (int i = 0; i != m_partNum2ActorSets.size(); ++i)
	{
		set<FlatNode*> nodes = m_partNum2ActorSets[i]->m_flatNodes;
		/*������ͨ����*/
		m_allComm += m_partNum2ActorSets[i]->computeAllComm();

		m_partNum2FlatNodes.insert(make_pair(i, vector<FlatNode*>(nodes.begin(), nodes.end())));
		for (set<FlatNode*>::iterator iter = nodes.begin(); iter != nodes.end(); ++iter)
		{
			m_flatNode2PartNum.insert(make_pair(*iter, i));
		}//for
	}//for
}

vector<ActorSet*> HClusterThreadDynamicPartition::topoSort(vector<ActorSet*> sets)
{
	vector<ActorSet *> ret,tmpStk;
	vector<int> inDegree;
	for (vector<ActorSet *>::iterator iter = sets.begin(); iter != sets.end(); ++iter)
	{
		inDegree.push_back((*iter)->getInSets().size());
	}

	for (int i = 0; i != inDegree.size(); ++i)
	{
		if (!inDegree[i])
			tmpStk.push_back(sets[i]);
	}

	while (!tmpStk.empty())
	{
		//��ջ
		ActorSet *s = tmpStk.back();
		vector<ActorSet*> outSets = s->getOutSets();
		tmpStk.pop_back();
		ret.push_back(s);

		for (int i = 0; i != outSets.size(); ++i)
		{
			for (int j = 0; j != sets.size(); ++j)
			{
				if (sets[j] == outSets[i])
				{
					if (!(--inDegree[j]))
						tmpStk.push_back(sets[j]);
				}
			}
		}
	}//while

	return ret;
}

void HClusterThreadDynamicPartition::consructCommMatrix()
{
	int idx = 0;
	vector<ActorSet *> curSets(m_sets.begin(), m_sets.end()), topoSets =topoSort(curSets);
	assert(m_sets.size() == topoSets.size());

	m_set2Idx.clear();
	m_set2Idx.clear();
	m_setIdx2ActualIdx.clear();
	m_setWorkload.clear();
	m_setComm.clear();
	for (vector<ActorSet*>::iterator tIter = topoSets.begin(); tIter != topoSets.end(); ++tIter)
	{
		m_set2Idx.insert(make_pair(*tIter, idx));
		m_idx2Set.insert(make_pair(idx, *tIter));
		m_setIdx2ActualIdx.push_back(idx);
		m_setWorkload.push_back((*tIter)->m_workload);
		m_setComm.push_back((*tIter)->m_comm);
		++idx;
	}//for

	assert(m_sets.size() == m_acturalParts);
	m_commofTwoSets.resize(m_sets.size());
	for (int i = 0; i != m_commofTwoSets.size(); ++i)
	{
		m_commofTwoSets[i].resize(m_sets.size());
	}//for
	 //�������Ӽ���֮���ͨ��������
	for (map<ActorSet*, int>::iterator sIter = m_set2Idx.begin(); sIter != m_set2Idx.end(); ++sIter)
	{
		map<ActorSet*, long> curOutSet2Comm = sIter->first->m_outSet2Comm;
		for (map<ActorSet*, long>::iterator oIter = curOutSet2Comm.begin(); oIter != curOutSet2Comm.end(); ++oIter)
		{
			int outIdx = m_set2Idx.find(oIter->first)->second;
			m_commofTwoSets[sIter->second][outIdx] = oIter->second;
		}//for
	}//for

}

bool HClusterThreadDynamicPartition::judgeSetUnion(int first, int second, int size)
{
	vector<vector<int>> tmpCommofTwoSets = m_commofTwoSets;
	for (int i = 0; i < size; ++i)
	{
		if (tmpCommofTwoSets[second][i] != -1)
		{
			tmpCommofTwoSets[first][i] += tmpCommofTwoSets[second][i];
		}
		if (tmpCommofTwoSets[i][second] != -1)
		{
			tmpCommofTwoSets[i][first] += tmpCommofTwoSets[i][second];
		}
	}//for
	tmpCommofTwoSets[first][first] = 0;

	/*ʹ�ñ��ںϺ�����Ӽ�����Ч��*/
	for (int i = 0; i < size; ++i)
	{
		tmpCommofTwoSets[second][i] = -1;
		tmpCommofTwoSets[i][second] = -1;
	}//for

	vector<int> inDegree(size,0), topoOrder, setStk;
	for (int i = 0; i < size; ++i)
	{
		if (tmpCommofTwoSets[i][i] == -1)
		{
			inDegree[i] = -1; //�����Ӽ���Ϊ��Ч�ڵ�
			continue;
		}

		//���㵱ǰ���Ӽ��ϵ����
		int inD = 0;
		for (int j = 0; j < size; ++j)
		{
			if (tmpCommofTwoSets[j][i] != 0 && tmpCommofTwoSets[j][i] != -1)
				++inD;
		}//for

		inDegree[i] = inD;
	}//for

	int invalidNum = 0;
	for (int i = 0; i < size; ++i)
	{
		if (!inDegree[i])
			setStk.push_back(i);
		else if (inDegree[i] == -1)
			++invalidNum;
	}//for

	while (!setStk.empty())
	{
		int curIdx = setStk.back();
		topoOrder.push_back(curIdx);
		setStk.pop_back();

		if(inDegree[curIdx] == -1)
			continue;

		for (int i = 0; i < size; ++i)
		{
			if (tmpCommofTwoSets[curIdx][i] != 0 && tmpCommofTwoSets[curIdx][i] != -1)
			{
				if (!(--inDegree[i]))
					setStk.push_back(i);
			}//if
		}//for
	}//while

	if ((topoOrder.size() + invalidNum) == size)
		return true;
	else
		return false;
}

void HClusterThreadDynamicPartition::movingFlatNodeInterSet(FlatNode *flatNode, ActorSet *srcGroup, ActorSet *snkGroup)
{
	std::map<FlatNode*, ActorSet*>::iterator f_iter = m_actor2Set.find(flatNode);
	assert(f_iter != m_actor2Set.end());
	assert(f_iter->second == srcGroup);
	f_iter->second = snkGroup;
	srcGroup->removeActor(flatNode);
	snkGroup->pushActor(flatNode);
	//������srcGroup��snkGroup��ص�groupͼ��������ϵ����Ҫ�޸ĵĹ�ϵ��Ҫ����srcGroup��snkGroup�����¶�group�Լ�srcGroup��snkGroup�����¶˵����¶�
	set<ActorSet *> modifyAdjDownGroup;//��¼��Ҫ�޸ĵ�group���¶�group
	set<ActorSet *> modifyAdjUpGroup;//��¼��Ҫ�޸ĵ�group���϶�group
											   //src��snk���¶˶�������Ҫ�ı�
	modifyAdjDownGroup.insert(srcGroup);
	modifyAdjDownGroup.insert(snkGroup);
	modifyAdjUpGroup.insert(srcGroup);
	modifyAdjUpGroup.insert(snkGroup);
	vector<ActorSet *>srcUpGroup = srcGroup->getInSets();
	vector<ActorSet *>srcDownGroup = srcGroup->getOutSets();
	vector<ActorSet *>snkUpGroup = snkGroup->getInSets();
	vector<ActorSet *>snkDownGroup = snkGroup->getOutSets();
	//src��snk�϶�group��ֻ��Ҫ�޸����¶�
	for (int i = 0; i != srcUpGroup.size(); i++)
		modifyAdjDownGroup.insert(srcUpGroup[i]);
	for (int i = 0; i != snkUpGroup.size(); i++)
		modifyAdjDownGroup.insert(snkUpGroup[i]);
	//src��snk�¶�group��ֻ��Ҫ�޸����϶�
	for (int i = 0; i != srcDownGroup.size(); i++)
		modifyAdjUpGroup.insert(srcDownGroup[i]);
	for (int i = 0; i != snkDownGroup.size(); i++)
		modifyAdjUpGroup.insert(snkDownGroup[i]);
	//�޸���Ҫ�޸����¶˵�group
	for (std::set<ActorSet *>::iterator dg_iter = modifyAdjDownGroup.begin(); dg_iter != modifyAdjDownGroup.end(); dg_iter++)
	{
		//��ɾ����ǰ��group�е��¶�group
		std::vector<ActorSet *> down_downGroup = (*dg_iter)->getOutSets();
		for (int i = 0; i != down_downGroup.size(); i++) (*dg_iter)->removeOutSet(down_downGroup[i]);
		std::set<FlatNode *> dg_flatNodes = (*dg_iter)->m_flatNodes;
		for (std::set<FlatNode *>::iterator dgf_iter = dg_flatNodes.begin(); dgf_iter != dg_flatNodes.end(); dgf_iter++)
		{//ȡdg_flatNodes�������
			for (int i = 0; i != (*dgf_iter)->outFlatNodes.size(); i++)
			{
				if (find(m_flatNodes.begin(), m_flatNodes.end(), (*dgf_iter)->outFlatNodes[i]) != m_flatNodes.end())
				{
					ActorSet *cur_tmp_downGroup = m_actor2Set.find((*dgf_iter)->outFlatNodes[i])->second;
					if (cur_tmp_downGroup != *dg_iter)(*dg_iter)->pushOutSet(cur_tmp_downGroup);
				}
			}
		}
	}
	//�޸���Ҫ�޸����϶˵�group
	for (std::set<ActorSet *>::iterator ug_iter = modifyAdjUpGroup.begin(); ug_iter != modifyAdjUpGroup.end(); ug_iter++)
	{
		//��ɾ����ǰ��group�е��϶�group
		std::vector<ActorSet *> up_upGroup = (*ug_iter)->getInSets();
		for (int i = 0; i != up_upGroup.size(); i++) (*ug_iter)->removeInSet(up_upGroup[i]);
		std::set<FlatNode *> ug_flatNodes = (*ug_iter)->m_flatNodes;
		for (std::set<FlatNode *>::iterator ugf_iter = ug_flatNodes.begin(); ugf_iter != ug_flatNodes.end(); ugf_iter++)
		{//ȡdg_flatNodes�������
			for (int i = 0; i != (*ugf_iter)->inFlatNodes.size(); i++)
			{
				if (find(m_flatNodes.begin(), m_flatNodes.end(), (*ugf_iter)->inFlatNodes[i]) != m_flatNodes.end())
				{
					ActorSet *cur_tmp_upGroup = m_actor2Set.find((*ugf_iter)->inFlatNodes[i])->second;
					if (cur_tmp_upGroup != *ug_iter) (*ug_iter)->pushInSet(cur_tmp_upGroup);
				}//if
			}
		}
	}
}

Bool HClusterThreadDynamicPartition::refiningPhase_IsAdjustableFlatNode(FlatNode * flatNode, ActorSet * snkGroup, int gain)
{
	ActorSet* srcGroup = m_actor2Set.find(flatNode)->second;//ȡflatNode���ڵ�partition

	int srcGroup_commData = srcGroup->m_comm * OVERHEAD_PER_COMMUNICATION_DATA;
	int snkGroup_commData = snkGroup->m_comm * OVERHEAD_PER_COMMUNICATION_DATA;
	int srcGroup_workload = srcGroup->m_workload;
	int snkGroup_workload = snkGroup->m_workload;

	movingFlatNodeInterSet(flatNode, srcGroup, snkGroup);
	std::vector<ActorSet *> groupVec(m_sets.begin(), m_sets.end());
	std::vector<ActorSet *> groupTopo = topoSort(groupVec);
	if (groupTopo.size() != m_sets.size())
	{//�л�
		movingFlatNodeInterSet(flatNode, snkGroup, srcGroup);//�����ϴε��ƶ�
		return FALSE;
	}
	//��������ƶ���Ч��--------����Ҫ����ʵ��������������
	if (((float)snkGroup->m_workload) / srcGroup->m_workload > 1.2)
	{
		movingFlatNodeInterSet(flatNode, snkGroup, srcGroup);//�����ϴε��ƶ�
		return FALSE;
	}
	if (gain > 0)
		return TRUE;
	else//gain <= 0
	{
		//��ص�����group�ļ���ͨ�űȵ������ĽǶ�������
		int srcGroup_commData_after = srcGroup->m_comm * OVERHEAD_PER_COMMUNICATION_DATA;
		int snkGroup_commData_after = snkGroup->m_comm * OVERHEAD_PER_COMMUNICATION_DATA;
		int srcGroup_workload_after = srcGroup->m_workload;
		int snkGroup_workload_after = snkGroup->m_workload;
		float radio = (float)srcGroup_workload / srcGroup_commData + (float)snkGroup_workload / snkGroup_commData;
		float radio_after = (float)srcGroup_workload_after / srcGroup_commData_after + (float)snkGroup_workload_after / snkGroup_commData_after;
		if (radio_after - radio > 0.1)
			return TRUE;//ע��float��0�ıȽ�
	}
	movingFlatNodeInterSet(flatNode, snkGroup, srcGroup);//�����ϴε��ƶ�
	return FALSE;
}

void HClusterThreadDynamicPartition::refiningPhase_UpdatePriQueue(FlatNode * flatNode, std::map<std::pair<FlatNode*, ActorSet*>, int>& flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode*, ActorSet*>>& gain2FlatNode2AdjPT)
{
	refiningPhase_RemoveOldGain(flatNode, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	insertNewFlatNodeGain(flatNode, m_actor2Set.find(flatNode)->second, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	//flatNode�϶˽ڵ���flatNode�ƶ���������ı仯���
	for (int i = 0; i != flatNode->inFlatNodes.size(); i++)
	{
		refiningPhase_RemoveOldGain(flatNode->inFlatNodes[i], flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
		insertNewFlatNodeGain(flatNode->inFlatNodes[i], m_actor2Set.find(flatNode->inFlatNodes[i])->second, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
	}
	//flatNode�¶˽ڵ���flatNode�ƶ���������ı仯���
	for (int i = 0; i != flatNode->outFlatNodes.size(); i++)
	{
		if (judgeInCurClusterNode(flatNode->outFlatNodes[i]))
		{
			refiningPhase_RemoveOldGain(flatNode->outFlatNodes[i], flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
			insertNewFlatNodeGain(flatNode->outFlatNodes[i], m_actor2Set.find(flatNode->outFlatNodes[i])->second, flatNode2AdjPT2Gain, gain2FlatNode2AdjPT);
		}
	}
}

void HClusterThreadDynamicPartition::refiningPhase_RemoveOldGain(FlatNode * flatNode, std::map<std::pair<FlatNode*, ActorSet*>, int>& flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode*, ActorSet*>>& gain2FlatNode2AdjPT)
{
	std::map<std::pair<FlatNode *, ActorSet*>, int >::iterator flatNode2AdjPT2Gain_iter = flatNode2AdjPT2Gain.begin();
	while (flatNode2AdjPT2Gain_iter != flatNode2AdjPT2Gain.end())
	{
		if ((flatNode2AdjPT2Gain_iter->first).first == flatNode)
		{//ɾ��
			std::map<std::pair<FlatNode *, ActorSet*>, int >::iterator tmp = flatNode2AdjPT2Gain_iter;
			flatNode2AdjPT2Gain_iter++;
			flatNode2AdjPT2Gain.erase(tmp);
		}
		else flatNode2AdjPT2Gain_iter++;
	}
	std::multimap<int, std::pair<FlatNode *, ActorSet*> >::iterator gain2FlatNode2AdjPT_iter = gain2FlatNode2AdjPT.begin();
	while (gain2FlatNode2AdjPT_iter != gain2FlatNode2AdjPT.end())
	{
		if ((gain2FlatNode2AdjPT_iter->second).first == flatNode)
		{//ɾ��
			std::multimap<int, std::pair<FlatNode *, ActorSet*> >::iterator tmp = gain2FlatNode2AdjPT_iter;
			gain2FlatNode2AdjPT_iter++;
			gain2FlatNode2AdjPT.erase(tmp);
		}
		else gain2FlatNode2AdjPT_iter++;
	}

}

void HClusterThreadDynamicPartition::insertNewFlatNodeGain(FlatNode * flatNode, ActorSet * curGroup, std::map<std::pair<FlatNode*, ActorSet*>, int>& flatNode2AdjPT2Gain, std::multimap<int, std::pair<FlatNode*, ActorSet*>>& gain2FlatNode2AdjPT)
{
	int internalDegree = 0;//��¼�ڵ���ڵĶ���
	std::map<ActorSet *, int> adjPT2ExternalDegree;//��ǰ�ڵ����ڻ��������֮�����ӱߵ���Ŀ���ڵ�����ڵĻ��֣��ڵ���û��������ߵ���Ŀ��
	for (int i = 0; i != flatNode->inFlatNodes.size(); i++)
	{
		if (judgeInCurClusterNode(flatNode->inFlatNodes[i]))
		{
			std::map<FlatNode *, ActorSet * >::iterator pt_iter = m_actor2Set.find(flatNode->inFlatNodes[i]);
			assert(pt_iter != m_actor2Set.end());
			if (pt_iter->second == curGroup) internalDegree++;
			else
			{//����adjPT2ExternalDegree
				std::pair<std::map<ActorSet *, int>::iterator, bool> ret = adjPT2ExternalDegree.insert(std::make_pair(pt_iter->second, 1));
				if (!ret.second) ret.first->second += 1;
			}
		}//if
	}
	for (int i = 0; i != flatNode->outFlatNodes.size(); i++)
	{
		if (find(m_flatNodes.begin(), m_flatNodes.end(), flatNode->outFlatNodes[i]) != m_flatNodes.end())
		{
			map<FlatNode *, ActorSet * >::iterator pt_iter = m_actor2Set.find(flatNode->outFlatNodes[i]);
			assert(pt_iter != m_actor2Set.end());
			if (pt_iter->second == curGroup) internalDegree++;
			else
			{//����adjPT2ExternalDegree
				std::pair<std::map<ActorSet *, int>::iterator, bool> ret = adjPT2ExternalDegree.insert(std::make_pair(pt_iter->second, 1));
				if (!ret.second) ret.first->second += 1;
			}
		}//if
		
	}
	//����adjPT2ExternalDegree��internalDegree����flatNode2AdjPT2Gain��gain2FlatNode2AdjPT
	for (std::map<ActorSet *, int>::iterator ED_iter = adjPT2ExternalDegree.begin(); ED_iter != adjPT2ExternalDegree.end(); ED_iter++)
	{
		if (ED_iter->second == 0) continue;//���ڷǱ߽�ڵ������ʱ���������ȵĶ����У��������Ӳ��Һ͸���ʱ��
		flatNode2AdjPT2Gain.insert(std::make_pair(std::make_pair(flatNode, ED_iter->first), ED_iter->second - internalDegree));
		gain2FlatNode2AdjPT.insert(std::make_pair(ED_iter->second - internalDegree, std::make_pair(flatNode, ED_iter->first)));
	}
}

ActorSet::ActorSet(set<FlatNode*> nodes, SchedulerSSG *sssg) :m_flatNodes(nodes), m_sssg(sssg),m_workload(0),m_comm(0)
{
	for (set<FlatNode*>::iterator iter = nodes.begin(); iter != nodes.end(); ++iter)
	{
		m_workload += (m_sssg->GetSteadyCount(*iter) * m_sssg->GetSteadyWork(*iter));
	}//for
	m_comm = computeAllComm();
}

/*�ڵ�ǰ���ϵ��϶˼��ϲ���ָ���ļ���*/
void ActorSet::pushInSet(ActorSet *inSet)
{	
	set<FlatNode *> inFlatNodes = inSet->m_flatNodes;
	long comm = 0;
	for (set<FlatNode*>::iterator iter = m_flatNodes.begin(); iter != m_flatNodes.end(); ++iter)
	{
		int sc = m_sssg->GetSteadyCount(*iter);
		for (int i = 0; i < (*iter)->nIn; ++i)
		{
			if (inFlatNodes.count((*iter)->inFlatNodes[i]))
			{
				comm += (sc * (*iter)->inPopWeights[i]);
			}//if
		}//for

	}//for

	pair<map<ActorSet*, long>::iterator, long> ret = m_inSet2Comm.insert(make_pair(inSet, comm));
	if (ret.second)
		m_comm += comm;
	else
	{
		m_comm -= comm;
		ret.first->second = comm;
	}
}

void ActorSet::removeInSet(ActorSet *inSet)
{
	map<ActorSet *, long>::iterator iter = m_inSet2Comm.find(inSet);
	if (iter != m_inSet2Comm.end())
	{
		m_comm -= iter->second;
		m_inSet2Comm.erase(inSet);
	}
}

void ActorSet::pushOutSet(ActorSet *outSet)
{
	set<FlatNode *> outFlatNodes = outSet->m_flatNodes;
	long comm = 0;
	for (set<FlatNode*>::iterator iter = m_flatNodes.begin(); iter != m_flatNodes.end(); ++iter)
	{
		int sc = m_sssg->GetSteadyCount(*iter);
		for (int i = 0; i < (*iter)->nOut; ++i)
		{
			if (outFlatNodes.count((*iter)->outFlatNodes[i]))
			{
				comm += (sc * (*iter)->outPushWeights[i]);
			}//if
		}//for

	}//for
	pair<map<ActorSet*, long>::iterator, long> ret = m_outSet2Comm.insert(make_pair(outSet, comm));
	if (ret.second)
		m_comm += comm;
	else
	{
		m_comm -= comm;
		ret.first->second = comm;
	}
}

void ActorSet::removeOutSet(ActorSet *outSet)
{
	map<ActorSet *, long>::iterator iter = m_outSet2Comm.find(outSet);
	if (iter != m_outSet2Comm.end())
	{
		m_comm -= iter->second;
		m_outSet2Comm.erase(outSet);
	}
}

void ActorSet::pushActor(FlatNode *node)
{
	m_flatNodes.insert(node);
	m_workload += (m_sssg->GetSteadyCount(node) * m_sssg->GetSteadyWork(node));
	m_comm = computeAllComm();

	m_inSet2Comm.clear();
	m_outSet2Comm.clear();
}

void ActorSet::removeActor(FlatNode *node)
{
	m_flatNodes.erase(node);
	m_workload -= (m_sssg->GetSteadyCount(node) * m_sssg->GetSteadyWork(node));
	m_comm = computeAllComm();

	m_inSet2Comm.clear();
	m_outSet2Comm.clear();

}

vector<ActorSet*> ActorSet::getInSets()
{
	vector<ActorSet *> ret;
	for (map<ActorSet*, long>::iterator iter = m_inSet2Comm.begin(); iter != m_inSet2Comm.end(); ++iter)
	{
		ret.push_back(iter->first);
	}
	return ret;
}

vector<ActorSet*> ActorSet::getOutSets()
{
	vector<ActorSet *> ret;
	for (map<ActorSet*, long>::iterator iter = m_outSet2Comm.begin(); iter != m_outSet2Comm.end(); ++iter)
	{
		ret.push_back(iter->first);
	}
	return ret;
}

long ActorSet::computeAllComm()
{
	long ret = 0;
	for (set<FlatNode*>::iterator iter = m_flatNodes.begin(); iter != m_flatNodes.end(); ++iter)
	{
		//��ִ̬�д���
		int sc = m_sssg->GetSteadyCount(*iter);

		for (int i = 0; i < (*iter)->nIn; ++i)
		{
			if (m_flatNodes.count((*iter)->inFlatNodes[i]) == 0)
			{
				ret += (sc * (*iter)->inPopWeights[i]);
			}//if
		}//for

		for (int j = 0; j < (*iter)->nOut; ++j)
		{
			if (m_flatNodes.count((*iter)->outFlatNodes[j]) == 0)
			{
				ret += (sc * (*iter)->outPushWeights[j]);
			}//if
		}//for
	}//for
	return ret;
}

long ActorSet::computeTheComm(ActorSet *s)
{
	map<ActorSet *, long>::iterator iter;
	
	if ((iter = m_inSet2Comm.find(s)) != m_inSet2Comm.end())
		return iter->second;
	else if ((iter = m_outSet2Comm.find(s)) != m_outSet2Comm.end())
		return iter->second;
	else
		return 0;
}

ActorSet * ActorSet::unionSet(ActorSet *s)
{
	if (this == s)
		return this;

	set<FlatNode *> sNodes = s->m_flatNodes;
	m_flatNodes.insert(sNodes.begin(), sNodes.end());
	m_workload += s->m_workload;

	/*�ӵ�ǰ���ϵ����¶�ɾ��s*/	
	removeOutSet(s);
	removeInSet(s);

	/*�޸��ںϺ�ļ��ϵ����¶˼���*/
	map<ActorSet*, long> tmpInSet2Comm = s->m_inSet2Comm,tmpOutSet2Comm = s->m_outSet2Comm;
	for (map<ActorSet*, long>::iterator iter = tmpInSet2Comm.begin(); iter != tmpInSet2Comm.end(); ++iter)
	{
		if (iter->first == this)
			continue;

		pair<map<ActorSet *, long>::iterator, long> r = m_inSet2Comm.insert(make_pair(iter->first, iter->second));
		if (!r.second)
			r.first->second += iter->second;
		m_comm += iter->second;
	}//for
	for (map<ActorSet*, long>::iterator iter = tmpOutSet2Comm.begin(); iter != tmpOutSet2Comm.end(); ++iter)
	{
		if(iter->first == this)
			continue;

		pair<map<ActorSet *, long>::iterator, long> r = m_outSet2Comm.insert(make_pair(iter->first, iter->second));
		if (!r.second)
			r.first->second += iter->second;
		m_comm += iter->second;
	}

	/*�޸�s���ϵ����¶˼��ϵ����¶˼���*/
	for (map<ActorSet*, long>::iterator iter = tmpInSet2Comm.begin(); iter != tmpInSet2Comm.end(); ++iter)
	{
		if (iter->first == this)
			continue;

		iter->first->removeOutSet(s);
		iter->first->pushOutSet(this);
	}
	for (map<ActorSet*, long>::iterator iter = tmpOutSet2Comm.begin(); iter != tmpOutSet2Comm.end(); ++iter)
	{
		if(iter->first == this)
			continue;

		iter->first->removeInSet(s);
		iter->first->pushInSet(this);
	}

	delete s;
	return this;
}
