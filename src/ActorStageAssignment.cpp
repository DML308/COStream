#include "ActorStageAssignment.h"
void StageAssignment::actorTopologicalorder(vector<FlatNode *>original)
{
	vector<FlatNode *>::iterator iter1,iter2,iter4;
	vector<FlatNode *>::iterator iter;
	vector<int> nInSet;//用于保存各节点的nIn
	vector<int>::iterator iter3;
	int nsize=original.size();
	int flag;
	for (iter1=original.begin();iter1!=original.end();++iter1)
	{
		nInSet.push_back((*iter1)->nIn);//将各节点的nIn保存
	}
	while (nsize)
	{
		for(iter1=original.begin();iter1!=original.end();++iter1)
		{
			if ((*iter1)->nIn==0)
			{
				flag=0;
				for (iter4=actortopo.begin();iter4!=actortopo.end();++iter4)
				{
					if ((*iter4)->name==(*iter1)->name)
					{
						flag=1;//如果结点已经存在于拓扑集合中
					}
				}
				if (flag==0)
				{
					for (iter2=(*iter1)->outFlatNodes.begin();iter2!=(*iter1)->outFlatNodes.end();++iter2)
					{
						(*iter2)->nIn--;
					}
					actortopo.push_back(*iter1);//将输入边为0的结点放入actortopo中
				//original.erase(iter1);//删除原始结点集合中的拓扑结点
					nsize--;
				}
				
			}
		}
	}
	
	for (iter1=original.begin(),iter3=nInSet.begin();iter1!=original.end()&&iter3!=nInSet.end();++iter1)
	{
		(*iter1)->nIn=(*iter3);
		iter3++;
	}
	//actortopo=original;
	/*cout<<"拓扑序列如下"<<endl;
	for (iter=actortopo.begin();iter!=actortopo.end();++iter)
	{
		cout<<(*iter)->name<<endl;
	}*/	
}


