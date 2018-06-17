#include "ActorEdgeInfo.h"
ActorEdgeInfo::ActorEdgeInfo(SchedulerSSG* sssg)
{
	this->flatNodes_ = sssg->flatNodes;
	vector<FlatNode *>::iterator iter_1,iter_2;
	for (iter_1=flatNodes_.begin();iter_1!=flatNodes_.end();++iter_1)//遍历所有结点
	{
		for (iter_2=(*iter_1)->outFlatNodes.begin();iter_2!=(*iter_1)->outFlatNodes.end();iter_2++)//遍历节点输出边连接的结点
		{
			Node *outputNode = NULL;
			List *outputList =(*iter_1)->contents->decl->u.decl.type->u.operdcl.outputs;
			ListMarker output_maker; 
			StreamType *streamType = NULL;
			IterateList(&output_maker,outputList);
			int outOrder = iter_2 - (*iter_1)->outFlatNodes.begin(),i=0;			
			while(NextOnList(&output_maker,(GenericREF)&outputNode))			//查找streamType
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
			while(NextOnList(&streamField_maker,(GenericREF)&fieldNode))		//查找streamType的类型域
			{
				if (fieldNode->u.decl.type->typ == Prim) // 基本类型
				{
					Node *typeNode = fieldNode->u.decl.type;
					int curSize;
					type = GetPrimDataType(typeNode,&curSize);
					typeName += (type + "_" + fieldNode->u.decl.name);
					typedefine += (type + " " + fieldNode->u.decl.name + ";\n");
					size += curSize;
				}
				else if(fieldNode->u.decl.type->typ == Adcl)
				{
					Node *typeNode = fieldNode->u.decl.type->u.adcl.type;
					int curSize;
					type = GetPrimDataType(typeNode, &curSize);
					xstemp = fieldNode->u.decl.name;
					typeName += (type + "_" + xstemp);
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
			if(stIter==vStreamTypeInfo.end())	//添加新类型
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
string ActorEdgeInfo::GetPrimDataType(Node *from,int *size)//类型定义
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
		type = "UInt";
		*size = sizeof(unsigned int);
		break;
	case Slong:
		type = "Long";;
		*size = sizeof(long);
		break;
	case Ulong:
		type = "ULong";;
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