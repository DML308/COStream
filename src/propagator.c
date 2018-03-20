#include"propagator.h"



List *DeclArrayListInWork = NULL;//������work�й�����������Ԫ�صĸ�ֵ��䣨������Ķ���һ����param��var�У�

GLOBAL List *gfrtaCallList = NULL;//����frta����

GLOBAL  Node *GetValue(Node *node)
{  
	Node *value=NULL;
	Node *newvalue=NewNode(Const);
	//assert(node);
	if(node==NULL)return NULL;
	if(node->typ==Id) value=node->u.id.value;
	else if (node->typ==Const)value=node;
	else if (node->typ==Array)value=node->u.array.value;
	else if(node->typ==Unary)value=node->u.unary.value;
	else if(node->typ==Binop)value=node->u.binop.value;
	else if(node->typ==Ternary)value=node->u.ternary.value;
	else if(node->typ==Call) value=node->u.call.name->u.id.value;
	else if(node->typ==Cast)value=node->u.cast.value;
	else if(node->typ==ImplicitCast)value=node->u.implicitcast.value;
	else if(node->typ==Call)value=node->u.call.name->u.id.value;
	else value=node; 	
	if(value==NULL) newvalue=NULL;
	else if (value->typ==Const)
	{
		newvalue->u.Const.type=value->u.Const.type;
		newvalue->u.Const.text=NULL;
		newvalue->u.Const.value.d=value->u.Const.value.d;
		newvalue->u.Const.value.f=value->u.Const.value.f;
		newvalue->u.Const.value.s=value->u.Const.value.s;
		newvalue->u.Const.value.ul=value->u.Const.value.ul;
		newvalue->u.Const.value.u=value->u.Const.value.u;
		newvalue->u.Const.value.l=value->u.Const.value.l;
		newvalue->u.Const.value.i=value->u.Const.value.i;
	}
	else newvalue=value;
	return newvalue;
}

GLOBAL void PrintConstFlowValue(FlowValue v)
{
	ListMarker marker;
	constIdNode *itemId = NULL;
	constArrayNode *itemArray  = NULL;

	if (v.undefined)
		printf("\n------------------------PrintInfo: FlowValue------------------------\nv.undefined = true_\n");
	else
		printf("\n------------------------PrintInfo: FlowValue------------------------\nv.undefined = false_\n");

	assert(v.u.ptr != NULL);
	IterateList(&marker, ((propagatorNode *)(v.u.ptr) )->idFlowList);
	while (NextOnList(&marker, (GenericREF) &itemId))
	{
		fprintf(stdout, "Id: %s\n  ", itemId->n->u.id.text);
		PrintNode(stdout, itemId->n->u.id.value, 0);
		printf("\n  ");
		fprintf(stdout, "Decl: (0x%p)\n", itemId->n->u.id.decl);
	}

	printf("--------------Adcl--------------\n");
	IterateList(&marker, ((propagatorNode *)(v.u.ptr) )->arrayFlowList);
	while (NextOnList(&marker, (GenericREF) &itemArray))
	{
		fprintf(stdout, "%s\n", itemArray->name);
		PrintNode(stdout, itemArray->decl, 0);
		printf("\n");
	}
	printf("--------------------------------------------------------------------\n");
	//system("pause");
}

GLOBAL int CheckAdcl(Node *node)
{  //����ά�ĳ���
	int len = 0;
	constIdNode *idNode = NULL;
	Node *value = NULL;

	assert(node && node->typ == Adcl);
	if(node->u.adcl.dim->typ == Const) // ά���ǳ���
		len = NodeConstantIntegralValue(node->u.adcl.dim);
	else  // ά���Ǳ���
	{
		//assert(node->u.adcl.dim->typ == Id); // Ŀǰֻ��������ά����һ��id�����
		//idNode = FindIdNode(node->u.adcl.dim->u.id.decl, v);
		//assert(idNode);
		//value = idNode->n->u.id.value;//ȡ����ֵ
		value = GetValue(node->u.adcl.dim);//zww:12.2.17�޸�
		assert(value->typ == Const );
#if 0
		if (value == NULL)
			PrintConstFlowValue(v);
#endif
		assert(value);
		if (!NodeIsConstant(value))
			SyntaxErrorCoord(node->u.adcl.dim->coord, "array dimension must be constant");
		else if (!IsIntegralType(NodeDataType(value)))
			SyntaxErrorCoord(node->u.adcl.dim->coord,"array dimension must be an integer type");
		else len = NodeConstantIntegralValue(value);

		if (len < 0) 
			SyntaxErrorCoord( node->u.adcl.dim->coord,"negative array dimension");
		else if (len == 0)
			WarningCoord(1, node->u.adcl.dim->coord, "array dimension is zero");
	}

	return len;
}

GLOBAL constIdNode *MakeConstIdNode(Node *node)
{
	constIdNode *itemId = HeapNew(constIdNode);

	assert(node && node->typ == Decl);

	itemId->n = NewNode(Id);
	itemId->n->u.id.text = node->u.decl.name;
	itemId->n->u.id.value = node->u.decl.init;
	itemId->n->u.id.decl = node;
	itemId->nac = FALSE;

	if(node->u.decl.init!= NULL && node->u.decl.init->typ == Const) //ֵ�Ƕ���
		itemId->undefine = FALSE;
	else//ֵ������������ֵ������һ�����ʽ��Ϊ��
		itemId->undefine = TRUE;

	return itemId;
}

//���֧��2ά����
GLOBAL constArrayNode *MakeConstArrayNode(int dim, int *lenPerDim, Node *node)
{
	constArrayNode *itemArray = HeapNew(constArrayNode);
	int i = 0, num = 1;

	assert(lenPerDim && node && node->typ == Decl );//zww ֧�ֶ�ά <---(lenPerDim && node && node->typ == Decl && dim <= 2) 12.2.10
	for(; i< dim; ++i)
		num *= lenPerDim[i];

	itemArray->dim = dim;
	itemArray->dimLen = lenPerDim;
	itemArray->num = num;

	itemArray->name = node->u.decl.name;
	itemArray->decl = node->u.decl.type;
	itemArray->element = (Node **)malloc(sizeof(Node*) * num);
	itemArray->undefine = (Bool *)malloc(sizeof(Bool) * num);
	itemArray->nac = (Bool *)malloc(sizeof(Bool) * num);

	return itemArray;
}

GLOBAL void InitConstArrayNode(int dim, int *lenPerDim, int num, constArrayNode *itemArray, Node *init)
{
	ListMarker marker, pm;
	Node *cn = NULL, *pn = NULL;
	int i = 0;

	assert(lenPerDim && itemArray && dim <= 2);
	for(i = 0; i < num; ++i)
	{
		itemArray->nac[i] = FALSE;
		itemArray->undefine[i] = TRUE;
		itemArray->element[i] = NULL;
	}

	i = 0;
	//��������ĳ�ʼ��
	if(dim == 2 && init != NULL)
	{
		IterateList(&pm, init->u.initializer.exprs);
		while (NextOnList(&pm, (GenericREF) &pn))
		{
			IterateList(&marker, pn->u.initializer.exprs);
			while(NextOnList(&marker, (GenericREF) &cn))
			{
				if(cn != NULL)
				{
					itemArray->element[i] = cn;	
					itemArray->nac[i] = FALSE;
					if(cn->typ == Const) itemArray->undefine[i] = FALSE;
					else itemArray->undefine[i] = TRUE;
				}
				else
				{
					itemArray->element[i] = cn;	
					itemArray->nac[i] = FALSE;
					itemArray->undefine[i] = TRUE;
				}
				i++;
			}
		}				
	}
	else if(dim == 1 && init != NULL)
	{
		IterateList(&pm, init->u.initializer.exprs);
		while (NextOnList(&pm, (GenericREF) &pn))
		{
			if(pn != NULL)
			{
				if(pn->typ == ImplicitCast)
					pn = pn->u.implicitcast.value;
				itemArray->element[i] = pn;	
				itemArray->nac[i] = FALSE;
				if(pn->typ == Const)
					itemArray->undefine[i] = FALSE;
				else 
					itemArray->undefine[i] = TRUE;
			}
			else
			{
				itemArray->element[i] = pn;	
				itemArray->nac[i] = FALSE;
				itemArray->undefine[i] = TRUE;
			}
			i++;		
		}
	}
}

PRIVATE void InitConstMultArrayNode(int dim, int *lenPerDim, int num, constArrayNode *itemArray, Node *init)  //zww�� �����ά����	 12.2.10
{
	ListMarker marker, pm;
	Node *cn = NULL, *pn = NULL;
	int i = 0;

	assert(lenPerDim && itemArray );
	for(i = 0; i < num; ++i)
	{
		itemArray->nac[i] = FALSE;
		itemArray->undefine[i] = TRUE;
		itemArray->element[i] = NULL;
	}
	if(init == NULL)return;
	i = 0;
	assert(init!=NULL);
	IterateList(&pm, init->u.initializer.exprs);
	while (NextOnList(&pm, (GenericREF) &pn))
	{
		if(pn != NULL)
		{
			itemArray->element[i] = pn;	
			itemArray->nac[i] = FALSE;
			if(pn->typ == Const)
				itemArray->undefine[i] = FALSE;
			else 
				itemArray->undefine[i] = TRUE;
		}
		else
		{
			itemArray->element[i] = pn;	
			itemArray->nac[i] = FALSE;
			itemArray->undefine[i] = TRUE;
		}
		i++;		
	}

}

//������������ת����constIdNode����,�����뵽�������У���Ҫ��decl���Ĵ��ݺ������õ�
PRIVATE inline FlowValue InitDeclNode(Node *node, FlowValue v)
{
	constIdNode *tmp = FindIdNode(node, v);

	assert(node && (node->typ == Id || node->typ == Decl) );

	if (tmp == NULL)
	{
		constIdNode *itemId = MakeConstIdNode(node);
		//���½��� itemId ��ӵ���������
		((propagatorNode*)(v.u.ptr))->idFlowList = AppendItem(((propagatorNode*)(v.u.ptr))->idFlowList, itemId);
	}
	else//�ñ�������ѭ���ڲ������
	{
		tmp->n->u.id.value = node->u.decl.init;
		if(node->u.decl.init != NULL && node->u.decl.init->typ == Const) 
			tmp->undefine = FALSE;
		else 
			tmp->undefine = TRUE;
		tmp->nac = FALSE;
	}
	
#if 0
	printf("%s\n", itemId->n->u.id.text);
	printf("%d\n", ListLength(((propagatorNode*)(v.u.ptr))->idFlowList));
#endif
	
	return v;
}