void StageAssignment::actorStageMap(map<FlatNode *,int>processor2actor)
{
	int maxstage,stage;
	bool flag;
	vector<FlatNode *>::iterator iter1,iter2;
	map<FlatNode *,int>::iterator iter3,iter4,iter5;
	map<FlatNode *,int>::iterator iter;
	for (iter1=actortopo.begin();iter1!=actortopo.end();++iter1)
	{
		maxstage=0;
		flag=false;
		for (iter2=(*iter1)->inFlatNodes.begin();iter2!=(*iter1)->inFlatNodes.end();++iter2)
		{
			iter=Actor2Stage.find(*iter2);
			if ((iter->second)>=maxstage)
			{
				maxstage=iter->second;
			}
			iter3=processor2actor.find(*iter1);//查找*iter1所对应的划分号
			iter4=processor2actor.find(*iter2);//查找*iter2所对应的划分号
			if (iter3->second!=iter4->second)
			{
				flag=true;
			}
		}
		if (flag)
		{
			stage=maxstage+1;
		}
		else
		{
			stage=maxstage;
		}
		Actor2Stage.insert(make_pair(*iter1,stage));
		Stage2Actor.insert(make_pair(stage,*iter1));
	}
	/*cout<<"阶段赋值结果如下"<<endl;
	for (iter5=Actor2Stage.begin();iter5!=Actor2Stage.end();++iter5)
	{
		cout<<iter5->first->name<<"     "<<iter5->second<<endl;
	}*/
}
void StageAssignment::actorStageMapForGPU(map<FlatNode *,int>processor2actor)
{
	int maxstage,stage;
	int flag;
	vector<FlatNode *>::iterator iter1,iter2;
	map<FlatNode *,int>::iterator iter3,iter4,iter5;
	map<FlatNode *,int>::iterator iter;
	FlatNode *tempactor;
	for (iter1=actortopo.begin();iter1!=actortopo.end();++iter1)
	{
		maxstage=0;
		flag=0;
		for (iter2=(*iter1)->inFlatNodes.begin();iter2!=(*iter1)->inFlatNodes.end();++iter2)
		{
			iter=Actor2Stage.find(*iter2);
			if ((iter->second)>=maxstage)
			{
				maxstage=iter->second;
			}
			iter3=processor2actor.find(*iter1);//查找*iter1所对应的划分号
			iter4=processor2actor.find(*iter2);//查找*iter2所对应的划分号
		    if ((iter3->second!=iter4->second)&&((*iter1)->GPUPart >= GpuNum || (*iter2)->GPUPart >= GpuNum))
			{
				flag = 1;
				tempactor = *iter2;
			}
		}
		if (flag == 0)
		{
			stage = maxstage;
		}
		else if(flag == 1)
		{
			stage = maxstage + 2;
			map<FlatNode*,int>tempmap,testmap;
			map<FlatNode*,int>::iterator iter_tempmap,iter_testmap;
			multimap<int,map<FlatNode*,int> >::iterator iter_datastage;
			if((*iter1)->GPUPart >= GpuNum && tempactor->GPUPart < GpuNum)
			{
				tempmap.clear();
				DataOfActor2Stage.insert(make_pair(tempactor,maxstage+1));
				Stage2DataOfActor.insert(make_pair(maxstage+1,tempactor));
				tempmap.insert(make_pair(tempactor,2));
				iter_datastage = datastage.find(maxstage+1);
				//测试tempmap和testmap是否相同
				iter_tempmap = tempmap.begin();
				if (iter_datastage!=datastage.end())
				{
					testmap = iter_datastage->second;
					iter_testmap = testmap.begin();
					if ((iter_tempmap->first->name == iter_testmap->first->name)&&(iter_tempmap->second == iter_testmap->second))
					{
						//do nothing
					}
					else
					{
						pair<multimap<int,map<FlatNode*,int> >::iterator , multimap<int,map<FlatNode*,int> >::iterator> pos = datastage.equal_range(maxstage+1);
						if (pos.first == pos.second)
						{
							datastage.insert(make_pair(maxstage+1,tempmap));
						}
						else
						{
							map<FlatNode*,int>::iterator iterofmap;
							bool IsInserted = false;
							while (pos.first!=pos.second)
							{
								for (iterofmap = pos.first->second.begin();iterofmap != pos.first->second.end();++iterofmap)
								{
									if (iterofmap->first == tempmap.begin()->first)
									{
										IsInserted = true;
									}
								}
								pos.first++;
							}
							if (IsInserted == false)
							{
								datastage.insert(make_pair(maxstage+1,tempmap));
							}
						}
					}	
				}
				else
				{
					pair<multimap<int,map<FlatNode*,int> >::iterator,multimap<int,map<FlatNode*,int> >::iterator> pos = datastage.equal_range(maxstage+1);
					if (pos.first == pos.second)
					{
						datastage.insert(make_pair(maxstage+1,tempmap));
					}
					else
					{
						map<FlatNode*,int>::iterator iterofmap;
						bool IsInserted = false;
						while (pos.first!=pos.second)
						{
							for (iterofmap = pos.first->second.begin();iterofmap != pos.first->second.end();++iterofmap)
							{
								if (iterofmap->first == tempmap.begin()->first)
								{
									IsInserted = true;
								}
							}
							pos.first++;
						}
						if (IsInserted == false)
						{
							datastage.insert(make_pair(maxstage+1,tempmap));
						}
					}
				}
			}
			else if((*iter1)->GPUPart < GpuNum && tempactor->GPUPart >= GpuNum)
			{
				tempmap.clear();
				DataOfActor2Stage.insert(make_pair(*iter1,maxstage+1));
				Stage2DataOfActor.insert(make_pair(maxstage+1,*iter1));
				tempmap.insert(make_pair(*iter1,1));
				iter_datastage = datastage.find(maxstage+1);
				//测试tempmap和testmap是否相同
				iter_tempmap = tempmap.begin();
				if (iter_datastage!=datastage.end())
				{
					testmap = iter_datastage->second;
					iter_testmap = testmap.begin();
					if ((iter_tempmap->first->name == iter_testmap->first->name)&&(iter_tempmap->second == iter_testmap->second))
					{
						//do nothing
					}
					else
					{
						pair<multimap<int,map<FlatNode*,int> >::iterator,multimap<int,map<FlatNode*,int> >::iterator> pos = datastage.equal_range(maxstage+1);
						if (pos.first == pos.second)
						{
							datastage.insert(make_pair(maxstage+1,tempmap));
						}
						else
						{
							map<FlatNode*,int>::iterator iterofmap;
							bool IsInserted = false;
							while (pos.first!=pos.second)
							{
								for (iterofmap = pos.first->second.begin();iterofmap != pos.first->second.end();++iterofmap)
								{
									if (iterofmap->first == tempmap.begin()->first)
									{
										IsInserted = true;
									}
								}
								pos.first++;
							}
							if (IsInserted == false)
							{
								datastage.insert(make_pair(maxstage+1,tempmap));
							}
						}
					}	
				}
				else
				{
					pair<multimap<int,map<FlatNode*,int> >::iterator,multimap<int,map<FlatNode*,int> >::iterator> pos = datastage.equal_range(maxstage+1);
					if (pos.first == pos.second)
					{
						datastage.insert(make_pair(maxstage+1,tempmap));
					}
					else
					{
						map<FlatNode*,int>::iterator iterofmap;
						bool IsInserted = false;
						while (pos.first!=pos.second)
						{
							for (iterofmap = pos.first->second.begin();iterofmap != pos.first->second.end();++iterofmap)
							{
								if (iterofmap->first == tempmap.begin()->first)
								{
									IsInserted = true;
								}
							}
							pos.first++;
						}
						if (IsInserted == false)
						{
							datastage.insert(make_pair(maxstage+1,tempmap));
						}
					}
				}
			}
		}
		Actor2Stage.insert(make_pair(*iter1,stage));
		Stage2Actor.insert(make_pair(stage,*iter1));
	}
	/*cout<<"阶段赋值结果如下"<<endl;
	for (iter5=Actor2Stage.begin();iter5!=Actor2Stage.end();++iter5)
	{
		cout<<iter5->first->name<<"     "<<iter5->second<<endl;
	}*/
}

