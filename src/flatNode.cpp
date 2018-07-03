#include "flatNode.h"


FlatNode::FlatNode(operatorNode *node, compositeNode *com, compositeNode *newCom)
{
	name = std::string(node->decl->u.decl.name);
	visitTimes = 0;
	memorizedNode = false;

	contents = node;
	composite = newCom;
	ListMarker marker;
	Node *item = NULL;

	IterateList(&marker, com->body->u.comBody.comstmts);
	while (NextOnList(&marker, GenericREF(&item)))
	{
		if (item->typ == Operator_ && strcmp(node->decl->u.decl.name, item->u.operator_.decl->u.decl.name) == 0)
		{
			oldContents = &item->u.operator_;
			break;
		}
	}
	oldComposite = com;

	nOut = 0;
	nIn = 0;

	currentIn = 0;
	currentOut = 0;

	// other inits
	currentIn = 0;
	currentOut = 0;
	schedMult = 0;
	schedDivider = 0;
	uin = 0;
	label = 0;
}

FlatNode::FlatNode(operatorNode *node)
{//20120717 zww 添加 
	name = std::string(node->decl->u.decl.name);
	visitTimes = 0;
	memorizedNode = false;

	contents = node;
	composite = NULL;
	oldContents = node;
	oldComposite = NULL;

	nOut = 0;
	nIn = 0;

	currentIn = 0;
	currentOut = 0;

	// other inits
	currentIn = 0;
	currentOut = 0;
	schedMult = 0;
	schedDivider = 0;
	uin = 0;
	label = 0;
}

void FlatNode::SetIOStreams()
{
	int len = 0;
	ListMarker marker;
	Node *item = NULL;
	char tmp[50];

	len = ListLength(oldContents->decl->u.decl.type->u.operdcl.outputs);
	assert(len == nOut); // 暂不支持一条边被多个operator重用

	IterateList(&marker, oldContents->decl->u.decl.type->u.operdcl.outputs); 
	while (NextOnList(&marker, (GenericREF) &item)) {
		assert(item->typ == Id || item->typ == Decl);
		if (item->typ == Id)
		{
			sprintf(tmp, "%s", item->u.id.text);
			pushString.push_back(tmp);
		}
		else 
		{
			sprintf(tmp, "%s", item->u.decl.name);
			pushString.push_back(tmp);
		}
	}
	len = ListLength(oldContents->decl->u.decl.type->u.operdcl.inputs);
	assert(len == nIn);// 暂不支持一条边被多个operator重用
	IterateList(&marker, oldContents->decl->u.decl.type->u.operdcl.inputs); 
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		assert(item->typ == Id);
		sprintf(tmp, "%s", item->u.id.text);
		peekString.push_back(tmp);
	}
}

void FlatNode::AddOutEdges(FlatNode* dest)
{
	outFlatNodes.push_back(dest);
	++nOut;
}

void FlatNode::AddInEdges(FlatNode *src)
{
	inFlatNodes.push_back(src);
	++nIn;
}

std::string FlatNode::GetOperatorName()
{
	return std::string(contents->decl->u.decl.name);
}

void FlatNode::VisitNode()
{
	visitTimes++;
}

void FlatNode::ResetVistTimes()
{
	visitTimes = 0;
}

int FlatNode::GetVisitTimes()
{
	return visitTimes;
}