PRIVATE FlowValue InitAdclNode(Node *node, FlowValue v)
{
	constArrayNode *itemArray = NULL, *tmpNode = NULL;
	Node *tmp = NULL, *init = NULL;
	Node *elementtype = NULL;//��¼�������Ԫ�ص�����
	propagatorNode *tmp2 = (propagatorNode *)(v.u.ptr);
	List *list = tmp2->arrayFlowList;
	List *tmpList = list;

	assert(node && node->typ == Decl);
	tmp = node->u.decl.type;
	assert(tmp && tmp->typ == Adcl);
	init = node->u.decl.init;

	//zww:12.03.02,�������������ַ�����ĳ�ʼ�������ݲ�������������char s[10]="abcdefghi"   ;
	elementtype = node->u.decl.type;
	while (elementtype->typ==Adcl)
	{
		elementtype = elementtype->u.adcl.type;
	}
	assert(elementtype->typ == Prim);
	if(elementtype->u.prim.basic == Char||(init!=NULL&&init->typ == Const)) return v;


	while (list) 
	{
		tmpNode = (constArrayNode*)(FirstItem(list));
		if(node->u.decl.name == tmpNode->name && node->u.decl.type == tmpNode->decl) // node->u.decl.name == tmpanode->name ��仰����Ҫ
			break;
		else 
			tmpNode = NULL;
		list = Rest(list);
	}

	if(tmpNode == NULL)
	{
		int dim = 0, num = 1, i = 0;
		int *lenPerDim = NULL;//zww �޸�֧�ָ�ά 12.2.13
		while(tmp->typ == Adcl)
		{
			++dim;
			tmp = tmp->u.adcl.type;
		}
		lenPerDim = (int *)malloc(sizeof(int)*dim);//zww �޸�֧�ָ�ά 12.2.13
		tmp = node->u.decl.type;
		i=0;
		while(tmp->typ == Adcl)	//zww �޸�֧�ָ�ά 12.2.13
		{
			lenPerDim[i++] = CheckAdcl(tmp);
			tmp = tmp->u.adcl.type;
		}
		itemArray = MakeConstArrayNode(dim, lenPerDim, node);
		num = itemArray->num;
		if(dim<=2)InitConstArrayNode(dim, lenPerDim, num, itemArray, init);	//zww ����Ӵ����ά 12.2.10
		else
		{
			assert(dim>2);
			init =	node->u.decl.prim_init;
			InitConstMultArrayNode(dim, lenPerDim, num, itemArray, init);

		}
		tmp2->arrayFlowList = AppendItem(tmpList, itemArray);

	}
	else
		if(tmpNode->dim<=2)InitConstArrayNode(tmpNode->dim, tmpNode->dimLen, tmpNode->num, tmpNode, init);
		else
		{
			assert(tmpNode->dim>2);
			init =	node->u.decl.prim_init;
			InitConstMultArrayNode(tmpNode->dim, tmpNode->dimLen, tmpNode->num, tmpNode, init);
		}

	return v; 
}

PRIVATE FlowValue AlterFlow(Node *node, Node *value, FlowValue v)
{
	List  *list = NULL;
	constIdNode *itemId = NULL;
	constArrayNode *itemArray = NULL;
	ListMarker marker;
	Node *n = NULL;
	int *pos = NULL, tmp = 0, i = 0, j = 0, place = 0;//placeָ�ľ���Ԫ����һλ�е�λ��

	
	//assert(node->typ == Id || node->typ == Array);

	if(node->typ == Id) 
	{
		assert(value && value->typ == Const);
		itemId = FindIdNode(node->u.id.decl, v);
		if(itemId == NULL) return v;//zww-20120314 ���������������������ֱ�ӷ��أ�state�еı����ǲ����������еģ�
		assert(itemId);
		node->u.id.value = value;
		itemId->n->u.id.value = value;
		itemId->undefine = FALSE;
		return v;
	}
	else if(node->typ == Array)
	{
		assert(value && value->typ == Const);
		itemArray = FindArrayNode(node,v);//���������Ԫ�ض�λ
		if(itemArray == NULL) return v;//zww-20120314 ���������������������ֱ�ӷ��أ�state�еı����ǲ����������еģ�
		assert(itemArray);
		assert(node->u.array.name->typ==Id);//��ֹ��sa.a[i]�����Ľڵ�
		pos = (int *)malloc(sizeof(int)*(itemArray->dim));
		IterateList(&marker, node->u.array.dims);
		while (NextOnList(&marker, (GenericREF) &n))
		{
			if(n->typ==Id&&n->u.id.value!=NULL) pos[i]=n->u.id.value->u.Const.value.i;
			else if(n->typ==Const)pos[i]=n->u.Const.value.i;
			else if (n->typ==Binop&&n->u.binop.value!=NULL)pos[i]=n->u.binop.value->u.Const.value.i;
			else if (n->typ==Unary&&n->u.unary.value!=NULL)pos[i]=n->u.unary.value->u.Const.value.i;
			else if (n->typ==Ternary&&n->u.ternary.value!=NULL)pos[i]=n->u.ternary.value->u.Const.value.i;
			else return v;
			i++;
		}
		for (i = 0; i < itemArray->dim; i++)
		{
			tmp = 1;
			for(j = i+1; j < itemArray->dim; j++)
				tmp = tmp * itemArray->dimLen[j];
			place = place + pos[i] * tmp;
		}

		node->u.array.value = value;  //�޸Ĺ�12.2.10
		//ConstFoldCast(node->u.id.value);
		itemArray->element[place] = value;
		itemArray->undefine[place] = FALSE;
		return v;
	}
	else if (node->typ==Binop && node->u.binop.op == '.')
	{
		assert(value);
		AlterDot(node,value,v);
	}
	return v;

	
}

//�Ƚ������������Ƿ����
PRIVATE inline Bool EqualConstFlow(FlowValue dest, FlowValue src)
{
	//һ����˵��dest.undefined == TRUE, src.undefined == FALSE����Ϊ���ڳ���������˵��src����ȷ������destһ�㲻ȷ����Ҫ����û��Ҫ����ȥ�ˡ�
	if (dest.undefined)
		return src.undefined;
	else //2013 4-16 YQJ ����else�ṹ ���� && ����
		return dest.undefined;

	PrintConstFlowValue(dest);
	UNREACHABLE;
}

//����������meet����
PRIVATE inline FlowValue MeetConstFlow(FlowValue dest, FlowValue src)
{
	//ͬEqualConstFlow����
	if (dest.undefined == TRUE)
		return src;
	else //2013 4-16 YQJ ����else�ṹ ���� && ����
		return dest;
	
	PrintConstFlowValue(dest);
	UNREACHABLE;
}

//����������id�ڵ�
PRIVATE inline constIdNode *FindIdNode(Node *node, FlowValue v)
{
	ListMarker marker;
	constIdNode *item = NULL;
	//ȡ��id������
	List *list = ((propagatorNode*)(v.u.ptr))->idFlowList;
	
	//assert(node && node->typ == Decl); // 12.13 ��������work
	IterateList(&marker, list);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		if(node == item->n->u.id.decl) 
			return item;
	}

	return NULL;
}

//����������array�ڵ�
PRIVATE inline constArrayNode *FindArrayNode(Node *node ,FlowValue v)
{
	assert(node && node->typ == Array);
	if(node->u.array.name->typ == Id){ // ��ͨ���� ����arr[1]
		ListMarker marker;
		constArrayNode *item = NULL;
		Node *type = NULL;
		List *list = ((propagatorNode*)(v.u.ptr))->arrayFlowList;//ȡ�� array ������
		type = node->u.array.name->u.id.decl->u.decl.type;
		IterateList(&marker, list);
		while (NextOnList(&marker, (GenericREF) &item)) 
			if(node->u.array.name->u.id.text == item->name && type == item->decl) return item; // node->u.array.name->u.id.text == item->name ����ɾ�� 
	}
	return NULL;
}

//����⺯������, �����⺯����δ����
PRIVATE inline Node *CallLibFunc(Node *node)
{   
	callNode call = node->u.call;
	Node *constNode = NULL;
	int i = 0;
	char *fooName[] = {"sin", "cos", "tan", "exp", "log","sqrt"};
	double (*foo[])(double) = {sin, cos, tan, exp, log, sqrt};
	int num = sizeof(foo)/sizeof(int);

	for (i = 0; i < num; ++i)
		if(strcmp(fooName[i],call.name->u.id.text) == 0) break;

	//������ڣ������޷����г�������
	assert(i != num);
	constNode = GetValue(((Node*)FirstItem(call.args)));
	NodeSetDoubleValue(node->u.call.name, foo[i](constNode->u.Const.value.d));
}

GLOBAL Node *ModifyCompositeParam(Node *node, comCallNode *u, FlowValue v)
{
	List *fromList = u->operdcl->u.operdcl.arguments;
	List *toList = NULL;
	ListMarker fromMarker, toMarker;
	Node *fromNode = NULL, *toNode = NULL;

	if(fromList == NULL) return node;
	toList = u->actual_composite->u.composite.body->u.comBody.param->u.param.parameters;
	assert(toList && ListLength(fromList) == ListLength(toList));

#if 0
	PrintList(stdout, fromList, -1);
	PrintList(stdout, toList, -1);
#endif

	IterateList(&fromMarker, fromList);
	IterateList(&toMarker, toList);
	while(NextOnList(&fromMarker, (GenericREF) &fromNode))
	{
		NextOnList(&toMarker, (GenericREF) &toNode);
		assert(toNode && toNode->typ == Decl);
		if(IsArithmeticType(NodeDataType(toNode)))
			toNode->u.decl.init = GetValue(fromNode);
		else // ��һ��ָ�������ָ��, if(IsPointerType(NodeDataType(toNode)) && fromNode->typ == ImplicitCast && fromNode->u.implicitcast.expr->typ == Id)
		{
			constArrayNode *tmpNode = NULL;
			char *name = NULL;
			Node *decl = NULL;
			List *list = ((propagatorNode*)(v.u.ptr))->arrayFlowList;
			int i = 0, j = 0;

			assert(IsPointerType(NodeDataType(toNode)));
			if (fromNode->typ == ImplicitCast)
			{
				name = fromNode->u.implicitcast.expr->u.id.text;
				decl = fromNode->u.implicitcast.expr->u.id.decl->u.decl.type;
			}
			else
			{
				name = fromNode->u.id.text;
				decl = fromNode->u.id.decl->u.decl.type;
			}
			while (list) 
			{
				tmpNode = (constArrayNode*)(FirstItem(list));
				if(name == tmpNode->name && decl == tmpNode->decl) 
					break;
				else tmpNode = NULL;
				list = Rest(list);
			}
			//assert(tmpNode && tmpNode->dim <= 2); // һ��Ҫ�ҵ���Ҫ��Ȼ�ͳ�����
			assert(tmpNode);
			toNode->u.decl.type = tmpNode->decl;

			if(tmpNode->dim == 1) // һά
			{
				Node *init = NewNode(Initializer);
				for(i = 0;i<tmpNode->num;i++)
					init->u.initializer.exprs = AppendItem(init->u.initializer.exprs, tmpNode->element[i]);
				toNode->u.decl.init = init;
			}
			else if(tmpNode->dim == 2)// ��ά  zww: 12.2.10
			{
				Node *initsNode = NewNode(Initializer);
				for(i = 0;i<tmpNode->dimLen[0];i++)
				{
					Node *init = NewNode(Initializer);
					for (j = 0; j < tmpNode->dimLen[1]; j++)
						init->u.initializer.exprs = AppendItem(init->u.initializer.exprs, tmpNode->element[i*(tmpNode->dimLen[i])+j]);
					initsNode->u.initializer.exprs = AppendItem(initsNode->u.initializer.exprs, init);
				}
				toNode->u.decl.init = initsNode;
			}
			else//��ά������2��	  //zww�������ά����Ĳ������ݣ����������⣩12.2.10
			{
				Node *init = NewNode(Initializer);
				Node *dtype = NodeDataType(toNode->u.decl.type);
				for(i = 0;i<tmpNode->num;i++)
				{
					assert(tmpNode->element[i]!=NULL);
					init->u.initializer.exprs = AppendItem(init->u.initializer.exprs, tmpNode->element[i]);					
				}
				assert(init);
				assert(init->typ == Initializer);
				toNode->u.decl.prim_init= NodeCopy(init,NodeOnly);
				toNode->u.decl.init = SemCheckInitList(toNode, dtype, NodeCopy(init,NodeOnly), TRUE);
			}
		}
	}

	return node;
}

