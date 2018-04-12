#include "ClusterGroupGraph.h"
#include <limits>
#include <float.h>
using namespace std;

const float MIN_WEIGHT = numeric_limits <float> ::min();

ClusterGroupGraph::ClusterGroupGraph(SchedulerSSG *SSSG)
{//�ȸ���SSG��flatNode����һ��group graph
	for(int i = 0; i!= SSSG->flatNodes.size();i++)
	{
		ClusterGroup *group = new ClusterGroup(SSSG->flatNodes[i]);
		clusterGroup2Radio.insert(make_pair(group,(float)(group->GetWorkload())/group->GetCommunicationCost()));
		flatNode2ClusterGroup.insert(make_pair(SSSG->flatNodes[i],group));
	}
	//����group���������ϵ��������Ҫ����һ�Խڵ������бߵ����ӵ���û�����ݴ��䣨push = 0,pop = 0����
	for (std::map<ClusterGroup *,float>::iterator iter = clusterGroup2Radio.begin();iter != clusterGroup2Radio.end(); iter++)
	{
		std::vector<FlatNode *> precedenceFlatNode = iter->first->GetPrecedenceFlatNode();
		std::vector<FlatNode *> successorFlatNode = iter->first->GetSuccessorFlatNode();
		std::map<FlatNode *,ClusterGroup *>::iterator pos;
		//ǰ��
		for (int i = 0; i != precedenceFlatNode.size(); i++)
		{
			pos = flatNode2ClusterGroup.find(precedenceFlatNode[i]);
			assert(pos != flatNode2ClusterGroup.end());
			iter->first->AddPrecedenceClusterGroup(pos->second);//��group��ǰ�����뵽clusterGroup��precedenceFlatNode��
		}
		//���
		for (int i = 0; i != successorFlatNode.size(); i++)
		{
			pos = flatNode2ClusterGroup.find(successorFlatNode[i]);
			assert(pos != flatNode2ClusterGroup.end());
			iter->first->AddSuccessorClusterGroup(pos->second);
			groupSrc2SnkChannel.insert(make_pair(iter->first, pos->second));
		}
	}	
}

//group����ں�
ClusterGroup* ClusterGroupGraph::FussGroup(ClusterGroup *group1,ClusterGroup *group2)
{ 
	ClusterGroup *newGroup = new ClusterGroup(group1,group2);
	ModifyConnectionOfGroup(group1,group2,newGroup);
	//�޸�group graph����Ϣ
	/*�޸�clusterGroup2Radio����Ϣ*/
	std::map<ClusterGroup *,float>::iterator pos1;
	std::map<ClusterGroup *,float>::iterator pos2;
	pos1 = this->clusterGroup2Radio.find(group1);
	this->clusterGroup2Radio.erase(pos1);
	pos2 = this->clusterGroup2Radio.find(group2);
	this->clusterGroup2Radio.erase(pos2);
	this->clusterGroup2Radio.insert(make_pair(newGroup, CompToCommRadio(newGroup)));
	//�޸�flatNode2ClusterGroup
	std::vector<FlatNode *> group1FlatNode = group1->GetFlatNodes();
	std::vector<FlatNode *> group2FlatNode = group2->GetFlatNodes();
	std::map<FlatNode *,ClusterGroup *>::iterator pos3;
	for(int i = 0; i != group1FlatNode.size(); i++)
	{
		pos3 = this->flatNode2ClusterGroup.find(group1FlatNode[i]);
		pos3->second = newGroup;
	}
	for(int i = 0; i != group2FlatNode.size(); i++ )
	{
		pos3 =this->flatNode2ClusterGroup.find(group2FlatNode[i]);
		pos3->second = newGroup;
	}
	//�޸�groupSrc2SnkChannel�е���Ϣ
	this->groupSrc2SnkChannel.erase(group1);//ɾ����group1��Ϊ���ı�
	this->groupSrc2SnkChannel.erase(group2);//ɾ����Group2��Ϊ���ı�
	//Ҫ�ĳ�ɾ����group1��group2 ��Ϊ�յ�ı�
	std::multimap<ClusterGroup*, ClusterGroup*>::iterator iter = this->groupSrc2SnkChannel.begin();
	Bool eraseflag = false;
	while (iter !=  this->groupSrc2SnkChannel.end())
	{
		eraseflag = false;
		std::multimap<ClusterGroup*, ClusterGroup*>::iterator iter1;
		if(iter->second == group1 || iter->second == group2) {eraseflag = TRUE; iter1 = iter;}
		iter++;
		if(eraseflag){ this->groupSrc2SnkChannel.erase(iter1);}
		
	}
	//��groupSrc2SnkChannel�в�����newgroup��Ϊǰ���ı�
	std::vector<ClusterGroup *> successorClusterGroup = newGroup->GetSuccessorClusterGroup();
	for(int i =0; i != successorClusterGroup.size(); i++)
	{
		this->groupSrc2SnkChannel.insert(make_pair(newGroup, successorClusterGroup[i]));
	}	
	std::vector<ClusterGroup *> procedenceClusterGroup = newGroup->GetPrecedenceClusterGroup();
	for(int i =0; i != procedenceClusterGroup.size(); i++)
	{
		this->groupSrc2SnkChannel.insert(make_pair( procedenceClusterGroup[i],newGroup));
	}	
	return newGroup;
}

