#include "propagator.h"

GLOBAL List *MakeFieldList(List *fieldsList)
{
	ListMarker marker;
	Node *field = NULL;
	List *newfield = NULL;
	IterateList(&marker, fieldsList);
	while (NextOnList(&marker, (GenericREF) &field)) {
		declNode *field_decl;
		Node *field_type;
		TypeQual field_sc ;
		assert(field->typ == Decl);
		field_decl = &(field->u.decl); //取出结构体中的每个成员的定义
		field_type = NodeDataType(field_decl->type);
		if(field_decl->type->typ == Tdef)
		{  //结构体成员中有tdef定义的类型
			constSUEFieldNode *sueFieldNode = HeapNew(constSUEFieldNode);
			constSUEid* field_Sid = MakeTdefNode(field);
			sueFieldNode->sutyp = SUE_Id;
			sueFieldNode->u.sueidNode = field_Sid;
			newfield=AppendItem(newfield,sueFieldNode); 

		}
		else if(field_decl->type->typ == Prim)
		{
			constSUEFieldNode *sueFieldNode = HeapNew(constSUEFieldNode);
			constIdNode* field_id = MakeConstIdNode(field);
			sueFieldNode->sutyp = Prim_Id;
			sueFieldNode->u.idnode = field_id;
			newfield=AppendItem(newfield,sueFieldNode);
		}
		else if(field_decl->type->typ == Sdcl)
		{
			constSUEFieldNode *sueFieldNode = HeapNew(constSUEFieldNode);
			constSUEid* field_Sid = MakeStructNode(field);
			sueFieldNode->sutyp = SUE_Id;
			sueFieldNode->u.sueidNode = field_Sid;
			newfield=AppendItem(newfield,sueFieldNode); 
		}
		else if(field_decl->type->typ ==Udcl)
		{
			constSUEFieldNode *sueFieldNode = HeapNew(constSUEFieldNode);
			constSUEid* field_Sid = MakeUnionNode(field);
			sueFieldNode->sutyp = SUE_Id;
			sueFieldNode->u.sueidNode = field_Sid;
			newfield=AppendItem(newfield,sueFieldNode);

		}
		else if(field_decl->type->typ ==Edcl)
		{
			constSUEFieldNode *sueFieldNode = HeapNew(constSUEFieldNode);
			constSUEid* field_Sid = MakeEnumNode(field);
			sueFieldNode->sutyp = SUE_Id;
			sueFieldNode->u.sueidNode = field_Sid;
			newfield=AppendItem(newfield,sueFieldNode);

		}
		else if(field_decl->type->typ == Adcl)
		{//结构体的成员不允许是数组 ，该分支不执行
			Node *tmptype= field_decl->type;
			int dim=0;
			int i=0;
			int *lenPerDim = NULL;
			while(tmptype->typ == Adcl)	
			{
				tmptype = tmptype->u.adcl.type;
				dim++;
			}
			tmptype= field_decl->type;
			lenPerDim = (int *)malloc(sizeof(int)*dim);
			i=0;
			while(tmptype->typ == Adcl)	
			{
				lenPerDim[i++] = CheckAdcl(tmptype);//检查各维的长度，返回各维长度的值
				tmptype = tmptype->u.adcl.type;
			}
			if(tmptype->typ==Tdef)
			{  ; //暂未处理

			}
			else if(tmptype->typ == Prim)
			{
				constSUEFieldNode *sueFieldNode = HeapNew(constSUEFieldNode);
				constArrayNode* field_array = MakeConstArrayNode(dim , lenPerDim , field);
				sueFieldNode->sutyp = Prim_Array;
				sueFieldNode->u.arrayNode = field_array;
				newfield = AppendItem(newfield , sueFieldNode);
			}
			else if(tmptype->typ == Sdcl)
			{
				constSUEFieldNode *sueFieldNode = HeapNew(constSUEFieldNode);
				constSUEarray* field_Sarray = MakeStructArrayNode(field);
				sueFieldNode->sutyp = SUE_Array;
				sueFieldNode->u.suearrayNode = field_Sarray;
				newfield = AppendItem(newfield , sueFieldNode);
			}
			else if(tmptype->typ == Udcl)
			{
				constSUEFieldNode *sueFieldNode = HeapNew(constSUEFieldNode);
				constSUEarray* field_Uarray = MakeUnionArrayNode(field);
				sueFieldNode->sutyp = SUE_Array;
				sueFieldNode->u.suearrayNode = field_Uarray;
				newfield = AppendItem(newfield , sueFieldNode);
			}
			else if(tmptype->typ == Edcl)
			{  ;//一般不会出现 ,暂不处理
			}			
		} 		
	}
	return newfield;

}

GLOBAL constSUEid* MakeStructNode(Node *node)
{  //处理结构体
	constSUEid *SUEidnode = HeapNew(constSUEid);
	Node *type = node->u.decl.type;	 //结构体的类型
	assert(node && type->typ==Sdcl);
	SUEidnode->decl = node;
	SUEidnode->SUEfields = MakeFieldList(type->u.sdcl.type->fields);
	return SUEidnode;
}

