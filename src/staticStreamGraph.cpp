#include "staticStreamGraph.h"

using namespace std;

/*���ø�composite����*/
void StaticStreamGraph::SetName(const char *name)
{
	comName = name;
}
/*ȡ��flatnode�Ĺ�����(��̬)*/
int StaticStreamGraph::GetSteadyWork(FlatNode *n)
{
	std::map<FlatNode *,int> ::iterator pos;
	pos = mapSteadyWork2FlatNode.find(n);

	if (pos == mapSteadyWork2FlatNode.end())
		return 0;
	else
		return pos->second;
}
/*ȡ��flatnode�Ĺ�����(��̬)*/
int StaticStreamGraph::GetInitWork(FlatNode *n)
{
	std::map<FlatNode *,int> ::iterator pos;
	pos = mapInitWork2FlatNode.find(n);

	if (pos == mapInitWork2FlatNode.end())
		return 0;
	else
		return pos->second;
}

/*����ssg���flatnodes������flatnode�ڵ�visttimes*/
void StaticStreamGraph::ResetFlatNodeVisitTimes()
{
	for (int i=0;i<flatNodes.size();i++)
	{
		this->flatNodes[i]->ResetVistTimes();
	}
}


/*����ssg��flatnode���������flatnode��nameֵ,����ÿ��flatnode��name����Ψһ*/
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
	bool flag = false;   //cwb �����cpu��gpu�ı߽���
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
	bool flag = false;   //cwb �����cpu��gpu�ı߽���
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

		mapEdge2UpFlatNode.insert(make_pair(type, src)); //��������ߡ����䡰�ϡ���operator��
	}
	
	AddFlatNode(src);

	dest = src; //�߱���
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.inputs);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		type = item->u.id.decl->u.decl.type;
		mapEdge2DownFlatNode.insert(make_pair(type,dest));//��������ߡ����䡰�¡���operator��

		pos = mapEdge2UpFlatNode.find(type);
		assert(pos != mapEdge2UpFlatNode.end()); //ȷ��ÿһ������߶���operator
		src = pos->second;
		src->AddOutEdges(dest);
		dest->AddInEdges(src);
	}
}

FlatNode * StaticStreamGraph::InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,FlatNode *oldFlatNode)
{//20120917 zww ���
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
		assert(out_count<=1);//��֤һ����ֻ��Ӧһ���¶˽ڵ�
		out_pos = mapEdge2DownFlatNode.find(type);
		Bool flag = TRUE;//Ҫ�����flatNode�������¶��Ѿ����ڣ��򲻲���
		if(out_pos != mapEdge2DownFlatNode.end())
		{//��ʾ�ҵ�
			FlatNode *buttomFlatNode = out_pos->second;
			int InIter =0;
			for (InIter = 0;InIter!= buttomFlatNode->inFlatNodes.size();InIter++)
			{
				if(buttomFlatNode->inFlatNodes[InIter] == oldFlatNode)break;
			}
			if(InIter< buttomFlatNode->inFlatNodes.size() && buttomFlatNode->inFlatNodes[InIter] == oldFlatNode) buttomFlatNode->inFlatNodes[InIter] = src;//�ҵ�,�޸�ֵ(split),���û�ҵ�ʲôҲ��������Ҫ��Ϊ�˴�����Ѻ������join�ڵ㣩	
		}
		mapEdge2UpFlatNode.insert(make_pair(type, src)); //��������ߡ����䡰�ϡ���operator��
	}
	AddFlatNode(src);

	dest = src; //�߱���
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.inputs);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		type = item->u.id.decl->u.decl.type;
		assert(type);
		mapEdge2DownFlatNode.insert(make_pair(type,dest));//��������ߡ����䡰�¡���operator��

		pos = mapEdge2UpFlatNode.find(type);//�ұߵ��϶˽ڵ�
		assert(pos != mapEdge2UpFlatNode.end()); //ȷ��ÿһ������߶���operator
		src = pos->second;
		/*�������϶˽ڵ��е�outFlatNodes���Ѿ��е�ǰ�߶�Ӧ���¶˵�FlatNode����ӣ�ֻ�޸����ĵ�ǰ�߶�Ӧ��ֵ*/
		int outIter =0;
		for (outIter = 0;outIter!= src->outFlatNodes.size();outIter++)
		{
			if(src->outFlatNodes[outIter] == oldFlatNode)break;
		}
		if(outIter< src->outFlatNodes.size()) 
			src->outFlatNodes[outIter] = dest;//�ҵ�,�޸�ֵ(split)	
		else src->AddOutEdges(dest);//û�ҵ�������
		dest->AddInEdges(src);
	}
	return newFlatNode;
}