PRIVATE inline Node *MakeNewIdValue(Node *value)
{
	Node *newNode = NULL, *type = NULL;

	assert(value && value->typ == Const);
	type = value->u.Const.type;
	assert(type && type->typ == Prim);

	switch (type->u.prim.basic) 
	{
	case Sint:   newNode = MakeConstSint(value->u.Const.value.i);break;
	case Uint:   newNode = MakeConstUint(value->u.Const.value.u);break;
	case Slong:  newNode = MakeConstSlong(value->u.Const.value.l);break;
	case Ulong:  newNode = MakeConstUlong(value->u.Const.value.ul);break;
	case Float:  newNode = MakeConstFloat(value->u.Const.value.f);break;
	case Double: newNode = MakeConstDouble(value->u.Const.value.d);break;
	default: UNREACHABLE;
	}

	return newNode;
}

PRIVATE inline FlowValue TransformId(Node *node,FlowValue v)
{
	List  *ilist = NULL;
	constIdNode *cn = NULL;

	assert(node && node->typ == Id);
	ilist = ((propagatorNode*)(v.u.ptr))->idFlowList;

	cn = FindIdNode(node->u.id.decl, v);

	if(cn == NULL)  return v;
	if(cn->n->u.id.value != NULL && cn->n->u.id.value->typ != Const)
	{
		cn->n->u.id.value = GetValue(cn->n->u.id.value);
		cn->undefine = FALSE;
	}
	if(cn->nac == TRUE || cn->undefine == TRUE) node->u.id.value = NULL;
	else if(cn->n->u.id.value->typ == Const && cn->nac == FALSE)
	{
		/*node->u.id.value=cn->n->u.id.value;*/
		node->u.id.value = MakeNewIdValue(cn->n->u.id.value);
	}
	else node->u.id.value = NULL;

	return v;
}

PRIVATE inline FlowValue TransformArray(Node *node,FlowValue v)
{
	ListMarker m;
	Node *n, *value;
	unsigned long *pos, tmp, i = 0, j;
	unsigned long place=0;//placeָ�ľ���Ԫ����һλ�е�λ��
	constArrayNode *cn=NULL;

	if(node->u.array.name->typ == Binop){ //���� s.x[3] �� s[1].x[1] �Ƚṹ���к��������Ա��
 		List *SUEflowlist = ((propagatorNode*)(v.u.ptr))->SUEFlowList;
		List *tmpsuelist=NULL;
		ListMarker m;
		Node *left,*right,*n,*value=NULL;
		Node *sueNode[10]; //����ָ������ ���� s.a.b.c ... �ṹ�����Ƕ��10��
		int key[10];    //�漰���Ľṹ�����������ֵ
		Node *leftiddecl,*tmp=NULL;
		SUEtype *leftsuetype;
		constSUE* leftsuetypeNode;
		constSUEid *sueidNode = NULL;
		constSUEarray *suearrayNode = NULL;
		constSUEFieldNode *rightField_arr = NULL;
		int i=0,j=0,num;  //iΪsueNode������±� jΪkey������±�

		IterateList(&m, node->u.array.dims);
		NextOnList(&m, (GenericREF) &n);
		value = GetValue(n);
		key[j++] = NodeConstantIntegralValue(value);

		left = node->u.array.name->u.binop.left;
		right = node->u.array.name->u.binop.right;
		assert(right->typ == Id);  //�ұ߶�Ӧ����id���͵�
		sueNode[i++] = right;

		while(left->typ != Id ){  //���� s.a[1].b.c.d.e[2]  ���ν�e��d,c,b��a ��������rightNode
			if(left->typ == Binop) //����s.x.x[1] ��s[1].x.x[1]
			{
				right = left->u.binop.right;  //rightһ��Ϊid����
				assert(right->typ == Id);
				left = left->u.binop.left;
				//if(i==0 || sueNode[i-1]->typ != Array)
					sueNode[i++] = right;
			}else if(left->typ == Array) //����s[1].x[2] ��ȡs���±� ����key����
			{
				IterateList(&m, left->u.array.dims);
				NextOnList(&m, (GenericREF) &n);
				value = GetValue(n);
				key[j++] = NodeConstantIntegralValue(value);
				sueNode[i++] = left;
				left = left->u.array.name;
			}
			else
				assert(1==0);  //���򱨴�
		}

		leftiddecl = left->u.id.decl;
		tmp = leftiddecl->u.decl.type;
		assert(tmp);
		while(tmp->typ != Sdcl){ // �ҵ���sdcl����
			if(tmp->typ == Tdef)
				tmp = tmp->u.tdef.type;
			else if(tmp->typ == Adcl)
				tmp = tmp->u.array.name;
			else
				break;
		}
		assert(tmp->typ == Sdcl);
		leftsuetype = tmp->u.sdcl.type;
		leftsuetypeNode = FindSUEtype(leftsuetype, SUEflowlist);
		assert(leftsuetypeNode);
		i--;
		for(;i>=0;i--){
			if(sueNode[i]->typ == Array){
				num = key[--j];
				if(tmpsuelist == NULL)
					suearrayNode = FindSUEarray(sueNode[i--],leftsuetypeNode->SUEnode->SUEarray);
				else
					continue;
				assert(suearrayNode);
				tmpsuelist = suearrayNode->SUEelement[num];
			}else if(sueNode[i]->typ == Id){
				if(tmpsuelist == NULL){
					sueidNode = FindSUEid(leftiddecl,leftsuetypeNode->SUEnode->SUEid);  
					tmpsuelist = sueidNode->SUEfields;
				}
			}
			rightField_arr = FindFieldSUEnode(tmpsuelist, sueNode[i]);
			if(rightField_arr->sutyp == Prim_Id)
				tmpsuelist = rightField_arr->u.idnode;
			else if(rightField_arr->sutyp == Prim_Array){
				if(j>0) j--;
				num = key[j];
				cn = rightField_arr->u.arrayNode;
				if(num>=cn->num)  
					SyntaxErrorCoord(node->coord, "cannot take the address of a non-lvalue");
				if(cn->nac[num]==TRUE) node->u.array.value=NULL;
				else if(cn->nac[num]==FALSE&&cn->element[num]!=NULL)
				{node->u.array.value =cn->element[num];}
				return v;
			}
			else if(rightField_arr->sutyp == SUE_Id)
				tmpsuelist = rightField_arr->u.sueidNode;
			else{
				num = key[--j];
				tmpsuelist = rightField_arr->u.suearrayNode->SUEelement[num];
			}
		}
		if(node->u.array.name->typ == Binop && tmpsuelist!=NULL)
			node->u.array.name->u.binop.valueList = tmpsuelist;
	}else{
		cn=FindArrayNode(node,v);//���������Ԫ�ض�λ
	}
	if(cn==NULL) return v;
	pos=(unsigned long *)malloc(sizeof(unsigned long)*cn->dim);
	IterateList(&m, node->u.array.dims);
	while (NextOnList(&m, (GenericREF) &n))
	{
		value=GetValue(n);
		// assert(value); // 12.13 ��������work
		if(value!=NULL) pos[i++]=NodeConstantIntegralValue(value);
		else return v;
	}
	for (i=0;i<cn->dim;i++)
	{
		tmp=1;
		for(j=i+1;j<cn->dim;j++)
			tmp=tmp*cn->dimLen[j];
		place=place+pos[i]*tmp;
	}
	//printf("%d\n",place);
	if(place>=cn->num)  
		SyntaxErrorCoord(node->coord, "cannot take the address of a non-lvalue");
	if(cn->nac[place]==TRUE)node->u.array.value=NULL;
	else if(cn->nac[place]==FALSE&&cn->element[place]!=NULL)
	{node->u.array.value =cn->element[place];}
	return v;
}

//����Decl���,������v������½ڵ�,����ȫ�ֱ���ֻ�ܶ�����д
PRIVATE inline FlowValue TransformDecl(Node *node, FlowValue v)
{
	
	TypeQual dl = NodeDeclLocation(node);
	TypeQual sc = NodeStorageClass(node);//Ҫ�ռ��ⲿ�ľ�̬�ĵ����еı�������
	constIdNode *inode = NULL;
	constArrayNode *anode = NULL;
	assert(node->typ==Decl);

	if ((dl == T_BLOCK_DECL || dl == T_FORMAL_DECL|| dl == T_TOP_DECL) ) 
	{
		if(IsScalarType(NodeDataType(node))&& (sc != T_TYPEDEF )&&(!IsSueType(NodeDataType(node))))
		{
			return InitDeclNode(node, v);
		}
		if (!IsScalarType(NodeDataType(node)) && sc == T_TYPEDEF && IsSueType(NodeDataType(node))) //�Խṹ�������ض��壬typedef
		{
			node->typ = Tdef; //���½ڵ�����
			return InsertSUEdecltoFlow(node, v);
		}
		else if(node->u.decl.type->typ == Adcl)
		{
			Node *tmptype = node->u.decl.type;
			while (tmptype->typ==Adcl)
			{
				tmptype = tmptype->u.adcl.type;
			}
			if(tmptype->typ==Prim) 
			{
				if(STORAGE_CLASS(node->u.decl.tq) == T_EXTERN)
					return v;
				return InitAdclNode(node, v);
			}
			else if(tmptype->typ == Sdcl || tmptype->typ == Tdef) return InsertSUEdecltoFlow(node, v);
			else return v;
		}
		else if(node->u.decl.type->typ == Sdcl || node->u.decl.type->typ == Tdef) return InsertSUEdecltoFlow(node, v);
	}
	//UNREACHABLE;
	return v;
}