int StageAssignment::FindStage(FlatNode* actor)
{
	map<FlatNode*,int>::iterator iter=Actor2Stage.find(actor);
	if (iter!=Actor2Stage.end())
	{
		return iter->second;
	}
	else return -1;
}
vector<FlatNode*> StageAssignment::FindActor(int stage)
{
	vector<FlatNode*> flatVec;
	pair<multimap<int,FlatNode*>::iterator,multimap<int,FlatNode*>::iterator> pos=Stage2Actor.equal_range(stage);
	while (pos.first!=pos.second)
	{
		flatVec.push_back(pos.first->second);
		ActorSet.push_back(pos.first->second);
		++pos.first;
	}
	return flatVec;
}

vector<FlatNode*> StageAssignment::FindDataOfActor(int stage)
{
	vector<FlatNode*> flatVec;
	vector<FlatNode*>::iterator iter;
	pair<multimap<int,FlatNode*>::iterator,multimap<int,FlatNode*>::iterator> pos=Stage2DataOfActor.equal_range(stage);
	while (pos.first!=pos.second)
	{
		bool IsFind = false;
		for (iter = flatVec.begin();iter != flatVec.end();++iter)
		{
			if ((*iter)->name == pos.first->second->name)
			{
				IsFind = true;
				break;
			}
		}
		if (!IsFind)
		{
			flatVec.push_back(pos.first->second);
			ActorSet.push_back(pos.first->second);
		}
		++pos.first;
	}
	return flatVec;
}
int StageAssignment::MaxStageNum()
{
	multimap<int,FlatNode*>::iterator iter1=Stage2Actor.end();
	iter1--;
	return iter1->first+1;
}
int StageAssignment::MaxStageNumForGPU()
{
	multimap<int,FlatNode*>::iterator iter1=Stage2Actor.end();
	iter1--;
	/*cout<<Stage2DataOfActor.size()<<endl;*/
	if (Stage2DataOfActor.size())
	{
		multimap<int,FlatNode*>::iterator iter2=Stage2DataOfActor.end();
		iter2--;
		return (iter1->first+1)>(iter2->first+1)?(iter1->first+1):(iter2->first+1);
	}
	else
	{
		return iter1->first+1;
	}
}
//map<FlatNode *,int> StageAssignment::ReturnDataOfActor2Stage()
//{
//	return DataOfActor2Stage;
//}
