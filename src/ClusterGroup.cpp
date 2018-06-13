#include "ClusterGroup.h"
using namespace std;


ClusterGroup::ClusterGroup(FlatNode *flatNode)//���캯��
{//ֻ��һ���ڵ㹹��group��һ��group�оͺ���һ��flatNode�ڵ㡣(����SDFͼ�����ʼflatNode)
	srcFlatNode.push_back(flatNode);//Դ
	snkFlatNode.push_back(flatNode);//��
	flatNodes.push_back(flatNode);//��㼯��
	flatNode2SteadyCount.insert(make_pair(flatNode,1));//��̬
	for(int i=0;i<flatNode->inFlatNodes.size();i++)
			if(flatNode->inPopWeights[i])precedenceFlatNode.push_back(flatNode->inFlatNodes[i]);//��ǰ��flatNode (�����ݴ���)
	for(int i=0;i<flatNode->outFlatNodes.size();i++)
			if(flatNode->outPushWeights[i])successorFlatNode.push_back(flatNode->outFlatNodes[i]);//����flatNode(�����ݴ���)
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

ClusterGroup::ClusterGroup(ClusterGroup* group1,ClusterGroup* group2)//���캯��
{//���������ڵ�group�ۺϳ�һ��
	//flatNode �ڵ�ļ���
	lock = FALSE;
	for(int i = 0; i!= group1->flatNodes.size(); i++)
	{
		flatNodes.push_back(group1->flatNodes[i]);
	}
	for(int i = 0; i!= group2->flatNodes.size(); i++)
	{
		flatNodes.push_back(group2->flatNodes[i]);
	}
	 // ǰ��flatNode
	Bool flag = TRUE;
	for (int i = 0;i != group1->precedenceFlatNode.size(); i++)
	{
		flag = TRUE;
		for (int j = 0;j != group2->flatNodes.size();j++)
		{
			if(group1->precedenceFlatNode[i]== group2->flatNodes[j]){flag = FALSE;break;}//���group1��ǰ����group2�У����ǰ��������Ϊ�µ�group��ǰ��
		}
		if(flag)  this->AddPrecedenceFlatNode(group1->precedenceFlatNode[i]);
	}
	for (int i = 0;i != group2->precedenceFlatNode.size(); i++)
	{//Ҫ�ų���group1��group1��ǰ����
		flag = TRUE;
		for (int j = 0;j != group1->flatNodes.size();j++)
		{
			for(int k = 0 ;k!= group1->precedenceFlatNode.size();k++)
			if(group2->precedenceFlatNode[i]== group1->flatNodes[j] ) {flag = FALSE;break;}	
		}
		if(flag) this->AddPrecedenceFlatNode(group2->precedenceFlatNode[i]);
	}
	//���flatNode
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
	//��src ��snk�����Ԫ��(�г�ȥ�ı߾����յ㣬�н����ı߾������)
	for (int i = 0; i != this->flatNodes.size(); i++)
	{
		int num = 0;
		flag = TRUE;
		int zeroEdge = 0;
		for (int j = 0; j != this->flatNodes[i]->inFlatNodes.size();j++)
		{
			if(this->flatNodes[i]->inPopWeights[j] == 0) {zeroEdge++;continue;}////20121204��ӣ������б����ӵ�û�����ݴ��䣩

			for (int kk = 0; kk!= this->srcFlatNode.size();kk++)
			{
				if(this->flatNodes[i]== this->srcFlatNode[kk] ){flag = FALSE; break;}//�Ѿ���src�оͲ�����
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
		if(this->flatNodes[i]->inFlatNodes.size() == 0) this->srcFlatNode.push_back(this->flatNodes[i]);//����source�ڵ�
		num = 0;
		flag = TRUE;
		zeroEdge = 0;
		for (int j = 0; j != this->flatNodes[i]->outFlatNodes.size();j++)
		{
			if(this->flatNodes[i]->outPushWeights[j] == 0){ zeroEdge++;continue;}////20121204��ӣ������б����ӵ�û�����ݴ��䣩
			for (int kk = 0; kk!= this->snkFlatNode.size();kk++)
			{
				if(this->flatNodes[i]== this->snkFlatNode[kk] ){flag = FALSE; break;}//�Ѿ���src�оͲ�����
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
	// group1��group2 ��Ϊ��group��ӵ��µ�group��
	subClusterGroups.push_back(group1);
	subClusterGroups.push_back(group2);
	//����
	flatNode2SteadyCount = SteadySchedulingGroup(flatNodes);
	//����ͨ�ſ���
	std::vector<int> sendDataSize;//��Ӧ������
	std::vector<int> recvDataSize;//��Ӧ����Դ
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
	//ȷ��group�Ĺ�����
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
{//ȡԴ
	return srcFlatNode;
}

std::vector<FlatNode *> ClusterGroup::GetSnkFlatNode()
{//ȡ��
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
{//�����group���ظ�
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
	boundaryGroupSet.swap(vecswap);//���ԭ�������ݣ����ͷ��ڴ�
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
			if(k == subClusterGroups.size()) {flag = TRUE;break;}//��ǰ������group��
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
			if(k == subClusterGroups.size()) {flag = TRUE;break;}//��ǰ������group��
		}
		boundaryGroupSet.push_back(successorClusterGroup[i]);
	}	
	return boundaryGroupSet;

}

void ClusterGroup::AddSuccessorClusterGroup(ClusterGroup *group)
{//�����group���ظ�
	Bool flag = TRUE;
	for(int i=0; i != successorClusterGroup.size(); i++)
	{
		if(successorClusterGroup[i] == group){flag = FALSE; break;}
	}
	if(flag) successorClusterGroup.push_back(group);
}

Bool ClusterGroup::DeleteSuccessorClusterGroup(ClusterGroup *group)
{//�����group���ظ�
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
{//�����group���ظ�
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

//��a,b�����Լ��
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



//��a,b����С������
int lcm(int a, int b)
{
	int product = a * b;

	return product/gcd(a,b);
}

GLOBAL std::map<FlatNode *, int> SteadySchedulingGroup(std::vector<FlatNode *>flatNodeVec)
{//����һ���ֲ�����̬
	
	list<FlatNode *> flatNodeList;
	std::map<FlatNode *,int>::iterator pos;
	std::map<FlatNode *, int> flatNode2SteadyCount;
	assert(flatNodeVec.size() > 0);
	map<FlatNode * ,Bool> flatNodesTag;//���ڱ�ʾflatNodeVec�еĽڵ��Ƿ񱻵���
	for (int indexNode = 0; indexNode != flatNodeVec.size(); indexNode++)
	{
		flatNodesTag.insert(make_pair(flatNodeVec[indexNode],FALSE));
	}
	// Ĭ�ϵ�һ���ڵ���Դ��Ҳ����˵peek��pop��Ϊ0,��ͼ�ı�ʾ���ݲ������ж��Դ���������ж��peek = pop = 0�ڵ�
	FlatNode *up = flatNodeVec[0], *down = NULL, *parent = NULL;
	int nPush = 0, nPop = 0, nLcm = 0;
	int x, y, i, j;
	Bool flag = FALSE;//ֻ�е�flatNode��flatNodeVec�вŽ��е���
	while (!flatNodesTag.empty())
	{	
		up = flatNodesTag.begin()->first;
		while(1)
		{		
			// ��̬����ϵ�г�ʼϵ��Ϊ1
			flatNode2SteadyCount.insert(make_pair(up, 1));
			flatNodesTag.erase(up);
			// �����ýڵ����������ڵ�up��child�ڵ���еĵ��ȣ���ѭ��ִ���꣬��up��parent�ڵ���е���
			/*����up��child�ڵ�*/
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
					nPush = up->outPushWeights[i]; // �϶˽ڵ��pushֵ
					down = up->outFlatNodes[i]; // �ҵ��¶˽ڵ�

					for (j = 0; down->inFlatNodes[j] != up; j++); // �¶˽ڵ��ҵ����϶˽ڵ��Ӧ�ı��
					nPop = down->inPopWeights[j]; // �¶˽ڵ�ȡ����Ӧ��popֵ

					// ���ýڵ��Ƿ��ѽ�����̬���ȣ�ÿ��ֻ����һ����̬����
					pos = flatNode2SteadyCount.find(down);
					// �ýڵ�δ������̬����
					if (pos == flatNode2SteadyCount.end())
					{
						// �õ��϶˽ڵ����̬����ϵ��
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;
						nPush *= x; // Ϊʲô��x*nPush�أ������̬���ȵĸ���--�ڵ�����ˮ���ȶ�������ִ�е����ٴ���
						if(nPush != 0)
						{
							// nPush, nPop����С������;
							nLcm = lcm(nPush, nPop);
							int temp = nLcm/nPush;
							if( temp != 1) // ��һ���жϣ����Ч�ʣ���1�ǲ���Ҫ��
							{
								// ���ݼ�����������
								for (pos = flatNode2SteadyCount.begin(); pos != flatNode2SteadyCount.end(); ++pos)
									pos->second *= temp;
							}

							flatNode2SteadyCount.insert(make_pair(down, nLcm/nPop));
							flatNodesTag.erase(down);
							// ��down����listNode��Ϊ�˶�down������ڵ���е���
							flatNodeList.push_back(down);
						}
					}
					else //�ýڵ��ѽ�����̬���ȣ����SDFͼ�Ƿ������̬����ϵ�У�һ�㲻���ڵĻ�������������
					{
						y = pos->second;
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;

						//nPop == 0 ˵���ڽ���join 0 ����
						if((nPop != 0 ) && (nPush * x) != (nPop * y)) 
						{
							cout<<"��������̬����1"<<endl;
							system("pause");
							exit(1); // ��ʾ��������̬����
						}
					}
				}
			}
			/*����up��parent�ڵ�*/
			for (i = 0; i < up->nIn; ++i)
			{
				flag = FALSE;
				for (int k = 0; k != flatNodeVec.size(); ++k)
				{//�ж�parent�ڵ��ڲ���flatNodeVec��
					if(up->inFlatNodes[i] == flatNodeVec[k]) 
					{
						flag = TRUE;break;
					}
				}

				if(flag)
				{
					nPop = up->inPopWeights[i]; // ��ǰ�ڵ��popֵ
					parent = up->inFlatNodes[i]; // �ҵ���ǰ�ڵ�ĸ��ڵ�

					for (j = 0; parent->outFlatNodes[j] != up; j++); // up�ڵ���parent������ڵ��ж�Ӧ�ı��
					nPush = parent->outPushWeights[j]; // parent�ڵ�ȡ����Ӧ��pushֵ

					// ���ýڵ��Ƿ��ѽ�����̬���ȣ�ÿ��ֻ����һ����̬����
					pos = flatNode2SteadyCount.find(parent);
					// �ýڵ�δ������̬����
					if (pos == flatNode2SteadyCount.end())
					{
						// �õ��϶˽ڵ����̬����ϵ��
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;
						nPop *= x; // Ϊʲô��x*nPush�أ������̬���ȵĸ���--�ڵ�����ˮ���ȶ�������ִ�е����ٴ���
						if(nPop != 0)
						{
							// nPush, nPop����С������;
							nLcm = lcm(nPush, nPop);
							int temp = nLcm/nPop;
							if( temp != 1) // ��һ���жϣ����Ч�ʣ���1�ǲ���Ҫ��
							{
								// ���ݼ�����������
								for (pos = flatNode2SteadyCount.begin(); pos != flatNode2SteadyCount.end(); ++pos)
									pos->second *= temp;
							}

							flatNode2SteadyCount.insert(make_pair(parent, nLcm/nPush));
							flatNodesTag.erase(parent);
							// ��down,parent����listNode��Ϊ�˶�down������ڵ���е���
							flatNodeList.push_back(parent);
						}					
					}
					else //�ýڵ��ѽ�����̬���ȣ����SDFͼ�Ƿ������̬����ϵ�У�һ�㲻���ڵĻ�������������
					{
						y = pos->second;
						pos = flatNode2SteadyCount.find(up);
						x = pos->second;

						//nPop == 0 ˵���ڽ���join 0 ����
						if((nPop != 0 ) && (nPop * x) != (nPush * y))
						{
							cout<<"��������̬����2"<<endl;
							system("pause");
							exit(1); // ��ʾ��������̬����
						}
					}
				}

			}
			if(flatNodeList.size() == 0) break; // ����Ϊ�գ�˵�����нڵ��ѵ������
			up = flatNodeList.front();
			flatNodeList.pop_front();
			
		}
	}
	return flatNode2SteadyCount;
}




