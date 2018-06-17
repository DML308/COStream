#include "staticStreamGraph.h"

using namespace std;

/*设置该composite名字*/
void StaticStreamGraph::SetName(const char *name)
{
	comName = name;
}
/*取出flatnode的工作量(稳态)*/
int StaticStreamGraph::GetSteadyWork(FlatNode *n)
{
	std::map<FlatNode *,int> ::iterator pos;
	pos = mapSteadyWork2FlatNode.find(n);

	if (pos == mapSteadyWork2FlatNode.end())
		return 0;
	else
		return pos->second;
}
/*取出flatnode的工作量(初态)*/
int StaticStreamGraph::GetInitWork(FlatNode *n)
{
	std::map<FlatNode *,int> ::iterator pos;
	pos = mapInitWork2FlatNode.find(n);

	if (pos == mapInitWork2FlatNode.end())
		return 0;
	else
		return pos->second;
}

/*重置ssg结点flatnodes内所有flatnode内的visttimes*/
void StaticStreamGraph::ResetFlatNodeVisitTimes()
{
	for (int i=0;i<flatNodes.size();i++)
	{
		this->flatNodes[i]->ResetVistTimes();
	}
}


/*重置ssg内flatnode结点内所有flatnode的name值,保持每个flatnode的name都是唯一*/
void StaticStreamGraph::ResetFlatNodeNames()
{
	for (int i=0;i<flatNodes.size();i++)
	{
		stringstream ss;
		string temp;
		ss<<i;
		ss>>temp;
		flatNodes[i]->name = flatNodes[i]->name + "_" + temp;
		//flatNodes[i]->PreName = flatNodes[i]->PreName + "_" + temp;
	}
}
bool StaticStreamGraph::IsUpBorder(FlatNode *actor)
{
	bool flag = false;   //cwb 如果是cpu与gpu的边界结点
	vector<FlatNode *>::iterator iter1;
	if (actor->inFlatNodes.size() != 0)
	{
		for (iter1 = actor->inFlatNodes.begin(); iter1 != actor->inFlatNodes.end(); ++iter1)
		{
			if(actor->GPUPart != (*iter1)->GPUPart)
			{
				flag = true;
				break;
			}
		}
	}
	return flag;
}

bool StaticStreamGraph::IsDownBorder(FlatNode *actor)
{
	bool flag = false;   //cwb 如果是cpu与gpu的边界结点
	vector<FlatNode *>::iterator iter1;
	if (actor->outFlatNodes.size() != 0)
	{
		for (iter1 = actor->outFlatNodes.begin(); iter1 != actor->outFlatNodes.end(); ++iter1)
		{
			if(actor->GPUPart != (*iter1)->GPUPart)
			{
				flag = true;
				break;
			}
		}
	}
	return flag;
}

void StaticStreamGraph::GetPreName()
{
	for (int i = 0; i < flatNodes.size(); i++)
	{
		flatNodes[i]->PreName = flatNodes[i]->name;
	}
}

void StaticStreamGraph::GenerateFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite)
{
	FlatNode *src = NULL, *dest = NULL;
	std::vector<FlatNode *> tmp;
	std::map<Node *, FlatNode *> ::iterator pos;
	ListMarker marker;
	Node *item = NULL, *type = NULL;

	assert(u && oldComposite);
	src = new FlatNode(u, oldComposite, newComposite);
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.outputs ); 
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		if (item->typ == Id)
			type = item->u.id.decl->u.decl.type;
		else 
			type = item->u.decl.type;

		mapEdge2UpFlatNode.insert(make_pair(type, src)); //将“有向边”与其“上”端operator绑定
	}
	
	AddFlatNode(src);

	dest = src; //边变了
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.inputs);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		type = item->u.id.decl->u.decl.type;
		mapEdge2DownFlatNode.insert(make_pair(type,dest));//将“有向边”与其“下”端operator绑定

		pos = mapEdge2UpFlatNode.find(type);
		assert(pos != mapEdge2UpFlatNode.end()); //确保每一条输入边都有operator
		src = pos->second;
		src->AddOutEdges(dest);
		dest->AddInEdges(src);
	}
}