GLOBAL constSUEid *MakeTdefNode(Node *node)
{//tdef后的类型暂时只考虑Sdcl  ,其他暂不处理
	Node *decl = NULL;   //记录tdef所指的结构他的sdcl
	Node *tdeftype = NULL;
	constSUEid *SUEidnode = HeapNew(constSUEid);
	assert(node->u.decl.type->typ== Tdef);
	decl = node->u.decl.type->u.tdef.type;
	assert(decl&&decl->typ == Sdcl);
	if(decl->typ == Sdcl)
	{ 
		SUEidnode->decl = node;
		SUEidnode->SUEfields = MakeFieldList(decl->u.sdcl.type->fields);
		return SUEidnode;
	}
	else 
		UNREACHABLE; 
}

GLOBAL constSUEid* MakeUnionNode(Node *node)
{  //处理共用体
	constSUEid *SUEidnode = HeapNew(constSUEid);
	Node *type = node->u.decl.type;
	assert(node && type->typ==Udcl);
	SUEidnode->decl = node;
	SUEidnode->SUEfields = MakeFieldList(type->u.udcl.type->fields);
	return SUEidnode;

}

GLOBAL constSUEarray* MakeStructArrayNode(Node *node)
{  //处理结构体数组
	constSUEarray *SUEarray = HeapNew(constSUEarray);
	int dim = 0,i = 0,num=1;
	Node *tmp_type = node->u.decl.type;	
	int *lenPerDim = NULL;
	SUEarray->SUEname = node->u.decl.name;
	SUEarray->SUEdecl = node;
	while(tmp_type->typ == Adcl)
	{//循环结束是tmp_type指向Sdcl
		++dim;
		tmp_type = tmp_type->u.adcl.type;
	}
	tmp_type = node->u.decl.type;
	SUEarray->SUEdim = dim;
	lenPerDim = (int *)malloc(sizeof(int)*dim);
	i=0;
	while(tmp_type->typ == Adcl)	
	{
		lenPerDim[i++] = CheckAdcl(tmp_type);//检查各维的长度，返回各维长度的值
		tmp_type = tmp_type->u.adcl.type;
	}
	SUEarray->SUEdimLen = lenPerDim;
	for (i=0;i<dim;i++)
		num *= lenPerDim[i];
	SUEarray->SUEnum = num;
	SUEarray->SUEelement = (List**)malloc(sizeof(Node*)*num);
	assert(tmp_type->typ == Sdcl);
	for (i=0;i<num;i++)
	{
		SUEarray->SUEelement[i] = MakeFieldList(tmp_type->u.sdcl.type->fields);
	}
	return SUEarray;

}

GLOBAL constSUEarray* MakeTdefArrayNode(Node *node)
{  //处理结构体数组
	constSUEarray *SUEarray = HeapNew(constSUEarray);
	int dim = 0,i = 0,num=1;
	Node *tdeftype=NULL;
	Node *tmp_type = node->u.decl.type;	
	int *lenPerDim = NULL;
	SUEarray->SUEname = node->u.decl.name;
	SUEarray->SUEdecl = node;
	while(tmp_type->typ == Adcl)
	{//循环结束是tmp_type指向Sdcl
		++dim;
		tmp_type = tmp_type->u.adcl.type;
	}
	tmp_type = node->u.decl.type;
	SUEarray->SUEdim = dim;
	lenPerDim = (int *)malloc(sizeof(int)*dim);
	i=0;
	while(tmp_type->typ == Adcl)	
	{
		lenPerDim[i++] = CheckAdcl(tmp_type);//检查各维的长度，返回各维长度的值
		tmp_type = tmp_type->u.adcl.type;
	}
	SUEarray->SUEdimLen = lenPerDim;
	for (i=0;i<dim;i++)
		num *= lenPerDim[i];
	SUEarray->SUEnum = num;
	SUEarray->SUEelement = (List**)malloc(sizeof(List*)*num);
	assert(tmp_type->typ == Tdef);
	tdeftype = tmp_type->u.tdef.type;
	assert(tdeftype && tdeftype->typ == Sdcl);
	for (i=0;i<num;i++)
	{
		SUEarray->SUEelement[i] = MakeFieldList(tdeftype->u.sdcl.type->fields);
	}
	return SUEarray;

}

GLOBAL constSUEarray* MakeUnionArrayNode(Node *node)
{//处理共用体数组
	constSUEarray *SUEarray = HeapNew(constSUEarray);
	int dim = 0,i = 0,num=1;
	Node *tmp_type = node->u.decl.type;	
	int *lenPerDim = NULL;
	SUEarray->SUEname = node->u.decl.name;
	SUEarray->SUEdecl = node;
	while(tmp_type->typ == Adcl)
	{//循环结束是tmp_type指向Udcl
		++dim;
		tmp_type = tmp_type->u.adcl.type;
	}
	SUEarray->SUEdim = dim;
	lenPerDim = (int *)malloc(sizeof(int)*dim);
	i=0;
	while(tmp_type->typ == Adcl)	
	{
		lenPerDim[i++] = CheckAdcl(tmp_type);//检查各维的长度，返回各维长度的值
		tmp_type = tmp_type->u.adcl.type;
	}
	SUEarray->SUEdimLen = lenPerDim;
	for (i=0;i<dim;i++)
		num *= lenPerDim[i];
	SUEarray->SUEnum = num;
	SUEarray->SUEelement = (List**)malloc(sizeof(Node*)*num);
	assert(tmp_type->typ == Udcl);
	for (i=0;i<num;i++)
	{
		SUEarray->SUEelement[i] = MakeFieldList(tmp_type->u.udcl.type->fields);
	}
	return SUEarray;
}

GLOBAL constSUEid* MakeEnumNode(Node *node)
{//暂时不处理
	return NULL;
}