//************************************
// Method:    InsertFlatNodes
// FullName:  StaticStreamGraph::InsertFlatNodes
// Access:    public 
// Returns:   FlatNode *
// Qualifier: /*zww 20121011 ��� */
// Parameter: operatorNode * u  �½���operator���д����뵽flatNodes��
// Parameter: compositeNode * oldComposite ������Ϊ�գ�
// Parameter: compositeNode * newComposite ������Ϊ�գ�
// Parameter: std::set<FlatNode * > oldDownFlatNode����u�������������oldDownFlatNode�ĸ���Ԫ�ص������ ������Ϊ�գ�
// Parameter: std::set<FlatNode * > oldUpFlatNode ����u�������������oldUpFlatNode�ĸ���Ԫ�ص������ ������Ϊ�գ�
//************************************
FlatNode *StaticStreamGraph::InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,std::set<FlatNode *> oldFlatNodeSet)//zww 20121011 ���
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
		assert(out_count<=1);//��֤һ����ֻ��Ӧһ���¶˽ڵ�
		out_pos = mapEdge2DownFlatNode.find(type);
		Bool flag = TRUE;//Ҫ�����flatNode�������¶��Ѿ����ڣ��򲻲���
		if(out_pos != mapEdge2DownFlatNode.end()) // u���¶˽ڵ��Ѿ�������
		{//��ʾ�ҵ�
					
			FlatNode *buttomFlatNode = out_pos->second;

			//���newflatNode������߶�Ӧ���¶˽ڵ��Ѿ���mapEdge2DownFlatNode�У���newflatNode��nOut++����������outFlatNodeҪ��Ӹ��¶˽ڵ�
			//mapEdge2UpFlatNode.insert(make_pair(type,newFlatNode));
			newFlatNode->AddOutEdges(buttomFlatNode);

			int InIter =0;
			for (InIter = 0;InIter!= buttomFlatNode->inFlatNodes.size();InIter++)//u���¶˽ڵ������߶�Ӧ�Ľڵ�
			{
				if(oldFlatNodeSet.count(buttomFlatNode->inFlatNodes[InIter]))
				{
					//cout<<"�¶�  "<<buttomFlatNode->inFlatNodes[InIter]->name<<endl;
					break;
				}
			}
			if(InIter< buttomFlatNode->inFlatNodes.size() && oldFlatNodeSet.count(buttomFlatNode->inFlatNodes[InIter]))
			{
				buttomFlatNode->inFlatNodes[InIter] = src;//�滻Ϊ�µ�flatNode�ڵ�
				//cout<<"�滻  "<<src->name<<endl;
			}
		}
		
		mapEdge2UpFlatNode.insert(make_pair(type, src)); //��������ߡ����䡰�ϡ���operator��
	}
	AddFlatNode(src);

	dest = src; //�߱���
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.inputs); //u�������
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		if (item->typ == Id)
			type = item->u.id.decl->u.decl.type;
		else 
			type = item->u.decl.type;
		assert(type);
		mapEdge2DownFlatNode.insert(make_pair(type,dest));//��������ߡ����䡰�¡���operator��

		pos = mapEdge2UpFlatNode.find(type);//������ߵ��϶˽ڵ�
		assert(pos != mapEdge2UpFlatNode.end()); //ȷ��ÿһ������߶���operator������ߵ��϶˽ڵ�����Ѿ����ڣ�
		src = pos->second;//ȡ����ߵ��϶˽ڵ�

// 		//���newflatNode������߶�Ӧ���϶˽ڵ��Ѿ���mapEdge2UpFlatNode�У���newflatNode��nIn++����������iinFlatNodeҪ��Ӹ��¶˽ڵ�
// 		mapEdge2UpFlatNode.insert(make_pair(type,buttomFlatNode));
// 		newFlatNode->AddOutEdges(buttomFlatNode);
		/*�������϶˽ڵ��е�outFlatNodes���Ѿ��е�ǰ�߶�Ӧ���¶˵�FlatNode����ӣ�ֻ�޸����ĵ�ǰ�߶�Ӧ��ֵ*/
		int outIter =0;
		for (outIter = 0;outIter!= src->outFlatNodes.size();outIter++)
		{
			if(oldFlatNodeSet.count(src->outFlatNodes[outIter]))
			{
				//cout<<"�϶�  "<<src->outFlatNodes[outIter]->name<<endl;
				break;
			}
		}
		if(outIter< src->outFlatNodes.size()) //�϶˽ڵ������߶�Ӧ���¶˽ڵ������oldDownFlatNode�����滻
		{
			src->outFlatNodes[outIter] = dest;//�ҵ�,�޸�ֵ	
			//cout<<"�滻  "<<dest->name<<endl;
		}
		else src->AddOutEdges(dest);//û�ҵ�������
		dest->AddInEdges(src);
	}
	return newFlatNode;
}

