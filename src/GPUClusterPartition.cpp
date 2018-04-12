#include "GPUClusterPartition.h"

using namespace std;

void GPUClusterPartition::SssgPartition(SchedulerSSG *sssg,int level)
{
	assert(level==1);
	nvtxs=sssg->GetFlatNodes().size();
	if(this->mnparts == 1)
	{
		for (int i=0;i<nvtxs;i++)
		{
			if (!DetectiveActorState(sssg->GetFlatNodes()[i])&& (UporDownStatelessNode(sssg->GetFlatNodes()[i]) != 3))
			{
				FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i],0));//�����ڵ㵽���ֱ�ŵ�ӳ��
				PartitonNum2FlatNode.insert(make_pair(0,sssg->GetFlatNodes()[i]));
				sssg->GetFlatNodes()[i]->GPUPart = 0;
			}
			else
			{
				FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i],1));//�����ڵ㵽���ֱ�ŵ�ӳ��
				PartitonNum2FlatNode.insert(make_pair(1,sssg->GetFlatNodes()[i]));
				sssg->GetFlatNodes()[i]->GPUPart = 1;
			}
		}

#if 1 //��ӡͼ
		DumpStreamGraph(sssg,this,"BSPartitionGraph.dot",NULL);
#endif
	}
	else
	{
		//�����ҳ�����GPU�ڵ�
		for (int i = 0; i < nvtxs; i++)
		{
			if (!DetectiveActorState(sssg->GetFlatNodes()[i]) && (UporDownStatelessNode(sssg->GetFlatNodes()[i]) != 3))
			{
				GPUNodes.push_back(sssg->GetFlatNodes()[i]);
				sssg->GetFlatNodes()[i]->BorderFlag = false;
				sssg->GetFlatNodes()[i]->GPUPart = 0;
			}
			else
			{
				sssg->GetFlatNodes()[i]->GPUPart = this->mnparts;
				sssg->GetFlatNodes()[i]->BorderFlag = false;
			}
		}

		//��ʼ��
		vector<int>EachPartComputing(this->mnparts,0);//��̬��������,���ֵ�ÿ��GPU�ϵ����нڵ���ܼ�����
		vector<int>NodeComputingValue(GPUNodes.size(),0); //ÿ���ڵ���GPU�µļ�����
		vector<int>EachPartComm(this->mnparts,0); //ÿ��GPU��ͨ����
		TotalComputing = 0;
		PartEdgeValue = 0;
		map<FlatNode *, int>::iterator iter2;
		map<FlatNode *, int> tmp = sssg->GetSteadyWorkMap();

		//����GPU�ܵļ�����
		int i = 0;
		vector<FlatNode*>::iterator iter, iter1;
		for (iter = GPUNodes.begin(); iter != GPUNodes.end(); ++iter)
		{
			if ((*iter)->nOut != 0)
			{
				for (int j = 0; j < (*iter)->nOut; j++)
				{
					NodeComputingValue[i] += sssg->GetSteadyCount((*iter)) * (*iter)->outPushWeights[j];
				}
				/*iter2 = tmp.find(*iter);
				NodeComputingValue[i] += sssg->GetSteadyCount((*iter)) * iter2->second;*/
				i++;
			}
		}
		for (int i = 0; i < GPUNodes.size(); i++)
		{
			TotalComputing += NodeComputingValue[i];
		}
		AvgComputing = TotalComputing / this->mnparts;

		//��GPU�ڵ㻮�ֳ�Nplaces��
		int parts = 0; //��ע����
		for (int i = 0; i < GPUNodes.size(); i++)
		{
			if (parts == (this->mnparts - 1))
			{
				GetGPUNodes()[i]->GPUPart = parts;
			}
			else
			{
				EachPartComputing[parts] += NodeComputingValue[i];
				if (EachPartComputing[parts] > AvgComputing)
				{
					EachPartComputing[parts++] -= NodeComputingValue[i];
					EachPartComputing[parts] += NodeComputingValue[i];
					GetGPUNodes()[i]->GPUPart = parts;
				}
				else
				{
					GetGPUNodes()[i]->GPUPart = parts;
				}
			}
			
		}

		//�Գ�ʼ������ϸ΢������Ȼ���ٽ�һ�����߽��ĵ���
		for (iter1 = GPUNodes.begin(); iter1 != GPUNodes.end(); ++iter1)
		{
			if (UporDownStatelessNode(*iter1) == 1) //���ڵ�ȫΪstateful�ڵ㣬������Ϊ�߽�㣬���ֵ�����GPU
			{
				if ((*iter1)->GPUPart != (*iter1)->outFlatNodes[0]->GPUPart)
				{
					(*iter1)->GPUPart = (*iter1)->outFlatNodes[0]->GPUPart;
				}
			}
			else if (UporDownStatelessNode(*iter1) == 2)//�ӽڵ�ȫΪstateful�ڵ�
			{
				if ((*iter1)->GPUPart !=(*iter1)->inFlatNodes[(*iter1)->nIn - 1]->GPUPart)
				{
					(*iter1)->GPUPart =(*iter1)->inFlatNodes[(*iter1)->nIn - 1]->GPUPart;
				}
			}
		}

		//��ע�߽�ڵ�
		for(iter = sssg->flatNodes.begin(); iter != sssg->flatNodes.end(); ++iter)
		{
			if ((*iter)->nOut != 0)
			{
				for (iter1 = (*iter)->outFlatNodes.begin(); iter1 != (*iter)->outFlatNodes.end(); ++iter1)
				{
					if ((*iter)->GPUPart != (*iter1)->GPUPart)
					{
						(*iter)->BorderFlag = true;
						(*iter1)->BorderFlag =true;
					}
				}
			}
		}	
		
		//�����ֵ
		for (iter = sssg->flatNodes.begin(); iter != sssg->flatNodes.end(); ++iter)
		{
			if ((*iter)->nOut != 0 && (*iter)->BorderFlag == true)
			{
				int j = 0;
				for (iter1 = (*iter)->outFlatNodes.begin(); iter1 != (*iter)->outFlatNodes.end(); ++iter1)
				{
					if ((*iter1)->BorderFlag == true && (*iter)->GPUPart != (*iter1)->GPUPart)
					{
						if (((*iter)->GPUPart == this->mnparts && (*iter1)->GPUPart != this->mnparts) || ((*iter)->GPUPart != this->mnparts && (*iter1)->GPUPart == this->mnparts))
						{
							PartEdgeValue += (*iter)->outPushWeights[j] * sssg->GetSteadyCount((*iter));
						}
						else if ((*iter)->GPUPart != this->mnparts && (*iter1)->GPUPart != this->mnparts)
						{
							PartEdgeValue += (*iter)->outPushWeights[j] * sssg->GetSteadyCount((*iter)) * 2;
						}
					}
					j++;
				}
			}
		}
		
		//�����߽�ڵ㣬ʹͨ��������
		int TempEdgeValue;  //�ݴ��ֵ
		for (int i = 0; i <= this->mnparts; i++)
		{
			if(i == this->mnparts)
			for (iter1 = GPUNodes.begin(); iter1 != GPUNodes.end(); ++iter1)
			{
				if ((*iter1)->BorderFlag == true && (*iter1)->GPUPart != i && JudgeNextBorder((*iter1),i))
				{
					TempEdgeValue = GetChangedValue(*iter1,sssg,PartEdgeValue,i);
					if (TempEdgeValue < PartEdgeValue)
					{
						PartEdgeValue = TempEdgeValue;
						AlterBorder(*iter1,i);
					}
				}			
			}	
		}

		//�õ����ջ��ֽ��
		for (int i=0;i<nvtxs;i++)
		{
			FlatNode2PartitionNum.insert(make_pair(sssg->GetFlatNodes()[i],sssg->GetFlatNodes()[i]->GPUPart));//�����ڵ㵽���ֱ�ŵ�ӳ��
			PartitonNum2FlatNode.insert(make_pair(sssg->GetFlatNodes()[i]->GPUPart,sssg->GetFlatNodes()[i]));
		}
		for (int i=0;i<nvtxs;i++)
		{
			cout<<sssg->GetFlatNodes()[i]->name<<" "<<FlatNode2PartitionNum[sssg->GetFlatNodes()[i]]<<endl;
		}

#if 1 //��ӡͼ
		DumpStreamGraph(sssg,this,"BSPartitionGraph.dot",NULL);
#endif
	}
}