GLOBAL constSUEarray* MakeEnumArrayNode(Node *node)
{//暂时不处理
	return NULL;
}

GLOBAL constSUE* FindSUEtype(SUEtype *suetype,List *SUEflowlist)
{//从数据流中查找type类型的节点(type是一种类型的定义节点)
	constSUE *ptrSUE=NULL;
	ListMarker marker;
	//Node *SUtype = NULL;
	IterateList(&marker, SUEflowlist);
	while (NextOnList(&marker, (GenericREF) &ptrSUE))
	{
		if(ptrSUE->type==suetype) return ptrSUE;
	}
	return NULL; 
}

GLOBAL	constIdNode *FindSUEid(Node *Decl ,List *sueidList)
{
	ListMarker marker;
	constSUEid *sueidNode = NULL;  
	constSUEid *tmpsueid = NULL;
	assert(sueidList);
	IterateList(&marker, sueidList);
	while (NextOnList(&marker, (GenericREF) &tmpsueid)) {   //找结构变量
		if(tmpsueid->decl == Decl) sueidNode = tmpsueid;
	}
	assert(sueidNode);
	return sueidNode;
}

GLOBAL	constSUEarray *FindSUEarray(Node *node ,List *suearrayList)
{
	ListMarker marker;
	constSUEarray *suearrayNode = NULL;  
	constSUEarray *tmpsuearray = NULL;
	assert(suearrayList);
	IterateList(&marker, suearrayList);
	while (NextOnList(&marker, (GenericREF) &tmpsuearray)) {   //找结构变量
		if(node->u.array.name->u.id.text == tmpsuearray->SUEname && node->u.array.name->u.id.decl == tmpsuearray->SUEdecl ) 
			suearrayNode = tmpsuearray;
	}
	assert(suearrayNode);
	return suearrayNode;
}

GLOBAL constSUEFieldNode *FindFieldSUEnode(List *fieldsList, Node *field_id)
{ //查找结构体成员
	ListMarker marker;
	constSUEFieldNode *field;
	const char *name;
	assert(fieldsList);
	assert(field_id->typ == Id);

	/* Find the field in the struct/union fields */
	IterateList(&marker, fieldsList);
	while (NextOnList(&marker, (GenericREF) &field)) {
		if(field->sutyp == Prim_Id)
		{
			if(field->u.idnode->n->u.id.text == field_id->u.id.text) return field;
		}
		else if(field->sutyp == Prim_Array)
		{
			if(field->u.arrayNode->name == field_id->u.id.text) return field;
		}
		else if(field->sutyp == SUE_Id)
		{
			if(field->u.sueidNode->decl->u.decl.name == field_id->u.id.text)  return field;
		}
		else if(field->sutyp == SUE_Array)
		{
			if(field->u.suearrayNode->SUEname == field_id->u.id.text)   return field;
		}
	}
	return NULL;
}

GLOBAL constSUEid *InitStructid(constSUEid *SUEid ,Node *decl)
{//结构体变量初始化
	ListMarker marker;
	ListMarker markerSidField;
	ListMarker makerinit;
	Node *field;
	constSUEFieldNode *fieldidNode=NULL;
	Node *idinit = NULL;
	//	List *fieldsList = decl->u.decl.type->u.sdcl.type->fields;
	//IterateList(&marker, fieldsList);
	//while (NextOnList(&marker, (GenericREF) &field)) {   //成员中有数组，结构体，共用体等不能进行直接初始化
	//declNode *field_decl;
	//assert(field->typ == Decl);
	//field_decl = &(field->u.decl); //取出结构体中的每个成员的定义
	//if(field_decl->type->typ == Sdcl||field_decl->type->typ == Udcl||field_decl->type->typ == Adcl)
	//return SUEid; 
	//}
	assert(decl->u.decl.init!=NULL);
	IterateList(&markerSidField, SUEid->SUEfields);
	IterateList(&makerinit, decl->u.decl.init->u.initializer.exprs);
	while ( NextOnList(&markerSidField, (GenericREF) &fieldidNode)&&NextOnList(&makerinit, (GenericREF) &idinit))
	{
		assert(fieldidNode);
		if(fieldidNode->sutyp == Prim_Id){
			fieldidNode->u.idnode->n->u.id.value = idinit;
			fieldidNode->u.idnode->undefine = FALSE;
		}
		else if(fieldidNode->sutyp == Prim_Array){
			int dim=0,num =1;
			int *lenPerDim = NULL;
			dim = fieldidNode->u.arrayNode->dim;
			lenPerDim = (int *)malloc(sizeof(int)*dim);
			num = fieldidNode->u.arrayNode->num;
			assert(dim <= 2);
			InitConstArrayNode(dim, lenPerDim, num, fieldidNode->u.arrayNode,idinit);
		}else if(fieldidNode->sutyp ==SUE_Id){
			decl->u.decl.init = idinit;
			InitStructid(fieldidNode->u.sueidNode,decl);
		}
	}
	return SUEid;
}