//����һ��group�ļ����ͨ�ű�
float ClusterGroupGraph::CompToCommRadio(ClusterGroup *group)
{
	float commCost = group->GetCommunicationCost();
	int workload = group->GetWorkload();
	return ((float)workload)/commCost;
}

//��group graph�������ˣ��޻��򷵻�true�����򷵻�false
Bool ClusterGroupGraph::HasGroupTopologicalSort(std::vector<ClusterGroup *>original)
{
	vector<ClusterGroup *>::iterator iter1,iter2,iter4;
	vector<ClusterGroup *>::iterator iter;
	vector<int> nInDegree;//���ڱ�����ڵ�����
	vector<ClusterGroup *> groupStack;
	vector<int>::iterator iter3;
	int nsize=original.size();
	int count = 0;
	for (iter1=original.begin();iter1!=original.end();++iter1)
	{
		nInDegree.push_back(((*iter1)->GetPrecedenceClusterGroup()).size());//�����ڵ���ȱ���
	}
	for (int i = 0; i != nInDegree.size();i++)
	{
		if(!nInDegree[i]) groupStack.push_back(original[i]);
	}

	while (!groupStack.empty())
	{
		ClusterGroup *group = groupStack.back();// ȡ��Ҫ��ջ�Ľڵ�
		std::vector<ClusterGroup *>tmpProClusterGroup = group->GetSuccessorClusterGroup();//ȡgroup�����к��
		groupStack.pop_back();
		++count;
		for(int i= 0; i != tmpProClusterGroup.size();i++)//������group���ڽӵ����ȼ�1
		{
			for (int j =0;j != original.size(); j++)
			{
				if(original[j] == tmpProClusterGroup[i])
				{
					if(!(--nInDegree[j])) groupStack.push_back(original[j]);//���Ϊ0���ջ
				}
			}
		}
	}
	if(count < original.size()) return FALSE;
	else return TRUE;
}