int GPUClusterPartition::UporDownStatelessNode(FlatNode *node)
{
	int marks=0,marks1 = 0,marks2 = 0;
	vector<FlatNode*>::iterator iter1;
	for (iter1 = node->inFlatNodes.begin(); iter1 != node ->inFlatNodes.end(); ++iter1)
	{
		if (!DetectiveActorState(*iter1))
		{
			marks1 = 0;
			break;
		}
		else
			marks1 = 1;
	}
	for (iter1 = node->outFlatNodes.begin(); iter1 != node->outFlatNodes.end(); ++iter1)
	{
		if (!DetectiveActorState(*iter1))
		{
			marks2 = 0;
			break;
		}
		else
			marks2 = 2;
	}
	if(marks1 != 0 && marks2 == 0)marks = marks1;
	else if (marks1 == 0 && marks2 != 0)
	{
		marks = marks2;
	}
	else if (marks1 != 0 && marks2 != 0)
	{
		marks = 3;
	}
	return marks;
}

bool GPUClusterPartition::JudgeNextBorder(FlatNode *node,int marks)
{
	bool flag = false;
	vector<FlatNode*>::iterator iter;
	for (iter = node->inFlatNodes.begin(); iter != node->inFlatNodes.end(); ++iter)
	{
		if ((*iter)->BorderFlag == true && (*iter)->GPUPart == marks)
		{
			flag = true;
			break;
		}
	}
	for (iter = node ->outFlatNodes.begin(); iter != node->outFlatNodes.end(); ++iter)
	{
		if ((*iter)->BorderFlag == true && (*iter)->GPUPart == marks)
		{
			flag = true;
			break;
		}
	}
	return flag;
}