FlatNode * StaticStreamGraph::InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,FlatNode *oldFlatNode)
{//20120917 zww 添加
	FlatNode *src = NULL, *dest = NULL;
	std::vector<FlatNode *> tmp;
	std::map<Node *, FlatNode *> ::iterator pos;
	ListMarker marker;
	Node *item = NULL, *type = NULL;
	FlatNode *newFlatNode = NULL;
	assert(u );
	newFlatNode = new FlatNode(u);
	src = newFlatNode;
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.outputs ); 
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		assert(item->typ == Id || item->typ == Decl);
		if (item->typ == Id)
			type = item->u.id.decl->u.decl.type;
		else 
			type = item->u.decl.type;
		assert(type);
		multimap<Node *, FlatNode *> ::iterator out_pos;
		int out_count = mapEdge2DownFlatNode.count(type);
		assert(out_count<=1);//保证一条边只对应一个下端节点
		out_pos = mapEdge2DownFlatNode.find(type);
		Bool flag = TRUE;//要插入的flatNode在他的下端已经存在，则不插入
		if(out_pos != mapEdge2DownFlatNode.end())
		{//表示找到
			FlatNode *buttomFlatNode = out_pos->second;
			int InIter =0;
			for (InIter = 0;InIter!= buttomFlatNode->inFlatNodes.size();InIter++)
			{
				if(buttomFlatNode->inFlatNodes[InIter] == oldFlatNode)break;
			}
			if(InIter< buttomFlatNode->inFlatNodes.size() && buttomFlatNode->inFlatNodes[InIter] == oldFlatNode) buttomFlatNode->inFlatNodes[InIter] = src;//找到,修改值(split),如果没找到什么也不做（主要是为了处理分裂后产生的join节点）	
		}
		mapEdge2UpFlatNode.insert(make_pair(type, src)); //将“有向边”与其“上”端operator绑定
	}
	AddFlatNode(src);

	dest = src; //边变了
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.inputs);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		type = item->u.id.decl->u.decl.type;
		assert(type);
		mapEdge2DownFlatNode.insert(make_pair(type,dest));//将“有向边”与其“下”端operator绑定

		pos = mapEdge2UpFlatNode.find(type);//找边的上端节点
		assert(pos != mapEdge2UpFlatNode.end()); //确保每一条输入边都有operator
		src = pos->second;
		/*如果变得上端节点中的outFlatNodes中已经有当前边对应的下端的FlatNode则不添加，只修改它的当前边对应的值*/
		int outIter =0;
		for (outIter = 0;outIter!= src->outFlatNodes.size();outIter++)
		{
			if(src->outFlatNodes[outIter] == oldFlatNode)break;
		}
		if(outIter< src->outFlatNodes.size()) 
			src->outFlatNodes[outIter] = dest;//找到,修改值(split)	
		else src->AddOutEdges(dest);//没找到，插入
		dest->AddInEdges(src);
	}
	return newFlatNode;
}