//************************************
// Qualifier: ��group graph �ںϳ�K�ݣ������յ�groupֻ����K�ڵ�
//************************************
void ClusterGroupGraph::CreateCoarseGraph(int k)
{
	coarseGroupSort.clear();//ʹ��ǰ�Ƚ������
	coarseGroupResult.clear();

	while( clusterGroup2Radio.size() > k)
	{
		//��һ��group���кϲ���Ҫ��ͨ�űȵ�������󣬲��Һϲ����󲻻����뻷
		//����ÿһ��group�ļ���ͨ�ű�
		std::vector<float> coarsingWeight;

		std::vector<ClusterGroup *> currentGroups;
		for(std::map<ClusterGroup *, float>::iterator cluster_iter = this->clusterGroup2Radio.begin();cluster_iter != this->clusterGroup2Radio.end();cluster_iter++)
		{
			currentGroups.push_back(cluster_iter->first);
		}		
		//����ÿһ������group�ļ���ͨ�ű�
		for(std::multimap<ClusterGroup* ,ClusterGroup *>::iterator iter= this->groupSrc2SnkChannel.begin();iter != this->groupSrc2SnkChannel.end(); iter++)
		{
			ClusterGroup* tmpGroup = new ClusterGroup(iter->first,iter->second);
			ModifyConnectionOfGroup(iter->first,iter->second,tmpGroup);
			std::vector<ClusterGroup *>::iterator pos1,pos2;
			for (std::vector<ClusterGroup *>::iterator iter1 = currentGroups.begin(); iter1 != currentGroups.end(); iter1++)
			{
				if((*iter1) == iter->first) {pos1 = iter1;break;}
			}
			assert(pos1 != currentGroups.end() && (*pos1) == iter->first);
			currentGroups.erase(pos1);
			for (std::vector<ClusterGroup *>::iterator iter1 = currentGroups.begin(); iter1 != currentGroups.end(); iter1++)
			{
				if((*iter1) == (iter->second)) {pos2 = iter1;break;}
			}
			assert(pos2 != currentGroups.end() && (*pos2) == (iter->second) );
			currentGroups.erase(pos2);
			currentGroups.push_back(tmpGroup);

			if(HasGroupTopologicalSort(currentGroups))
			{
				coarsingWeight.push_back((iter->first->GetCompCommRadio() + iter->second->GetCompCommRadio()) - tmpGroup->GetCompCommRadio() );//�޻�������
			}
			else 
			{
				coarsingWeight.push_back(MIN_WEIGHT);
			}
			currentGroups.pop_back();
			currentGroups.push_back(iter->first);
			currentGroups.push_back(iter->second);
			
			//�����ڹ����µ�groupʱ�޸���iter->first����iter->second�е�ǰ��group�ĺ�̣��ͺ��group�е�ǰ��������Ҫ��ԭ
			vector<ClusterGroup *>tmpPreGroup = tmpGroup->GetPrecedenceClusterGroup();
			vector<ClusterGroup *>tmpSuccGroup = tmpGroup->GetSuccessorClusterGroup();
			vector<FlatNode *> tmpFlatNodes1 = iter->first->GetFlatNodes();
			vector<FlatNode *> tmpFlatNodes2 = iter->second->GetFlatNodes();
			Bool flag1 = FALSE;
			Bool flag2 = FALSE;
			for(int i = 0;i !=tmpPreGroup.size(); i++)
			{
				flag1 = FALSE;
				flag2 = FALSE;
				if(tmpPreGroup[i]->DeleteSuccessorClusterGroup(tmpGroup))
				{
					//ǰ����snk�ڵ�				
					vector<FlatNode *> tmpSnkFlatNode = tmpPreGroup[i]->GetSnkFlatNode();
					for(int j =0; j != tmpSnkFlatNode.size();j++)
					{
						
						for(int k = 0; k < tmpSnkFlatNode[j]->nOut;k++)
						{
							if(tmpSnkFlatNode[j]->outPushWeights[k] == 0)  continue;//20130119 zww ���
							for (int l = 0; l != tmpFlatNodes1.size(); l++)
							{
								if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
							}
							for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
							{
								if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
							}
						}
					}
					if(flag1)  tmpPreGroup[i]->AddSuccessorClusterGroup(iter->first);
					if(flag2)  tmpPreGroup[i]->AddSuccessorClusterGroup(iter->second);
				}						
			}
			for(int i = 0;i !=tmpSuccGroup.size(); i++)
			{
				flag1 = FALSE;
				flag2 = FALSE;
				if(tmpSuccGroup[i]->DeletePrecedenceClusterGroup(tmpGroup))
				{
					//ǰ����src�ڵ�				
					vector<FlatNode *> tmpSrcFlatNode = tmpSuccGroup[i]->GetSrcFlatNode();

					for(int j =0; j != tmpSrcFlatNode.size();j++)
					{
						for(int k = 0; k < tmpSrcFlatNode[j]->nIn;k++)
						{
							if(tmpSrcFlatNode[j]->inPopWeights[k] == 0)  continue;//20130119 zww ���
							for (int l = 0; l != tmpFlatNodes1.size(); l++)
							{
								if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
							}
							for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
							{
								if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
							}
						}
					}
					if(flag1)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(iter->first);
					if(flag2)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(iter->second);
				}								
			}
			assert(currentGroups.size() == this->clusterGroup2Radio.size());//��֤��������Ĳ�����group����Ŀû�б�
		}
		//�����ߵ�λ��
		float maxWeight = MIN_WEIGHT;
		int maxIter = 0;
		for(int i = 0; i!= coarsingWeight.size();i++)
		{
			if(coarsingWeight[i]>maxWeight)
			{
					maxWeight = coarsingWeight[i];
					maxIter = i;
			}
		}
		int p = 0;
		std::multimap<ClusterGroup* ,ClusterGroup *>::iterator wait_pos;
		for(std::multimap<ClusterGroup* ,ClusterGroup *>::iterator iter3= this->groupSrc2SnkChannel.begin();iter3 != this->groupSrc2SnkChannel.end();iter3++)
		{	
			if(p == maxIter){wait_pos = iter3;break;}
			p++;
		}
		coarseGroupSort.push_back(make_pair(wait_pos->first, wait_pos->second));
		//�ں�
		ClusterGroup *resultGroup = FussGroup(wait_pos->first, wait_pos->second);	
		coarseGroupResult.push_back(resultGroup);
		coarsingWeight.clear();
		currentGroups.clear();
	}
	mngroups = k;
}

