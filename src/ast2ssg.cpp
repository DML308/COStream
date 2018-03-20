#include "staticStreamGraph.h"

using namespace std;

PRIVATE StaticStreamGraph *ssg = NULL;
GLOBAL bool hasUncertainty(StaticStreamGraph *ssg)			//判断是否存在uncertainty
{
	std::vector<FlatNode*> flatNodes = ssg->flatNodes;

	for (int i = 0; i < flatNodes.size(); ++i)
	{
		//循环算子
		List* windowlist = flatNodes[i]->contents->body->u.operBody.window;
		ListMarker maker;
		IterateList(&maker, windowlist);
		Node*node = NULL;
		while (NextOnList(&maker, (GenericREF)&node))
		{
			NodeType nodetype = node->u.window.wtype->typ;
			if (nodetype == Uncertainty)
				return true;
		}
	}
	return false;
}
GLOBAL bool CheckSplitUncertainty(StaticStreamGraph *ssg)			//判断splitjoin/roundrobin中是否存在uncertain节点,有则返回false，否则返回true
{
	//bool splitjoinFlag = false;

	std::vector<FlatNode*> flatNodes = ssg->flatNodes;
	for (int i = 0; i < flatNodes.size(); i++)
	{

		if (strstr(flatNodes[i]->name.c_str(), "Duplicate") || strstr(flatNodes[i]->name.c_str(), "Roundrobin") != NULL)		//找到duplicate/roundrobin节点
		{
			//接下来需要对其中的join节点进行处理
			//splitjoinFlag = true;
			for (int j = i; j < flatNodes.size(); j++)		//从剩下的节点中找到第一个join节点
			{

				if (strstr(flatNodes[j]->name.c_str(), "Join") != NULL)
				{
					break;
				}
				else
				{
					List * windowlist = flatNodes[j]->contents->body->u.operBody.window;
					if (ListLength(windowlist) == 1)
					{
						Node* windowNode = (Node*)FirstItem(windowlist);
						if (windowNode->typ == Uncertainty)
							return false;
					}

				}
			}
		}

	}
	return true;
}

GLOBAL bool CheckSplitUncertaintybeifen(StaticStreamGraph *ssg)
{
	
	std::vector<FlatNode*> flatNodes = ssg->flatNodes;
	for (int i = 0;i < flatNodes.size();i++)
	{
		List* windowlist = flatNodes[i]->contents->body->u.operBody.window;
		ListMarker maker;
		IterateList(&maker, windowlist);
		Node*node = NULL;
		while (NextOnList(&maker, (GenericREF)&node))
		{
			NodeType nodetype = node->u.window.wtype->typ;
			if (nodetype == Uncertainty)
			{
				int incount = 0;
				int outcount = 0;
				//找到uncertaintynode
				for (int j = 0;j <=i;j++)
				{
					incount += flatNodes[j]->nIn;
					outcount += flatNodes[j]->nOut;
				}
				if (incount == outcount);
				else 
				{
					SyntaxError("uncertainty place error");
					return false;
				}
					
			}
		}
	}
	return true;
}

bool CheckCompositeNumber(List *proc, Node **com)
{
	int count = 0;
	while (proc != NULL)
	{
		Node *tmp = (Node *)FirstItem(proc);
		if (tmp->typ == Composite)
		{
			if(count != 0) return false;
			*com = tmp;
			count++;
		}
		proc = Rest(proc);
	}
	return true;
}

PRIVATE void GraphToOperators(Node *composite, Node *oldComposite)
{
	ListMarker marker;
	Node *item = NULL, *graph = NULL;
	static int count = 0;

	assert(composite && composite->typ == Composite);
	assert(oldComposite && oldComposite->typ == Composite);
	IterateList(&marker, composite->u.composite.body->u.comBody.comstmts);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		
		switch (item->typ) 
		{
		case Operator_:
			//printf("count = %d\t, operator name : %s\n", count++, oper->u.operator_.decl->u.decl.name);
			ssg->GenerateFlatNodes(&item->u.operator_, &oldComposite->u.composite, &composite->u.composite);
			break;

		case CompositeCall:
			GraphToOperators(item->u.comCall.actual_composite, item->u.comCall.call);
			break;
		case SplitJoin:
			GraphToOperators(item->u.splitJoin.replace_composite, item->u.splitJoin.replace_composite);
			break;
		case Pipeline:
			GraphToOperators(item->u.pipeline.replace_composite, item->u.pipeline.replace_composite);
			break;
		default:
			break;
		}
	}
}

GLOBAL StaticStreamGraph *AST2FlatStaticStreamGraph(Node *mainComposite)
{
	Node *compositeCall = NULL, *operNode = NULL;
	List *operators = NULL;	

	assert(mainComposite && mainComposite->typ == Composite && mainComposite->u.composite.decl->u.decl.type->u.comdcl.inout == NULL);
	assert(strcmp(mainComposite->u.composite.decl->u.decl.name, "Main") == 0);

	ssg = new StaticStreamGraph();
	GraphToOperators(mainComposite, mainComposite);

	ssg->SetTopLevel();
	ssg->SetFlatNodesWeights();
	ssg->ResetFlatNodeNames(); /*重置ssg内flatNodes的每个flatNode的name, 便于打印dot图*/

#if 0
	ssg->PrintFlatNodes();
	PrintNode(stdout, gMainComposite, 0);
	system("pause");
#endif
	
	return ssg;
}
