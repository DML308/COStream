#include "DNBPartition.h"

DNBPartition::DNBPartition(HAFLPartition *haflp,SchedulerSSG *sssg)
{
	Cpu2GpuVelocity = 8000*1024;  //cpu��GPU��������8000MB/s,��λΪkB/s
	CpuFrequency = 2400000000;  //cpu����Ƶ��2.4GHz
	GpuFrequency = 1150000000;  //gpu����Ƶ��1.15GHz
	PENumber = 32;    //GPU  ÿ��CU��PE����
	CUNumber = 14;   //GPU  CU����
	MaxCputhreadNum = 8-2*GpuNum;
	CPUTotalWork = 0;
	GPUTotalWork = 0;
	CpuGpuTotalComm = 0;
	Sssg = sssg;
	pEdgeInfo = new ActorEdgeInfo(Sssg);
	CPUNodes = haflp->CPUNodes;
	GPUNodes = haflp->GPUNodes;
	MultiNum2FlatNode = haflp->MultiNum2FlatNode;
	FlatNode2PartitionNum = haflp->GetFlatNode2PartitionNum();
}

void DNBPartition::DiscreteNodePartition()
{
	//�ֱ�õ�cpu�ˣ�gpu��actor�ܹ�����
	double CpuTime = 0,GpuTime = 0,CommTime = 0; //��λms
	vector<FlatNode*>::iterator iter;
	for (iter = CPUNodes.begin(); iter != CPUNodes.end(); iter++)
		CPUTotalWork = CPUTotalWork + Sssg->GetInitWork(*iter)*Sssg->GetInitCount(*iter) + Sssg->GetSteadyWork(*iter)*Sssg->GetSteadyCount(*iter)*MultiNum2FlatNode[*iter];
	CpuTime = CPUTotalWork/CpuFrequency*1000;

	for (iter = GPUNodes.begin(); iter != GPUNodes.end(); iter++)
	{
		int count = Sssg->GetSteadyCount(*iter)*MultiNum2FlatNode[*iter];
		if(count%(PENumber*CUNumber) != 0)
			count = count/(PENumber*CUNumber) + 1;
		else
			count = count/(PENumber*CUNumber);
		GPUTotalWork = GPUTotalWork + Sssg->GetInitWork(*iter)*Sssg->GetInitCount(*iter) + Sssg->GetSteadyWork(*iter)*Sssg->GetSteadyCount(*iter)*count;
	}
	GPUTotalWork = CpuFrequency/GpuFrequency*GPUTotalWork;
	GpuTime = GPUTotalWork/GpuFrequency*1000;

	for (iter = CPUNodes.begin(); iter != CPUNodes.end(); iter++)
	{
		for (int i = 0; i < (*iter)->inFlatNodes.size(); i++)
		{
			if((*iter)->inFlatNodes[i]->GPUPart < GpuNum)
			{
				if(pEdgeInfo->getEdgeInfo((*iter)->inFlatNodes[i],(*iter)).typeName == "int_x")
					CpuGpuTotalComm += (*iter)->inPopWeights[i]*Sssg->GetSteadyCount(*iter)*MultiNum2FlatNode[(*iter)->inFlatNodes[i]]*sizeof(int)/1024;
				else if(pEdgeInfo->getEdgeInfo((*iter)->inFlatNodes[i],(*iter)).typeName == "double_x")
					CpuGpuTotalComm += (*iter)->inPopWeights[i]*Sssg->GetSteadyCount(*iter)*MultiNum2FlatNode[(*iter)->inFlatNodes[i]]*sizeof(double)/1024;
			}
		}
		for (int i = 0; i < (*iter)->outFlatNodes.size(); i++)
		{
			if ((*iter)->outFlatNodes[i]->GPUPart < GpuNum)
			{
				if(pEdgeInfo->getEdgeInfo((*iter),(*iter)->outFlatNodes[i]).typeName == "int_x")
					CpuGpuTotalComm += (*iter)->outPushWeights[i]*Sssg->GetSteadyCount(*iter)*MultiNum2FlatNode[*iter]*sizeof(int)/1024;
				else if(pEdgeInfo->getEdgeInfo((*iter),(*iter)->outFlatNodes[i]).typeName == "double_x")
					CpuGpuTotalComm += (*iter)->outPushWeights[i]*Sssg->GetSteadyCount(*iter)*MultiNum2FlatNode[*iter]*sizeof(double)/1024;
			}
		}
	}
	CommTime = CpuGpuTotalComm/Cpu2GpuVelocity*1000;

	double MaxTime = (GpuTime > CommTime)?GpuTime:CommTime;
	if(CpuTime <= MaxTime)
	{
		NCpuThreads = 1;
		for (iter = CPUNodes.begin(); iter != CPUNodes.end(); iter++)
		{
			Cputhread2Actor.insert(make_pair(0,make_pair(*iter,(*iter)->name)));
			actorName2count.insert(make_pair((*iter)->name,make_pair(0,Sssg->GetSteadyCount(*iter)*MultiNum2FlatNode[*iter])));
		}
	}
	else
	{
		if (CPUNodes.size() == 1 && Sssg->GetSteadyCount(CPUNodes[0])*MultiNum2FlatNode[CPUNodes[0]] == 1)
		{
			NCpuThreads = 1;
			Cputhread2Actor.insert(make_pair(0,make_pair(*iter,(*iter)->name)));
			actorName2count.insert(make_pair((*iter)->name,make_pair(0,Sssg->GetSteadyCount(*iter)*MultiNum2FlatNode[*iter])));
		}
		else
		{
			double maxTotalWork = MaxTime*CpuFrequency/1000;
			NCpuThreads = (int)(CpuTime/MaxTime) + 1;
			if(NCpuThreads > MaxCputhreadNum)
			{
				NCpuThreads = MaxCputhreadNum;
				maxTotalWork = CPUTotalWork/NCpuThreads;
			}
			//��ʼ��ÿ��cputhread�Ĺ�����
			for(int i = 0; i < NCpuThreads; i++)
				CputhreadWorkValue.push_back(0);
			//���cpu�ڵ��е�stateless�ڵ�
			for (int i = 0; i < CPUNodes.size(); i++)
			{
				if(!DetectiveActorState(CPUNodes[i]))
					statelessNodes.push_back(CPUNodes[i]);
			}
			//����Խϴ��stateless�ڵ���з���,��֤ÿ��stateless�ڵ�ļ�������������maxTotalWork
			
			for (int i = 0; i < statelessNodes.size(); i++)
			{
				int tempWorkvalue = Sssg->GetInitWork(statelessNodes[i])*Sssg->GetInitCount(statelessNodes[i]) + Sssg->GetSteadyWork(statelessNodes[i])*Sssg->GetSteadyCount(statelessNodes[i])*MultiNum2FlatNode[statelessNodes[i]];
			    if (tempWorkvalue > maxTotalWork)
			    {
					FissionNodes.push_back(statelessNodes[i]);
					int count = tempWorkvalue/maxTotalWork + 1;
					int j;
					for (j = 0; j < count-1; j++)
					{
						FissionNodes2count.insert(make_pair(statelessNodes[i],make_pair(Sssg->GetSteadyCount(statelessNodes[i])*MultiNum2FlatNode[statelessNodes[i]]/count*j,Sssg->GetSteadyCount(statelessNodes[i])*MultiNum2FlatNode[statelessNodes[i]]/count)));
					}
					FissionNodes2count.insert(make_pair(statelessNodes[i],make_pair(Sssg->GetSteadyCount(statelessNodes[i])*MultiNum2FlatNode[statelessNodes[i]]/count*j,Sssg->GetSteadyCount(statelessNodes[i])*MultiNum2FlatNode[statelessNodes[i]]-Sssg->GetSteadyCount(statelessNodes[i])*MultiNum2FlatNode[statelessNodes[i]]/count*j)));
			    }
			}
			//���ѹ��Ľڵ���1��ʾ��δ������0��ʾ
			int i = 0,j = 0;
			while(i < CPUNodes.size() && j < FissionNodes.size())
			{
				if(CPUNodes[i] != FissionNodes[j])
				{
					actor2fission.insert(make_pair(CPUNodes[i],false));
					i++;
				}
				else
				{
					actor2fission.insert(make_pair(CPUNodes[i],true));
					i++;j++;
				}
			}
			while(i < CPUNodes.size())
			{
				actor2fission.insert(make_pair(CPUNodes[i],false));
				i++;
			}
			//����actor�������thread
			int index = 0;
			map<FlatNode*,bool>::iterator iter;
			typedef multimap<FlatNode*,pair<int,int> >::iterator fission_iter;
			for (iter = actor2fission.begin(); iter != actor2fission.end(); ++iter)
			{
				if (iter->second == false)
				{
					int tempWorkvalue = Sssg->GetInitWork(iter->first)*Sssg->GetInitCount(iter->first) + Sssg->GetSteadyWork(iter->first)*Sssg->GetSteadyCount(iter->first)*MultiNum2FlatNode[iter->first];
					if (index == NCpuThreads)
					{
						int tempNum = minThreadnumofWorkvalue();
						CputhreadWorkValue[tempNum] += tempWorkvalue;
						Cputhread2Actor.insert(make_pair(tempNum,make_pair(iter->first,iter->first->name)));
						actorName2count.insert(make_pair(iter->first->name,make_pair(0,Sssg->GetSteadyCount(iter->first)*MultiNum2FlatNode[iter->first])));
					}
					else
					{
						if(CputhreadWorkValue[index] == 0 || CputhreadWorkValue[index]+tempWorkvalue < maxTotalWork)
						{
							CputhreadWorkValue[index] += tempWorkvalue;
							Cputhread2Actor.insert(make_pair(index,make_pair(iter->first,iter->first->name)));
							actorName2count.insert(make_pair(iter->first->name,make_pair(0,Sssg->GetSteadyCount(iter->first)*MultiNum2FlatNode[iter->first])));
						}
						else
						{
							index++;
							if(index == NCpuThreads)
							{
								int tempNum = minThreadnumofWorkvalue();
								CputhreadWorkValue[tempNum] += tempWorkvalue;
								Cputhread2Actor.insert(make_pair(tempNum,make_pair(iter->first,iter->first->name)));
								actorName2count.insert(make_pair(iter->first->name,make_pair(0,Sssg->GetSteadyCount(iter->first)*MultiNum2FlatNode[iter->first])));
							}
							else
							{
								CputhreadWorkValue[index] += tempWorkvalue;
								Cputhread2Actor.insert(make_pair(index,make_pair(iter->first,iter->first->name)));
								actorName2count.insert(make_pair(iter->first->name,make_pair(0,Sssg->GetSteadyCount(iter->first)*MultiNum2FlatNode[iter->first])));
							}
						}
					}
				}
				else
				{
					pair<fission_iter,fission_iter> pos = FissionNodes2count.equal_range(iter->first);
					int fissionNum = 0;
					while(pos.first != pos.second)
					{
						int tempWorkvalue = Sssg->GetInitWork(iter->first)*Sssg->GetInitCount(iter->first) + Sssg->GetSteadyWork(iter->first)*pos.first->second.second;
						if (index == NCpuThreads)
						{
							int tempNum = minThreadnumofWorkvalue();
							CputhreadWorkValue[tempNum] += tempWorkvalue;
							//Cputhread2Actor.insert(make_pair(tempNum,make_pair(iter->first,make_pair(pos.first->second.first,pos.first->second.second))));
						    stringstream ss;
							string tempstr;
							ss<<iter->first->name<<"_"<<fissionNum;
							ss>>tempstr;
							Cputhread2Actor.insert(make_pair(tempNum,make_pair(iter->first,tempstr)));
							actorName2count.insert(make_pair(tempstr,make_pair(pos.first->second.first,pos.first->second.second)));
						}
						else
						{
							if(CputhreadWorkValue[index] == 0 || CputhreadWorkValue[index]+tempWorkvalue < maxTotalWork)
							{
								CputhreadWorkValue[index] += tempWorkvalue;
								//Cputhread2Actor.insert(make_pair(index,make_pair(iter->first,make_pair(pos.first->second.first,pos.first->second.second))));
								stringstream ss;
								string tempstr;
								ss<<iter->first->name<<"_"<<fissionNum;
								ss>>tempstr;
								Cputhread2Actor.insert(make_pair(index,make_pair(iter->first,tempstr)));
								actorName2count.insert(make_pair(tempstr,make_pair(pos.first->second.first,pos.first->second.second)));
							}
							else
							{
								index++;
								if(index == NCpuThreads)
								{
									int tempNum = minThreadnumofWorkvalue();
									CputhreadWorkValue[tempNum] += tempWorkvalue;
									//Cputhread2Actor.insert(make_pair(tempNum,make_pair(iter->first,make_pair(pos.first->second.first,pos.first->second.second))));
									stringstream ss;
									string tempstr;
									ss<<iter->first->name<<"_"<<fissionNum;
									ss>>tempstr;
									Cputhread2Actor.insert(make_pair(tempNum,make_pair(iter->first,tempstr)));
									actorName2count.insert(make_pair(tempstr,make_pair(pos.first->second.first,pos.first->second.second)));
								}
								else
								{
									CputhreadWorkValue[index] += tempWorkvalue;
									//Cputhread2Actor.insert(make_pair(index,make_pair(iter->first,make_pair(pos.first->second.first,pos.first->second.second))));
									stringstream ss;
									string tempstr;
									ss<<iter->first->name<<"_"<<fissionNum;
									ss>>tempstr;
									Cputhread2Actor.insert(make_pair(index,make_pair(iter->first,tempstr)));
									actorName2count.insert(make_pair(tempstr,make_pair(pos.first->second.first,pos.first->second.second)));
								}
							}
						}
						++pos.first;
						++fissionNum;
					}	
				}
			}
		}
	}
	//�õ�actor��Ӧ�Ļ��ֱ��
	typedef multimap<int,pair<FlatNode*,string> >::iterator cpuActoriter;
	for (int i = 0; i < NCpuThreads; i++)
	{
		pair<cpuActoriter,cpuActoriter> pos = Cputhread2Actor.equal_range(i);
		while(pos.first != pos.second)
		{
			FlatNode2PartitionNum[pos.first->second.first] += i;
			++pos.first;
		}
	}
}

int DNBPartition::minThreadnumofWorkvalue()
{
	double minvalue = CputhreadWorkValue[0];
	int index = 0;
	for (int i = 1; i < NCpuThreads; i++)
	{
		if(minvalue > CputhreadWorkValue[i])
		{
			minvalue = CputhreadWorkValue[i];
			index = i;
		}
	}
	return index;
}