GLOBAL constSUEarray *InitTdefarray(constSUEarray *SUEarray ,Node *decl){
	ListMarker marker;
	ListMarker maketmpfield,markerSarrayField;
	ListMarker makerinittmp,makerinit;
	Node *field,tmpinitnode;
	constSUEFieldNode *fieldarrayNode=NULL;
	Node *tmpnode,*arrayinit = NULL;
	int i,dim,num; 
	dim = SUEarray->SUEdim;
	assert(dim == 1); // 暂只支持一维结构体数组
	num = SUEarray->SUEnum; //每维中的数组元素个数
	IterateList(&makerinittmp, decl->u.decl.init->u.initializer.exprs);
	for(i=0;i<num;i++){
		IterateList(&maketmpfield, SUEarray->SUEelement[i]);
		NextOnList(&makerinittmp, (GenericREF) &tmpnode);
		assert(tmpnode->typ == Initializer);
		IterateList(&makerinit, tmpnode->u.initializer.exprs);
		while(NextOnList(&maketmpfield, (GenericREF) &fieldarrayNode) && NextOnList(&makerinit, (GenericREF) &arrayinit)){
			if(fieldarrayNode->sutyp == Prim_Id){
				fieldarrayNode->u.idnode->n->u.id.value = arrayinit;
				fieldarrayNode->u.idnode->undefine = FALSE;
			}
			else if(fieldarrayNode->sutyp == Prim_Array){
				int dim=0,num =1;
				int *lenPerDim = NULL;
				dim = fieldarrayNode->u.arrayNode->dim;
				lenPerDim = (int *)malloc(sizeof(int)*dim);
				num = fieldarrayNode->u.arrayNode->num;
				assert(dim <= 2);
				InitConstArrayNode(dim, lenPerDim, num, fieldarrayNode->u.arrayNode,arrayinit);
			}else if(fieldarrayNode->sutyp ==SUE_Id){
				decl->u.decl.init = arrayinit;
				InitStructid(fieldarrayNode->u.sueidNode,decl);
			}else if(fieldarrayNode->sutyp == SUE_Array){
				decl->u.decl.init = arrayinit;
				InitTdefarray(fieldarrayNode->u.suearrayNode,decl);
			}
			;
		}

	}
	return SUEarray;

}
GLOBAL FlowValue InitSUENode(Node *node)
{//（暂时未用，它的功能在InsertSUEtoFlow中代替了）初始化结构体
	//不支持共用体以及结构体数组，以及结构体中含有数组成员的初始化	
	Node *dtype = node->u.decl.type;//node的类型
	constSUE *itemtype = NULL;//记录dtype是否已在数据流中的位置
	constSUEid *SUEid = NULL;
	constSUEarray *SUEarray = NULL;
	assert(node && node->typ==Decl);
	if(dtype->typ==Sdcl) 
	{
		SUEid = MakeStructNode(node);
		if(node->u.decl.init!=NULL)SUEid = InitStructid(SUEid,node);	
	}
	else if(dtype->typ == Udcl)
	{ //不支持初始化 
		SUEid = MakeUnionNode(node);
	}
	else if(dtype->typ == Edcl) 
	{
		SUEid = MakeEnumNode(node);		
	}
	else if (dtype->typ == Tdef)
	{
		SUEid = MakeTdefNode(node);
	}
	else if(dtype->typ== Adcl)
	{
		while (dtype->typ == Adcl)
		{
			dtype = dtype->u.adcl.type;
		} 		
		if(dtype->typ == Sdcl) SUEarray = MakeStructArrayNode(node);
		else if (dtype->typ == Tdef)SUEarray = MakeTdefArrayNode(node);
		else if(dtype->typ == Udcl) SUEarray = MakeUnionArrayNode(node);
		else if (dtype->typ == Edcl) SUEarray = MakeEnumArrayNode(node);
	}
}

GLOBAL List *InsertSUEidtoFlow(Node *node,List *SUElist)
{
	constSUE *ptrSUE=NULL;
	assert(node && node->typ==Decl);
	if(node->u.decl.type->typ == Sdcl)
	{
		ptrSUE = FindSUEtype(node->u.sdcl.type,SUElist);
		if (ptrSUE == NULL)
		{
			constSUENode *SUENode = HeapNew(constSUENode);
			ptrSUE = HeapNew(constSUE);
			SUENode->SUEid = AppendItem(SUENode->SUEid,MakeStructNode(node));   //这个可能需要改成能初始化的函数  ，下面类似
			ptrSUE->type = node->u.sdcl.type;
			ptrSUE->SUEnode = SUENode;
		}
		else 
		{
			assert(ptrSUE);
			ptrSUE->SUEnode->SUEid = AppendItem(ptrSUE->SUEnode->SUEid,MakeStructNode(node));
		}
	}
	else if(node->u.decl.type->typ == Udcl) 
	{
		ptrSUE = FindSUEtype(node->u.udcl.type,SUElist);
		if (ptrSUE == NULL)
		{
			constSUENode *SUENode = HeapNew(constSUENode);
			ptrSUE = HeapNew(constSUE);
			SUENode->SUEid = AppendItem(SUENode->SUEid,MakeUnionNode(node));
			ptrSUE->type = node->u.udcl.type;
			ptrSUE->SUEnode = SUENode;
		}
		else 
		{
			assert(ptrSUE);
			ptrSUE->SUEnode->SUEid = AppendItem(ptrSUE->SUEnode->SUEid,MakeUnionNode(node));
		}
	}
	else if(node->u.decl.type->typ == Tdef)
	{
		ptrSUE = FindSUEtype(node->u.tdef.type->u.sdcl.type,SUElist);
		if (ptrSUE == NULL)
		{
			constSUENode *SUENode = HeapNew(constSUENode);
			ptrSUE = HeapNew(constSUE);
			SUENode->SUEid = AppendItem(SUENode->SUEid,MakeStructNode(node));   //这个可能需要改成能初始化的函数  ，下面类似
			ptrSUE->type = node->u.tdef.type->u.sdcl.type;
			ptrSUE->SUEnode = SUENode;
		}
		else 
		{
			assert(ptrSUE);
			ptrSUE->SUEnode->SUEid = AppendItem(ptrSUE->SUEnode->SUEid,MakeTdefNode(node));
		}
	}
	else if(node->u.decl.type->typ == Edcl);//暂不处理

	return SUElist;

}