/*
1.����operatorû�����룬��FlatNode��Ա����inPeekWeights��inPopWeights��Ϊ0
2.����operatorû���������FlatNode��Ա����inPeekWeights��inPopWeights����4����
3.����operator������������룬��FlatNode��Ա����inPeekWeights��inPopWeights����4����
4.һ��window�Ǻ�һ���߶�Ӧ�ģ�inPeekWeights��inPopWeights��outPushWeights�����·�������
	(1)����window��outPushWeights���ô��ڱ�ʶID��Tumbling�ؼ���ָ��
	(2)����windowΪsliding�����inPeekWeights��inPopWeights�ɵ�1,2��countֵ����
	(3)����windowΪtumbling�����inPeekWeights��inPopWeights��Ⱦ��ɵ�һ��count����
	(4)�����Ǿ�̬���㣬��˲���window���ſպʹ�������Ϊtime���������㣬ֻ����count���͵�window����
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

		//�ȶ����е�pop, peek, push��ʼ��Ϊ1
		for(j = 0; j < flatNode->nIn; j++)
		{
			// ��inPeekWeightsΪ�յ�����£�����ʹ�������±꣨inPeekWeights[j]�����ʣ�������ֶ��Դ���������ͬ
			(flatNode->inPeekWeights).push_back(1); 
			(flatNode->inPopWeights).push_back(1);
			sprintf(tmp, "nPeek_%d",j);
			(flatNode->inPeekString).push_back(tmp); 
			sprintf(tmp, "nPop_%d",j);
			(flatNode->inPopString).push_back(tmp);
		}
		for(j = 0; j < flatNode->nOut; j++)
		{
			(flatNode->outPushWeights).push_back(1); // ����ÿһ��FlatNode��˵��Ϊ1����ͬ
			sprintf(tmp, "nPush_%d",j);
			(flatNode->outPushString).push_back(tmp);
		}
			
		// ����window��Ϊ�յ�operator�ڵ㣬Ϊ����ȡ����Ĭ��ֵ
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
				src = pos->second; // ÿ��������ֻ��һ���϶˽ڵ�
				if(src != flatNode) // ˵����windowָʾ����peek��popֵ
				{
					NodeType nodeType = item->u.window.wtype->typ;
					assert(nodeType == Sliding || nodeType == Tumbling);

					for(j = 0; src != flatNode->inFlatNodes[j] ; j++); //�ҵ���Ӧ��j,׼��д��ֵ
					if(nodeType == Sliding) // sliding window
					{
						int tmp = 0;
						//ȡ��sliding�ڵ��ֵ
						aptr = item->u.window.wtype->u.sliding.sliding_value->u.comma.exprs;
						Node *trigger = (Node *)FirstItem(aptr);
						aptr = Rest(aptr);
						Node *eviction = (Node *)FirstItem(aptr);

						val = GetValue(eviction);
						tmp = flatNode->inPopWeights[j] = val->u.Const.value.i;
						
						if(trigger) // ��peek���ֲ�Ϊ�գ���ȡֵ
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
						else // peek����Ϊ�գ���peek == pop
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

//						flatNode->AddPopAtCodeGen[j] = 0;//zww:20120213,��ʼAddPopAtCodeGen
						
						//flatNode->inPeekString[j] = val->u.Const.value.i;
						//flatNode->inPopString[j] = flatNode->inPeekWeights[j];
					}
				}
				else // ˵����windowָʾ����pushֵ
				{
					int total = mapEdge2DownFlatNode.count(type); // ��mapΪmultimap�����Ի��ж����Ӧֵ��Ҫ��һ����

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

	//�ȶ����е�pop, peek, push��ʼ��Ϊ1
	for(j = 0; j < flatNode->nIn; j++)
	{
		// ��inPeekWeightsΪ�յ�����£�����ʹ�������±꣨inPeekWeights[j]�����ʣ�������ֶ��Դ���������ͬ
		(flatNode->inPeekWeights).push_back(1); 
		(flatNode->inPopWeights).push_back(1);
		sprintf(tmp, "nPeek_%d",j);
		(flatNode->inPeekString).push_back(tmp); 
		sprintf(tmp, "nPop_%d",j);
		(flatNode->inPopString).push_back(tmp);
	}
	for(j = 0; j < flatNode->nOut; j++)
	{
		(flatNode->outPushWeights).push_back(1); // ����ÿһ��FlatNode��˵��Ϊ1����ͬ
		sprintf(tmp, "nPush_%d",j);
		(flatNode->outPushString).push_back(tmp);
	}
		
	// ����window��Ϊ�յ�operator�ڵ㣬Ϊ����ȡ����Ĭ��ֵ
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
			src = pos->second; // ÿ��������ֻ��һ���϶˽ڵ�
			if(src != flatNode) // ˵����windowָʾ����peek��popֵ
			{
				NodeType nodeType = item->u.window.wtype->typ;
				assert(nodeType == Sliding || nodeType == Tumbling);

				for(j = 0; src != flatNode->inFlatNodes[j] ; j++); //�ҵ���Ӧ��j,׼��д��ֵ
				if(nodeType == Sliding) // sliding window
				{
					int tmp = 0;
					//ȡ��sliding�ڵ��ֵ
					aptr = item->u.window.wtype->u.sliding.sliding_value->u.comma.exprs;
					Node *trigger = (Node *)FirstItem(aptr);
					aptr = Rest(aptr);
					Node *eviction = (Node *)FirstItem(aptr);

					val = GetValue(item->u.window.wtype->u.sliding.sliding_value);
					tmp = flatNode->inPopWeights[j] = val->u.Const.value.i;
					
//					flatNode->AddPopAtCodeGen[j] = 0;//zww:20120213,��ʼAddPopAtCodeGen

					if(trigger) // ��peek���ֲ�Ϊ�գ���ȡֵ
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
					else // peek����Ϊ�գ���peek == pop
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

//					flatNode->AddPopAtCodeGen[j] = 0;//zww:20120213,��ʼAddPopAtCodeGen

					//flatNode->inPeekString[j] = val->u.Const.value.i;
					//flatNode->inPopString[j] = flatNode->inPeekWeights[j];
				}
			}
			else // ˵����windowָʾ����pushֵ
			{
				int total = mapEdge2DownFlatNode.count(type); // ��mapΪmultimap�����Ի��ж����Ӧֵ��Ҫ��һ����

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