/*
//�ںϣ������ںϵĽ���ǲ����ģ���K�Ĳ���Ԥ������ģ����Ǹ���ͼ��������ȷ���ģ����������ںϺ��group����ĿҪ���ڼ�Ⱥ�ڵ����Ŀ��
void ClusterGroupGraph::CreateCoarseGraph()
{//(�ú���δ����)
	std::vector< std::pair<ClusterGroup*,ClusterGroup*> > vecswap;
	coarseGroupSort.swap(vecswap);//ʹ��ǰ�Ƚ������
	coarseGroupResult.clear();
	int coarsingcount = 0;

	while(1)
	{
		int groupcount = 0;

		//��һ��group���кϲ���Ҫ��ͨ�űȵ�������󣬲��Һϲ����󲻻����뻷
		//����ÿһ��group�ļ���ͨ�ű�
		std::vector<float> coarsingWeight;

		std::vector<ClusterGroup *> currentGroups;
		for(std::map<ClusterGroup *, float>::iterator cluster_iter = this->clusterGroup2Radio.begin();cluster_iter != this->clusterGroup2Radio.end();cluster_iter++)
		{
			currentGroups.push_back(cluster_iter->first);
		}
		cout<<"===��ǰgroup����Ŀ  "<<clusterGroup2Radio.size()<<"  �ߵ���Ŀ  "<<groupSrc2SnkChannel.size() <<endl;			
		//����ÿһ������group�ļ���ͨ�ű�
		for(std::multimap<ClusterGroup* ,ClusterGroup *>::iterator iter= this->groupSrc2SnkChannel.begin();iter != this->groupSrc2SnkChannel.end();iter++)
		{
			groupcount++;
			coarsingcount++;

			ClusterGroup* tmpGroup = new ClusterGroup(iter->first,iter->second);
			ModifyConnectionOfGroup(iter->first,iter->second,tmpGroup);
			std::vector<ClusterGroup *>::iterator pos1,pos2;
			for (std::vector<ClusterGroup *>::iterator iter1 = currentGroups.begin(); iter1 != currentGroups.end(); iter1++)
			{
				if((*iter1) == iter->first) {pos1 = iter1;break;}
			}
			assert(pos1 != currentGroups.end() && (*pos1) == iter->first);
			currentGroups.erase(pos1);
			for (std::vector<ClusterGroup *>::iterator iter1 = currentGroups.begin(); iter1 != currentGroups.end(); iter1++)
			{
				if((*iter1) == (iter->second)) {pos2 = iter1;break;}
			}
			assert(pos2 != currentGroups.end() && (*pos2) == (iter->second) );
			currentGroups.erase(pos2);
			currentGroups.push_back(tmpGroup);
			cout<<"   ����ͨ�ű����� "<<tmpGroup->GetCompCommRadio()-(iter->first->GetCompCommRadio() + iter->second->GetCompCommRadio())<<endl;
			if(HasGroupTopologicalSort(currentGroups))
			{
				coarsingWeight.push_back(tmpGroup->GetCompCommRadio()-(iter->first->GetCompCommRadio() + iter->second->GetCompCommRadio()));//�޻�������
				cout<<"   �޻�  "<<groupcount<<"   "<<coarsingcount<<endl;
			}
			else 
			{
				cout<<"   �л�   "<<groupcount<<"   "<<coarsingcount<<endl;
				coarsingWeight.push_back(MIN_WEIGHT);
			}
			currentGroups.pop_back();
			currentGroups.push_back(iter->first);
			currentGroups.push_back(iter->second);

			//�����ڹ����µ�groupʱ�޸���iter->first����iter->second�е�ǰ��group�ĺ�̣��ͺ��group�е�ǰ��������Ҫ��ԭ
			vector<ClusterGroup *>tmpPreGroup = tmpGroup->GetPrecedenceClusterGroup();
			vector<ClusterGroup *>tmpSuccGroup = tmpGroup->GetSuccessorClusterGroup();
			vector<FlatNode *> tmpFlatNodes1 = iter->first->GetFlatNodes();
			vector<FlatNode *> tmpFlatNodes2 = iter->second->GetFlatNodes();
			Bool flag1 = FALSE;
			Bool flag2 = FALSE;
			for(int i = 0;i !=tmpPreGroup.size(); i++)
			{
				flag1 = FALSE;
				flag2 = FALSE;
				if(tmpPreGroup[i]->DeleteSuccessorClusterGroup(tmpGroup))
				{
					//ǰ����snk�ڵ�				
					vector<FlatNode *> tmpSnkFlatNode = tmpPreGroup[i]->GetSnkFlatNode();

					for(int j =0; j != tmpSnkFlatNode.size();j++)
					{
						for(int k = 0; k < tmpSnkFlatNode[j]->nOut;k++)
						{
							for (int l = 0; l != tmpFlatNodes1.size(); l++)
							{
								if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
							}
							for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
							{
								if(tmpSnkFlatNode[j]->outFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
							}
						}

					}
					if(flag1)  tmpPreGroup[i]->AddSuccessorClusterGroup(iter->first);
					if(flag2)  tmpPreGroup[i]->AddSuccessorClusterGroup(iter->second);
				}						
			}
			for(int i = 0;i !=tmpSuccGroup.size(); i++)
			{
				flag1 = FALSE;
				flag2 = FALSE;
				if(tmpSuccGroup[i]->DeletePrecedenceClusterGroup(tmpGroup))
				{
					//ǰ����snk�ڵ�				
					vector<FlatNode *> tmpSrcFlatNode = tmpSuccGroup[i]->GetSrcFlatNode();

					for(int j =0; j != tmpSrcFlatNode.size();j++)
					{
						for(int k = 0; k < tmpSrcFlatNode[j]->nIn;k++)
						{
							for (int l = 0; l != tmpFlatNodes1.size(); l++)
							{
								if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes1[l]) {flag1 = TRUE;break;}
							}
							for(int ll = 0; ll != tmpFlatNodes2.size(); ll++)
							{
								if(tmpSrcFlatNode[j]->inFlatNodes[k] == tmpFlatNodes2[ll]) {flag2 = TRUE;break;}
							}
						}
					}
					if(flag1)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(iter->first);
					if(flag2)  tmpSuccGroup[i]->AddPrecedenceClusterGroup(iter->second);
				}								
			}
			assert(currentGroups.size() == this->clusterGroup2Radio.size());//��֤��������Ĳ�����group����Ŀû�б�
		}
		//�����ߵ�λ��
		float maxWeight = MIN_WEIGHT;
		int maxIter = 0;
		for(int i = 0; i!= coarsingWeight.size();i++)
		{
			if(coarsingWeight[i]>maxWeight)
			{
				maxWeight = coarsingWeight[i];
				maxIter = i;
			}
		}
		if(maxWeight < THRESHOD_COMP_COMM_RADIO) break;
		int p = 0;
		std::multimap<ClusterGroup* ,ClusterGroup *>::iterator wait_pos;
		for(std::multimap<ClusterGroup* ,ClusterGroup *>::iterator iter3= this->groupSrc2SnkChannel.begin();iter3 != this->groupSrc2SnkChannel.end();iter3++)
		{	
			if(p == maxIter){wait_pos = iter3;break;}
			p++;
		}
		coarseGroupSort.push_back(make_pair(wait_pos->first, wait_pos->second));
		//�ں�
		ClusterGroup *resultGroup = FussGroup(wait_pos->first, wait_pos->second);	
		//currentGroups.push_back(resultGroup);
		coarseGroupResult.push_back(resultGroup);
		vector<float> swapVec1;
		coarsingWeight.swap(swapVec1);
		vector<ClusterGroup *> swapVec;
		currentGroups.swap(swapVec);

	}
	SetGroupNum(clusterGroup2Radio.size());//�������յ�group��Ŀ
}
*/

