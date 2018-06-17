#include "staticStreamGraph.h"

using namespace std;

PRIVATE StaticStreamGraph *ssg = NULL;

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
	ssg->GetPreName(); //cwb
	//ssg->SetTemplateNode(); //获取所有模板结点 chenwenbin 20140724
	//ssg->ResetTemplateName();
	ssg->ResetFlatNodeNames(); /*重置ssg内flatNodes的每个flatNode的name, 便于打印dot图*/
	

#if 0
	ssg->PrintFlatNodes();
	PrintNode(stdout, gMainComposite, 0);
	system("pause");
#endif
	
	return ssg;
}