//����ֵ�����͸��ϸ�ֵ����
PRIVATE inline FlowValue TransformAssignment(Node *node,FlowValue v)
{
	Node   *left, *right, *ltype, *rtype, *leftvalue=NULL, *rightvalue=NULL;
	OpType  opcode;
	assert(node);
	
	left   = node->u.binop.left;
	right  = node->u.binop.right;
	if(right->u.binop.op==ARROW||left->u.binop.op==ARROW) return v;
	assert(left);
	assert(right);
	ltype  = NodeDataType(left);
	rtype  = NodeDataType(right);
	assert(ltype);
	assert(rtype);
	opcode = node->u.binop.op;
	if(left->typ==Binop&&left->u.binop.value!=NULL) leftvalue=left->u.binop.value;
	else if(left->typ==Id&&left->u.id.value!=NULL) leftvalue=left->u.id.value;
	else if(left->typ==Array&&left->u.array.value!=NULL) leftvalue=left->u.array.value;
	else if(left->typ==Unary&&left->u.unary.value!=NULL)leftvalue=left->u.unary.value;
	else if(left->typ==Ternary&&left->u.ternary.value!=NULL)leftvalue=left->u.ternary.value;
	else if(left->typ==Cast&&left->u.cast.value!=NULL)leftvalue=left->u.cast.value;
	else if(left->typ==ImplicitCast&&left->u.cast.value!=NULL)leftvalue=left->u.implicitcast.value;
	else if(left->typ==Unary&&left->u.unary.value!=NULL)leftvalue=right->u.unary.value;
	else leftvalue=left;
	if(right->typ==Binop&&right->u.binop.value!=NULL) rightvalue=right->u.binop.value;
	else if(right->typ==Id&&right->u.id.value!=NULL) rightvalue=right->u.id.value;
	else if(right->typ==Array&&right->u.array.value!=NULL) rightvalue=right->u.array.value;
	else if(right->typ==Unary&&right->u.unary.value!=NULL)rightvalue=right->u.unary.value;
	else if(right->typ==Ternary&&right->u.ternary.value!=NULL)rightvalue=right->u.ternary.value;
	else if(right->typ==Call&&right->u.call.name->u.id.value!=NULL)rightvalue=right->u.call.name->u.id.value;
	else if(right->typ==Cast&&right->u.cast.value!=NULL)rightvalue=right->u.cast.value;
	else if(right->typ==ImplicitCast&&right->u.cast.value!=NULL)rightvalue=right->u.implicitcast.value;
	else if(right->typ==Const)rightvalue=right;
	else rightvalue=right;
	switch(opcode){
		 case '=':
			 //printf("%s=%d\n",node->u.binop.left->u.id.text,rightvalue->u.Const.value.i);
			 {//��ӶԽṹ��ֱ�Ӹ�ֵ�Ĵ���
				 return AlterFlow(node->u.binop.left,rightvalue,v);
			 }			 
		 case MULTassign://*=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {
				 if (NodeTypeIsSint(leftvalue))
				 { int lval = NodeConstantSintValue(leftvalue),
				 rval = NodeConstantSintValue(rightvalue);
				 NodeSetSintValue(node, lval * rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 { unsigned int lval = NodeConstantUintValue(leftvalue),
				 rval = NodeConstantUintValue(rightvalue);
				 NodeSetUintValue(node, lval * rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 { long lval = NodeConstantSlongValue(leftvalue),
				 rval = NodeConstantSlongValue(rightvalue);
				 NodeSetSlongValue(node, lval * rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 { unsigned long lval = NodeConstantUlongValue(leftvalue),
				 rval = NodeConstantUlongValue(rightvalue);
				 NodeSetUlongValue(node, lval * rval);
				 }
				 else if (NodeTypeIsFloat(leftvalue))
				 { float lval = NodeConstantFloatValue(leftvalue),
				 rval = NodeConstantFloatValue(rightvalue);
				 NodeSetFloatValue(node, lval * rval);
				 }
				 else if (NodeTypeIsDouble(leftvalue))
				 { double lval = NodeConstantDoubleValue(leftvalue),
				 rval = NodeConstantDoubleValue(rightvalue);
				 NodeSetDoubleValue(node, lval * rval);
				 }
			 }
			 return AlterFlow(node->u.binop.left, node->u.binop.value, v);

		 case DIVassign:// /=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {
				 if (NodeTypeIsSint(leftvalue))
				 { int lval = NodeConstantSintValue(leftvalue),
				 rval = NodeConstantSintValue(rightvalue);

				 if (rval == 0)
					 SyntaxErrorCoord(node->coord, "attempt to divide constant by 0");
				 else
					 NodeSetSintValue(node, lval / rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 { unsigned int lval = NodeConstantUintValue(leftvalue),
				 rval = NodeConstantUintValue(rightvalue);

				 if (rval == 0)
					 SyntaxErrorCoord(node->coord, "attempt to divide constant by 0");
				 else
					 NodeSetUintValue(node, lval / rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 { long lval = NodeConstantSlongValue(leftvalue),
				 rval = NodeConstantSlongValue(rightvalue);

				 if (rval == 0)
					 SyntaxErrorCoord(node->coord, "attempt to divide constant by 0");
				 else
					 NodeSetSlongValue(node, lval / rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 { unsigned long lval = NodeConstantUlongValue(leftvalue),
				 rval = NodeConstantUlongValue(rightvalue);

				 if (rval == 0)
					 SyntaxErrorCoord(node->coord, "attempt to divide constant by 0");
				 else
					 NodeSetUlongValue(node, lval / rval);
				 }
				 else if (NodeTypeIsFloat(leftvalue))
				 { float lval = NodeConstantFloatValue(leftvalue),
				 rval = NodeConstantFloatValue(rightvalue);

				 if (rval == 0)
					 SyntaxErrorCoord(node->coord, "attempt to divide constant by 0");
				 else
					 NodeSetFloatValue(node, lval / rval);
				 }
				 else if (NodeTypeIsDouble(leftvalue))
				 { double lval = NodeConstantDoubleValue(leftvalue),
				 rval = NodeConstantDoubleValue(rightvalue);

				 if (rval == 0)
					 SyntaxErrorCoord(node->coord, "attempt to divide constant by 0");
				 else
					 NodeSetDoubleValue(node, lval / rval);
				 }
				 //node->u.binop.type = ltype;
			 }
			return AlterFlow(node->u.binop.left,node->u.binop.value,v);
		 case MODassign:// %=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {

				 if (NodeTypeIsSint(leftvalue))
				 { int lval = NodeConstantSintValue(leftvalue),
				 rval = NodeConstantSintValue(rightvalue);
				 NodeSetSintValue(node, lval % rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 { unsigned int lval = NodeConstantUintValue(leftvalue),
				 rval = NodeConstantUintValue(rightvalue);
				 NodeSetUintValue(node, lval % rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 { long lval = NodeConstantSlongValue(leftvalue),
				 rval = NodeConstantSlongValue(rightvalue);
				 NodeSetSlongValue(node, lval % rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 { unsigned long lval = NodeConstantUlongValue(leftvalue),
				 rval = NodeConstantUlongValue(rightvalue);
				 NodeSetUlongValue(node, lval % rval);
				 }
				
			 }
			  return AlterFlow(node->u.binop.left,node->u.binop.value,v);
		 case PLUSassign: // +=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {
				 if (NodeTypeIsSint(leftvalue))
				 {
					 int lval = NodeConstantSintValue(leftvalue),
						 rval = NodeConstantSintValue(rightvalue);
					 NodeSetSintValue(node, lval + rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 {
					 unsigned int lval = NodeConstantUintValue(leftvalue),
						 rval = NodeConstantUintValue(rightvalue);
					 NodeSetUintValue(node, lval + rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 {
					 long lval = NodeConstantSlongValue(leftvalue),
						 rval = NodeConstantSlongValue(rightvalue);

					 NodeSetSlongValue(node, lval + rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 {
					 unsigned long lval = NodeConstantUlongValue(leftvalue),
						 rval = NodeConstantUlongValue(rightvalue);
					 NodeSetUlongValue(node, lval + rval);
				 }

				 else if (NodeTypeIsFloat(leftvalue))
				 { 
					 float lval = NodeConstantFloatValue(leftvalue),
						 rval = NodeConstantFloatValue(rightvalue);
					 NodeSetFloatValue(node, lval + rval);
				 }
				 else if (NodeTypeIsDouble(leftvalue))
				 { 
					 double lval = NodeConstantDoubleValue(leftvalue),
						 rval = NodeConstantDoubleValue(rightvalue);
					 NodeSetDoubleValue(node, lval + rval);
				 }
				 AlterFlow(node->u.binop.left,node->u.binop.value,v);
			 }
			return AlterFlow(node->u.binop.left,node->u.binop.value,v);
		 case MINUSassign:// -=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {
				 if (NodeTypeIsSint(leftvalue))
				 { int lval = NodeConstantSintValue(leftvalue),
				 rval = NodeConstantSintValue(rightvalue);

				 NodeSetSintValue(node, lval - rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 { unsigned int lval = NodeConstantUintValue(leftvalue),
				 rval = NodeConstantUintValue(rightvalue);

				 NodeSetUintValue(node, lval - rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 { long lval = NodeConstantSlongValue(leftvalue),
				 rval = NodeConstantSlongValue(rightvalue);
				 NodeSetSlongValue(node, lval - rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 { unsigned long lval = NodeConstantUlongValue(leftvalue),
				 rval = NodeConstantUlongValue(rightvalue);
				 NodeSetUlongValue(node, lval - rval);
				 }

				 else if (NodeTypeIsFloat(leftvalue))
				 { float lval = NodeConstantFloatValue(leftvalue),
				 rval = NodeConstantFloatValue(rightvalue);
				 NodeSetFloatValue(node, lval - rval);
				 }
				 else if (NodeTypeIsDouble(leftvalue))
				 { double lval = NodeConstantDoubleValue(leftvalue),
				 rval = NodeConstantDoubleValue(rightvalue);
				 NodeSetDoubleValue(node, lval - rval);
				 }
				 //node=node->u.binop.value;
			 }
			return AlterFlow(node->u.binop.left,node->u.binop.value,v);
		 case LSassign:// <<=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {
				 unsigned long rval = NodeConstantIntegralValue(rightvalue);
				 if (NodeTypeIsSint(leftvalue))
				 { int lval = NodeConstantSintValue(leftvalue);
				 NodeSetSintValue(node, lval << rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 { unsigned int lval = NodeConstantUintValue(leftvalue);
				 NodeSetUintValue(node, lval << rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 { long lval = NodeConstantSlongValue(leftvalue);
				 NodeSetSlongValue(node, lval << rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 { unsigned long lval = NodeConstantUlongValue(leftvalue);

				 NodeSetUlongValue(node, lval << rval);
				 }
			 }
			 return AlterFlow(node->u.binop.left,node->u.binop.value,v);

		 case RSassign:// >>=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {
				 unsigned long rval = NodeConstantIntegralValue(rightvalue);

				 if (NodeTypeIsSint(leftvalue))
				 { int lval = NodeConstantSintValue(leftvalue);

				 NodeSetSintValue(node, lval >> rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 { unsigned int lval = NodeConstantUintValue(leftvalue);

				 NodeSetUintValue(node, lval >> rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 { long lval = NodeConstantSlongValue(leftvalue);

				 NodeSetSlongValue(node, lval >> rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 { unsigned long lval = NodeConstantUlongValue(leftvalue);

				 NodeSetUlongValue(node, lval >> rval);
				 }
				 // node=node->u.binop.value;
			 }
			  return AlterFlow(node->u.binop.left,node->u.binop.value,v);
		 case ANDassign:// &=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {
				 if (NodeTypeIsSint(leftvalue))
				 { int lval = NodeConstantSintValue(leftvalue),
				 rval = NodeConstantSintValue(rightvalue);
				 NodeSetSintValue(node, lval & rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 { unsigned int lval = NodeConstantUintValue(leftvalue),
				 rval = NodeConstantUintValue(rightvalue);
				 NodeSetUintValue(node, lval & rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 { long lval = NodeConstantSlongValue(leftvalue),
				 rval = NodeConstantSlongValue(rightvalue);

				 NodeSetSlongValue(node, lval & rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 { unsigned long lval = NodeConstantUlongValue(leftvalue),
				 rval = NodeConstantUlongValue(rightvalue);
				 NodeSetUlongValue(node, lval & rval);
				 }
			 }
			  return AlterFlow(node->u.binop.left,node->u.binop.value,v);
		 case ERassign:// ^=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {
				 if (NodeTypeIsSint(leftvalue))
				 { int lval = NodeConstantSintValue(leftvalue),
				 rval = NodeConstantSintValue(rightvalue);
				 NodeSetSintValue(node, lval ^ rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 { unsigned int lval = NodeConstantUintValue(leftvalue),
				 rval = NodeConstantUintValue(rightvalue);
				 NodeSetUintValue(node, lval ^ rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 { long lval = NodeConstantSlongValue(leftvalue),
				 rval = NodeConstantSlongValue(rightvalue);
				 NodeSetSlongValue(node, lval ^ rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 { unsigned long lval = NodeConstantUlongValue(leftvalue),
				 rval = NodeConstantUlongValue(rightvalue);
				 NodeSetUlongValue(node, lval ^ rval);
				 }
			 }
			  return AlterFlow(node->u.binop.left,node->u.binop.value,v);
		 case ORassign:// |=
			 if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
			 {
				 if (NodeTypeIsSint(leftvalue))
				 { int lval = NodeConstantSintValue(leftvalue),
				 rval = NodeConstantSintValue(rightvalue);

				 NodeSetSintValue(node, lval | rval);
				 }
				 else if (NodeTypeIsUint(leftvalue))
				 { unsigned int lval = NodeConstantUintValue(leftvalue),
				 rval = NodeConstantUintValue(rightvalue);

				 NodeSetUintValue(node, lval | rval);
				 }
				 else if (NodeTypeIsSlong(leftvalue))
				 { long lval = NodeConstantSlongValue(leftvalue),
				 rval = NodeConstantSlongValue(rightvalue);

				 NodeSetSlongValue(node, lval | rval);
				 }
				 else if (NodeTypeIsUlong(leftvalue))
				 { unsigned long lval = NodeConstantUlongValue(leftvalue),
				 rval = NodeConstantUlongValue(rightvalue);

				 NodeSetUlongValue(node, lval | rval);
				 }
				 //  node=node->u.binop.value;
			 }
			 return AlterFlow(node->u.binop.left,node->u.binop.value,v);
		 default:
			 return v;
	}
}

//������������
PRIVATE inline FlowValue TransformArithmetic(Node *node,FlowValue v)
{
	Node   *left, *right, *ltype, *rtype,*leftvalue=NULL,*rightvalue=NULL;
	OpType  opcode;
	//constIdNode *cn=NULL;
	assert(node);

	left= UsualUnaryConversions(node->u.binop.left,  FALSE),
	right  = UsualUnaryConversions(node->u.binop.right, FALSE),
	opcode = node->u.binop.op;
	if(right->u.binop.op==ARROW||right->u.binop.op==ARROW) return v;
	//left=node->u.binop.left;
	//right=node->u.binop.right;
	if(left->typ==Binop&&left->u.binop.value!=NULL) leftvalue=left->u.binop.value;
	else if(left->typ==Id&&left->u.id.value!=NULL) 
	{
		leftvalue=left->u.id.value;
		//printf("%d\n",leftvalue->u.Const.value.i);
	}
	else if(left->typ==Array&&left->u.array.value!=NULL) leftvalue=left->u.array.value;
	else if(left->typ==Unary&&left->u.unary.value!=NULL)leftvalue=left->u.unary.value;
	else if(left->typ==Ternary&&left->u.ternary.value!=NULL)leftvalue=left->u.ternary.value;
	else if(left->typ==Cast&&left->u.cast.value!=NULL)leftvalue=left->u.cast.value;
	else if(left->typ==ImplicitCast&&left->u.cast.value!=NULL)leftvalue=left->u.implicitcast.value;
	else if(left->typ==Unary&&left->u.unary.value!=NULL)leftvalue=left->u.unary.value;
	else if(left->typ==Call&&left->u.call.name->u.id.value!=NULL)leftvalue=left->u.call.name->u.id.value;
	else leftvalue=left;
	if(right->typ==Binop&&right->u.binop.value!=NULL) rightvalue=right->u.binop.value;
	else if(right->typ==Id&&right->u.id.value!=NULL) rightvalue=right->u.id.value;
	else if(right->typ==Array&&right->u.array.value!=NULL) rightvalue=right->u.array.value;
	else if(right->typ==Unary&&right->u.unary.value!=NULL)rightvalue=right->u.unary.value;
	else if(right->typ==Ternary&&right->u.ternary.value!=NULL)rightvalue=right->u.ternary.value;
	else if(right->typ==Call&&right->u.call.name->u.id.value!=NULL)rightvalue=right->u.call.name->u.id.value;
	else if(right->typ==Cast&&right->u.cast.value!=NULL)rightvalue=right->u.cast.value;
	else if(right->typ==ImplicitCast&&right->u.implicitcast.value!=NULL)rightvalue=right->u.implicitcast.value;
	else if(right->typ==Call&&right->u.call.name->u.id.value!=NULL)rightvalue=right->u.call.name->u.id.value;
	else rightvalue=right;
	assert(leftvalue);
	assert(rightvalue);
	switch(opcode) {
	case LS:
	case RS:
		break;
	default:
		UsualBinaryConversions(&leftvalue, &rightvalue);
	}

	assert(leftvalue);
	assert(rightvalue);

	ltype  = NodeDataType(left);
	rtype  = NodeDataType(right);

	if (ltype == NULL) {
		PrintNode(stdout, left, 0);
		printf("\n");
	}

	assert(left);
	assert(right);
	assert(ltype);
	assert(rtype);

	switch(opcode) {
 /* case '=':
	  return AlterFlow(node->u.binop.left,rightvalue,v);*/
  case '+':
	  /* Canonicalize PTR + INT expressions */    
	  node->u.binop.type = ltype;

	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  if (NodeTypeIsSint(leftvalue))
		  {
			  int lval = NodeConstantSintValue(leftvalue),
				  rval = NodeConstantSintValue(rightvalue);
			  NodeSetSintValue(node, lval + rval);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  {
			  unsigned int lval = NodeConstantUintValue(leftvalue),
				  rval = NodeConstantUintValue(rightvalue);

			  NodeSetUintValue(node, lval + rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  {
			  long lval = NodeConstantSlongValue(leftvalue),
				  rval = NodeConstantSlongValue(rightvalue);

			  NodeSetSlongValue(node, lval + rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  {
			  unsigned long lval = NodeConstantUlongValue(leftvalue),
				  rval = NodeConstantUlongValue(rightvalue);

			  NodeSetUlongValue(node, lval + rval);
		  }

		  else if (NodeTypeIsFloat(leftvalue))
		  { 
			  float lval = NodeConstantFloatValue(leftvalue),
				  rval = NodeConstantFloatValue(rightvalue);

			  NodeSetFloatValue(node, lval + rval);
		  }
		  else if (NodeTypeIsDouble(leftvalue))
		  { 
			  double lval = NodeConstantDoubleValue(leftvalue),
				  rval = NodeConstantDoubleValue(rightvalue);

			  NodeSetDoubleValue(node, lval + rval);
		  }
	  }

	  break;

  case '-':
	  node->u.binop.type = ltype;

	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  if (NodeTypeIsSint(leftvalue))
		  { int lval = NodeConstantSintValue(leftvalue),
		  rval = NodeConstantSintValue(rightvalue);

		  NodeSetSintValue(node, lval - rval);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  { unsigned int lval = NodeConstantUintValue(leftvalue),
		  rval = NodeConstantUintValue(rightvalue);

		  NodeSetUintValue(node, lval - rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  { long lval = NodeConstantSlongValue(leftvalue),
		  rval = NodeConstantSlongValue(rightvalue);

		  NodeSetSlongValue(node, lval - rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  { unsigned long lval = NodeConstantUlongValue(leftvalue),
		  rval = NodeConstantUlongValue(rightvalue);

		  NodeSetUlongValue(node, lval - rval);
		  }

		  else if (NodeTypeIsFloat(leftvalue))
		  { float lval = NodeConstantFloatValue(leftvalue),
		  rval = NodeConstantFloatValue(rightvalue);

		  NodeSetFloatValue(node, lval - rval);
		  }
		  else if (NodeTypeIsDouble(leftvalue))
		  { double lval = NodeConstantDoubleValue(leftvalue),
		  rval = NodeConstantDoubleValue(rightvalue);

		  NodeSetDoubleValue(node, lval - rval);
		  }
		  //node=node->u.binop.value;
	  }
	  break;

  case '*':

	  node->u.binop.type = ltype;

	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  if (NodeTypeIsSint(leftvalue))
		  { int lval = NodeConstantSintValue(leftvalue),
		  rval = NodeConstantSintValue(rightvalue);

		  NodeSetSintValue(node, lval * rval);
		  // printf("%d\n",leftvalue->u.Const.value.i);
		  // printf("%d\n",rightvalue->u.Const.value.i);
		  // printf("%d\n",node->u.binop.value->u.Const.value.i);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  { unsigned int lval = NodeConstantUintValue(leftvalue),
		  rval = NodeConstantUintValue(rightvalue);

		  NodeSetUintValue(node, lval * rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  { long lval = NodeConstantSlongValue(leftvalue),
		  rval = NodeConstantSlongValue(rightvalue);

		  NodeSetSlongValue(node, lval * rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  { unsigned long lval = NodeConstantUlongValue(leftvalue),
		  rval = NodeConstantUlongValue(rightvalue);

		  NodeSetUlongValue(node, lval * rval);
		  }

		  else if (NodeTypeIsFloat(leftvalue))
		  { float lval = NodeConstantFloatValue(leftvalue),
		  rval = NodeConstantFloatValue(rightvalue);

		  NodeSetFloatValue(node, lval * rval);
		  }
		  else if (NodeTypeIsDouble(leftvalue))
		  { double lval = NodeConstantDoubleValue(leftvalue),
		  rval = NodeConstantDoubleValue(rightvalue);

		  NodeSetDoubleValue(node, lval * rval);
		  }
		  //node=node->u.binop.value;
	  }
	  break;

  case '/':

	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  if (NodeTypeIsSint(leftvalue))
		  { int lval = NodeConstantSintValue(leftvalue),
		  rval = NodeConstantSintValue(rightvalue);

		  if (rval == 0)
			  SyntaxErrorCoord(node->coord,
			  "attempt to divide constant by 0");
		  else
			  NodeSetSintValue(node, lval / rval);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  { unsigned int lval = NodeConstantUintValue(leftvalue),
		  rval = NodeConstantUintValue(rightvalue);

		  if (rval == 0)
			  SyntaxErrorCoord(node->coord,
			  "attempt to divide constant by 0");
		  else
			  NodeSetUintValue(node, lval / rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  { long lval = NodeConstantSlongValue(leftvalue),
		  rval = NodeConstantSlongValue(rightvalue);

		  if (rval == 0)
			  SyntaxErrorCoord(node->coord,
			  "attempt to divide constant by 0");
		  else
			  NodeSetSlongValue(node, lval / rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  { unsigned long lval = NodeConstantUlongValue(leftvalue),
		  rval = NodeConstantUlongValue(rightvalue);

		  if (rval == 0)
			  SyntaxErrorCoord(node->coord,
			  "attempt to divide constant by 0");
		  else
			  NodeSetUlongValue(node, lval / rval);
		  }

		  else if (NodeTypeIsFloat(leftvalue))
		  { float lval = NodeConstantFloatValue(leftvalue),
		  rval = NodeConstantFloatValue(rightvalue);

		  if (rval == 0)
			  SyntaxErrorCoord(node->coord,
			  "attempt to divide constant by 0");
		  else
			  NodeSetFloatValue(node, lval / rval);
		  }
		  else if (NodeTypeIsDouble(leftvalue))
		  { double lval = NodeConstantDoubleValue(leftvalue),
		  rval = NodeConstantDoubleValue(rightvalue);

		  if (rval == 0)
			  SyntaxErrorCoord(node->coord,
			  "attempt to divide constant by 0");
		  else
			  NodeSetDoubleValue(node, lval / rval);
		  }
		  //node->u.binop.type = ltype;
	  }

	  break;

  case '%':
	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  if (NodeTypeIsSint(leftvalue))
		  { int lval = NodeConstantSintValue(leftvalue),
		  rval = NodeConstantSintValue(rightvalue);

		  NodeSetSintValue(node, lval % rval);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  { unsigned int lval = NodeConstantUintValue(leftvalue),
		  rval = NodeConstantUintValue(rightvalue);

		  NodeSetUintValue(node, lval % rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  { long lval = NodeConstantSlongValue(leftvalue),
		  rval = NodeConstantSlongValue(rightvalue);

		  NodeSetSlongValue(node, lval % rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  { unsigned long lval = NodeConstantUlongValue(leftvalue),
		  rval = NodeConstantUlongValue(rightvalue);

		  NodeSetUlongValue(node, lval % rval);
		  }
		  // node=node->u.binop.value;
	  }
	  node->u.binop.type = ltype;
	  break;
  case LS:
	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  unsigned long rval = NodeConstantIntegralValue(rightvalue);

		  if (NodeTypeIsSint(leftvalue))
		  { int lval = NodeConstantSintValue(leftvalue);

		  NodeSetSintValue(node, lval << rval);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  { unsigned int lval = NodeConstantUintValue(leftvalue);

		  NodeSetUintValue(node, lval << rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  { long lval = NodeConstantSlongValue(leftvalue);

		  NodeSetSlongValue(node, lval << rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  { unsigned long lval = NodeConstantUlongValue(leftvalue);

		  NodeSetUlongValue(node, lval << rval);
		  }
		  // node=node->u.binop.value;
	  }
	  node->u.binop.type = ltype;
	  break;
  case RS:
	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  unsigned long rval = NodeConstantIntegralValue(rightvalue);

		  if (NodeTypeIsSint(leftvalue))
		  { int lval = NodeConstantSintValue(leftvalue);

		  NodeSetSintValue(node, lval >> rval);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  { unsigned int lval = NodeConstantUintValue(leftvalue);

		  NodeSetUintValue(node, lval >> rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  { long lval = NodeConstantSlongValue(leftvalue);

		  NodeSetSlongValue(node, lval >> rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  { unsigned long lval = NodeConstantUlongValue(leftvalue);

		  NodeSetUlongValue(node, lval >> rval);
		  }
		  // node=node->u.binop.value;
	  }
	  node->u.binop.type = ltype;

	  break;
  case '&':
	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  if (NodeTypeIsSint(leftvalue))
		  { int lval = NodeConstantSintValue(leftvalue),
		  rval = NodeConstantSintValue(rightvalue);

		  NodeSetSintValue(node, lval & rval);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  { unsigned int lval = NodeConstantUintValue(leftvalue),
		  rval = NodeConstantUintValue(rightvalue);

		  NodeSetUintValue(node, lval & rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  { long lval = NodeConstantSlongValue(leftvalue),
		  rval = NodeConstantSlongValue(rightvalue);

		  NodeSetSlongValue(node, lval & rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  { unsigned long lval = NodeConstantUlongValue(leftvalue),
		  rval = NodeConstantUlongValue(rightvalue);

		  NodeSetUlongValue(node, lval & rval);
		  }
		  //  node=node->u.binop.value;
	  }
	  node->u.binop.type = ltype;
	  break;
  case '^':
	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  if (NodeTypeIsSint(leftvalue))
		  { int lval = NodeConstantSintValue(leftvalue),
		  rval = NodeConstantSintValue(rightvalue);

		  NodeSetSintValue(node, lval ^ rval);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  { unsigned int lval = NodeConstantUintValue(leftvalue),
		  rval = NodeConstantUintValue(rightvalue);

		  NodeSetUintValue(node, lval ^ rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  { long lval = NodeConstantSlongValue(leftvalue),
		  rval = NodeConstantSlongValue(rightvalue);

		  NodeSetSlongValue(node, lval ^ rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  { unsigned long lval = NodeConstantUlongValue(leftvalue),
		  rval = NodeConstantUlongValue(rightvalue);

		  NodeSetUlongValue(node, lval ^ rval);
		  }
		  // node=node->u.binop.value;
	  }
	  node->u.binop.type = ltype;
	  break;
  case '|':
	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  {
		  if (NodeTypeIsSint(leftvalue))
		  { int lval = NodeConstantSintValue(leftvalue),
		  rval = NodeConstantSintValue(rightvalue);

		  NodeSetSintValue(node, lval | rval);
		  }
		  else if (NodeTypeIsUint(leftvalue))
		  { unsigned int lval = NodeConstantUintValue(leftvalue),
		  rval = NodeConstantUintValue(rightvalue);

		  NodeSetUintValue(node, lval | rval);
		  }
		  else if (NodeTypeIsSlong(leftvalue))
		  { long lval = NodeConstantSlongValue(leftvalue),
		  rval = NodeConstantSlongValue(rightvalue);

		  NodeSetSlongValue(node, lval | rval);
		  }
		  else if (NodeTypeIsUlong(leftvalue))
		  { unsigned long lval = NodeConstantUlongValue(leftvalue),
		  rval = NodeConstantUlongValue(rightvalue);

		  NodeSetUlongValue(node, lval | rval);
		  }
		  //  node=node->u.binop.value;
	  }
	  node->u.binop.type = ltype;
	  break;

  case ANDAND:
	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  { int lval = IsConstantZero(leftvalue),
	  rval = IsConstantZero(rightvalue);

	  NodeSetSintValue(node, !lval && !rval);
	  }
	  node->u.binop.type = PrimSint;
	  break;
  case OROR:
	  if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
	  { int lval = IsConstantZero(leftvalue),
	  rval = IsConstantZero(rightvalue);

	  NodeSetSintValue(node, !lval || !rval);
	  // node=node->u.binop.value;
	  }
	  node->u.binop.type = PrimSint;
	  break;
  default:
	  break;
	}
	//if(node->u.binop.value!=NULL)printf("%d\n",node->u.binop.value->u.Const.value.i);
	return v;
}

//����Ƚ�����
PRIVATE inline FlowValue TransformComparison(Node *node, FlowValue v)
{

	Node *left = UsualUnaryConversions(node->u.binop.left, FALSE), *right = UsualUnaryConversions(node->u.binop.right, FALSE);
	Node *ltype = NodeDataType(left), *rtype = NodeDataType(right);
	Node *leftvalue = GetValue(left), *rightvalue = GetValue(right);
	OpType  opcode = node->u.binop.op;
	Bool cmp = FALSE;
	double lval = 0, rval = 0;

	assert(node && node->typ == Binop);

	UsualBinaryConversions(&leftvalue, &rightvalue);
	node->u.binop.type = PrimSint;

	if (IsArithmeticType(ltype) && IsArithmeticType(rtype)) 
	{
		if (NodeIsConstant(leftvalue) && NodeIsConstant(rightvalue))
		{
			if (NodeTypeIsSint(leftvalue))
			{
				lval = NodeConstantSintValue(leftvalue);
				rval = NodeConstantSintValue(rightvalue);
			}
			else if (NodeTypeIsUint(leftvalue))
			{
				lval = NodeConstantUintValue(leftvalue);
				rval = NodeConstantUintValue(rightvalue);
			}
			else if (NodeTypeIsSlong(leftvalue))
			{ 
				lval = NodeConstantSlongValue(leftvalue);
				rval = NodeConstantSlongValue(rightvalue);
			}
			else if (NodeTypeIsUlong(leftvalue))
			{ 
				lval = NodeConstantUlongValue(leftvalue);
				rval = NodeConstantUlongValue(rightvalue);
			}
			else if (NodeTypeIsFloat(leftvalue))
			{ 
				lval = NodeConstantFloatValue(leftvalue);
				rval = NodeConstantFloatValue(rightvalue);
			}
			else if (NodeTypeIsDouble(leftvalue))
			{ 
				lval = NodeConstantDoubleValue(leftvalue);
				rval = NodeConstantDoubleValue(rightvalue);
			}
		}
	}
	else/* if (IsPointerType(ltype) && IsPointerType(rtype))*///SPL��֧��ָ�룬ָ�����Ͳ����ܵ�������
		UNREACHABLE;// ע�⣺���������ڴ˽�����

	switch(opcode)
	{
	case '<':
		NodeSetSintValue(node, lval < rval);
		break;
	case LE: // '<='
		NodeSetSintValue(node, lval <= rval);
		break;
	case '>':
		NodeSetSintValue(node, lval > rval);
		break;
	case GE: // '>='
		NodeSetSintValue(node, lval >= rval);
		break;
	case EQ: // '=='
		NodeSetSintValue(node, lval == rval);
		break;
	case NE:
		NodeSetSintValue(node, lval != rval);
		break;
	default:
		fprintf(stdout, "Internal Error: Unrecognized comparison operator\n");
		assert(FALSE);
	}

	node->u.binop.left  = left;
	node->u.binop.right = right;
	v.undefined=FALSE;

	return v;
}

//����һԪ���㣬������Ҫremove any decls referenced by &��ȡ��ַ����
PRIVATE inline FlowValue TransformUnary(Node *node, unaryNode *u, FlowValue v)
{
	Node *value = GetValue(u->expr);

	switch (u->op) {
		/* Must be arithmetic.  Apply usual conversions */
	case UMINUS: //ȡ������
		if (NodeIsConstant(value))
		{
			if (NodeTypeIsSint(value))
			{ 
				int eval = NodeConstantSintValue(value);
				NodeSetSintValue(node, 0 - eval);
			}
			else if (NodeTypeIsUint(value))
			{ 
				unsigned int eval = NodeConstantUintValue(value);
				NodeSetUintValue(node, 0 - eval);
			}
			else if (NodeTypeIsSlong(value))
			{ 
				long eval = NodeConstantSlongValue(value);
				NodeSetSlongValue(node, 0 - eval);
			}
			else if (NodeTypeIsUlong(value))
			{ 
				unsigned long eval = NodeConstantUlongValue(value);
				NodeSetUlongValue(node, 0 - eval);
			}
			else if (NodeTypeIsFloat(value))
			{ 
				float eval = NodeConstantFloatValue(value);
				NodeSetFloatValue(node, 0 - eval);
			}
			else if (NodeTypeIsDouble(value))
			{ 
				double eval = NodeConstantDoubleValue(value);
				NodeSetDoubleValue(node, 0 - eval);
			}
		}
		break;
	case UPLUS:
		return v;

	case '!':
		value = UsualUnaryConversions(value, FALSE);
		u->type = NodeDataType(value);
		u->type = PrimSint;

		if (NodeIsConstant(value))
		{
			if (NodeTypeIsSint(value))
			{ 
				int eval = NodeConstantSintValue(value);
				NodeSetSintValue(node, eval == 0);
			}
			else if (NodeTypeIsUint(value))
			{ 
				unsigned int eval = NodeConstantUintValue(value);
				NodeSetSintValue(node, eval == 0);
			}
			else if (NodeTypeIsSlong(value))
			{ 
				long eval = NodeConstantSlongValue(value);
				NodeSetSintValue(node, eval == 0);
			}
			else if (NodeTypeIsUlong(value))
			{ 
				unsigned long eval = NodeConstantUlongValue(value);
				NodeSetSintValue(node, eval == 0);
			}
			else if (NodeTypeIsFloat(value))
			{ 
				float eval = NodeConstantFloatValue(value);
				NodeSetSintValue(node, eval == 0);
			}
			else if (NodeTypeIsDouble(value))
			{ 
				double eval = NodeConstantDoubleValue(value);
				NodeSetSintValue(node, eval == 0);
			}
		}
		break;

		/* Must be integral.  Apply usual conversions. */
	case '~':
		value= UsualUnaryConversions(value, FALSE);
		u->type = NodeDataType(value);
		if (NodeIsConstant(value))
		{
			if (NodeTypeIsSint(value))
			{ 
				int eval = NodeConstantSintValue(value);
				NodeSetSintValue(node, ~eval);
			}
			else if (NodeTypeIsUint(value))
			{ 
				unsigned int eval = NodeConstantUintValue(value);
				NodeSetUintValue(node, ~eval);
			}
			else if (NodeTypeIsSlong(value))
			{ long eval = NodeConstantSlongValue(value);
			NodeSetSlongValue(node, ~eval);
			}
			else if (NodeTypeIsUlong(value))
			{ 
				unsigned long eval = NodeConstantUlongValue(value);
				NodeSetUlongValue(node, ~eval);
			}
		}
		break;

		/* Must be scalar modifiable lval.  Apply usual conversions */
	case PREINC:
		value= UsualUnaryConversions(value, FALSE);
		u->type = NodeDataType(value);
		if (NodeIsConstant(value))
		{
			if (NodeTypeIsSint(value))
			{ 
				int eval = NodeConstantSintValue(value);
				NodeSetSintValue(node, eval+1);
				NodeSetSintValue(value,eval+1);
			}
			else if (NodeTypeIsUint(value))
			{ 
				unsigned int eval = NodeConstantUintValue(value);
				NodeSetUintValue(node, eval+1);
				NodeSetUintValue(value, eval+1);
			}
			else if (NodeTypeIsSlong(value))
			{ 
				long eval = NodeConstantSlongValue(value);
				NodeSetSlongValue(node, eval+1);
				NodeSetSlongValue(value, eval+1);
			}
			else if (NodeTypeIsUlong(value))
			{ 
				unsigned long eval = NodeConstantUlongValue(value);
				NodeSetUlongValue(node, eval+1);
				NodeSetSlongValue(value, eval+1);
			}
		}
		return AlterFlow(u->expr,value,v);

	case PREDEC:
		value= UsualUnaryConversions(value, FALSE);
		u->type = NodeDataType(value);
		if (NodeIsConstant(value))
		{
			if (NodeTypeIsSint(value))
			{ 
				int eval = NodeConstantSintValue(value);
				NodeSetSintValue(node, eval-1);
				NodeSetSintValue(value,eval-1);
			}
			else if (NodeTypeIsUint(value))
			{ 
				unsigned int eval = NodeConstantUintValue(value);
				NodeSetUintValue(node, eval-1);
				NodeSetUintValue(value, eval-1);
			}
			else if (NodeTypeIsSlong(value))
			{ 
				long eval = NodeConstantSlongValue(value);
				NodeSetSlongValue(node, eval-1);
				NodeSetSlongValue(value, eval-1);
			}
			else if (NodeTypeIsUlong(value))
			{ 
				unsigned long eval = NodeConstantUlongValue(value);
				NodeSetUlongValue(node, eval-1);
				NodeSetSlongValue(value, eval-1);
			}
		}
		return AlterFlow(u->expr,value,v);

	case POSTINC:
		value= UsualUnaryConversions(value, FALSE);
		u->type = NodeDataType(value);
		if (NodeIsConstant(value))
		{
			if (NodeTypeIsSint(value))
			{ 
				int eval = NodeConstantSintValue(value);
				NodeSetSintValue(node, eval);
				NodeSetSintValue(value,eval+1);
			}
			else if (NodeTypeIsUint(value))
			{ 
				unsigned int eval = NodeConstantUintValue(value);
				NodeSetUintValue(node, eval);
				NodeSetUintValue(value, eval+1);
			}
			else if (NodeTypeIsSlong(value))
			{ 
				long eval = NodeConstantSlongValue(value);
				NodeSetSlongValue(node, eval);
				NodeSetSlongValue(value, eval+1);
			}
			else if (NodeTypeIsUlong(value))
			{ 
				unsigned long eval = NodeConstantUlongValue(value);
				NodeSetUlongValue(node, eval);
				NodeSetSlongValue(value, eval+1);
			}
		}
		return AlterFlow(u->expr,value,v);

	case POSTDEC:
		value= UsualUnaryConversions(value, FALSE);
		u->type = NodeDataType(value);
		if (NodeIsConstant(value))
		{
			if (NodeTypeIsSint(value))
			{ 
				int eval = NodeConstantSintValue(value);
				NodeSetSintValue(node, eval);
				NodeSetSintValue(value,eval-1);
			}
			else if (NodeTypeIsUint(value))
			{ 
				unsigned int eval = NodeConstantUintValue(value);
				NodeSetUintValue(node, eval);
				NodeSetUintValue(value, eval-1);
			}
			else if (NodeTypeIsSlong(value))
			{ 
				long eval = NodeConstantSlongValue(value);
				NodeSetSlongValue(node, eval);
				NodeSetSlongValue(value, eval-1);
			}
			else if (NodeTypeIsUlong(value))
			{ 
				unsigned long eval = NodeConstantUlongValue(value);
				NodeSetUlongValue(node, eval);
				NodeSetSlongValue(value, eval-1);
			}
		}
		return AlterFlow(u->expr,value,v);

		/* Memory leak */
	case SIZEOF:
		NodeSetUintValue(node, NodeSizeof(value, u->type));
		u->type = PrimUint; 
		break;

	case ADDRESS:
		if (IsLvalue(value) || IsFunctionType( u->type) || IsArrayType( u->type))
			u->type= MakePtr(EMPTY_TQ,  u->type);
		else
			SyntaxErrorCoord(node->coord, "cannot take the address of a non-lvalue");

		break;

		/* Must be a pointer or Adcl.  Result type is referenced object */
	case INDIR:
		if (u->type->typ == Ptr)
			u->type = u->type->u.ptr.type;
		else if (u->type->typ == Adcl)
			u->type =u->type->u.adcl.type;
		else if (u->type->typ == Fdcl)
			; /* Fdcl automatically becomes Ptr(Fdcl), then INDIR
			  eliminates the Ptr */
		else
			SyntaxErrorCoord(node->coord, "2 cannot dereference non-pointer type");
		break;

	default:
		fprintf(stdout, "Unsupported unary operator \"%s\"\n", Operator[u->op].text);
		assert(FALSE);
	}
	return v;
}

//�����Ԫ����
PRIVATE inline FlowValue TransformBinop(Node *node, FlowValue v)
{
	OpType op = node->u.binop.op;
	if (IsAssignmentOp(op))
		return TransformAssignment(node,v);
	else if (IsArithmeticOp(op))
		return TransformArithmetic(node,v);
	else if (IsComparisonOp(op))
		return TransformComparison(node,v);
	else if(op == '.') 
	{
		if (node->u.binop.left->typ==STRdcl)
		{
			return v;
		}
		else return TransfromDot(node , v);
	}
	else if ( op == ARROW) // �ݲ�����
		return v;
	else 
		return v; 
}

//��ʱֻ����⺯�����ã����Խ�call����滻Ϊ����Node���Ż��ֶ�֮һ�����˴���û����
PRIVATE inline FlowValue TransformCall(Node *node, FlowValue v)
{
	CallLibFunc(node);

	return v;
}

PRIVATE FlowValue TransformConstFlow(Node *node, FlowValue v, Point p, Bool final)
{
	switch (p) 
	{
	case EntryPoint:
		return v;
	case ExitPoint:
		if(final == FALSE) return v;
		switch(node->typ)
		{
		case Const: 
			return v;
		case Id:  
			return TransformId(node,v);
		case Binop: 
			return TransformBinop(node,v);
		case Unary:
			return TransformUnary(node, &node->u.unary, v);
		case Cast:
		case ImplicitCast:
			ConstFoldCast(node);return v;
		case Array: 
			return TransformArray(node,v);
		case Call:  
			if(strcmp(node->u.call.name->u.id.text,"frta")==0){
				gfrtaCallList = JoinLists(gfrtaCallList, MakeNewList(node));
				return v;
			}
			else
				return TransformCall(node,v);
		case Decl:
			return TransformDecl(node,v);
		case Goto: //��������ʱ������goto���
			return v;
		default:
			return v;
		}
	default:
		UNREACHABLE;
	}
}

//��������;
GLOBAL List *PropagateProgram(List *program)
{
	ListMarker marker;
	Node *item = NULL;
	propagatorNode *pNode = HeapNew(propagatorNode);
	gDeclList = NULL;//��ȫ����������Ϊ�� add by wangliang
#ifdef SPL_DEBUG
	printf("\n-----------------------------DataFlow Module-----------------------------\n");
#endif

	//��¼����ast����������ast�Ĳ���
	gProgram = program; 
	pNode->idFlowList = NULL;
	pNode->arrayFlowList = NULL;
	initflow.undefined = FALSE;
	initflow.u.ptr = pNode;

	IterateList(&marker, program);
	//��ȫ�ֱ������������������뵽gDeclList�У���ȫ�ֺ���������뵽gProcList��
	while (NextOnList(&marker, (GenericREF)&item))
	{
		if (item->typ == Decl && (IsScalarType(NodeDataType(item)) || item->u.decl.type->typ == Adcl || item->u.decl.type->typ == Sdcl ||
			item->u.decl.type->typ == Udcl || item->u.decl.type->typ == Edcl || item->u.decl.type->typ == Tdef || item->u.decl.type->typ == Fdcl
			))
		{
			//������ȫ�ֵ�������뵽��������
			initflow.u.ptr = (TransformDecl(item, initflow)).u.ptr;
			gDeclList = JoinLists(gDeclList, MakeNewList(item));
		}
		if (item->typ == Sdcl || item->typ == Udcl || item->typ == Edcl)//�������struct���嵽Globar.h��
		{
			//initflow.u.ptr = (TransformDecl(item, initflow)).u.ptr;
			gDeclList = JoinLists(gDeclList, MakeNewList(item));
		}
		if (item->typ == Proc)
		{
			gProcList = JoinLists(gProcList, MakeNewList(item));
		}
	}

#ifdef SPL_DEBUG
	//PrintConstFlowValue(initflow);
#endif

	//���粻���ڳ�����ڣ��򱨴�
	if(gMainComposite == NULL)
	{
		SyntaxError("Syntax error: There is no Main composite in the program!");
		assert(FALSE);
	}
	IteratePropagatorDataFlow(gMainComposite, initflow, Forwards, MeetConstFlow, EqualConstFlow, TransformConstFlow);
	
	return program;
}

void RWV_astwalk(Node *n,FlowValue v);
void RWV_listwalk(List *l,FlowValue v)
{
	ListMarker _listwalk_marker; Node *_listwalk_ref; 
	IterateList(&_listwalk_marker, l); 
	while (NextOnList(&_listwalk_marker, (GenericREF)&_listwalk_ref)) { 
		if (_listwalk_ref) { RWV_astwalk(_listwalk_ref,v);}                     
		SetCurrentOnList(&_listwalk_marker, (Generic *)_listwalk_ref); 
	}	
}

void RWV_astwalk(Node *n,FlowValue v)
{
	switch ((n)->typ) { 
	case Const:         break; 
	case Id:		  {
						constIdNode *cn= FindIdNode(n->u.id.decl,v);
						Node *id_value = NULL;
						if (cn==NULL) break;
						id_value=GetValue(cn->n->u.id.value);
						n->u.id.value = id_value;
						if(id_value!=NULL&&id_value->typ == Const)
						{//ֱ�ӽ���id��value�滻ԭ����id�ڵ� zww-20120314
							n->typ = Const;
							n->u.Const.type=id_value->u.Const.type;
							n->u.Const.text=NULL;
							n->u.Const.value.d=id_value->u.Const.value.d;
							n->u.Const.value.f=id_value->u.Const.value.f;
							n->u.Const.value.s=id_value->u.Const.value.s;
							n->u.Const.value.ul=id_value->u.Const.value.ul;
							n->u.Const.value.u=id_value->u.Const.value.u;
							n->u.Const.value.l=id_value->u.Const.value.l;
							n->u.Const.value.i=id_value->u.Const.value.i;
						}
						break;
					   }
	case Ternary:       if ((n)->u.ternary.cond) {RWV_astwalk((n)->u.ternary.cond,v);} if ((n)->u.ternary.true_) {RWV_astwalk((n)->u.ternary.true_,v);} if ((n)->u.ternary.false_) {RWV_astwalk((n)->u.ternary.false_,v);} break;
	case Binop:         
		{
			OpType op = n->u.binop.op;			
			Node *l=NULL,*r=NULL;
			if(IsAssignmentOp(op))
			{//Ҫ��Ӷ���ߵĴ���
				if ((n)->u.binop.left) {RWV_astwalk((n)->u.binop.left,v);}
				if ((n)->u.binop.right) {RWV_astwalk((n)->u.binop.right,v);}
			}
			else 
			{
				if ((n)->u.binop.right) {RWV_astwalk((n)->u.binop.right,v);} 
				if ((n)->u.binop.left) {RWV_astwalk((n)->u.binop.left,v);}
				l=GetValue((n)->u.binop.left);
				r=GetValue((n)->u.binop.right);
				if(l!=NULL&&r!=NULL&&l->typ==Const&&r->typ==Const){ 
					TransformArithmetic(n,v);
					break;
				}
			}
			break;
		}
	case Unary:         
		{	
			Node *tmp=NULL; 
			if ((n)->u.unary.expr)
			{
				RWV_astwalk((n)->u.unary.expr,v);
				tmp=GetValue((n)->u.unary.expr);
				if(tmp!=NULL&&tmp->typ==Const)
					TransformUnary(n,&n->u.unary,v);
				break;
			}
			else break; 
		}
	case Cast:          if ((n)->u.cast.expr) {RWV_astwalk((n)->u.cast.expr,v);} break; 
	case Comma:         if ((n)->u.comma.exprs) {RWV_listwalk((n)->u.comma.exprs,v);} break; 
	case Array:         
		{
			Node *tmpCGNode = NULL;
			while (n->u.array.name->typ == Binop)
			{
				tmpCGNode = n->u.array.name;
				 n->u.array.name = n->u.array.name->u.binop.left;
			 }
			if (FindItem(DeclArrayListInWork,n->u.array.name->u.id.decl) == NULL && NodeDeclLocation(n->u.array.name->u.id.decl)!=T_TOP_DECL)
			{
				Node *arrayDecl = NULL;
				Node *init = NewNode(Initializer);//����һ�������ʼ���Ľڵ㣻
				constArrayNode *tmpNode = NULL;
				List *list = ((propagatorNode*)(v.u.ptr))->arrayFlowList;
				char *name = n->u.array.name;
				Node *decl = NULL;
				int i;
				Node *dtype = NULL;
				tmpNode=FindArrayNode(n,v);//���������в���
				if(tmpNode!=NULL) 
				{
					arrayDecl = NodeCopy(n->u.array.name->u.id.decl,Subtree);
					dtype = NodeDataType(arrayDecl->u.decl.type);
					decl = arrayDecl;
					assert(arrayDecl->typ == Decl);
					for(i = 0;i<tmpNode->num;i++)
					{
						if (tmpNode->element[i] == NULL){//����Ĭ�ϳ�ֵΪ0 add by wangliang
							tmpNode->element[i] = MakeConstSint(0);
						}
						assert(tmpNode->element[i]!=NULL);
						init->u.initializer.exprs = AppendItem(init->u.initializer.exprs, tmpNode->element[i]);					
					}
					assert(init);
					assert(init->typ == Initializer);
					assert(ListLength(init->u.initializer.exprs)==tmpNode->num);
					arrayDecl->u.decl.prim_init= NodeCopy(init,NodeOnly);
					arrayDecl->u.decl.init = SemCheckInitList(arrayDecl, dtype, NodeCopy(arrayDecl->u.decl.prim_init,NodeOnly), TRUE);
					DeclArrayListInWork = AppendItem(DeclArrayListInWork,arrayDecl);
				}
			}

			if((n)->u.array.type) {RWV_astwalk((n)->u.array.type,v);}
			if((n)->u.array.name) {RWV_astwalk((n)->u.array.name,v);}
			if(n->u.array.dims) {RWV_listwalk(n->u.array.dims,v);}		
			TransformArray(n,v);			
				 
			
			break;
		}
	case Call:           
		if ((n)->u.call.args) {RWV_listwalk((n)->u.call.args,v);}
		break; 
	case Initializer:   if ((n)->u.initializer.exprs) {RWV_listwalk((n)->u.initializer.exprs,v);} break; 
	case ImplicitCast:  if ((n)->u.implicitcast.expr) {RWV_astwalk((n)->u.implicitcast.expr,v);} break; 
	case Label:         if ((n)->u.label.stmt) {RWV_astwalk((n)->u.label.stmt,v);} break; 
	case Switch:        if ((n)->u.Switch.expr) {RWV_astwalk((n)->u.Switch.expr,v);} if ((n)->u.Switch.stmt) {RWV_astwalk((n)->u.Switch.stmt,v);} if ((n)->u.Switch.cases) {RWV_listwalk((n)->u.Switch.cases,v);}break; 
	case Case:          if ((n)->u.Case.expr) {RWV_astwalk((n)->u.Case.expr,v);} if ((n)->u.Case.stmt) {RWV_astwalk((n)->u.Case.stmt,v);} break; 
	case Default:       if ((n)->u.Default.stmt) {RWV_astwalk((n)->u.Default.stmt,v);} break; 
	case If:            if ((n)->u.If.expr) {RWV_astwalk((n)->u.If.expr,v);} if ((n)->u.If.stmt) {RWV_astwalk((n)->u.If.stmt,v);} break; 
	case IfElse:        if ((n)->u.IfElse.expr) {RWV_astwalk((n)->u.IfElse.expr,v);} if ((n)->u.IfElse.true_) {RWV_astwalk((n)->u.IfElse.true_,v);} if ((n)->u.IfElse.false_) {RWV_astwalk((n)->u.IfElse.false_,v);} break; 
	case While:         if ((n)->u.While.expr) {RWV_astwalk((n)->u.While.expr,v);} if ((n)->u.While.stmt) {RWV_astwalk((n)->u.While.stmt,v);} break; 
	case Do:            if ((n)->u.Do.stmt) {RWV_astwalk((n)->u.Do.stmt,v);} if ((n)->u.Do.expr) {RWV_astwalk((n)->u.Do.expr,v);} break; 
	case For:           if ((n)->u.For.init) {RWV_astwalk((n)->u.For.init,v);}if ((n)->u.For.cond) {RWV_astwalk((n)->u.For.cond,v);} if ((n)->u.For.next) {RWV_astwalk((n)->u.For.next,v);} 
						if ((n)->u.For.stmt) {RWV_astwalk((n)->u.For.stmt,v);} break; 
	case Goto:          break; 
	case Continue:      break; 
	case Break:         break; 
	case Return:        if ((n)->u.Return.expr) {RWV_astwalk((n)->u.Return.expr,v);} break; //spl����
	case Block:         if ((n)->u.Block.decl) {RWV_listwalk((n)->u.Block.decl,v);} if ((n)->u.Block.stmts) {RWV_listwalk((n)->u.Block.stmts,v);} break; 
	case Prim:          break; 
	case Adcl:          if((n)->u.adcl.type){RWV_astwalk((n)->u.adcl.type,v);}if((n)->u.adcl.dim){RWV_astwalk((n)->u.adcl.dim,v);}break; 
	case Decl:          if ((n)->u.decl.type) {RWV_astwalk((n)->u.decl.type,v);}if ((n)->u.decl.init) {RWV_astwalk((n)->u.decl.init,v);}  break;
	case Text:          break;   
	default:            break; 
	}
}
GLOBAL void ReplaceWorkVar(Node *node,FlowValue v)
{//��work���õ��ı���ֵȫ��ȡ���������滻(δ�滻)
	assert(node);
	//	PrintConstFlowValue(v);
	DeclArrayListInWork = NULL;
	RWV_astwalk(node,v);
	node->u.Block.decl = JoinLists(node->u.Block.decl,DeclArrayListInWork);
}