GLOBAL List *InsertSUEarraytoFlow(Node *node,List *SUElist)
{
	constSUE *ptrSUE=NULL;
	Node *tmptype = node->u.adcl.type;
	assert(node && node->typ==Decl&&tmptype->typ == Adcl);
	while (tmptype->typ == Adcl)
	{
		tmptype = tmptype->u.adcl.type;
	}
	if(tmptype->typ == Sdcl)
	{
		ptrSUE = FindSUEtype(tmptype,SUElist);
		if (ptrSUE == NULL)
		{
			constSUENode *SUENode = HeapNew(constSUENode);
			ptrSUE = HeapNew(constSUE);
			SUENode->SUEarray = AppendItem(SUENode->SUEarray,MakeStructArrayNode(node));   //这个可能需要改成能初始化的函数  ，下面类似
			ptrSUE->type = node->u.sdcl.type;
			ptrSUE->SUEnode = SUENode;
		}
		else 
		{
			assert(ptrSUE);
			ptrSUE->SUEnode->SUEarray = AppendItem(ptrSUE->SUEnode->SUEarray,MakeStructArrayNode(node));
		}
	}
	else if(tmptype->typ == Udcl) 
	{
		ptrSUE = FindSUEtype(node->u.udcl.type,SUElist);
		if (ptrSUE == NULL)
		{
			constSUENode *SUENode = HeapNew(constSUENode);
			ptrSUE = HeapNew(constSUE);
			SUENode->SUEarray = AppendItem(SUENode->SUEarray,MakeUnionArrayNode(node));
			ptrSUE->type = node->u.udcl.type;
			ptrSUE->SUEnode = SUENode;
		}
		else 
		{
			assert(ptrSUE);
			ptrSUE->SUEnode->SUEarray = AppendItem(ptrSUE->SUEnode->SUEarray,MakeUnionArrayNode(node));
		}
	}
	else if(tmptype->typ == Tdef)
	{
		ptrSUE = FindSUEtype(node->u.tdef.type->u.sdcl.type,SUElist);
		if (ptrSUE == NULL)
		{
			constSUENode *SUENode = HeapNew(constSUENode);
			ptrSUE = HeapNew(constSUE);
			SUENode->SUEarray = AppendItem(SUENode->SUEarray,MakeTdefArrayNode(node));   //这个可能需要改成能初始化的函数  ，下面类似
			ptrSUE->type = node->u.tdef.type->u.sdcl.type;
			ptrSUE->SUEnode = SUENode;
		}
		else 
		{
			assert(ptrSUE);
			ptrSUE->SUEnode->SUEarray = AppendItem(ptrSUE->SUEnode->SUEarray,MakeStructNode(node));
		}
	}
	else if(tmptype->typ == Edcl);//暂不处理

	return SUElist;

}

GLOBAL FlowValue InsertSUEdecltoFlow(Node *decl,FlowValue v)
{
	Node *dtype=decl->u.decl.type;
	SUEtype *suetype = NULL;
	constSUE* ptrSUE = NULL;
	constSUENode *suenode = NULL;
	constSUEid *sueid = NULL;
	constSUEarray *suearray = NULL;
	List *SUEflowlist = ((propagatorNode*)(v.u.ptr))->SUEFlowList;
	if (dtype->typ == Sdcl)
	{
		suetype = dtype->u.sdcl.type;
		sueid = MakeStructNode(decl);
		if(decl->u.decl.init!=NULL)sueid = InitStructid(sueid,decl);
	}
	else if(dtype->typ == Tdef)
	{	//暂不处理
		suetype = dtype->u.tdef.type->u.sdcl.type;
		sueid = MakeTdefNode(decl);
		if(decl->u.decl.init !=NULL) sueid = InitStructid(sueid,decl);
	}
	else if (dtype->typ ==Udcl)
	{
		suetype = dtype->u.udcl.type;
		sueid = MakeUnionNode(decl);
	}
	else if(dtype->typ ==Edcl)
	{
		suetype = dtype->u.edcl.type;
		sueid = MakeEnumNode(decl);
	}

	else if(dtype->typ == Adcl)
	{		
		while (dtype->typ == Adcl)
		{
			dtype = dtype->u.adcl.type;
		}
		if (dtype->typ == Sdcl)
		{
			suetype = dtype->u.sdcl.type;
			suearray = MakeStructArrayNode(decl);
		}
		else if(dtype->typ == Tdef)
		{	
			suetype = dtype->u.tdef.type->u.sdcl.type;
			suearray = MakeTdefArrayNode(decl);
			if(decl->u.decl.init != NULL) suearray = InitTdefarray(suearray,decl);
		}
		else if (dtype->typ ==Udcl)
		{
			suetype = dtype->u.udcl.type;
			suearray = MakeUnionArrayNode(decl);
		}
		else if(dtype->typ ==Edcl)
		{
			suetype = dtype->u.edcl.type;
			suearray = MakeEnumArrayNode(decl);
		}
	}
	ptrSUE = FindSUEtype(suetype,SUEflowlist);
	if(ptrSUE == NULL)
	{
		constSUE *sue =HeapNew(constSUE);
		dtype=decl->u.decl.type;
		suenode = HeapNew(constSUENode);
		if(dtype->typ == Adcl) suenode->SUEarray = AppendItem(suenode->SUEarray,suearray);
		else if(dtype->typ == Sdcl||dtype->typ == Udcl || dtype->typ ==Edcl ||dtype->typ == Tdef)
			suenode->SUEid =AppendItem(suenode->SUEid,sueid);
		sue->type = suetype;
		sue->SUEnode = suenode; 
		((propagatorNode*)(v.u.ptr))->SUEFlowList = AppendItem(SUEflowlist,sue);
	}
	else
	{
		dtype=decl->u.decl.type;
		suenode = ptrSUE->SUEnode;
		if(dtype->typ == Adcl) suenode->SUEarray = AppendItem(suenode->SUEarray,suearray);
		else if(dtype->typ == Sdcl||dtype->typ == Udcl || dtype->typ ==Edcl ||dtype->typ == Tdef)
			suenode->SUEid =AppendItem(suenode->SUEid,sueid);
	}
	return v;
}