int GPUClusterPartition::FindOutpushNum(FlatNode *node1,FlatNode *node2)
{
	int num = 0;
	vector<FlatNode*>::iterator iter;
	for (iter = node1->outFlatNodes.begin(); iter != node1->outFlatNodes.end(); ++iter)
	{
		if ((*iter) == node2)
			break;
		else
			num++;
	}
	return num;
}

int GPUClusterPartition::GetChangedValue(FlatNode *node,SchedulerSSG * sssg,int partvalue,int marks)
{
	int changevalue = partvalue;
	vector<FlatNode*>::iterator iter1;
	for (iter1 = node->inFlatNodes.begin(); iter1 != node->inFlatNodes.end(); ++iter1)
	{
		if ((*iter1)->GPUPart == node->GPUPart)
		{
			if (marks == this->mnparts)
			{
				changevalue += (*iter1)->outPushWeights[FindOutpushNum(*iter1,node)] * sssg->GetSteadyCount(*iter1);
			}
			else
			    changevalue += (*iter1)->outPushWeights[FindOutpushNum(*iter1,node)] * sssg->GetSteadyCount(*iter1) * 2;
		}
		else if ((*iter1)->GPUPart == marks)
		{
			if (marks == this->mnparts)
			{
				changevalue -= (*iter1)->outPushWeights[FindOutpushNum(*iter1,node)] * sssg->GetSteadyCount(*iter1);
			}
			else
				changevalue -= (*iter1)->outPushWeights[FindOutpushNum(*iter1,node)] * sssg->GetSteadyCount(*iter1) * 2;
		}
	}
	for (iter1 = node->outFlatNodes.begin(); iter1 != node->outFlatNodes.end(); ++iter1)
	{
		if ((*iter1)->GPUPart == node->GPUPart)
		{
			if (marks == this->mnparts)
			{
				changevalue += node->outPushWeights[FindOutpushNum(node,*iter1)] * sssg->GetSteadyCount(node);
			}
			else
				changevalue += node->outPushWeights[FindOutpushNum(node,*iter1)] * sssg->GetSteadyCount(node) * 2;
		}
		else if ((*iter1)->GPUPart == marks)
		{
			if (marks == this->mnparts)
			{
				changevalue -= node->outPushWeights[FindOutpushNum(node,*iter1)] * sssg->GetSteadyCount(node);
			}
			else
				changevalue -= node->outPushWeights[FindOutpushNum(node,*iter1)] * sssg->GetSteadyCount(node) * 2;
		}
	}
	return changevalue;
}

void GPUClusterPartition::AlterBorder(FlatNode *node,int marks)
{
	vector<FlatNode*>::iterator iter1,iter2;
	for (iter1 = node->inFlatNodes.begin(); iter1 != node->inFlatNodes.end(); ++iter1)
	{
		if ((*iter1)->GPUPart == node->GPUPart) 
		{
			(*iter1)->BorderFlag = true;
		}
		else if ((*iter1)->GPUPart == marks)
		{
			bool flag = false;
			for (iter2 = (*iter1)->inFlatNodes.begin(); iter2 != (*iter1)->inFlatNodes.end(); ++iter2)
			{
				if ((*iter2)->BorderFlag)
				{
					flag = true;
					break;
				}
			}
			if (!flag)
			{
				(*iter1)->BorderFlag == false;
			}
		}
	}
	for (iter1 = node->outFlatNodes.begin(); iter1 != node->outFlatNodes.end(); ++iter1)
	{
		if ( (*iter1)->GPUPart == node->GPUPart)
		{
			(*iter1)->BorderFlag == true;
		}
		else if ((*iter1)->GPUPart == marks)
		{
			bool flag = false;
			for (iter2 = (*iter1)->outFlatNodes.begin(); iter2 != (*iter1)->outFlatNodes.end(); ++iter2)
			{
				if ((*iter2)->BorderFlag)
				{
					flag = true;
					break;
				}
			}
			if (!flag)
			{
				(*iter1)->BorderFlag == false;
			}	
		}
	}
	node->GPUPart = marks;
}