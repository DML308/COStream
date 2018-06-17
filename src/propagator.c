#include"propagator.h"


List *DeclArrayListInWork = NULL;//用于在work中构造对数组各个元素的赋值语句（该数组的定义一般在param，var中）
extern operatorNode *tempoperatornode;
GLOBAL Bool DimFlag = FALSE; //cwb 标识param变量是否是数组空间维度
GLOBAL List *gfrtaCallList = NULL;//用于frta调用

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
{  //检查各维的长度
	int len = 0;
	constIdNode *idNode = NULL;
	Node *value = NULL;

	assert(node && node->typ == Adcl);
	if(node->u.adcl.dim->typ == Const) // 维数是常量
		len = NodeConstantIntegralValue(node->u.adcl.dim);
	else  // 维数是变量
	{
		//assert(node->u.adcl.dim->typ == Id); // 目前只考虑数组维数是一个id的情况
		//idNode = FindIdNode(node->u.adcl.dim->u.id.decl, v);
		//assert(idNode);
		//value = idNode->n->u.id.value;//取结点的值
		value = GetValue(node->u.adcl.dim);//zww:12.2.17修改
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

	if(node->u.decl.init!= NULL && node->u.decl.init->typ == Const) //值是定的
		itemId->undefine = FALSE;
	else//值不定，即它的值可能是一个表达式或为空
		itemId->undefine = TRUE;

	return itemId;
}

//最多支持2维数组
GLOBAL constArrayNode *MakeConstArrayNode(int dim, int *lenPerDim, Node *node)
{
	constArrayNode *itemArray = HeapNew(constArrayNode);
	int i = 0, num = 1;

	assert(lenPerDim && node && node->typ == Decl );//zww 支持多维 <---(lenPerDim && node && node->typ == Decl && dim <= 2) 12.2.10
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
	//处理数组的初始化
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

PRIVATE void InitConstMultArrayNode(int dim, int *lenPerDim, int num, constArrayNode *itemArray, Node *init)  //zww： 处理多维数组	 12.2.10
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

//将变量的声明转化成constIdNode类型,并插入到数据流中，主要在decl结点的传递函数中用到
PRIVATE inline FlowValue InitDeclNode(Node *node, FlowValue v)
{
	constIdNode *tmp = FindIdNode(node, v);

	assert(node && (node->typ == Id || node->typ == Decl) );

	if (tmp == NULL)
	{
		constIdNode *itemId = MakeConstIdNode(node);
		//将新建的 itemId 添加到数据流中
		((propagatorNode*)(v.u.ptr))->idFlowList = AppendItem(((propagatorNode*)(v.u.ptr))->idFlowList, itemId);
	}
	else//该变量是在循环内部定义的
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
	Node *elementtype = NULL;//记录数组各个元素的类型
	propagatorNode *tmp2 = (propagatorNode *)(v.u.ptr);
	List *list = tmp2->arrayFlowList;
	List *tmpList = list;

	assert(node && node->typ == Decl);
	tmp = node->u.decl.type;
	assert(tmp && tmp->typ == Adcl);
	init = node->u.decl.init;

	//zww:12.03.02,常量传播对于字符数组的初始化问题暂不处理，即类似于char s[10]="abcdefghi"   ;
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
		if(node->u.decl.name == tmpNode->name && node->u.decl.type == tmpNode->decl) // node->u.decl.name == tmpanode->name 这句话必须要
			break;
		else 
			tmpNode = NULL;
		list = Rest(list);
	}

	if(tmpNode == NULL)
	{
		int dim = 0, num = 1, i = 0;
		int *lenPerDim = NULL;//zww 修改支持高维 12.2.13
		while(tmp->typ == Adcl)
		{
			++dim;
			tmp = tmp->u.adcl.type;
		}
		lenPerDim = (int *)malloc(sizeof(int)*dim);//zww 修改支持高维 12.2.13
		tmp = node->u.decl.type;
		i=0;
		while(tmp->typ == Adcl)	//zww 修改支持高维 12.2.13
		{
			lenPerDim[i++] = CheckAdcl(tmp);
			tmp = tmp->u.adcl.type;
		}
		itemArray = MakeConstArrayNode(dim, lenPerDim, node);
		num = itemArray->num;
		if(dim<=2)InitConstArrayNode(dim, lenPerDim, num, itemArray, init);	//zww ：添加处理多维 12.2.10
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
	int *pos = NULL, tmp = 0, i = 0, j = 0, place = 0;//place指的就是元素在一位中的位置

	
	//assert(node->typ == Id || node->typ == Array);

	if(node->typ == Id) 
	{
		assert(value && value->typ == Const);
		itemId = FindIdNode(node->u.id.decl, v);
		if(itemId == NULL) return v;//zww-20120314 如果变量不在数据流中这直接返回（state中的变量是不在数据流中的）
		assert(itemId);
		node->u.id.value = value;
		itemId->n->u.id.value = value;
		itemId->undefine = FALSE;
		return v;
	}
	else if(node->typ == Array)
	{
		assert(value && value->typ == Const);
		itemArray = FindArrayNode(node,v);//下面对数组元素定位
		if(itemArray == NULL) return v;//zww-20120314 如果变量不在数据流中这直接返回（state中的变量是不在数据流中的）
		assert(itemArray);
		assert(node->u.array.name->typ==Id);//阻止像sa.a[i]这样的节点
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

		node->u.array.value = value;  //修改过12.2.10
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

//比较两个数据流是否相等
PRIVATE inline Bool EqualConstFlow(FlowValue dest, FlowValue src)
{
	//一般来说，dest.undefined == TRUE, src.undefined == FALSE，因为对于常量传播来说，src必须确定，而dest一般不确定，要不就没必要流进去了。
	if (dest.undefined)
		return src.undefined;
	else //2013 4-16 YQJ 增加else结构 用于 && 运算
		return dest.undefined;

	PrintConstFlowValue(dest);
	UNREACHABLE;
}

//数据流进行meet操作
PRIVATE inline FlowValue MeetConstFlow(FlowValue dest, FlowValue src)
{
	//同EqualConstFlow操作
	if (dest.undefined == TRUE)
		return src;
	else //2013 4-16 YQJ 增加else结构 用于 && 运算
		return dest;
	
	PrintConstFlowValue(dest);
	UNREACHABLE;
}

//查找数据流id节点
PRIVATE inline constIdNode *FindIdNode(Node *node, FlowValue v)
{
	ListMarker marker;
	constIdNode *item = NULL;
	//取出id数据流
	List *list = ((propagatorNode*)(v.u.ptr))->idFlowList;
	
	//assert(node && node->typ == Decl); // 12.13 数据流进work
	IterateList(&marker, list);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		if(node == item->n->u.id.decl) 
			return item;
	}

	return NULL;
}

//查找数据流array节点
PRIVATE inline constArrayNode *FindArrayNode(Node *node ,FlowValue v)
{
	assert(node && node->typ == Array);
	if(node->u.array.name->typ == Id){ // 普通数组 形如arr[1]
		ListMarker marker;
		constArrayNode *item = NULL;
		Node *type = NULL;
		List *list = ((propagatorNode*)(v.u.ptr))->arrayFlowList;//取出 array 数据流
		type = node->u.array.name->u.id.decl->u.decl.type;
		IterateList(&marker, list);
		while (NextOnList(&marker, (GenericREF) &item)) 
			if(node->u.array.name->u.id.text == item->name && type == item->decl) return item; // node->u.array.name->u.id.text == item->name 不能删除 
	}
	return NULL;
}

//处理库函数调用, 其他库函数暂未处理
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

	//必须存在，否则无法进行常量传播
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
		else // 是一个指向数组的指针, if(IsPointerType(NodeDataType(toNode)) && fromNode->typ == ImplicitCast && fromNode->u.implicitcast.expr->typ == Id)
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
			//assert(tmpNode && tmpNode->dim <= 2); // 一定要找到，要不然就出错了
			assert(tmpNode);
			toNode->u.decl.type = tmpNode->decl;

			if(tmpNode->dim == 1) // 一维
			{
				Node *init = NewNode(Initializer);
				for(i = 0;i<tmpNode->num;i++)
					init->u.initializer.exprs = AppendItem(init->u.initializer.exprs, tmpNode->element[i]);
				toNode->u.decl.init = init;
			}
			else if(tmpNode->dim == 2)// 二维  zww: 12.2.10
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
			else//高维（大于2）	  //zww：处理高维数组的参数传递（可能有问题）12.2.10
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
	unsigned long place=0;//place指的就是元素在一位中的位置
	constArrayNode *cn=NULL;

	if(node->u.array.name->typ == Binop){ //形如 s.x[3] 或 s[1].x[1] 等结构体中含有数组成员的
 		List *SUEflowlist = ((propagatorNode*)(v.u.ptr))->SUEFlowList;
		List *tmpsuelist=NULL;
		ListMarker m;
		Node *left,*right,*n,*value=NULL;
		Node *sueNode[10]; //定义指针数组 形如 s.a.b.c ... 结构体最多嵌套10层
		int key[10];    //涉及到的结构体数组的索引值
		Node *leftiddecl,*tmp=NULL;
		SUEtype *leftsuetype;
		constSUE* leftsuetypeNode;
		constSUEid *sueidNode = NULL;
		constSUEarray *suearrayNode = NULL;
		constSUEFieldNode *rightField_arr = NULL;
		int i=0,j=0,num;  //i为sueNode数组的下标 j为key数组的下标

		IterateList(&m, node->u.array.dims);
		NextOnList(&m, (GenericREF) &n);
		value = GetValue(n);
		key[j++] = NodeConstantIntegralValue(value);

		left = node->u.array.name->u.binop.left;
		right = node->u.array.name->u.binop.right;
		assert(right->typ == Id);  //右边都应该是id类型的
		sueNode[i++] = right;

		while(left->typ != Id ){  //形如 s.a[1].b.c.d.e[2]  依次将e，d,c,b，a 加入容器rightNode
			if(left->typ == Binop) //类似s.x.x[1] 或s[1].x.x[1]
			{
				right = left->u.binop.right;  //right一定为id类型
				assert(right->typ == Id);
				left = left->u.binop.left;
				//if(i==0 || sueNode[i-1]->typ != Array)
					sueNode[i++] = right;
			}else if(left->typ == Array) //类似s[1].x[2] 获取s的下标 存入key数组
			{
				IterateList(&m, left->u.array.dims);
				NextOnList(&m, (GenericREF) &n);
				value = GetValue(n);
				key[j++] = NodeConstantIntegralValue(value);
				sueNode[i++] = left;
				left = left->u.array.name;
			}
			else
				assert(1==0);  //否则报错
		}

		leftiddecl = left->u.id.decl;
		tmp = leftiddecl->u.decl.type;
		assert(tmp);
		while(tmp->typ != Sdcl){ // 找到其sdcl类型
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
		cn=FindArrayNode(node,v);//下面对数组元素定位
	}
	if(cn==NULL) return v;
	pos=(unsigned long *)malloc(sizeof(unsigned long)*cn->dim);
	IterateList(&m, node->u.array.dims);
	while (NextOnList(&m, (GenericREF) &n))
	{
		value=GetValue(n);
		// assert(value); // 12.13 数据流进work
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

//传递Decl结点,就是向v中添加新节点,对于全局变量只能读不能写
PRIVATE inline FlowValue TransformDecl(Node *node, FlowValue v)
{
	
	TypeQual dl = NodeDeclLocation(node);
	TypeQual sc = NodeStorageClass(node);//要收集外部的静态的等所有的变量声明
	constIdNode *inode = NULL;
	constArrayNode *anode = NULL;
	assert(node->typ==Decl);

	if ((dl == T_BLOCK_DECL || dl == T_FORMAL_DECL|| dl == T_TOP_DECL) ) 
	{
		if(IsScalarType(NodeDataType(node))&& (sc != T_TYPEDEF )&&(!IsSueType(NodeDataType(node))))
		{
			return InitDeclNode(node, v);
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

//处理赋值操作和复合赋值操作
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
			 {//添加对结构体直接赋值的处理
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

//处理算数运算
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

//处理比较运算
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
	else/* if (IsPointerType(ltype) && IsPointerType(rtype))*///SPL不支持指针，指针类型不可能到这里来
		UNREACHABLE;// 注意：函数调用在此进不来

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

//处理一元运算，在这里要remove any decls referenced by &（取地址符）
PRIVATE inline FlowValue TransformUnary(Node *node, unaryNode *u, FlowValue v)
{
	Node *value = GetValue(u->expr);

	switch (u->op) {
		/* Must be arithmetic.  Apply usual conversions */
	case UMINUS: //取负操作
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

//处理二元运算
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
	else if ( op == ARROW) // 暂不考虑
		return v;
	else 
		return v; 
}

//暂时只处理库函数调用，可以将call结点替换为常量Node（优化手段之一），此处还没有做
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
		case Goto: //程序中暂时不考虑goto语句
			return v;
		default:
			return v;
		}
	default:
		UNREACHABLE;
	}
}

//常量传播;
GLOBAL List *PropagateProgram(List *program)
{
	ListMarker marker;
	Node *item = NULL;
	propagatorNode *pNode = HeapNew(propagatorNode);

#ifdef SPL_DEBUG
	printf("\n-----------------------------DataFlow Module-----------------------------\n");
#endif

	//记录整个ast，方便后面对ast的查找
	gProgram = program; 
	pNode->idFlowList = NULL;
	pNode->arrayFlowList = NULL;
	initflow.undefined = FALSE;
	initflow.u.ptr = pNode;

	IterateList(&marker, program);
	//将全局变量加入数据流并加入到gDeclList中，将全局函数定义加入到gProcList中
	while (NextOnList(&marker, (GenericREF) &item))
	{
		if (item->typ == Decl && (IsScalarType(NodeDataType(item)) || item->u.decl.type->typ == Adcl || item->u.decl.type->typ == Sdcl || item->u.decl.type->typ == Tdef
			))
		{ 
			//变量是全局的则将其加入到数据流中
			initflow.u.ptr = (TransformDecl(item, initflow)).u.ptr;
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

	//假如不存在程序入口，则报错
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
						paramList *temp = NULL,*p = NULL,*temp2 = NULL;
						Node *id_value = NULL;
						if (cn==NULL) break;
						//cwb 将param常量与operator绑定，但在代码生成时不进行传播
						id_value = GetValue(cn->n->u.id.value);
						n->u.id.value = id_value;
						if (!DimFlag)
						{
							temp = (paramList *)malloc(sizeof(paramList));
							temp->paramnode = n;
							temp->next = NULL;
							p = tempoperatornode->params;
							if(p == NULL)
							{
								tempoperatornode->params = temp;
								tempoperatornode->paramSize = 1;
							}
							else
							{
								while(p != NULL)
								{
									if(p->paramnode->u.id.text == temp->paramnode->u.id.text)
									{
										free(temp);
										break;
									}
									else
									{
										if(p->next != NULL)
											p = p->next;
										else
										{
											p->next = temp;
											tempoperatornode->paramSize++;
											break;
										}
									}
								}
							}
						}
						else
						{
							temp2 = (paramList *)malloc(sizeof(paramList));
							temp2->paramnode = n;
							temp2->next = NULL;
							p = tempoperatornode->dimParams;
							if(p == NULL)
							{
								tempoperatornode->dimParams = temp2;
							}
							else
							{
								while(p != NULL)
								{
									if(p->paramnode->u.id.text == temp2->paramnode->u.id.text)
									{
										free(temp2);
										break;
									}
									else
									{
										if(p->next != NULL)
											p = p->next;
										else
										{
											p->next = temp2;
											break;
										}
									}
								}
							}
							if(id_value!=NULL&&id_value->typ == Const)
							{//直接将用id的value替换原来的id节点 zww-20120314
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
							DimFlag = FALSE;
						}
						break;
					   }
	case Ternary:       if ((n)->u.ternary.cond) {RWV_astwalk((n)->u.ternary.cond,v);} if ((n)->u.ternary.true_) {RWV_astwalk((n)->u.ternary.true_,v);} if ((n)->u.ternary.false_) {RWV_astwalk((n)->u.ternary.false_,v);} break;
	case Binop:         
		{
			OpType op = n->u.binop.op;			
			Node *l=NULL,*r=NULL;
			if(IsAssignmentOp(op))
			{//要添加对左边的处理
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
			Node *tmpCGNode=NULL; // 13-5-20 YQJ 结构体的成员中含有数组时作为临时结点
			if(n->u.array.name->typ == Binop)
			{
				tmpCGNode = n->u.array.name;
				n->u.array.name = n->u.array.name->u.binop.left;
			//	RWV_astwalk(n->u.array.name, v);
			}
			else
			{
				if (FindItem(DeclArrayListInWork,n->u.array.name->u.id.decl) == NULL && NodeDeclLocation(n->u.array.name->u.id.decl)!=T_TOP_DECL)
				{
					Node *arrayDecl = NULL;
					Node *init = NewNode(Initializer);//定义一个数组初始化的节点；
					constArrayList *p = NULL,*temp = NULL;
					constArrayNode *tmpNode = NULL;
					List *list = ((propagatorNode*)(v.u.ptr))->arrayFlowList;
					char *name = n->u.array.name;
					Node *decl = NULL;
					int i;
					Node *dtype = NULL;
					tmpNode=FindArrayNode(n,v);//在数据流中查找
					if(tmpNode!=NULL) 
					{
						arrayDecl = NodeCopy(n->u.array.name->u.id.decl,Subtree);
						dtype = NodeDataType(arrayDecl->u.decl.type);
						decl = arrayDecl;
						assert(arrayDecl->typ == Decl);
						for(i = 0;i<tmpNode->num;i++)
						{
							assert(tmpNode->element[i]!=NULL);
							init->u.initializer.exprs = AppendItem(init->u.initializer.exprs, tmpNode->element[i]);					
						}
						assert(init);
						assert(init->typ == Initializer);
						assert(ListLength(init->u.initializer.exprs)==tmpNode->num);
						//cwb 初始化数组成operator绑定
						temp = (constArrayList *)malloc(sizeof(constArrayList));
						temp->arraynode = init;
						temp->next = NULL;
						p = tempoperatornode->ArrayInit;
						if(p == NULL)
						{
							tempoperatornode->ArrayInit = temp;
						}
						else
						{
							while(p != NULL)
							{
								if(p->arraynode == temp->arraynode)
								{
									free(temp);
									break;
								}
								else
								{
									if(p->next != NULL)
										p = p->next;
									else
									{
										p->next = temp;
										break;
									}
								}
							}
						}
						arrayDecl->u.decl.prim_init= NodeCopy(init,NodeOnly);
						arrayDecl->u.decl.init = SemCheckInitList(arrayDecl, dtype, NodeCopy(arrayDecl->u.decl.prim_init,NodeOnly), TRUE);
						DeclArrayListInWork = AppendItem(DeclArrayListInWork,arrayDecl);
					}
				}

				if((n)->u.array.type) {RWV_astwalk((n)->u.array.type,v);}
				if((n)->u.array.name) {RWV_astwalk((n)->u.array.name,v);}
				if(n->u.array.dims) {RWV_listwalk(n->u.array.dims,v);}		
				TransformArray(n,v);			
				if(tmpCGNode !=NULL){
					tmpCGNode->u.binop.left = n->u.array.name;
					n->u.array.name = tmpCGNode;
				} 
			}
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
	case For:           if ((n)->u.For.init) {RWV_astwalk((n)->u.For.init,v);}
						if ((n)->u.For.cond) {RWV_astwalk((n)->u.For.cond,v);} 
						if ((n)->u.For.next) {RWV_astwalk((n)->u.For.next,v);} 
						if ((n)->u.For.stmt) {RWV_astwalk((n)->u.For.stmt,v);}
						break; 
	case Goto:          break; 
	case Continue:      break; 
	case Break:         break; 
	case Return:        if ((n)->u.Return.expr) {RWV_astwalk((n)->u.Return.expr,v);} break; //spl少有
	case Block:         if ((n)->u.Block.decl) {RWV_listwalk((n)->u.Block.decl,v);} if ((n)->u.Block.stmts) {RWV_listwalk((n)->u.Block.stmts,v);} break; 
	case Prim:          break; 
	case Adcl:          if((n)->u.adcl.type){RWV_astwalk((n)->u.adcl.type,v);}if((n)->u.adcl.dim){if((n)->u.adcl.dim->typ != Const && ((n)->u.adcl.dim->typ == Id || (n)->u.adcl.dim->u.binop.right->typ == Id || (n)->u.adcl.dim->u.binop.left->typ == Id))DimFlag = TRUE;RWV_astwalk((n)->u.adcl.dim,v);}break; 
	case Decl:          if ((n)->u.decl.type) {RWV_astwalk((n)->u.decl.type,v);}if ((n)->u.decl.init) {RWV_astwalk((n)->u.decl.init,v);}  break;
	case Text:          break;   
	default:            break; 
	}
}
GLOBAL void ReplaceWorkVar(Node *node,FlowValue v)
{//将work中用到的变量值全部取出并进行替换(未替换)
	assert(node);
	//	PrintConstFlowValue(v);
	DeclArrayListInWork = NULL;
	RWV_astwalk(node,v);
	node->u.Block.decl = JoinLists(node->u.Block.decl,DeclArrayListInWork);
}