//************************************
// Method:    InsertFlatNodes
// FullName:  StaticStreamGraph::InsertFlatNodes
// Access:    public 
// Returns:   FlatNode *
// Qualifier: /*zww 20121011 添加 */
// Parameter: operatorNode * u  新建的operator，有待插入到flatNodes中
// Parameter: compositeNode * oldComposite （可以为空）
// Parameter: compositeNode * newComposite （可以为空）
// Parameter: std::set<FlatNode * > oldDownFlatNode——u的输入边来自于oldDownFlatNode的各个元素的输入边 （可以为空）
// Parameter: std::set<FlatNode * > oldUpFlatNode ——u的输出边来自于oldUpFlatNode的各个元素的输出边 （可以为空）
//************************************
FlatNode *StaticStreamGraph::InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,std::set<FlatNode *> oldFlatNodeSet)//zww 20121011 添加
{
	FlatNode *src = NULL, *dest = NULL;
	std::vector<FlatNode *> tmp;
	std::map<Node *, FlatNode *> ::iterator pos;
	ListMarker marker;
	Node *item = NULL, *type = NULL;
	FlatNode *newFlatNode = NULL;
	assert(u );
	newFlatNode = new FlatNode(u);
	src = newFlatNode;
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.outputs ); 
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		assert(item->typ == Id || item->typ == Decl);
		if (item->typ == Id)
			type = item->u.id.decl->u.decl.type;
		else 
			type = item->u.decl.type;
		assert(type);
		multimap<Node *, FlatNode *> ::iterator out_pos;
		int out_count = mapEdge2DownFlatNode.count(type);
		assert(out_count<=1);//保证一条边只对应一个下端节点
		out_pos = mapEdge2DownFlatNode.find(type);
		Bool flag = TRUE;//要插入的flatNode在他的下端已经存在，则不插入
		if(out_pos != mapEdge2DownFlatNode.end()) // u的下端节点已经存在了
		{//表示找到
					
			FlatNode *buttomFlatNode = out_pos->second;

			//如果newflatNode的输出边对应的下端节点已经在mapEdge2DownFlatNode中，则newflatNode的nOut++，并且它的outFlatNode要添加该下端节点
			//mapEdge2UpFlatNode.insert(make_pair(type,newFlatNode));
			newFlatNode->AddOutEdges(buttomFlatNode);

			int InIter =0;
			for (InIter = 0;InIter!= buttomFlatNode->inFlatNodes.size();InIter++)//u的下端节点的输入边对应的节点
			{
				if(oldFlatNodeSet.count(buttomFlatNode->inFlatNodes[InIter]))
				{
					//cout<<"下端  "<<buttomFlatNode->inFlatNodes[InIter]->name<<endl;
					break;
				}
			}
			if(InIter< buttomFlatNode->inFlatNodes.size() && oldFlatNodeSet.count(buttomFlatNode->inFlatNodes[InIter]))
			{
				buttomFlatNode->inFlatNodes[InIter] = src;//替换为新的flatNode节点
				//cout<<"替换  "<<src->name<<endl;
			}
		}
		
		mapEdge2UpFlatNode.insert(make_pair(type, src)); //将“有向边”与其“上”端operator绑定
	}
	AddFlatNode(src);

	dest = src; //边变了
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.inputs); //u的输入边
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		if (item->typ == Id)
			type = item->u.id.decl->u.decl.type;
		else 
			type = item->u.decl.type;
		assert(type);
		mapEdge2DownFlatNode.insert(make_pair(type,dest));//将“有向边”与其“下”端operator绑定

		pos = mapEdge2UpFlatNode.find(type);//找输入边的上端节点
		assert(pos != mapEdge2UpFlatNode.end()); //确保每一条输入边都有operator（输入边的上端节点必须已经存在）
		src = pos->second;//取输入边的上端节点

// 		//如果newflatNode的输入边对应的上端节点已经在mapEdge2UpFlatNode中，则newflatNode的nIn++，并且它的iinFlatNode要添加该下端节点
// 		mapEdge2UpFlatNode.insert(make_pair(type,buttomFlatNode));
// 		newFlatNode->AddOutEdges(buttomFlatNode);
		/*如果变得上端节点中的outFlatNodes中已经有当前边对应的下端的FlatNode则不添加，只修改它的当前边对应的值*/
		int outIter =0;
		for (outIter = 0;outIter!= src->outFlatNodes.size();outIter++)
		{
			if(oldFlatNodeSet.count(src->outFlatNodes[outIter]))
			{
				//cout<<"上端  "<<src->outFlatNodes[outIter]->name<<endl;
				break;
			}
		}
		if(outIter< src->outFlatNodes.size()) //上端节点的输出边对应的下端节点如果在oldDownFlatNode中则替换
		{
			src->outFlatNodes[outIter] = dest;//找到,修改值	
			//cout<<"替换  "<<dest->name<<endl;
		}
		else src->AddOutEdges(dest);//没找到，插入
		dest->AddInEdges(src);
	}
	return newFlatNode;
}

