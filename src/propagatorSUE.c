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
		field_decl = &(field->u.decl); //ȡ���ṹ���е�ÿ����Ա�Ķ���
		field_type = NodeDataType(field_decl->type);
		if(field_decl->type->typ == Tdef)
		{  //�ṹ���Ա����tdef���������
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
		{//�ṹ��ĳ�Ա������������ ���÷�֧��ִ��
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
				lenPerDim[i++] = CheckAdcl(tmptype);//����ά�ĳ��ȣ����ظ�ά���ȵ�ֵ
				tmptype = tmptype->u.adcl.type;
			}
			if(tmptype->typ==Tdef)
			{  ; //��δ����

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
			{  ;//һ�㲻����� ,�ݲ�����
			}			
		} 		
	}
	return newfield;

}

GLOBAL constSUEid* MakeStructNode(Node *node)
{  //����ṹ��
	constSUEid *SUEidnode = HeapNew(constSUEid);
	Node *type = node->u.decl.type;	 //�ṹ�������
	assert(node && type->typ==Sdcl);
	SUEidnode->decl = node;
	SUEidnode->SUEfields = MakeFieldList(type->u.sdcl.type->fields);
	return SUEidnode;
}

GLOBAL constSUEid *MakeTdefNode(Node *node)
{//tdef���������ʱֻ����Sdcl  ,�����ݲ�����
	Node *decl = NULL;   //��¼tdef��ָ�Ľṹ����sdcl
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
{  //��������
	constSUEid *SUEidnode = HeapNew(constSUEid);
	Node *type = node->u.decl.type;
	assert(node && type->typ==Udcl);
	SUEidnode->decl = node;
	SUEidnode->SUEfields = MakeFieldList(type->u.udcl.type->fields);
	return SUEidnode;

}

GLOBAL constSUEarray* MakeStructArrayNode(Node *node)
{  //����ṹ������
	constSUEarray *SUEarray = HeapNew(constSUEarray);
	int dim = 0,i = 0,num=1;
	Node *tmp_type = node->u.decl.type;	
	int *lenPerDim = NULL;
	SUEarray->SUEname = node->u.decl.name;
	SUEarray->SUEdecl = node;
	while(tmp_type->typ == Adcl)
	{//ѭ��������tmp_typeָ��Sdcl
		++dim;
		tmp_type = tmp_type->u.adcl.type;
	}
	tmp_type = node->u.decl.type;
	SUEarray->SUEdim = dim;
	lenPerDim = (int *)malloc(sizeof(int)*dim);
	i=0;
	while(tmp_type->typ == Adcl)	
	{
		lenPerDim[i++] = CheckAdcl(tmp_type);//����ά�ĳ��ȣ����ظ�ά���ȵ�ֵ
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
{  //����ṹ������
	constSUEarray *SUEarray = HeapNew(constSUEarray);
	int dim = 0,i = 0,num=1;
	Node *tdeftype=NULL;
	Node *tmp_type = node->u.decl.type;	
	int *lenPerDim = NULL;
	SUEarray->SUEname = node->u.decl.name;
	SUEarray->SUEdecl = node;
	while(tmp_type->typ == Adcl)
	{//ѭ��������tmp_typeָ��Sdcl
		++dim;
		tmp_type = tmp_type->u.adcl.type;
	}
	tmp_type = node->u.decl.type;
	SUEarray->SUEdim = dim;
	lenPerDim = (int *)malloc(sizeof(int)*dim);
	i=0;
	while(tmp_type->typ == Adcl)	
	{
		lenPerDim[i++] = CheckAdcl(tmp_type);//����ά�ĳ��ȣ����ظ�ά���ȵ�ֵ
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
{//������������
	constSUEarray *SUEarray = HeapNew(constSUEarray);
	int dim = 0,i = 0,num=1;
	Node *tmp_type = node->u.decl.type;	
	int *lenPerDim = NULL;
	SUEarray->SUEname = node->u.decl.name;
	SUEarray->SUEdecl = node;
	while(tmp_type->typ == Adcl)
	{//ѭ��������tmp_typeָ��Udcl
		++dim;
		tmp_type = tmp_type->u.adcl.type;
	}
	SUEarray->SUEdim = dim;
	lenPerDim = (int *)malloc(sizeof(int)*dim);
	i=0;
	while(tmp_type->typ == Adcl)	
	{
		lenPerDim[i++] = CheckAdcl(tmp_type);//����ά�ĳ��ȣ����ظ�ά���ȵ�ֵ
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
{//��ʱ������
	return NULL;
}

GLOBAL constSUEarray* MakeEnumArrayNode(Node *node)
{//��ʱ������
	return NULL;
}

GLOBAL constSUE* FindSUEtype(SUEtype *suetype,List *SUEflowlist)
{//���������в���type���͵Ľڵ�(type��һ�����͵Ķ���ڵ�)
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
	while (NextOnList(&marker, (GenericREF) &tmpsueid)) {   //�ҽṹ����
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
	while (NextOnList(&marker, (GenericREF) &tmpsuearray)) {   //�ҽṹ����
		if(node->u.array.name->u.id.text == tmpsuearray->SUEname && node->u.array.name->u.id.decl == tmpsuearray->SUEdecl ) 
			suearrayNode = tmpsuearray;
	}
	assert(suearrayNode);
	return suearrayNode;
}

GLOBAL constSUEFieldNode *FindFieldSUEnode(List *fieldsList, Node *field_id)
{ //���ҽṹ���Ա
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
{//�ṹ�������ʼ��
	ListMarker marker;
	ListMarker markerSidField;
	ListMarker makerinit;
	Node *field;
	constSUEFieldNode *fieldidNode=NULL;
	Node *idinit = NULL;
	//	List *fieldsList = decl->u.decl.type->u.sdcl.type->fields;
	//IterateList(&marker, fieldsList);
	//while (NextOnList(&marker, (GenericREF) &field)) {   //��Ա�������飬�ṹ�壬������Ȳ��ܽ���ֱ�ӳ�ʼ��
	//declNode *field_decl;
	//assert(field->typ == Decl);
	//field_decl = &(field->u.decl); //ȡ���ṹ���е�ÿ����Ա�Ķ���
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
	assert(dim == 1); // ��ֻ֧��һά�ṹ������
	num = SUEarray->SUEnum; //ÿά�е�����Ԫ�ظ���
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
{//����ʱδ�ã����Ĺ�����InsertSUEtoFlow�д����ˣ���ʼ���ṹ��
	//��֧�ֹ������Լ��ṹ�����飬�Լ��ṹ���к��������Ա�ĳ�ʼ��	
	Node *dtype = node->u.decl.type;//node������
	constSUE *itemtype = NULL;//��¼dtype�Ƿ������������е�λ��
	constSUEid *SUEid = NULL;
	constSUEarray *SUEarray = NULL;
	assert(node && node->typ==Decl);
	if(dtype->typ==Sdcl) 
	{
		SUEid = MakeStructNode(node);
		if(node->u.decl.init!=NULL)SUEid = InitStructid(SUEid,node);	
	}
	else if(dtype->typ == Udcl)
	{ //��֧�ֳ�ʼ�� 
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
			SUENode->SUEid = AppendItem(SUENode->SUEid,MakeStructNode(node));   //���������Ҫ�ĳ��ܳ�ʼ���ĺ���  ����������
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
			SUENode->SUEid = AppendItem(SUENode->SUEid,MakeStructNode(node));   //���������Ҫ�ĳ��ܳ�ʼ���ĺ���  ����������
			ptrSUE->type = node->u.tdef.type->u.sdcl.type;
			ptrSUE->SUEnode = SUENode;
		}
		else 
		{
			assert(ptrSUE);
			ptrSUE->SUEnode->SUEid = AppendItem(ptrSUE->SUEnode->SUEid,MakeTdefNode(node));
		}
	}
	else if(node->u.decl.type->typ == Edcl);//�ݲ�����

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
			SUENode->SUEarray = AppendItem(SUENode->SUEarray,MakeStructArrayNode(node));   //���������Ҫ�ĳ��ܳ�ʼ���ĺ���  ����������
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
			SUENode->SUEarray = AppendItem(SUENode->SUEarray,MakeTdefArrayNode(node));   //���������Ҫ�ĳ��ܳ�ʼ���ĺ���  ����������
			ptrSUE->type = node->u.tdef.type->u.sdcl.type;
			ptrSUE->SUEnode = SUENode;
		}
		else 
		{
			assert(ptrSUE);
			ptrSUE->SUEnode->SUEarray = AppendItem(ptrSUE->SUEnode->SUEarray,MakeStructNode(node));
		}
	}
	else if(tmptype->typ == Edcl);//�ݲ�����

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
	{	//�ݲ�����
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
{  //���������������Ҫ��ֵ����'.'��value��.dot����ֵҪô��id��Ҫô�����飬����ֵҪô��id��Ҫô�����飬Ҫô��dot
	Node *tmpnode = NULL;
	Node *field = NULL,
		*left, *right, *ltype, *rtype, *type;
	binopNode *u;
	Node *dottype;//dot�����;�������ֵ������
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
		sueidNode = FindSUEid(leftiddecl,leftsuetypeNode->SUEnode->SUEid);//�ҵ��ṹ�壬��dot����ֵ
		assert(sueidNode);

		if (dottype->typ == Prim)
		{//����s.x
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
		{//YQJ �����������֧��  ����s.x[2]��
			// ���ڲ��ܻ�����������ֵ �ʸ���TransformArray �д���

		}
		else if(dottype->typ == Sdcl)
		{//����sa.sb.x���еĵ�һ��dot
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
			//node->u.binop.value->u.sdcl.type->fields = temsuetype->fields;//�Ѳ��ҳ������ڲ��ṹ��ĸ���Աֵdot��value�У���value�����sdcl�ڵ㣩
		}
		else if (dottype->typ == Udcl)
		{  ;//�ݲ�����
		}
		else if (dottype->typ == Edcl)
		{ ; //�ݲ�����
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
			suearrayNode = FindSUEarray(left,leftsuetypeNode->SUEnode->SUEarray);//�ҵ��ṹ�壬��dot����ֵ
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
		{//����s[2].x
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
		{//����s[3].x[2]   ��ʱ��֧�֣�δ����
			;
		}
		else if(dottype->typ == Sdcl)
		{//����sa[3].sb.x���еĵ�һ��dot
			constSUEFieldNode *rightField_Struct = NULL;
			SUEtype* temsuetype = NULL;
			rightField_Struct = FindFieldSUEnode(sueArrayElement, right);
			assert(rightField_Struct);
			//node->u.binop.value = NewNode(Sdcl);
			//node->u.binop.value->u.sdcl.type = node->u.binop.type;
			temsuetype = HeapNew(SUEtype);
			temsuetype->fields = rightField_Struct->u.sueidNode->SUEfields;  //����Ҫ�������дһ��listcopy����
			node->u.binop.valueList = temsuetype->fields;
			//node->u.binop.value->u.sdcl.type->fields = temsuetype->fields;

		}
		else if (dottype->typ == Udcl)
		{ ; //�ݲ�����
		}
		else if (dottype->typ == Edcl)
		{  ;//�ݲ�����
		}
		else UNREACHABLE;
	}
	else if (left->typ==Binop&&left->u.binop.op =='.' && right->typ == Id )
	{//����ֵ��dot��value������ֵ��Id
		List *leftstructfields;
		assert(left->u.binop.valueList);
		//leftstructfields = left->u.binop.value->u.sdcl.type->fields;
		leftstructfields = left->u.binop.valueList;

		if (dottype->typ == Prim)
		{  //sa.sb.x  �Եڶ���dot�Ĵ���
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
		{;//�ݲ�֧�֣��ݲ�����

		}
		else if(dottype->typ == Sdcl)
		{//sa.sa.sc.x �Եڶ���dot�Ĵ���
			constSUEFieldNode *rightField_Struct = NULL;
			SUEtype* temsuetype = NULL;
			rightField_Struct = FindFieldSUEnode(leftstructfields, right);
			assert(rightField_Struct);
			//node->u.binop.value = NewNode(Sdcl);
			//node->u.binop.value->u.sdcl.type = node->u.binop.type;
			temsuetype = HeapNew(SUEtype);
			temsuetype->fields = rightField_Struct->u.sueidNode->SUEfields;  //����Ҫ�������дһ��listcopy����
			node->u.binop.valueList = temsuetype->fields;
			//node->u.binop.value->u.sdcl.type = temsuetype;
		}
		else if (dottype->typ == Udcl)
		{  ; //�ݲ�����
		}
		else if (dottype->typ == Edcl)
		{  ; //�ݲ�����
		}
		else UNREACHABLE;
	}
	return v;
}

GLOBAL FlowValue AlterDot(Node *node, Node *value, FlowValue v)
{//node��dot��㣬value�Ǹ��º��ֵ
	Node *right=NULL ,*left=NULL;
	Node *dottype = node->u.binop.type;//dot�����;�������ֵ������
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
		sueidNode = FindSUEid(leftiddecl,leftsuetypeNode->SUEnode->SUEid);//�ҵ��ṹ�壬��dot����ֵ
		assert(sueidNode);

		if (dottype->typ == Prim)
		{//����s.x
			constSUEFieldNode *rightField_id = NULL;	
			rightField_id = FindFieldSUEnode(sueidNode->SUEfields, right);
			assert(rightField_id);
			rightField_id->u.idnode->n->u.id.value = GetValue(value);
			right->u.id.value = GetValue(value);
			node->u.binop.value = right->u.id.value;
		}
		else if (dottype->typ == Adcl)
		{//����s.x[2]����ֵ������һ�����飬�ṹ�������,��ʱ��֧�֣�δ����
			;
		}
		else if(dottype->typ == Sdcl)
		{//����sa.sb.x���еĵ�һ��dot, ʲô��������
			return v;
		}
		else if (dottype->typ == Udcl)
		{ ; //�ݲ�����
		}
		else if (dottype->typ == Edcl)
		{ ; //�ݲ�����
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
		suearrayNode = FindSUEarray(left,leftsuetypeNode->SUEnode->SUEarray);//�ҵ��ṹ�壬��dot����ֵ
		assert(suearrayNode);

		pos=(unsigned long *)malloc(sizeof(unsigned long)*suearrayNode->SUEdim);
		IterateList(&m, left->u.array.dims);
		while (NextOnList(&m, (GenericREF) &n))
		{
			dimvalue=GetValue(n);
			// assert(value); // 12.13 ��������work
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
		{//����s[2].x
			constSUEFieldNode *rightField_id = NULL;	
			rightField_id = FindFieldSUEnode(sueArrayElement, right);
			assert(rightField_id);
			rightField_id->u.idnode->n->u.id.value = GetValue(value);
			right->u.id.value = GetValue(value);
		//	node->u.binop.value = right->u.id.value;
			node->u.binop.value = GetValue(value);
		}
		else if (dottype->typ == Adcl)
		{//����s[3].x[2]   ��ʱ��֧�֣�δ����
			;
		}
		else if(dottype->typ == Sdcl)
		{//����sa[3].sb.x���еĵ�һ��dot ,ʲôҲ����
			return v;
		}
		else if (dottype->typ == Udcl)
		{ ; //�ݲ�����
		}
		else if (dottype->typ == Edcl)
		{  ;//�ݲ�����
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
		{  //sa.sb.x  �Եڶ���dot�Ĵ���
			constSUEFieldNode *rightField_id = NULL;	
			rightField_id = FindFieldSUEnode(leftstructfields, right);
			assert(rightField_id);
			rightField_id->u.idnode->n->u.id.value = GetValue(value);
			right->u.id.value = GetValue(value);
			node->u.binop.value = right->u.id.value;
		}
		else if (dottype->typ == Adcl)
		{;//�ݲ�֧�֣��ݲ�����

		}
		else if(dottype->typ == Sdcl)
		{//sa.sa.sc.x �Եڶ���dot�Ĵ���ʲôҲ����
			return v;
		}
		else if (dottype->typ == Udcl)
		{  ; //�ݲ�����
		}
		else if (dottype->typ == Edcl)
		{  ; //�ݲ�����
		}
		else UNREACHABLE;
	}
	return v;
}