GLOBAL  FlowValue TransfromDot(Node *node , FlowValue v)
{  //经过这个函数就是要把值放在'.'的value中.dot的右值要么是id，要么是数组，而左值要么是id，要么是数组，要么是dot
	Node *tmpnode = NULL;
	Node *field = NULL,
		*left, *right, *ltype, *rtype, *type;
	binopNode *u;
	Node *dottype;//dot的类型就是它右值的类型
	constSUE *ptrSUE=NULL;
	List *SUEflowlist = ((propagatorNode*)(v.u.ptr))->SUEFlowList;
	assert(node);
	assert(SUEflowlist);
	u = &(node->u.binop);
	dottype = node->u.binop.type;
	left  = u->left;
	right = u->right;
	assert(left);
	assert(right);
	ltype = NodeDataType(left);
	rtype = NodeDataType(right);
	assert(ltype);
	type  = NodeDataType(ltype);
	assert(type);

	if(left->typ == Id && right->typ == Id)
	{
		Node *leftiddecl;
		SUEtype *leftsuetype;
		constSUE* leftsuetypeNode;
		constSUEid *sueidNode = NULL;
		constSUEFieldNode *rightField_id = NULL;

		leftiddecl = left->u.id.decl;
		if(leftiddecl->u.decl.type->typ == Tdef)
			leftsuetype = leftiddecl->u.decl.type->u.tdef.type->u.sdcl.type;
		else if(leftiddecl->u.decl.type->typ == Sdcl)
			leftsuetype = leftiddecl->u.decl.type->u.sdcl.type;
		leftsuetypeNode = FindSUEtype(leftsuetype, SUEflowlist);



		assert(leftsuetypeNode);
		sueidNode = FindSUEid(leftiddecl,leftsuetypeNode->SUEnode->SUEid);//找到结构体，即dot的左值
		assert(sueidNode);

		if (dottype->typ == Prim)
		{//形如s.x
			constSUEFieldNode *rightField_id = NULL;	
			rightField_id = FindFieldSUEnode(sueidNode->SUEfields, right);
			assert(rightField_id);
			if(rightField_id->u.idnode->n->u.id.value!=NULL && rightField_id->u.idnode->n->u.id.value->typ ==Const)
			{
				node->u.binop.value = GetValue(rightField_id->u.idnode->n->u.id.value);
				right->u.id.value = GetValue(rightField_id->u.idnode->n->u.id.value);
			}
		}
		else if (dottype->typ == Adcl)
		{//YQJ 新增对数组的支持  形如s.x[2]，
			// 由于不能获得数组的索引值 故改在TransformArray 中处理

		}
		else if(dottype->typ == Sdcl)
		{//形如sa.sb.x，中的第一个dot
			constSUEFieldNode *rightField_Struct = NULL;
			SUEtype* temsuetype = NULL;
			rightField_Struct = FindFieldSUEnode(sueidNode->SUEfields, right);
			assert(rightField_Struct);
			//node->u.binop.value = NewNode(Sdcl);
			//node->u.binop.value->u.sdcl.type = node->u.binop.type;
			temsuetype = HeapNew(SUEtype);
			temsuetype->fields = rightField_Struct->u.sueidNode->SUEfields;
			temsuetype->typ = Sdcl;
			node->u.binop.valueList = temsuetype->fields;
			//node->u.binop.value->u.sdcl.type->fields = temsuetype->fields;//把查找出来的内部结构体的各成员值dot的value中（将value改造成sdcl节点）
		}
		else if (dottype->typ == Udcl)
		{  ;//暂不处理
		}
		else if (dottype->typ == Edcl)
		{ ; //暂不处理
		}
		else UNREACHABLE;

	}
	else if(left->typ == Array && right->typ == Id)
	{
		Node *leftarraydecl = left->u.array.type;
		SUEtype *leftsuetype = leftarraydecl->u.sdcl.type;
		constSUE* leftsuetypeNode = FindSUEtype(leftsuetype, SUEflowlist);
		unsigned long *pos, tmp, i = 0, j;
		unsigned long place=0;
		ListMarker m;
		Node *n, *value =NULL;
		List *sueArrayElement=NULL;

		constSUEarray *suearrayNode = NULL;
		constSUEid *sueidNode = NULL;
		constSUEFieldNode *rightField_id = NULL;

		if(leftsuetypeNode == NULL) 
			return v;
		assert(leftsuetypeNode);
		if(leftsuetypeNode->SUEnode->SUEarray != NULL){
			suearrayNode = FindSUEarray(left,leftsuetypeNode->SUEnode->SUEarray);//找到结构体，即dot的左值
			assert(suearrayNode);
			pos=(unsigned long *)malloc(sizeof(unsigned long)*suearrayNode->SUEdim);
			IterateList(&m, left->u.array.dims);
			while (NextOnList(&m, (GenericREF) &n))
			{
				value=GetValue(n);
				if(value!=NULL) pos[i++]=NodeConstantIntegralValue(value);
				else return v;
			}
			for (i=0;i<suearrayNode->SUEdim;i++)
			{
				tmp=1;
				for(j=i+1;j<suearrayNode->SUEdim;j++)
					tmp=tmp*suearrayNode->SUEdimLen[j];
				place=place+pos[i]*tmp;
			}
			sueArrayElement = suearrayNode->SUEelement[place];
		}
		else{
			assert(node->u.binop.left->u.array.name->u.binop.valueList);
			sueArrayElement = node->u.binop.left->u.array.name->u.binop.valueList;
		}





		if (dottype->typ == Prim)
		{//形如s[2].x
			constSUEFieldNode *rightField_id = NULL;	
			rightField_id = FindFieldSUEnode(sueArrayElement, right);
			assert(rightField_id);
			if(rightField_id->u.idnode->n->u.id.value!=NULL &&rightField_id->u.idnode->n->u.id.value->typ ==Const)
			{
				node->u.binop.value = GetValue(rightField_id->u.idnode->n->u.id.value);
				right->u.id.value = GetValue(rightField_id->u.idnode->n->u.id.value);
			}
		}
		else if (dottype->typ == Adcl)
		{//形如s[3].x[2]   暂时不支持，未处理
			;
		}
		else if(dottype->typ == Sdcl)
		{//形如sa[3].sb.x，中的第一个dot
			constSUEFieldNode *rightField_Struct = NULL;
			SUEtype* temsuetype = NULL;
			rightField_Struct = FindFieldSUEnode(sueArrayElement, right);
			assert(rightField_Struct);
			//node->u.binop.value = NewNode(Sdcl);
			//node->u.binop.value->u.sdcl.type = node->u.binop.type;
			temsuetype = HeapNew(SUEtype);
			temsuetype->fields = rightField_Struct->u.sueidNode->SUEfields;  //可能要深拷贝，重写一个listcopy函数
			node->u.binop.valueList = temsuetype->fields;
			//node->u.binop.value->u.sdcl.type->fields = temsuetype->fields;

		}
		else if (dottype->typ == Udcl)
		{ ; //暂不处理
		}
		else if (dottype->typ == Edcl)
		{  ;//暂不处理
		}
		else UNREACHABLE;
	}
	else if (left->typ==Binop&&left->u.binop.op =='.' && right->typ == Id )
	{//从左值的dot的value中找右值的Id
		List *leftstructfields;
		assert(left->u.binop.valueList);
		//leftstructfields = left->u.binop.value->u.sdcl.type->fields;
		leftstructfields = left->u.binop.valueList;

		if (dottype->typ == Prim)
		{  //sa.sb.x  对第二个dot的处理
			constSUEFieldNode *rightField_id = NULL;	
			rightField_id = FindFieldSUEnode(leftstructfields, right);
			assert(rightField_id);
			if(rightField_id->u.idnode->n->u.id.value!=NULL &&rightField_id->u.idnode->n->u.id.value->typ ==Const)
			{
				node->u.binop.value = GetValue(rightField_id->u.idnode->n->u.id.value);
				right->u.id.value = GetValue(rightField_id->u.idnode->n->u.id.value);
			}
		}
		else if (dottype->typ == Adcl)
		{;//暂不支持，暂不处理

		}
		else if(dottype->typ == Sdcl)
		{//sa.sa.sc.x 对第二个dot的处理
			constSUEFieldNode *rightField_Struct = NULL;
			SUEtype* temsuetype = NULL;
			rightField_Struct = FindFieldSUEnode(leftstructfields, right);
			assert(rightField_Struct);
			//node->u.binop.value = NewNode(Sdcl);
			//node->u.binop.value->u.sdcl.type = node->u.binop.type;
			temsuetype = HeapNew(SUEtype);
			temsuetype->fields = rightField_Struct->u.sueidNode->SUEfields;  //可能要深拷贝，重写一个listcopy函数
			node->u.binop.valueList = temsuetype->fields;
			//node->u.binop.value->u.sdcl.type = temsuetype;
		}
		else if (dottype->typ == Udcl)
		{  ; //暂不处理
		}
		else if (dottype->typ == Edcl)
		{  ; //暂不处理
		}
		else UNREACHABLE;
	}
	return v;
}