/*
1.若该operator没有输入，则FlatNode成员变量inPeekWeights，inPopWeights均为0
2.若该operator没有输出，则FlatNode成员变量inPeekWeights，inPopWeights根据4来定
3.若该operator有输出，有输入，则FlatNode成员变量inPeekWeights，inPopWeights根据4来定
4.一个window是和一条边对应的，inPeekWeights，inPopWeights，outPushWeights按如下方法计算
	(1)所有window的outPushWeights均用窗口标识ID和Tumbling关键字指定
	(2)若该window为sliding风格，则inPeekWeights，inPopWeights由第1,2个count值决定
	(3)若该window为tumbling风格，则inPeekWeights和inPopWeights相等均由第一个count决定
	(4)由于是静态计算，因此不对window的排空和触发策略为time类型做计算，只限于count类型的window策略
*/
void StaticStreamGraph::SetFlatNodesWeights()
 {
	std::map<Node *, FlatNode *>::iterator pos;
	std::multimap<Node *, FlatNode *>::iterator posMulti;
	char tmp[50];
	int i,j;

	for(i = 0; i < flatNodes.size(); i++)
	{
		FlatNode *flatNode = flatNodes[i];
		operatorNode *contents = flatNode->contents;
		List *windowList = contents->body->u.operBody.window;

		//先对所有的pop, peek, push初始化为1
		for(j = 0; j < flatNode->nIn; j++)
		{
			// 在inPeekWeights为空的情况下，不能使用数组下标（inPeekWeights[j]）访问，否则出现断言错误，以下类同
			(flatNode->inPeekWeights).push_back(1); 
			(flatNode->inPopWeights).push_back(1);
			sprintf(tmp, "nPeek_%d",j);
			(flatNode->inPeekString).push_back(tmp); 
			sprintf(tmp, "nPop_%d",j);
			(flatNode->inPopString).push_back(tmp);
		}
		for(j = 0; j < flatNode->nOut; j++)
		{
			(flatNode->outPushWeights).push_back(1); // 对于每一个FlatNode来说都为1，下同
			sprintf(tmp, "nPush_%d",j);
			(flatNode->outPushString).push_back(tmp);
		}
			
		// 处理window不为空的operator节点，为空则取上述默认值
		if( windowList != NULL) 
		{
			ListMarker marker;
			Node *item = NULL, *type = NULL;

			IterateList(&marker, windowList);
			while ( NextOnList(&marker, (GenericREF) &item) ) 
			{
				int current = 0;
				FlatNode *src = NULL, *dest = NULL;
				Node *val = NULL;
				List *aptr;

				type = item->u.window.id->u.id.decl->u.decl.type;
				pos = mapEdge2UpFlatNode.find(type);
				assert(pos != mapEdge2UpFlatNode.end()); 
				src = pos->second; // 每条边有且只有一个上端节点
				if(src != flatNode) // 说明该window指示的是peek和pop值
				{
					NodeType nodeType = item->u.window.wtype->typ;
					assert(nodeType == Sliding || nodeType == Tumbling);

					for(j = 0; src != flatNode->inFlatNodes[j] ; j++); //找到对应的j,准备写入值
					if(nodeType == Sliding) // sliding window
					{
						int tmp = 0;
						//取出sliding节点的值
						aptr = item->u.window.wtype->u.sliding.sliding_value->u.comma.exprs;
						Node *trigger = (Node *)FirstItem(aptr);
						aptr = Rest(aptr);
						Node *eviction = (Node *)FirstItem(aptr);

						val = GetValue(eviction);
						tmp = flatNode->inPopWeights[j] = val->u.Const.value.i;
						
						if(trigger) // 若peek部分不为空，则取值
						{
							val = GetValue(trigger);
							if (tmp > val->u.Const.value.i)
							{
								SyntaxError("peek must be greater than pop!\npeek = %d, pop = %d\n", val->u.Const.value.i, tmp);
								assert(FALSE);
							}
							int i = val->u.Const.value.i;
							flatNode->inPeekWeights[j] = val->u.Const.value.i;
							//flatNode->inPeekString[j] = val->u.Const.value.i;
						}
						else // peek部分为空，则peek == pop
						{
							flatNode->inPeekWeights[j] = val->u.Const.value.i;
							//flatNode->inPeekString[j] = val->u.Const.value.i;
						}
							
					}
					else // tumbling window
					{
						val = GetValue(item->u.window.wtype->u.tumbling.tumbling_value);
						flatNode->inPeekWeights[j] = val->u.Const.value.i;
						flatNode->inPopWeights[j] = flatNode->inPeekWeights[j];

//						flatNode->AddPopAtCodeGen[j] = 0;//zww:20120213,初始AddPopAtCodeGen
						
						//flatNode->inPeekString[j] = val->u.Const.value.i;
						//flatNode->inPopString[j] = flatNode->inPeekWeights[j];
					}
				}
				else // 说明该window指示的是push值
				{
					int total = mapEdge2DownFlatNode.count(type); // 该map为multimap，所以会有多个对应值，要逐一查找

					val = GetValue(item->u.window.wtype->u.tumbling.tumbling_value);
					posMulti = mapEdge2DownFlatNode.find(type);
					for(int k = 0; k < total ; k++)
					{
						dest = posMulti->second;
						for(j = 0; dest != flatNode->outFlatNodes[j]; j++);
						flatNode->outPushWeights[j] = val->u.Const.value.i;
						//flatNode->outPushString[j] = val->u.Const.value.i;
						posMulti++;
					}
				}
			}
		}
	}
}

