#include "ActorEdgeInfo.h"

ActorEdgeInfo::ActorEdgeInfo(DividedStaticGraph*dsg)
{
	this->flatNodes_ = dsg->getAllFlatNodes();
	vector<FlatNode*>::iterator iter_1, iter_2;
	for (iter_1 = flatNodes_.begin(); iter_1 != flatNodes_.end(); iter_1++)
	{
		//�������нڵ�
		for (iter_2 = (*iter_1)->outFlatNodes.begin(); iter_2 != (*iter_1)->outFlatNodes.end(); ++iter_2)
		{
			//�����ڵ���������ӵĽڵ�
			Node*outputNode = NULL;
			List*outputList = (*iter_1)->contents->decl->u.decl.type->u.operdcl.outputs;
			//����б�
			ListMarker output_maker;
			StreamType *streamType = NULL;
			IterateList(&output_maker, outputList);
			int outOrder = iter_2 - (*iter_1)->outFlatNodes.begin(), i = 0;
			//outOrder��ʾiter2��iter1������ڵ��еı��
			while (NextOnList(&output_maker, (GenericREF)&outputNode))
			{
				if (i == outOrder)	//�ҵ���Ӧ�ڵ�����streamType
				{
					if (outputNode->typ == Id) streamType = outputNode->u.id.decl->u.decl.type->u.strdcl.type;
					else if (outputNode->typ == Decl)streamType = outputNode->u.decl.type->u.strdcl.type;
					break;
				}
				if (i < outOrder)
				{
					i++;
				}
			}
			List*streamField = streamType->fields;
			ListMarker streamField_maker;
			IterateList(&streamField_maker, streamField);
			Node*fieldNode = NULL;
			string type, typeName, typedefine, xstemp;
			int size = 0;
			while (NextOnList(&streamField_maker, (GenericREF)&fieldNode))
			{
				if (fieldNode->u.decl.type->typ == Prim)
				{
					Node *typeNode = fieldNode->u.decl.type;
					int curSize;
					type = GetPrimDataType(typeNode, &curSize);
					typeName += (type + "_" + fieldNode->u.decl.name);
					//typeName += ",";
					typedefine += (type + " " + fieldNode->u.decl.name + ";\n");
					size += curSize;
				}
				else if (fieldNode->u.decl.type->typ == Adcl)
				{
					Node*typeNode = fieldNode->u.decl.type->u.adcl.type;
					int curSize;
					type = GetPrimDataType(typeNode, &curSize);
					xstemp = fieldNode->u.decl.name;
					typeName += (type + "_" + xstemp);
					//typeName += ",";
					typedefine += (type + " " + xstemp + "[" + fieldNode->u.decl.type->u.adcl.dim->u.Const.text + "]" + ";\n");
					size += curSize;
				}
			}
			/*���������޸�*/
			/*int stringsize = typeName.size();
			if (typeName[stringsize - 1] == ',)
			{
				typeName = typeName.substr(0, stringsize - 1);
			}
			*/

			vector<StreamEdgeInfo>::iterator stIter = vStreamTypeInfo.begin();
			for (; stIter != vStreamTypeInfo.end(); ++stIter)
			{
				if (typeName == (*stIter).typeName){
					mapEdge2TypeInfo.insert(make_pair(make_pair((*iter_1), (*iter_2)), (*stIter)));
					break;
				}
			}
			if (stIter == vStreamTypeInfo.end())
			{
				//���������
				vStreamTypeInfo.push_back(StreamEdgeInfo(size, typeName, typedefine));
				mapEdge2TypeInfo.insert(make_pair(make_pair((*iter_1), (*iter_2)), vStreamTypeInfo[vStreamTypeInfo.size() - 1]));
			}
		}
	}
}