GLOBAL FlowValue AlterDot(Node *node, Node *value, FlowValue v)
{//node是dot结点，value是更新后的值
	Node *right=NULL ,*left=NULL;
	Node *dottype = node->u.binop.type;//dot的类型就是它右值的类型
	constSUE *ptrSUE=NULL;
	List *SUEflowlist = ((propagatorNode*)(v.u.ptr))->SUEFlowList;
	assert(value);
	assert(node && node->u.binop.op == '.');
	right = node->u.binop.right;
	left = node->u.binop.left;
	assert(left);
	assert(right);

	if(left->typ == Id && right->typ ==Id)
	{  //sa.x
		Node *leftiddecl = left->u.id.decl;
		SUEtype *leftsuetype = leftiddecl->u.decl.type->u.tdef.type->u.sdcl.type;
		constSUE* leftsuetypeNode = FindSUEtype(leftsuetype, SUEflowlist);
		constSUEid *sueidNode = NULL;
		constSUEFieldNode *rightField_id = NULL;
		assert(leftsuetypeNode);
		sueidNode = FindSUEid(leftiddecl,leftsuetypeNode->SUEnode->SUEid);//找到结构体，即dot的左值
		assert(sueidNode);

		if (dottype->typ == Prim)
		{//形如s.x
			constSUEFieldNode *rightField_id = NULL;	
			rightField_id = FindFieldSUEnode(sueidNode->SUEfields, right);
			assert(rightField_id);
			rightField_id->u.idnode->n->u.id.value = GetValue(value);
			right->u.id.value = GetValue(value);
			node->u.binop.value = right->u.id.value;
		}
		else if (dottype->typ == Adcl)
		{//形如s.x[2]，右值可能是一般数组，结构体数组等,暂时不支持，未处理
			;
		}
		else if(dottype->typ == Sdcl)
		{//形如sa.sb.x，中的第一个dot, 什么都不用做
			return v;
		}
		else if (dottype->typ == Udcl)
		{ ; //暂不处理
		}
		else if (dottype->typ == Edcl)
		{ ; //暂不处理
		}
		else UNREACHABLE; 
	}	
	else if (left->typ == Array && right->typ == Id)
	{  //sa[2].x
		Node *leftarraydecl = left->u.array.type;
		SUEtype *leftsuetype = leftarraydecl->u.sdcl.type;
		constSUE* leftsuetypeNode = FindSUEtype(leftsuetype, SUEflowlist);
		unsigned long *pos, tmp, i = 0, j;
		unsigned long place=0;
		ListMarker m;
		Node *n, *dimvalue =NULL;
		List *sueArrayElement=NULL;

		constSUEarray *suearrayNode = NULL;
		constSUEFieldNode *rightField_id = NULL;

		assert(leftsuetypeNode);
		suearrayNode = FindSUEarray(left,leftsuetypeNode->SUEnode->SUEarray);//找到结构体，即dot的左值
		assert(suearrayNode);

		pos=(unsigned long *)malloc(sizeof(unsigned long)*suearrayNode->SUEdim);
		IterateList(&m, left->u.array.dims);
		while (NextOnList(&m, (GenericREF) &n))
		{
			dimvalue=GetValue(n);
			// assert(value); // 12.13 数据流进work
			if(value!=NULL) pos[i++]=NodeConstantIntegralValue(dimvalue);
			else return v;
		}
		for (i=0;i<suearrayNode->SUEdim;i++)
		{
			tmp=1;
			for(j=i+1;j<suearrayNode->SUEdim;j++)
				tmp=tmp*suearrayNode->SUEdimLen[j];
			place=place+pos[i]*tmp;
		}
		sueArrayElement = suearrayNode->SUEelement[place];

		if (dottype->typ == Prim)
		{//形如s[2].x
			constSUEFieldNode *rightField_id = NULL;	
			rightField_id = FindFieldSUEnode(sueArrayElement, right);
			assert(rightField_id);
			rightField_id->u.idnode->n->u.id.value = GetValue(value);
			right->u.id.value = GetValue(value);
		//	node->u.binop.value = right->u.id.value;
			node->u.binop.value = GetValue(value);
		}
		else if (dottype->typ == Adcl)
		{//形如s[3].x[2]   暂时不支持，未处理
			;
		}
		else if(dottype->typ == Sdcl)
		{//形如sa[3].sb.x，中的第一个dot ,什么也不做
			return v;
		}
		else if (dottype->typ == Udcl)
		{ ; //暂不处理
		}
		else if (dottype->typ == Edcl)
		{  ;//暂不处理
		}
		else UNREACHABLE;

	}
	else if (left->typ==Binop && left->u.binop.op=='.' && right->typ == Id)
	{  //sa.sb.x
		List *leftstructfields;
		assert(left->u.binop.valueList);
		//leftstructfields = left->u.binop.value->u.sdcl.type->fields;
		leftstructfields = left->u.binop.valueList;

		if (dottype->typ == Prim)
		{  //sa.sb.x  对第二个dot的处理
			constSUEFieldNode *rightField_id = NULL;	
			rightField_id = FindFieldSUEnode(leftstructfields, right);
			assert(rightField_id);
			rightField_id->u.idnode->n->u.id.value = GetValue(value);
			right->u.id.value = GetValue(value);
			node->u.binop.value = right->u.id.value;
		}
		else if (dottype->typ == Adcl)
		{;//暂不支持，暂不处理

		}
		else if(dottype->typ == Sdcl)
		{//sa.sa.sc.x 对第二个dot的处理，什么也不做
			return v;
		}
		else if (dottype->typ == Udcl)
		{  ; //暂不处理
		}
		else if (dottype->typ == Edcl)
		{  ; //暂不处理
		}
		else UNREACHABLE;
	}
	return v;
}