void StaticStreamGraph::SetFlatNodesWeights(FlatNode *flatNode)
{
	std::map<Node *, FlatNode *>::iterator pos;
	std::multimap<Node *, FlatNode *>::iterator posMulti;
	char tmp[50];
	int i,j;
	operatorNode *contents = flatNode->contents;
	List *windowList = contents->body->u.operBody.window;

	//先对所有的pop, peek, push初始化为1
	for(j = 0; j < flatNode->nIn; j++)
	{
		// 在inPeekWeights为空的情况下，不能使用数组下标（inPeekWeights[j]）访问，否则出现断言错误，以下类同
		(flatNode->inPeekWeights).push_back(1); 
		(flatNode->inPopWeights).push_back(1);
		sprintf(tmp, "nPeek_%d",j);
		(flatNode->inPeekString).push_back(tmp); 
		sprintf(tmp, "nPop_%d",j);
		(flatNode->inPopString).push_back(tmp);
	}
	for(j = 0; j < flatNode->nOut; j++)
	{
		(flatNode->outPushWeights).push_back(1); // 对于每一个FlatNode来说都为1，下同
		sprintf(tmp, "nPush_%d",j);
		(flatNode->outPushString).push_back(tmp);
	}
		
	// 处理window不为空的operator节点，为空则取上述默认值
	if( windowList != NULL) 
	{
		ListMarker marker;
		Node *item = NULL, *type = NULL;

		IterateList(&marker, windowList);
		while ( NextOnList(&marker, (GenericREF) &item) ) 
		{
			int current = 0;
			FlatNode *src = NULL, *dest = NULL;
			Node *val = NULL;
			List *aptr;

			type = item->u.window.id->u.id.decl->u.decl.type;
			pos = mapEdge2UpFlatNode.find(type);
			assert(pos != mapEdge2UpFlatNode.end()); 
			src = pos->second; // 每条边有且只有一个上端节点
			if(src != flatNode) // 说明该window指示的是peek和pop值
			{
				NodeType nodeType = item->u.window.wtype->typ;
				assert(nodeType == Sliding || nodeType == Tumbling);

				for(j = 0; src != flatNode->inFlatNodes[j] ; j++); //找到对应的j,准备写入值
				if(nodeType == Sliding) // sliding window
				{
					int tmp = 0;
					//取出sliding节点的值
					aptr = item->u.window.wtype->u.sliding.sliding_value->u.comma.exprs;
					Node *trigger = (Node *)FirstItem(aptr);
					aptr = Rest(aptr);
					Node *eviction = (Node *)FirstItem(aptr);

					val = GetValue(item->u.window.wtype->u.sliding.sliding_value);
					tmp = flatNode->inPopWeights[j] = val->u.Const.value.i;
					
//					flatNode->AddPopAtCodeGen[j] = 0;//zww:20120213,初始AddPopAtCodeGen

					if(trigger) // 若peek部分不为空，则取值
					{
						val = GetValue(trigger);
						if (tmp > val->u.Const.value.i)
						{
							SyntaxError("peek must be greater than pop!\npeek = %d, pop = %d\n", val->u.Const.value.i, tmp);
							assert(FALSE);
						}
						flatNode->inPeekWeights[j] = val->u.Const.value.i;
						//flatNode->inPeekString[j] = val->u.Const.value.i;
					}
					else // peek部分为空，则peek == pop
					{
						flatNode->inPeekWeights[j] = val->u.Const.value.i;
						//flatNode->inPeekString[j] = val->u.Const.value.i;
					}
						
				}
				else // tumbling window
				{
					val = GetValue(item->u.window.wtype->u.tumbling.tumbling_value);
					flatNode->inPeekWeights[j] = val->u.Const.value.i;
					flatNode->inPopWeights[j] = flatNode->inPeekWeights[j];

//					flatNode->AddPopAtCodeGen[j] = 0;//zww:20120213,初始AddPopAtCodeGen

					//flatNode->inPeekString[j] = val->u.Const.value.i;
					//flatNode->inPopString[j] = flatNode->inPeekWeights[j];
				}
			}
			else // 说明该window指示的是push值
			{
				int total = mapEdge2DownFlatNode.count(type); // 该map为multimap，所以会有多个对应值，要逐一查找

				val = GetValue(item->u.window.wtype->u.tumbling.tumbling_value);
				posMulti = mapEdge2DownFlatNode.find(type);
				for(int k = 0; k < total ; k++)
				{
					dest = posMulti->second;
					for(j = 0; dest != flatNode->outFlatNodes[j]; j++);
					flatNode->outPushWeights[j] = val->u.Const.value.i;
					//flatNode->outPushString[j] = val->u.Const.value.i;
					posMulti++;
				}
			}
		}
	}

}