std::vector< std::pair<ClusterGroup*,ClusterGroup*> > ClusterGroupGraph::GetCoarseGroupSort()
{//�����ںϵ�˳��
	return coarseGroupSort;
}

std::vector<ClusterGroup *> ClusterGroupGraph::GetCoarseGroupResult()
{//�����ںϵ�˳��
	return coarseGroupResult;
}

std::vector<ClusterGroup *> ClusterGroupGraph::GetClusterGroupSet()
{
	std::vector<ClusterGroup *> vecswap;
	clusterGroupVec.swap(vecswap);//���ԭ�������ݣ����ͷ��ڴ�
	typedef std::map<ClusterGroup *,float>::iterator Num2NodeIter;
	for(Num2NodeIter iter=clusterGroup2Radio.begin();iter!=clusterGroup2Radio.end();++iter)
	{
		clusterGroupVec.push_back(iter->first);
	}
	return clusterGroupVec;
}

ClusterGroup *ClusterGroupGraph::GetClusterGroup(FlatNode *floatNode)
{
	std::map<FlatNode *,ClusterGroup *>::iterator pos;
	pos = flatNode2ClusterGroup.find(floatNode);
	return pos->second;	
}

int ClusterGroupGraph::GetGroupNum()
{
	return mngroups;
}