ActorEdgeInfo::ActorEdgeInfo(SchedulerSSG* sssg)
{
	this->flatNodes_ = sssg->flatNodes;
	vector<FlatNode *>::iterator iter_1,iter_2;
	for (iter_1=flatNodes_.begin();iter_1!=flatNodes_.end();++iter_1)//�������н��
	{
		for (iter_2=(*iter_1)->outFlatNodes.begin();iter_2!=(*iter_1)->outFlatNodes.end();iter_2++)//�����ڵ���������ӵĽ��
		{
			Node *outputNode = NULL;
			List *outputList =(*iter_1)->contents->decl->u.decl.type->u.operdcl.outputs;
			ListMarker output_maker; 
			StreamType *streamType = NULL;
			IterateList(&output_maker,outputList);
			int outOrder = iter_2 - (*iter_1)->outFlatNodes.begin(),i=0;			
			while(NextOnList(&output_maker,(GenericREF)&outputNode))			//����streamType
			{
				if(i == outOrder){
					if(outputNode->typ == Id) streamType = outputNode->u.id.decl->u.decl.type->u.strdcl.type;
					else if (outputNode->typ == Decl)  streamType = outputNode->u.decl.type->u.strdcl.type;
					break;
				}
				if(i<outOrder)
					i++;
			}
			List *streamField = streamType->fields;
			ListMarker streamField_maker;
			IterateList(&streamField_maker,streamField);
			Node *fieldNode = NULL;
			string type,typeName,typedefine, xstemp;
			int size = 0;
			while(NextOnList(&streamField_maker,(GenericREF)&fieldNode))		//����streamType��������
			{
				if (fieldNode->u.decl.type->typ == Prim) // ��������
				{
					Node *typeNode = fieldNode->u.decl.type;
					int curSize;
					type = GetPrimDataType(typeNode,&curSize);
					typeName += "_"; // add by mobisnheng
					typeName += (/*type + "_" + */fieldNode->u.decl.name); // modify by mobinsheng
					typedefine += (type + " " + fieldNode->u.decl.name + ";\n");
					size += curSize;
				}
				else if(fieldNode->u.decl.type->typ == Adcl)
				{
					Node *typeNode = fieldNode->u.decl.type->u.adcl.type;
					int curSize;
					type = GetPrimDataType(typeNode, &curSize);
					xstemp = fieldNode->u.decl.name;
					typeName += "_"; // add by mobinsheng
					typeName += (/*type + "_" + */xstemp);// modify by mobinsheng
					typedefine += (type + " " + xstemp + "[" + fieldNode->u.decl.type->u.adcl.dim->u.Const.text + "]" + ";\n");
					size += curSize;
				}
			}
			vector<StreamEdgeInfo>::iterator stIter = vStreamTypeInfo.begin();
			for(;stIter!=vStreamTypeInfo.end();stIter++)
			{
				if(typeName == (*stIter).typeName){
					mapEdge2TypeInfo.insert(make_pair(make_pair((*iter_1),(*iter_2)),(*stIter)));
					break;
				}
			}
			if(stIter==vStreamTypeInfo.end())	//���������
			{
				vStreamTypeInfo.push_back(StreamEdgeInfo(size,typeName,typedefine));
				mapEdge2TypeInfo.insert(make_pair(make_pair((*iter_1),(*iter_2)),vStreamTypeInfo[vStreamTypeInfo.size()-1]));
			}

		}
	}
}
StreamEdgeInfo ActorEdgeInfo::getEdgeInfo(FlatNode* actorSend,FlatNode* actorRecv)
{
	string edgename = actorSend->name + actorRecv->name;
	return mapEdge2TypeInfo.find(make_pair(actorSend,actorRecv))->second;

}
void ActorEdgeInfo::DeclEdgeType(stringstream &buf)
{
	for(int i=0;i<this->vStreamTypeInfo.size();i++)
	{	
		buf<<"struct ";
		buf<<vStreamTypeInfo.at(i).typeName<<"\n{\n\t"<<vStreamTypeInfo.at(i).typedefine<<"};\n";
	}
}

string GetPrimDataType(Node *from,int *size)//���Ͷ���
{
	string type;

	switch(from->u.prim.basic){
	case Sshort:
	case Sint:
		type = "int";
		*size = sizeof(int);
		break;
	case Uint:
	case Ushort:
		type = "unsigned int"; // modify by mobinsheng
		*size = sizeof(unsigned int);
		break;
	case Slong:
		type = "long";; // modify by mobinsheng
		*size = sizeof(long);
		break;
	case Ulong:
		type = "unsigned long";; // modify by mobinsheng
		*size = sizeof(unsigned long);
		break;
	case Float:
		type = "float";
		*size = sizeof(float);
		break;
	case Double:
		type = "double";
		*size = sizeof(double);
		break;
	case Char:
	case Schar:
	case Uchar:
		type = "char";
		*size = sizeof(char);
		break;
	default: type = "Any";
		break;
	}
	return type;
}

string GetPrimDataType(Node * from)
{
	string type;
	switch (from->u.prim.basic) {
	case Sshort:
	case Sint:
		type = "int";;
		break;
	case Uint:
	case Ushort:
		type = "unsigned int";; // modify by mobinsheng
		break;
	case Slong:
		type = "long";; // modify by mobinsheng
		break;
	case Ulong:
		type = "unsigned long";; // modify by mobinsheng
		break;
	case Float:
		type = "float";
		break;
	case Double:
		type = "double";
		break;
	case Char:
	case Schar:
	case Uchar:
		type = "char";
		break;
	case Void:
		type = "void";
		break;
	default: type = "Any";
		break;
	}
	return type;
}

string GetArrayDataType(Node *node)
{
	string type;
	if (node->typ == Prim) //��������
	{
		type = GetPrimDataType(node);
	}
	else if (node->typ == Ptr) //chenzhen20151208
	{

		if (node->u.ptr.type->typ == Ptr)  //�༶ָ�룬�ݹ�ִ��
		{
			type = GetArrayDataType(node->u.ptr.type);
		}
		else if (node->u.ptr.type->typ == Tdef)
		{
			type = node->u.ptr.type->u.tdef.name;
		}
		else if (node->u.ptr.type->typ == Prim)
		{
			type = GetPrimDataType(node->u.ptr.type);
		}
		else
		{

		}

		type += "*";
	}
	else if (node->typ == Tdef)  //chenzhen20151208
	{
		type = node->u.tdef.name;
	}
	else if (node->typ == Adcl) // Ҳ�Ǹ�������ݹ��������
	{
		stringstream ss;
		ss << GetArrayDataType(node->u.adcl.type);
		type = ss.str();
	}
	else // �������ĳ�Ա�Ǹ������ͣ����д���չ
	{
		Warning(1, "this arrayDataType can not be handle!");
		type = "Any";// ��ʱ����һ��ͨ������
		UNREACHABLE;
	}
	return type;
}

/*��������ά��*/
string GetArrayDim(Node * node)
{
	string dim;
	if (node->typ == Const)
	{
		if (node->u.Const.text)
			dim = node->u.Const.text;
		else
		{
			char *tmpdim = (char *)malloc(20);
			sprintf(tmpdim, "%d", node->u.Const.value.l);
			dim = tmpdim;
		}//else
	}//if
	else if (node->typ == Id) {
		dim = node->u.id.text;
	}
	return dim;
}