void StaticStreamGraph::AddSteadyWork(FlatNode *f,int w)
{
	mapSteadyWork2FlatNode.insert(make_pair(f,w));
}

void StaticStreamGraph::AddInitWork(FlatNode *f,int w)
{
	mapInitWork2FlatNode.insert(make_pair(f,w));
}

std::vector<FlatNode *> StaticStreamGraph::GetFlatNodes(int place_id)
{
	assert(place_id >= -1);
	if (place_id == -1)
		return flatNodes;
	else
	{
		tmpFlatNodes.clear();
		for (int i = 0; i < flatNodes.size(); i++)
		{
			if (flatNodes[i]->place_id == place_id)
			{
				tmpFlatNodes.push_back(flatNodes[i]);
			}
		}
		return tmpFlatNodes;
	}
}

std::vector<FlatNode *> StaticStreamGraph::GetFlatNodes(int place_id, int thread_id)
{
	assert(place_id >= 0 && thread_id >= 0);

	tmpFlatNodes.clear();
	for (int i = 0; i < flatNodes.size(); i++)
	{
		if (flatNodes[i]->place_id == place_id && flatNodes[i]->thread_id == thread_id)
		{
			tmpFlatNodes.push_back(flatNodes[i]);
		}
	}
	return tmpFlatNodes;
}