void ClusterGroupGraph::SetGroupNum(int n)
{
	mngroups = n;
}

std::multimap<ClusterGroup*, ClusterGroup*> ClusterGroupGraph::GetGroupSrc2SnkChannel()
{
	return groupSrc2SnkChannel;
}


void ClusterGroupGraph::ModifyConnectionOfGroup(ClusterGroup *group1,ClusterGroup *group2,ClusterGroup* group)
{
	//ǰ��clusterGroup
	//����group�ϲ�����Ҫ�޸����ǵ�ǰ���ĺ���Լ���̵�ǰ������ע��֮��Ҫ��ԭ��
	vector<ClusterGroup*> precedenceClusterGroup1 = group1->GetPrecedenceClusterGroup();
	vector<ClusterGroup*> precedenceClusterGroup2 = group2->GetPrecedenceClusterGroup();
	vector<ClusterGroup*> successorClusterGroup1 = group1->GetSuccessorClusterGroup();
	vector<ClusterGroup*> successorClusterGroup2 = group2->GetSuccessorClusterGroup();;
	for(int i = 0; i != precedenceClusterGroup1.size(); i++)
	{
		if(precedenceClusterGroup1[i] != group2) 
		{
			group->AddPrecedenceClusterGroup(precedenceClusterGroup1[i]);
			//Ҫ�޸�ǰ���ĺ��
			if(precedenceClusterGroup1[i]->DeleteSuccessorClusterGroup(group1))//�޸�ǰ���ĺ��
				precedenceClusterGroup1[i]->AddSuccessorClusterGroup(group);
		}
	}
	for (int i = 0; i !=precedenceClusterGroup2.size(); i++ )
	{
		if(precedenceClusterGroup2[i] != group1)
		{
			group->AddPrecedenceClusterGroup(precedenceClusterGroup2[i]);
			//Ҫ�޸�ǰ���ĺ��
			if(precedenceClusterGroup2[i]->DeleteSuccessorClusterGroup(group2))//�޸�ǰ���ĺ��
				precedenceClusterGroup2[i]->AddSuccessorClusterGroup(group);
		}		
	}
	//���clusterGroup
	for(int i = 0; i != successorClusterGroup1.size(); i++)
	{
		if(successorClusterGroup1[i] != group2) 
		{
			group->AddSuccessorClusterGroup(successorClusterGroup1[i]);
			//Ҫ�޸ĺ�̵�ǰ��
			if(successorClusterGroup1[i]->DeletePrecedenceClusterGroup(group1))//�޸ĺ�̵�ǰ��
				successorClusterGroup1[i]->AddPrecedenceClusterGroup(group);
		}
	}
	for (int i = 0; i !=successorClusterGroup2.size(); i++ )
	{
		if(successorClusterGroup2[i] != group1)
		{
			group->AddSuccessorClusterGroup(successorClusterGroup2[i]);
			if(successorClusterGroup2[i]->DeletePrecedenceClusterGroup(group2))//�޸ĺ�̵�ǰ��
				successorClusterGroup2[i]->AddPrecedenceClusterGroup(group);
		}		
	}
}

std::map<FlatNode *,ClusterGroup *> ClusterGroupGraph::GetFlatNode2ClusterGroup()
{	//����flatNode��group֮���map
	return flatNode2ClusterGroup;
}