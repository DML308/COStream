#include "HorizontalFusionSSG.h"
#include "MetisPartiton.h"
#include "ActorStateDetector.h"
#include <set>
#include "rename.h"


#define THRESHOLD 0.05

PRIVATE int HFStreamNum = 0;//解决新构造的stream的重命名
PRIVATE int HFOperatorNum = 0;//解决新构造的operator的重命名
PRIVATE int HFVarNum = 0;



//************************************
// Qualifier: 构造函数
// Parameter: SchedulerSSG * sssg
//************************************
HorizontalFusionSSG::HorizontalFusionSSG(SchedulerSSG *sssg,int clusterNum)
{
	hfsssg = sssg;
	partitionNum = clusterNum;
	//flatNode2SteadyCount = hfsssg->mapSteadyCount2FlatNode;
	nvtxs = hfsssg->GetFlatNodes().size();
	bak_nvtxs = nvtxs;
	xadj = (int *)malloc(sizeof(int ) * (nvtxs + 1));
	bak_xadj = (int *)malloc(sizeof(int ) * (nvtxs + 1));
	edgenum = sssg->GetMapEdge2DownFlatNode().size() + sssg->GetMapEdge2UpFlatNode().size();
	adjncy = (int *)malloc(sizeof(int) * edgenum);
	vwgt = (int *)malloc(sizeof(int) * nvtxs);
	vsize = (int *)malloc(sizeof(int) * nvtxs);

	bak_adjncy = (int *)malloc(sizeof(int) * edgenum);
	bak_vwgt = (int *)malloc(sizeof(int) * nvtxs);
	bak_vsize = (int *)malloc(sizeof(int) * nvtxs);

	mpart = (int *)malloc(sizeof(int ) * nvtxs);

	for (int i = 0; i !=nvtxs; i++)
	{
		vwgt[i] = 0;
		bak_vwgt[i] = 0;
		vsize[i] = 0;
		bak_vsize[i] = 0;
		mpart[i] = 0;
	}
	for (int i = 0; i !=nvtxs +	1; i++)
	{
		xadj[i] = 0;
		bak_xadj[i] = 0;
	}
	for (int i = 0; i !=edgenum; i++)
	{
		adjncy[i] = 0;
		bak_adjncy[i] = 0;
	}
}

//************************************  
// Qualifier: 析构函数
//************************************
HorizontalFusionSSG::~HorizontalFusionSSG()
{
	free(xadj);
	free(adjncy);
	free(vwgt);
	free(vsize);
	free(bak_xadj);
	free(bak_adjncy);
	free(bak_vwgt);
	free(bak_vsize);
	free(mpart);
}

//************************************
// Qualifier: 对SDF图做拓扑排序
//************************************
vector<FlatNode *> HorizontalFusionSSG::TopoSortFilter()
{
	vector<FlatNode *>original = hfsssg->GetFlatNodes();
	vector<FlatNode *> 	actorTopoOrder;
	vector<int> nInDegree;//用于保存各节点的入度
	vector<FlatNode *> flatNodeStack;
	int nsize=original.size();
	for (int i = 0;i != nsize;++i)
	{
		nInDegree.push_back(original[i]->nIn);//将各节点入度保存
	}
	for (int i = 0; i != nInDegree.size();i++)
	{
		if(!nInDegree[i]) flatNodeStack.push_back(original[i]);
	}
	while (!flatNodeStack.empty())
	{
		FlatNode *tmpFlatNode = flatNodeStack.back();// 取将要出栈的节点
		actorTopoOrder.push_back(tmpFlatNode);
		flatNodeStack.pop_back();
		for(int i= 0; i != tmpFlatNode->nOut;i++)
		{
			for (int j =0;j != original.size(); j++)
			{
				if(original[j] == tmpFlatNode->outFlatNodes[i])
				{
					if(!(--nInDegree[j])) flatNodeStack.push_back(original[j]);//入度为0点进栈
				}
			}
		}
	}
	assert(actorTopoOrder.size() == original.size());
	return actorTopoOrder;
}



Node *HorizontalFusionSSG::MakeWindowNode(Node *id,Node *decl, Node *count, int style)
{
	Node *window = NULL;
	Node *sliding = NULL, *tumbling = NULL;

	assert(id && id->typ == Id && count && decl && decl->typ == Decl);
	id->u.id.decl = decl;

	if (style == 0) // sliding window
	{
		sliding = MakeWindowSlidingCoord(EMPTY_TQ, count, UnknownCoord);

		window = MakeWindowCoord(id, sliding, UnknownCoord); 
	}
	else // tumbling window
	{
		tumbling = MakeWindowTumbingCoord(EMPTY_TQ, count, UnknownCoord);
		window = MakeWindowCoord(id, tumbling, UnknownCoord); 
	}

	return window;
}

List *HorizontalFusionSSG::makeHFusionJoinWindow(List *inputList,List *outputList,List *pop_arg, Node *pushArg)
{
	assert(ListLength(outputList) == 1 && ListLength(inputList)>1&&ListLength(inputList) == ListLength(pop_arg));
	List *windowList = NULL;
	ListMarker input_maker;
	ListMarker pop_maker;
	Node *inputNode = NULL;
	Node *inputDecl = NULL;
	Node *pop_value = NULL;
	/*输入窗口*/
	IterateList(&input_maker,inputList);
	IterateList(&pop_maker,pop_arg);
	while (NextOnList(&input_maker,(GenericREF)&inputNode) && NextOnList(&pop_maker,(GenericREF)&pop_value))
	{
		assert(inputNode&&pop_value);
		assert(inputNode->typ == Id ||inputNode->typ == Decl);
		Node *inputId = NULL;
		if (inputNode->typ == Id)
		{
				inputId = MakeNewStreamId(inputNode->u.id.text,inputNode->u.id.decl);
				inputDecl = inputNode->u.id.decl;
		}
		else 
		{
			inputId = MakeNewStreamId(inputNode->u.decl.name,inputNode);
			inputDecl = inputNode;
		}
		windowList = AppendItem(windowList,MakeWindowNode(inputId,inputDecl,pop_value,0));	//输入边的窗口
	}

	/*输出窗口*/
	Node *outputNode = (Node *)FirstItem(outputList);
	assert(outputNode->typ == Id || outputNode->typ == Decl);
	Node *outputId = NULL;
	Node *outputDecl = NULL;

	if(outputNode->typ ==Id)
	{
		outputId = MakeNewStreamId(outputNode->u.id.text,outputNode->u.id.decl);
		outputDecl = outputNode->u.id.decl;
	}
	else 
	{
		outputId = MakeNewStreamId(outputNode->u.decl.name,inputNode);
		outputDecl = inputNode;
	}
	windowList =AppendItem(windowList,MakeWindowNode(outputId,outputDecl,pushArg,1));   //输出
	return windowList;
}

List *HorizontalFusionSSG::makeHFusionRoundRobinWindow(List *inputList,List *outputList,List *push_arg, Node *popArg)
{
	assert(ListLength(inputList) == 1 && ListLength(outputList)>1&&ListLength(outputList) == ListLength(push_arg));
	List *windowList = NULL;
	ListMarker output_maker;
	ListMarker push_maker;
	Node *outputNode = NULL;
	Node *outputDecl = NULL;
	Node *push_value = NULL;
	/*输出窗口*/
	IterateList(&output_maker,outputList);
	IterateList(&push_maker,push_arg);
	while (NextOnList(&output_maker,(GenericREF)&outputNode) && NextOnList(&push_maker,(GenericREF)&push_value))
	{
		assert(outputNode&&push_value);
		assert(outputNode->typ == Id ||outputNode->typ == Decl);
		Node *outputId = NULL;
		if (outputNode->typ == Id)
		{
				outputId = MakeNewStreamId(outputNode->u.id.text,outputNode->u.id.decl);
				outputDecl = outputNode->u.id.decl;
		}
		else 
		{
			outputId = MakeNewStreamId(outputNode->u.decl.name,outputNode);
			outputDecl = outputNode;
		}
		windowList = AppendItem(windowList,MakeWindowNode(outputId,outputDecl,push_value,1));	//输出边的窗口
	}
	/*输入窗口*/
	Node *inputNode = (Node *)FirstItem(inputList);
	assert(inputNode->typ == Id || inputNode->typ == Decl);
	Node *inputId = NULL;
	Node *inputDecl = NULL;
	if(inputNode->typ ==Id)
	{
		inputId = MakeNewStreamId(inputNode->u.id.text,inputNode->u.id.decl);
		inputDecl = inputNode->u.id.decl;
	}
	else 
	{
		inputId = MakeNewStreamId(inputNode->u.decl.name,inputNode);
		inputDecl = inputNode;
	}
	windowList =AppendItem(windowList,MakeWindowNode(inputId,inputDecl,popArg,0));
	return windowList;
}

List *HorizontalFusionSSG::makeHFusionDuplicateWindow(List *inputList,List *outputList,Node *pop_value)
{
	assert(ListLength(inputList) == 1 && ListLength(outputList)>1);
	List *windowList = NULL;
	ListMarker output_maker;
	ListMarker push_maker;
	Node *outputNode = NULL;
	Node *outputDecl = NULL;
	/*输出窗口*/
	IterateList(&output_maker,outputList);
	while (NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		assert(outputNode->typ == Id ||outputNode->typ == Decl);
		Node *outputId = NULL;
		if (outputNode->typ == Id)
		{
			outputId = MakeNewStreamId(outputNode->u.id.text,outputNode->u.id.decl);
			outputDecl = outputNode->u.id.decl;
		}
		else 
		{
			outputId = MakeNewStreamId(outputNode->u.decl.name,outputNode);
			outputDecl = outputNode;
		}
		windowList = AppendItem(windowList,MakeWindowNode(outputId,outputDecl,pop_value,1));	//输出边的窗口
	}

	/*输入窗口*/
	Node *inputNode = (Node *)FirstItem(inputList);
	assert(inputNode->typ == Id || inputNode->typ == Decl);
	Node *inputId = NULL;
	Node *inputDecl = NULL;
	if(inputNode->typ ==Id)
	{
		inputId = MakeNewStreamId(inputNode->u.id.text,inputNode->u.id.decl);
		inputDecl = inputNode->u.id.decl;
	}	
	else 
	{
		inputId = MakeNewStreamId(inputNode->u.decl.name,inputNode);
		inputDecl = inputNode;
	}
	windowList =AppendItem(windowList,MakeWindowNode(inputId,inputDecl,pop_value,0));
	return windowList;
}

Node *HorizontalFusionSSG::makeHFusionJoin(List *joinInputList, List *joinPopArg, Node *joinOutputNode, Node *joinPushArg)
{//构造join节点
	char *operatorName = (char *)malloc(60);
	sprintf(operatorName,"%s_%s_%d","HFusion","join",HFOperatorNum++);
	//新的filter的输出边List
	assert(joinOutputNode->typ== Decl || joinOutputNode->typ== Id);
	Node *joinOutputDecl = NULL;
	if(joinOutputNode->typ== Decl) joinOutputDecl = joinOutputNode;
	else joinOutputDecl = joinOutputNode->u.id.decl;
	
	Node *tmp_outputNode = MakeNewStreamId(joinOutputNode->u.decl.name,joinOutputNode);
	List *outputList = NULL;
	outputList = AppendItem(outputList,tmp_outputNode); 

	gIsUnfold=TRUE;
	Node *newOperatorHead = MakeOperatorHead(operatorName,outputList,joinInputList);//新节点的头
	Node *newOperator = DefineOperator(newOperatorHead);
	newOperator->u.operator_.ot = Join_;
	Node *newOperatorWork = MakeJoinWork(tmp_outputNode,joinInputList,joinPopArg);
	//新的filter构造window
	List *newOperatorWindow = makeHFusionJoinWindow(joinInputList,outputList,joinPopArg,joinPushArg);
	Node *newOperatorBody = MakeOperatorBody(newOperatorWork,newOperatorWindow);
	newOperator = SetOperatorBody(newOperator,newOperatorBody);//newoperator构造完成
	gIsUnfold=FALSE;
	return newOperator;
}

Node *HorizontalFusionSSG::makeHFusionSplit(Node *splitInputNode,Node *splitPopArg,List *splitOutputList, List *splitPushArg, SplitStyle style)
{//构造split节点(要确定采用的是哪种split的方式——duplicate，roundrobin)
	assert(splitInputNode->typ== Decl || splitInputNode->typ== Id);
	Node *tmp_inputNode = NULL;
	if(splitInputNode->typ== Decl ) tmp_inputNode = MakeNewStreamId(splitInputNode->u.decl.name,splitInputNode);
	else tmp_inputNode = splitInputNode;
	List *tmp_inputList = NULL;
	tmp_inputList = AppendItem(tmp_inputList,tmp_inputNode);
	char *operatorName = (char *)malloc(60);
	Node *newOperator = NULL;
	gIsUnfold=TRUE;
	if(style == S_RoundRobin)
	{
		sprintf(operatorName,"%s_%s_%d","HFusion","RoundRobin",HFOperatorNum++);
		Node *rouOperHead = MakeOperatorHead(operatorName,splitOutputList,tmp_inputList); //operator的头
		newOperator = DefineOperator(rouOperHead);
		newOperator->u.operator_.ot = Roundrobin_;
		Node *rouWorkNode = MakeRoundrobinWork(splitOutputList,tmp_inputNode,splitPushArg);//构造operator的work函数
		List *rouWindowList = makeHFusionRoundRobinWindow(tmp_inputList,splitOutputList,splitPushArg,splitPopArg);
		Node *rouOperBodyNode = MakeOperatorBody(rouWorkNode,rouWindowList);//operator的body
		newOperator = SetOperatorBody(newOperator,rouOperBodyNode);//构造operator

	}
	else 
	{//style == S_Duplicate
		sprintf(operatorName,"%s_%s_%d","HFusion","Duplicate",HFOperatorNum++);
		Node *rouOperHead = MakeOperatorHead(operatorName,splitOutputList,tmp_inputList); //operator的头
		newOperator = DefineOperator(rouOperHead);
		newOperator->u.operator_.ot = Duplicate_;
		Node *rouWorkNode = MakeDuplicateWork(splitOutputList,splitInputNode,splitPopArg);//构造operator的work函数
		List *rouWindowList = makeHFusionDuplicateWindow(tmp_inputList,splitOutputList,splitPopArg);
		Node *rouOperBodyNode = MakeOperatorBody(rouWorkNode,rouWindowList);//operator的body
		newOperator = SetOperatorBody(newOperator,rouOperBodyNode);//构造operator
	}
	gIsUnfold=FALSE;
	return newOperator;
}

void HorizontalFusionSSG::MWIOS_List(List *l,Node *oldInputDecl,Node *oldOutputDecl,Node *newInputDecl,Node *newOutputDecl,Node *iterNode,int pushvalue,int popvalue,int pushOffset,int popOffset)
{
	ListMarker _listwalk_marker; Node *_listwalk_ref; 
	IterateList(&_listwalk_marker, l); 
	while (NextOnList(&_listwalk_marker, (GenericREF)&_listwalk_ref)) { 
		if (_listwalk_ref) 
		{ 
			MWIOS_astwalk(_listwalk_ref,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);
		}                     
		SetCurrentOnList(&_listwalk_marker, (Generic *)_listwalk_ref); 
	}

}

void HorizontalFusionSSG::MWIOS_astwalk(Node *n,Node *oldInputDecl,Node *oldOutputDecl,Node *newInputDecl,Node *newOutputDecl,Node *iterNode,int pushvalue,int popvalue,int pushOffset,int popOffset)
{
	switch ((n)->typ) { 
	case Const:         break;
	case Id:         	break;
	case Binop:		 if ((n)->u.binop.left) {MWIOS_astwalk((n)->u.binop.left,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);}if ((n)->u.binop.right) {MWIOS_astwalk((n)->u.binop.right,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Unary:         if ((n)->u.unary.expr)  {MWIOS_astwalk((n)->u.unary.expr,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);}	 break; 
	case Cast:          if ((n)->u.cast.type) {MWIOS_astwalk((n)->u.cast.type,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.cast.expr) {MWIOS_astwalk((n)->u.cast.expr,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Comma:         if ((n)->u.comma.exprs) {MWIOS_List((n)->u.comma.exprs, oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Ternary:       if ((n)->u.ternary.cond) {MWIOS_astwalk((n)->u.ternary.cond,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.ternary.true_) {MWIOS_astwalk((n)->u.ternary.true_,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.ternary.false_) {MWIOS_astwalk((n)->u.ternary.false_,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break;
	case Array:        {
		if ((n)->u.array.name) {MWIOS_astwalk((n)->u.array.name,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);}
		if ((n)->u.array.dims) {MWIOS_List((n)->u.array.dims,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} 
		//具体处理
		if(n->u.array.name->u.id.decl->u.decl.type->typ == STRdcl)//表明数组的名是一个stream
		{
			Node *tmpDecl = n->u.array.name->u.id.decl;
			if(tmpDecl == oldInputDecl)
			{
				Node *streamId = MakeNewStreamId(newInputDecl->u.decl.name, newInputDecl);
				n->u.array.name = streamId;
				assert(ListLength(n->u.array.dims)==1);
				Node *dim = (Node *)FirstItem(n->u.array.dims);
				Node *binNode_1 = MakeBinopCoord('+',dim,MakeConstSint(popOffset),UnknownCoord);
				Node *binNode_2 = MakeBinopCoord('*',iterNode,MakeConstSint(popvalue),UnknownCoord);
				Node *binNode = MakeBinopCoord('+',binNode_1,binNode_2,UnknownCoord);
				List *dimList = NULL;
				dimList = AppendItem(dimList,binNode);
				n->u.array.dims = dimList;
			}
			else if(tmpDecl == oldOutputDecl)
			{
				Node *streamId = MakeNewStreamId(newOutputDecl->u.decl.name, newOutputDecl);
				n->u.array.name = streamId;
				assert(ListLength(n->u.array.dims)==1);
				Node *dim = (Node *)FirstItem(n->u.array.dims);
				Node *binNode_1 = MakeBinopCoord('+',dim,MakeConstSint(pushOffset),UnknownCoord);
				Node *binNode_2 = MakeBinopCoord('*',iterNode,MakeConstSint(pushvalue),UnknownCoord);
				Node *binNode = MakeBinopCoord('+',binNode_1,binNode_2,UnknownCoord);
				List *dimList = NULL;
				dimList = AppendItem(dimList,binNode);
				n->u.array.dims = dimList;
			}
		}
		break;
					   }
	case Call:          if ((n)->u.call.args) {MWIOS_List((n)->u.call.args,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Initializer:   if ((n)->u.initializer.exprs) {MWIOS_List((n)->u.initializer.exprs,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case ImplicitCast:  if ((n)->u.implicitcast.expr) {MWIOS_astwalk((n)->u.implicitcast.expr,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Label:         if ((n)->u.label.stmt) {MWIOS_astwalk((n)->u.label.stmt, oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Switch:        if ((n)->u.Switch.expr) {MWIOS_astwalk((n)->u.Switch.expr ,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.Switch.stmt) {MWIOS_astwalk((n)->u.Switch.stmt, oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Case:          if ((n)->u.Case.expr) {MWIOS_astwalk((n)->u.Case.expr ,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.Case.stmt) {MWIOS_astwalk((n)->u.Case.stmt, oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Default:       if ((n)->u.Default.stmt) {MWIOS_astwalk((n)->u.Default.stmt,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case If:            if ((n)->u.If.expr) {MWIOS_astwalk((n)->u.If.expr,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.If.stmt) {MWIOS_astwalk((n)->u.If.stmt,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case IfElse:        if ((n)->u.IfElse.expr) {MWIOS_astwalk((n)->u.IfElse.expr,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.IfElse.true_) {MWIOS_astwalk((n)->u.IfElse.true_, oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.IfElse.false_) {MWIOS_astwalk((n)->u.IfElse.false_,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break;
	case While:         if ((n)->u.While.expr) {MWIOS_astwalk((n)->u.While.expr,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.While.stmt) {MWIOS_astwalk((n)->u.While.stmt,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Do:            if ((n)->u.Do.stmt) {MWIOS_astwalk((n)->u.Do.stmt, oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.Do.expr) {MWIOS_astwalk((n)->u.Do.expr, oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case For:           if ((n)->u.For.init) {MWIOS_astwalk((n)->u.For.init,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.For.cond) {MWIOS_astwalk((n)->u.For.cond,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.For.next) {MWIOS_astwalk((n)->u.For.next,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.For.stmt) {MWIOS_astwalk((n)->u.For.stmt,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Goto:          break; 
	case Continue:      break; 
	case Break:         break; 
	case Return:        if ((n)->u.Return.expr) {MWIOS_astwalk((n)->u.Return.expr,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Block:         if ((n)->u.Block.decl) {MWIOS_List((n)->u.Block.decl,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.Block.stmts) {MWIOS_List((n)->u.Block.stmts,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Prim:          break; 
	case Tdef:          break; 
	case Ptr:           if ((n)->u.ptr.type) {MWIOS_astwalk((n)->u.ptr.type,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Adcl:          if ((n)->u.adcl.type) {MWIOS_astwalk((n)->u.adcl.type,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.adcl.dim) {MWIOS_astwalk((n)->u.adcl.dim,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Fdcl:          break; 
	case Sdcl:          break; 
	case Udcl:          break; 
	case Edcl:          break; 
	case STRdcl:		break;
	case Decl:          if ((n)->u.decl.type) {MWIOS_astwalk((n)->u.decl.type,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.decl.init) {MWIOS_astwalk((n)->u.decl.init,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.decl.bitsize) {MWIOS_astwalk((n)->u.decl.bitsize,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Attrib:        if (n->u.attrib.arg) {MWIOS_astwalk(n->u.attrib.arg,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Proc:          if ((n)->u.proc.decl) {MWIOS_astwalk((n)->u.proc.decl,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} if ((n)->u.proc.body) {MWIOS_astwalk((n)->u.proc.body,oldInputDecl,oldOutputDecl,newInputDecl,newOutputDecl,iterNode, pushvalue, popvalue, pushOffset, popOffset);} break; 
	case Text:          break; 
	default:            
		FAIL("Unrecognized node type"); break; 

	}
}

List *HorizontalFusionSSG::makeHFusionSInSOutWindow(Node *inputNode,Node *outputNode,int popValue,int peekValue,int pushValue)
{
	//不管原来的窗口有没有，现在按照新的值重新构造	 
	assert(inputNode && inputNode->typ == Decl);
	assert(outputNode &&inputNode->typ == Decl);
	List *finalWinList = NULL;//用于返回最后修改后的window
	//输入窗口为空，建一个输入window
	List *outputs = AppendItem(AppendItem(outputs,MakeConstSint(peekValue)),MakeConstSint(popValue));
	Node *output = MakeCommaCoord(outputs,UnknownCoord);
	Node *sliding = MakeWindowSlidingCoord(EMPTY_TQ,output,UnknownCoord);
	Node *input_winIdNode = MakeNewStreamId(inputNode->u.decl.name,inputNode);
	Node *input_Window = MakeWindowCoord(input_winIdNode,sliding,UnknownCoord); //输入窗口

	Node *tumbling = MakeWindowTumbingCoord(EMPTY_TQ, MakeConstSint(pushValue), UnknownCoord);  //push
	Node *output_winIdNode = MakeNewStreamId(outputNode->u.decl.name,outputNode);
	Node *output_Window = MakeWindowCoord(output_winIdNode,tumbling,UnknownCoord);//输出边的窗口
	finalWinList = AppendItem(finalWinList,input_Window);

	finalWinList = AppendItem(finalWinList,output_Window);

	return finalWinList;

}

//************************************
// Qualifier: 修改普通的operator的内容（输入输出边，以及构造最小稳定的迭代循环，修改数据访问的偏移值）
// Parameter: operatorNode * operNode ——要修改的operator(单入单出的)
// Parameter: int popvalue	——原operator的pop值
// Parameter: int pushvalue——原operator的push值
// Parameter: Node * newInputStream  ——修改后的新的operator的输入边
// Parameter: Node * newOutputStream ——修改后的新的operator的输出边
// Parameter: int popOffset	——从输入流中取数据偏移量
// Parameter: int pushOffset——从输出流中取数据偏移量
// Parameter: int steadyCount ——最小稳态的执行次数（用于构造新的operator的外层迭代循环——针对operator的work而言）
// Parameter: int index——主要用于构造外层循环的循环变量是消除重名 
//************************************
void HorizontalFusionSSG::commonOperatorTransform(operatorNode *operNode,int popvalue, int pushvalue, Node *newInputStream,Node *newOutputStream,int popOffset,int pushOffset,int steadyCount,int index)
{
	//1.取原来operator的输入输出边的decl
	List *inputList = operNode->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = operNode->decl->u.decl.type->u.operdcl.outputs;
	assert(ListLength(inputList) == ListLength(outputList) == 1);
	Node *inputNode = (Node *)FirstItem(inputList);
	Node *outputNode = (Node *)FirstItem(outputList);
	Node *inputDecl = NULL;
	Node *outputDecl = NULL;
		//取输入输出流的定义
	if(inputNode->typ == Id) inputDecl =  inputNode->u.id.decl;
	else inputDecl = inputNode;
	if(outputNode->typ == Id)outputDecl = outputNode->u.id.decl;
	else outputDecl = outputNode;
	//2.构造外层的循环（for循环,循环的循环体是修改和的operator的work函数）
	char *forIdName = (char *)malloc(sizeof(char)*30);
	sprintf(forIdName, "%s_%d", "__work_",index);
	Node *forVarId = MakeIdCoord(forIdName, UnknownCoord);
	Node *forVarDecl = MakeNewDecl(forIdName, PrimSint, MakeConstSint(0), Redecl);
	forVarId->u.id.decl = forVarDecl;
	Node *init = NULL, *cond = NULL, *next = NULL;
	List *decls = NULL, *stmts = NULL;
	decls = MakeNewList(forVarDecl);
	init = MakeBinopCoord('=', forVarId, MakeConstSint(0), UnknownCoord);
	cond = MakeBinopCoord('<', forVarId, MakeConstSint(steadyCount), UnknownCoord);
	next = MakeUnaryCoord(PREINC, forVarId, UnknownCoord);

	//3.修改operator（包括operator的decl，以及work）
	List *newInputList = NULL;
	List *newOutputList = NULL;
	assert(newOutputStream->typ == Decl && newInputStream->typ == Decl);
	Node *newInputId = MakeNewStreamId(newInputStream->u.decl.name,newInputStream);
	Node *newOutputId = MakeNewStreamId(newOutputStream->u.decl.name,newOutputStream);
	newInputList = AppendItem(newInputList,newInputId);
	newOutputList = AppendItem(newOutputList,newOutputId);
		//修改operator的输入输出边（operator的decl中）
	operNode->decl->u.decl.type->u.operdcl.inputs = newInputList;
	operNode->decl->u.decl.type->u.operdcl.outputs = newOutputList;
	assert(ListLength(operNode->decl->u.decl.type->u.operdcl.inputs) == ListLength(operNode->decl->u.decl.type->u.operdcl.outputs) == 1);
		//修改operator的work函数中的相关内容
	Node *workNode = operNode->body->u.operBody.work;
	assert(workNode);
	MWIOS_astwalk(workNode,inputDecl,outputDecl,newInputStream,newOutputStream,forVarId,pushvalue,popvalue,pushOffset,popOffset);
		//将work作为for循环的stmt
	Node *tmpForNode = MakeForCoord(init,cond,next,workNode,UnknownCoord);
	stmts = AppendItem(stmts, tmpForNode);//新work的语句
	Node *newWorkNode = MakeBlockCoord(PrimVoid, decls, stmts, UnknownCoord, UnknownCoord);//构造新的work函数
		//修改operator的work
	operNode->body->u.operBody.work = newWorkNode;
	//4.修改operator的window（可以重新构造新的window覆盖旧的window）
	operNode->body->u.operBody.window = makeHFusionSInSOutWindow(newInputStream,newOutputStream,popvalue*steadyCount,popvalue*steadyCount,pushvalue*steadyCount);
}

Node *HorizontalFusionSSG::fusingOperators(vector<operatorNode *>operNodes,int popvalue,int pushvalue)
{//不用构造新的operator，只要将operNodes中所有operator的init，work以及state组合在一起，将各部分放到operNodes的第一个元素中作为最终的返回值
	assert(operNodes.size() >1 );
	List *stateList =NULL;
	List *initStmtsList = NULL;
	List *workStmtsList = NULL;
	for(int i = 0; i != operNodes.size();i++)
	{
		//1.所有operator的state拼接到一起
		if(operNodes[i]->body->u.operBody.state)stateList= JoinLists(stateList, operNodes[i]->body->u.operBody.state);
		//2.所有operator的init拼接到一起
		if(operNodes[i]->body->u.operBody.init)initStmtsList = AppendItem(initStmtsList,operNodes[i]->body->u.operBody.init);
		//3.所有operator的work拼接到一起
		if(operNodes[i]->body->u.operBody.work)workStmtsList = AppendItem(workStmtsList,operNodes[i]->body->u.operBody.work);
	}
	operatorNode *tmpOper = operNodes[0];
	tmpOper->body->u.operBody.state = stateList;
	tmpOper->body->u.operBody.init = MakeBlockCoord(PrimVoid, NULL, initStmtsList, UnknownCoord, UnknownCoord);
	tmpOper->body->u.operBody.work = MakeBlockCoord(PrimVoid, NULL, workStmtsList, UnknownCoord, UnknownCoord);
	Node *inputNode = (Node *)FirstItem(tmpOper->decl->u.decl.type->u.operdcl.inputs);
	Node *outputNode = (Node *)FirstItem(tmpOper->decl->u.decl.type->u.operdcl.outputs);
	Node *inputDecl = NULL;
	Node *outputDecl = NULL;
	//取输入输出流的定义
	if(inputNode->typ == Id) inputDecl =  inputNode->u.id.decl;
	else inputDecl = inputNode;
	if(outputNode->typ == Id)outputDecl = outputNode->u.id.decl;
	else outputDecl = outputNode;
	tmpOper->body->u.operBody.window = makeHFusionSInSOutWindow(inputDecl,outputDecl,popvalue,popvalue,pushvalue);
	
	char *operatorName = (char *)malloc(60);
	sprintf(operatorName,"%s_%s_%d","HFusion",tmpOper->decl->u.decl.name,HFOperatorNum++);
	tmpOper->decl->u.decl.name = operatorName;
	Node *finalOperatorNode = MakeOperator(tmpOper->decl,tmpOper->body);//finalOperatorNode 作为最终的返回
	return finalOperatorNode;
}

//************************************
// Returns:   Node *（返回的是融合后形成的新的节点）
// Qualifier: 将flatNode中的所有的operator节点融合成一个节点，inputNode和outputNode是融合后节点的输入输出边
// Parameter: vector<FlatNode * >flatNodes
// Parameter: Node * inputNode
// Parameter: Node * outputNode
//************************************
Node *HorizontalFusionSSG::fusingNodes(vector<FlatNode *>flatNodes,Node *inputNode,Node *outputNode)
{//在每一个operator的外面要做一个循环（循环的次数是最小周期数），对operator中的变量重命名,修改operator的输入输出边名称以及数据访问的偏移
	assert(inputNode->typ == Decl && outputNode->typ == Decl);
	vector<operatorNode *>fusionOperNodes;//将经过处理过后的flatNode的contents保存在该vector中
	int popOffset = 0;
	int pushOffset = 0;
	for(int i = flatNodes.size()-1; i >= 0; i--)
	{//根据图中节点自左向右
		//1.重命名operator中的各个变量
		operatorNode *tmpOperNode = flatNodes[i]->contents;
		//2.修改输入输出边以及数据偏移的位置并在work函数外层做一个最小稳定状态的循环
		int steadyCount = _sjflatNode2smallSteadyCount.find(flatNodes[i])->second;
		int popvalue = flatNodes[i]->inPopWeights[0];
		int pushvalue = flatNodes[i]->outPushWeights[0];
		
		if(flatNodes[i]->inFlatNodes[0]->contents->ot != Duplicate_ )
		{
			commonOperatorTransform(tmpOperNode,popvalue,pushvalue,inputNode,outputNode,popOffset,pushOffset,steadyCount,i);
			popOffset += steadyCount * popvalue;
		}
		else
		{
			assert(flatNodes[i]->inFlatNodes[0]->contents->ot == Duplicate_);
			commonOperatorTransform(tmpOperNode,popvalue,pushvalue,inputNode,outputNode,0,pushOffset,steadyCount,i);
			popOffset = steadyCount * popvalue;
		}
		pushOffset += steadyCount * pushvalue;
		//3.将修改后的operator添加到fusionOperNodes
		fusionOperNodes.push_back(tmpOperNode);
	}
	//将fusionOperNodes中的所有operator融合成一个operator
	Node *finalOperatorNode =  fusingOperators(fusionOperNodes,popOffset,pushOffset);
	assert(finalOperatorNode);
	return finalOperatorNode;
}

void HorizontalFusionSSG::InsertExternalOutputStreamToSSG(FlatNode *flatNode)
{
	ListMarker marker;
	Node *streamNode = NULL;
	Node *stream_type = NULL;
	IterateList(&marker, flatNode->contents->decl->u.decl.type->u.operdcl.outputs ); //u的输出边
	while (NextOnList(&marker, (GenericREF) &streamNode)) 
	{
		if (streamNode->typ == Id)   //取流的类型
			stream_type = streamNode->u.id.decl->u.decl.type;
		else 
			stream_type = streamNode->u.decl.type;
		/*这条边已经在mapEdge2DownFlatNode中存在了*/
		std::multimap<Node *, FlatNode *> ::iterator pos;
		FlatNode *buttomFlatNode=NULL;
		pos = hfsssg->mapEdge2DownFlatNode.find(stream_type);
		assert(pos != hfsssg->mapEdge2DownFlatNode.end());
		buttomFlatNode = pos->second;
		hfsssg->mapEdge2UpFlatNode.insert(make_pair(stream_type,flatNode));
		flatNode->AddOutEdges(buttomFlatNode);
	}

}

//************************************
// Returns:   vector<FlatNode *> ——返回的是新的operator转换成的FlatNode的结合
// Qualifier: 在ssg中用newFlatNodes代替oldFlatNodes,(oldflatNodes和newflatNodes是一个子图，他们在ssg中有相同的上端和下端节点)
// Parameter: vector<FlatNode * >oldFlatNodes（要从ssg中删除的flatNode的集合）
// Parameter: vector<operatorNode * >newNodes 在该结构的所有节点中有且仅有一个节点的输入不在newNodes,有且仅有一个节点的输出不在newNodes中(将newNode中的节点构造成FlatNode，并插入到ssg中）
//************************************
vector<FlatNode *> HorizontalFusionSSG::replaceHfusionFlatNodes(vector<FlatNode *>oldFlatNodes,vector<operatorNode *>newNodes)
{
	assert(newNodes.size() > 0);
	vector<FlatNode *>fusedResult;
	//删除旧的信息
	for(int i = 0; i!= oldFlatNodes.size(); i++)
	{
		//删除flatNodes中的内容
		vector<FlatNode *>::iterator erase_iter = hfsssg->flatNodes.end();
		for(vector<FlatNode *>::iterator iter = hfsssg->flatNodes.begin(); iter != hfsssg->flatNodes.end(); iter++)
		{
			if (oldFlatNodes[i] == *iter) { erase_iter = iter; break;}
		}
		assert(erase_iter != hfsssg->flatNodes.end());
		hfsssg->flatNodes.erase(erase_iter);
		//删除mapEdge2UpFlatNode中的内容
		map<Node *, FlatNode *>::iterator mapEdge2UpFlatNode_iter = hfsssg->mapEdge2UpFlatNode.begin();
		while (mapEdge2UpFlatNode_iter != hfsssg->mapEdge2UpFlatNode.end())
		{
			if(mapEdge2UpFlatNode_iter->second == oldFlatNodes[i])
			{
				map<Node *, FlatNode *>::iterator mapEdge2UpFlatNode_eraseIter = mapEdge2UpFlatNode_iter;
				mapEdge2UpFlatNode_iter++;
				hfsssg->mapEdge2UpFlatNode.erase(mapEdge2UpFlatNode_eraseIter);
			}
			else mapEdge2UpFlatNode_iter++;
		}
		//删除mapEdge2DownFlatNode中的元素
		multimap<Node *, FlatNode *>::iterator mapEdge2DownFlatNode_iter = hfsssg->mapEdge2DownFlatNode.begin();
		while (mapEdge2DownFlatNode_iter != hfsssg->mapEdge2DownFlatNode.end())
		{
			if(mapEdge2DownFlatNode_iter->second == oldFlatNodes[i])
			{
				multimap<Node *, FlatNode *>::iterator mapEdge2DownFlatNode_eraseIter = mapEdge2DownFlatNode_iter;
				mapEdge2DownFlatNode_iter++;
				hfsssg->mapEdge2DownFlatNode.erase(mapEdge2DownFlatNode_eraseIter);
			}
			else mapEdge2DownFlatNode_iter++;
		}
		//删除在工作量估计中的值
		hfsssg->mapInitWork2FlatNode.erase(oldFlatNodes[i]);
		hfsssg->mapSteadyWork2FlatNode.erase(oldFlatNodes[i]);
	}
	//添加新的信息
	set<FlatNode *> oldFlatNodeSet(oldFlatNodes.begin(),oldFlatNodes.end());
	if(newNodes.size() == 1)
	{
		FlatNode *firstFlatNode = hfsssg->InsertFlatNodes(newNodes[0],NULL,NULL,oldFlatNodeSet);
		fusedResult.push_back(firstFlatNode);
		//InsertExternalOutputStreamToSSG(firstFlatNode);
		hfsssg->SetFlatNodesWeights(firstFlatNode);
	}
	else if(newNodes.size() == 2)
	{
		FlatNode *firstFlatNode = hfsssg->InsertFlatNodes(newNodes[0],NULL,NULL,oldFlatNodeSet);
		fusedResult.push_back(firstFlatNode);
		FlatNode *lastFlatNode = hfsssg->InsertFlatNodes(newNodes[newNodes.size()-1],NULL,NULL,oldFlatNodeSet);
		fusedResult.push_back(lastFlatNode);

		/*要将输出边不在newNodes中的节点的输出边插入的flatNode以及相应的结构中*/
		//InsertExternalOutputStreamToSSG(lastFlatNode);

		hfsssg->SetFlatNodesWeights(firstFlatNode);
		hfsssg->SetFlatNodesWeights(lastFlatNode);

	}
	else
	{
		FlatNode *firstFlatNode = hfsssg->InsertFlatNodes(newNodes[0],NULL,NULL,oldFlatNodeSet);
		fusedResult.push_back(firstFlatNode);
		//将Node *构造成FlatNode*节点同时插入到hfssg中
		vector<FlatNode *>tmpNewFlatNode;
		for(int i = 1; i != newNodes.size()-1; i++)
		{
			FlatNode * tmpflatNode = hfsssg->InsertFlatNodes(newNodes[i],NULL,	NULL,oldFlatNodeSet);
			//stringstream newName;
			//newName<<"HFusion_"<<tmpflatNode->name<<"_"<<HFOperatorNum++;
			//tmpflatNode->name = newName.str();
			tmpNewFlatNode.push_back(tmpflatNode);
			fusedResult.push_back(tmpflatNode);//将新节点转化成FlatNode并插入到ssg中	
		}
		FlatNode *lastFlatNode = hfsssg->InsertFlatNodes(newNodes[newNodes.size()-1],NULL,NULL,oldFlatNodeSet);
		fusedResult.push_back(lastFlatNode);

		/*要将输出边不在newNodes中的节点的输出边插入的flatNode以及相应的结构中*/
		//InsertExternalOutputStreamToSSG(lastFlatNode);

		hfsssg->SetFlatNodesWeights(firstFlatNode);
		for(int i = 0; i != tmpNewFlatNode.size(); i++)
		{
			hfsssg->SetFlatNodesWeights(tmpNewFlatNode[i]);
		}
		hfsssg->SetFlatNodesWeights(lastFlatNode);
	}
	//对新的节做工作量估计
	for(int i = 0; i != fusedResult.size(); i++)
	{
		hfsssg->AddSteadyWork(fusedResult[i], workEstimate(fusedResult[i]->contents->body, 0));
		hfsssg->AddInitWork(fusedResult[i], workEstimate_init(fusedResult[i]->contents->body, 0));
		_sjflatNode2smallSteadyCount.insert(make_pair(fusedResult[i],1));
	}
	return fusedResult;
}

//************************************
// Qualifier: 待融合节点的上端不是split，下端不是join,将待融合节点融合成一个节点同时在上端构造一个join，在下端构造一个split节点
// Parameter: vector<FlatNode * > flatNodes
//************************************
void HorizontalFusionSSG::fusingFlatNodesCommon(vector<FlatNode *> flatNodes)
{//对operator中的变量重命名，修改operator的输入输出边，在输入输出边中取数据的位置的偏移，上端构造join下端构造split……
	//0.构造融合后新节点的输入输出边（各一条）——对融合后新节点而言的输入输出
	operatorNode *curOperator = flatNodes[0]->contents;
	char *inputName = NULL;
	Node *inputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.inputs);
	Node *input_Decl = NULL;
	if(inputNode->typ == Id) input_Decl = inputNode->u.id.decl;
	else input_Decl = inputNode;
	Node *inputStreamType = input_Decl->u.decl.type;
	inputName = (char *) malloc(strlen(input_Decl->u.decl.name)+20);
	sprintf(inputName, "%s_%d", input_Decl->u.decl.name, ++HFStreamNum);
	Node *inputNewStream = MakeNewStream(inputName,inputStreamType);
	assert(inputNewStream && inputNewStream ->typ ==Decl);

	char *outputName = NULL;
	Node *outputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.outputs);
	Node *output_Decl = NULL;
	if(outputNode->typ == Id) output_Decl = outputNode->u.id.decl;
	else output_Decl = outputNode;
	Node *outputStreamType = output_Decl->u.decl.type;
	outputName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
	sprintf(outputName, "%s_%d", output_Decl->u.decl.name, ++HFStreamNum);
	Node *outputNewStream = MakeNewStream(outputName,outputStreamType);
	assert(outputNewStream && outputNewStream ->typ ==Decl);	
	
	//1.先构造join节点(利用新构造的输入边以及flatNodes中所有节点的输入边构造join节点)
	List *joinInputList = NULL;//flatNodes中的节点的输入
	List *joinPopArg = NULL;
	int joinPushValue = 0;
	for(int i = flatNodes.size() - 1; i >= 0;i--)//要注意边的相对顺序
	{
		assert( ListLength(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.inputs) == 1 );
		joinInputList = AppendItem(joinInputList, (Node *)FirstItem(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.inputs));
		int popValue = _sjflatNode2smallSteadyCount.find(flatNodes[i])->second * flatNodes[i]->inPopWeights[0];
		joinPopArg = AppendItem(joinPopArg, MakeConstSint(popValue));
		joinPushValue += popValue;
	}
	Node *joinPushArg = MakeConstSint(joinPushValue);
	Node *jNode = makeHFusionJoin(joinInputList,joinPopArg,inputNewStream,joinPushArg);
	
	//2.构造split节点(利用新构造的输出边以及flatNodes中所有节点的输出边构造join节点)
	List *splitOutputList = NULL;
	List *splitPushArg = NULL;
	int splitPopValue = 0;
	for(int i = flatNodes.size() - 1; i >= 0;i--)
	{
		assert( ListLength(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.outputs) == 1 );
		splitOutputList = AppendItem(splitOutputList, (Node *)FirstItem(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.outputs));
		int pushValue = _sjflatNode2smallSteadyCount.find(flatNodes[i])->second * flatNodes[i]->outPushWeights[0];
		splitPushArg = AppendItem(splitPushArg, MakeConstSint(pushValue));
		splitPopValue += pushValue;
	}
	Node *splitPopArg = MakeConstSint(splitPopValue);
	Node *sNode = makeHFusionSplit(outputNewStream,splitPopArg,splitOutputList,splitPushArg, S_RoundRobin);//此处构造的split应该是roundrobin方式

	//3.真正的融合——在每一个operator的外面要做一个循环（循环的次数是最小周期数），对operator中的变量重命名,修改operator的输入输出边名称以及数据访问的偏移
	Node *fusedOperatorNode = fusingNodes(flatNodes,inputNewStream,outputNewStream);

	//4.修改SSG的信息——删除融合掉的operator的flatNode节点，同时添加新增加的operator(sNode,jNode)
	vector<operatorNode *> newOperatorNodes;//按照拓扑序列来排列的
	newOperatorNodes.push_back(&(jNode->u.operator_));
	newOperatorNodes.push_back(&(fusedOperatorNode->u.operator_));
	newOperatorNodes.push_back(&(sNode->u.operator_));
	vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(flatNodes, newOperatorNodes);
}

//************************************
// Qualifier: /*根据flatNodes与join的关系，将根据flatNodes融合得到的operNode的输出边的数据进行重排序（在work函数中添加该过程） */
// Parameter: Node * operNode ——由flatNodes节点融合得到的新的operNode
// Parameter: FlatNode * joinFlatNode——flatNodes的下端的join节点
// Parameter: vector<FlatNode * > flatNodes
//************************************
Node *HorizontalFusionSSG::reorderFusionJoinNode(Node *operNode,FlatNode *joinFlatNode,vector<FlatNode *> flatNodes)
{
	//1、取join对flatNodes中的各个边数据的pop值
	int begin_index = joinFlatNode->inFlatNodes.size();//记录flatNodes的第一个节点在join分支的位置
	set<FlatNode *> tmpJoinInputSet(flatNodes.begin(),flatNodes.end());

	for(int i = 0;i !=  joinFlatNode->inFlatNodes.size(); i++)
	{
		if( tmpJoinInputSet.count(joinFlatNode->inFlatNodes[i])){ begin_index = i; break;}
	}

	assert(begin_index != joinFlatNode->inFlatNodes.size());
	vector<int > pop_values;
	pop_values.push_back(0);
	int push_value = 0;
	for(int j = 0; j!= flatNodes.size();j++)
	{
		pop_values.push_back(joinFlatNode->inPopWeights[begin_index+j]);
		push_value += joinFlatNode->inPopWeights[begin_index+j];
	}
	int joinSteadycount = _sjflatNode2smallSteadyCount.find(joinFlatNode)->second;//join节点在最小稳态的执行次数
	int pushNum = push_value * joinSteadycount;//最小稳定状态执行完后产生的数据总量
	//取operNode的输出边的流类型的成员声明列表
	Node *outputNode = (Node *)FirstItem(operNode->u.operator_.decl->u.decl.type->u.operdcl.outputs);
	Node *outputDecl = NULL;
	if(outputNode->typ == Id) outputDecl = outputNode->u.id.decl;
	else outputDecl = outputNode;
	List *stream_fields = outputDecl->u.decl.type->u.strdcl.type->fields;
	//构造重排序需要的中间变量的类型（struct）
	SUEtype* tmpStructType = NULL;
	tmpStructType = HeapNew(SUEtype);
	tmpStructType->complete = TRUE;
	tmpStructType->coord = UnknownCoord;
	char * tmpStructName = (char *) malloc(30);
	sprintf(tmpStructName, "%s_%d", "output_streamtype_struct",HFVarNum);
	tmpStructType->name = tmpStructName;
	tmpStructType->fields = stream_fields;
	tmpStructType->typ = Sdcl;
	tmpStructType->visited = FALSE;
	Node *structDecl = MakeSdclCoord(T_SUE_ELABORATED,tmpStructType, UnknownCoord);
	Node *bufferAdcl = MakeAdclCoord(EMPTY_TQ,structDecl,MakeConstSint(pushNum),UnknownCoord);

	char * buffername = (char *) malloc(30);
	sprintf(buffername, "%s_%d", "output_buffer",HFVarNum);
	Node *bufferDecl = MakeDeclCoord(buffername,T_BLOCK_DECL,bufferAdcl,NULL,NULL,UnknownCoord);//将结构体构造完成，bufferDecl——要被添加到work函数中
	
	//构造 将operNode的输出流中的数据临时保存到buffer中 的语句——需要被添加到work中
		//构造一个循环
	char * _iname = (char *) malloc(8);
	sprintf(_iname, "%s_%d", "__i",HFVarNum);
	Node *idI = MakeIdCoord(_iname, UnknownCoord);
	Node *declI = MakeNewDecl(_iname, PrimSint, MakeConstSint(0), Redecl);//构造循环变量的定义——要加到work中
	idI->u.id.decl = declI;
	Node *init = NULL, *cond = NULL, *next = NULL;
	init = MakeBinopCoord('=', idI, MakeConstSint(0), UnknownCoord);
	cond = MakeBinopCoord('<', idI, MakeConstSint(pushNum), UnknownCoord);
	next = MakeUnaryCoord(PREINC, idI, UnknownCoord);
	//下面构造赋值语句	
	Node *outputId = MakeNewStreamId(outputDecl->u.decl.name,outputDecl);//构造输出流对应的idNode
	Node *bufferId = MakeIdCoord(buffername,UnknownCoord);//构造缓冲区对应的idnode
	bufferId->u.id.decl = bufferDecl;
	Node *bufferArray = ExtendArray(bufferId,idI,UnknownCoord);
	Node *outputArray = ExtendArray(outputId,idI,UnknownCoord);
	List *assignmentList_1 = NULL;//赋值语句list

	ListMarker maker;
	Node *field = NULL;
	IterateList(&maker,stream_fields);
	while (NextOnList(&maker , (GenericREF)&field))
	{
		assert(field->typ == Decl);
		Node *tmpId = MakeIdCoord(field->u.decl.name,UnknownCoord);
		tmpId->u.id.decl = field;
		Node *left = MakeBinopCoord('.',bufferArray,tmpId,UnknownCoord);
		Node *right = MakeBinopCoord('.',outputArray,tmpId,UnknownCoord);
		Node *assignNode = MakeBinopCoord('=',left,right,UnknownCoord);
		assignmentList_1 = AppendItem(assignmentList_1,assignNode);
	}
	Node *for_stmt = MakeBlockCoord(PrimVoid,NULL,assignmentList_1,UnknownCoord,UnknownCoord);
	Node *initBufferNode = MakeForCoord(init,cond,next,for_stmt,UnknownCoord);//构造初始化buffer的语句——要加到work中
	//利用buffer作为中转站将输出流中的数据重排序
	char * _jname = (char *) malloc(8);
	sprintf(_jname, "%s_%d", "__j",HFVarNum);
	char * _kname = (char *) malloc(8);
	sprintf(_kname, "%s_%d", "__k",HFVarNum);
	Node *idJ = MakeIdCoord(_jname, UnknownCoord);
	Node *idK = MakeIdCoord(_kname, UnknownCoord);
	Node *declJ = MakeNewDecl(_jname, PrimSint, MakeConstSint(0), Redecl);// 加到work中
	Node *declK = MakeNewDecl(_kname, PrimSint, MakeConstSint(0), Redecl);//加到work中
	HFVarNum++;
	idJ->u.id.decl = declJ; 
	idK->u.id.decl = declK;
	Node *initI = MakeBinopCoord('=', idI, MakeConstSint(0), UnknownCoord);
	Node *condI = MakeBinopCoord('<', idI, MakeConstSint(joinSteadycount), UnknownCoord);
	Node *nextI = MakeUnaryCoord(POSTINC, idI, UnknownCoord);
	Node *nextK = MakeUnaryCoord(POSTINC, idK, UnknownCoord);
	List *stmts = NULL;
	int offsetDate = 0;
	Node *initJ = NULL, *nextJ = NULL, *condJ = NULL;
	for(int i = 0; i < flatNodes.size(); i++)
	{	
		offsetDate += pop_values[i] * joinSteadycount ;	

		Node *offdetNode1 = MakeBinopCoord('*',MakeConstSint(pop_values[i+1]),idI,UnknownCoord);
		Node *offdetNode2 = MakeBinopCoord('+',MakeConstSint(offsetDate),idJ,UnknownCoord);

		initJ = MakeBinopCoord('=', idJ, MakeConstSint(0), UnknownCoord);
		condJ = MakeBinopCoord('<', idJ, MakeConstSint(pop_values[i+1]), UnknownCoord);
		nextJ = MakeUnaryCoord(POSTINC, idJ, UnknownCoord);

		outputArray  = ExtendArray(outputId,nextK,UnknownCoord);
		bufferArray = ExtendArray(bufferId,MakeBinopCoord('+',offdetNode1,offdetNode2,UnknownCoord),UnknownCoord);
		List *assignmentList_2 = NULL;//赋值语句list

		IterateList(&maker,stream_fields);
		while (NextOnList(&maker , (GenericREF)&field))
		{
			assert(field->typ == Decl);
			Node *tmpId = MakeIdCoord(field->u.decl.name,UnknownCoord);
			tmpId->u.id.decl = field;
			Node *left = MakeBinopCoord('.',outputArray,tmpId,UnknownCoord);
			Node *right = MakeBinopCoord('.',bufferArray,tmpId,UnknownCoord);
			Node *assignNode = MakeBinopCoord('=',left,right,UnknownCoord);
			assignmentList_2 = AppendItem(assignmentList_2,assignNode);
		}

		Node *stmtJ =  MakeBlockCoord(PrimVoid,NULL,assignmentList_2,UnknownCoord,UnknownCoord);

		Node *internForNode = MakeForCoord(initJ,condJ,nextJ,stmtJ,UnknownCoord);
		stmts = AppendItem(stmts, internForNode);
	}
	Node *externForBody = MakeBlockCoord(PrimVoid,NULL,stmts,UnknownCoord,UnknownCoord);
	Node *reorderBufferNode = MakeForCoord(initI,condI,nextI,externForBody,UnknownCoord);//重排序的最外层循环节点构造完成——要加到work函数中
	//将新造的节点添加到work中
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,declI);
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,declJ);
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,declK);
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,bufferDecl);
	operNode->u.operator_.body->u.operBody.work->u.Block.stmts = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.stmts,initBufferNode);
	operNode->u.operator_.body->u.operBody.work->u.Block.stmts = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.stmts,reorderBufferNode);
	return operNode;
}

//************************************
// Qualifier: 如果joinflatNode的上端的分支数和flatNodes中的节点数目相同融合成一个新的节点，同时在flatNodes的上端还要构造出一个类似于join的节点
//				如果joinflatNode的上端的分支数大于flatNode，则将flatNode节点融合在一起，同时构造新的join节点代替joinflatNode，并且在flatNodes的上端还要构造出一个类似于join的节点
// Parameter: FlatNode * joinflatNode 
// Parameter: vector<FlatNode * > flatNodes join上端的节点（待融合的节点集）
//************************************
void HorizontalFusionSSG::fusingFlatNodesJoin(FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{
	int nbranch = joinflatNode->nIn;

	operatorNode *curOperator = flatNodes[0]->contents;
		//先构造输入边 ——对融合后的的节点而言
	char *inputName = NULL;
	Node *inputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.inputs);
	Node *input_Decl = NULL;
	if(inputNode->typ == Id) input_Decl = inputNode->u.id.decl;
	else input_Decl = inputNode;
	Node *inputStreamType = input_Decl->u.decl.type;
	inputName = (char *) malloc(strlen(input_Decl->u.decl.name)+20);
	sprintf(inputName, "%s_%d", input_Decl->u.decl.name, ++HFStreamNum);
	Node *inputNewStream = MakeNewStream(inputName,inputStreamType);
	assert(inputNewStream && inputNewStream ->typ ==Decl);

	//1.先构造新的join节点（放在flatNodes中的节点的上端）
	List *joinInputList = NULL;//flatNodes中的节点的输入
	List *joinPopArg = NULL;
	int newjoinPushValue = 0;//新建的join节点的push值
	
	for(int i = flatNodes.size() - 1; i >=0;i--)
	{
		assert( ListLength(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.inputs) == 1 );
		joinInputList = AppendItem(joinInputList, (Node *)FirstItem(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.inputs));
		int popValue = _sjflatNode2smallSteadyCount.find(flatNodes[i])->second * flatNodes[i]->inPopWeights[0];
		joinPopArg = AppendItem(joinPopArg, MakeConstSint(popValue));
		newjoinPushValue += popValue;
	}
	Node *joinPushArg = MakeConstSint(newjoinPushValue);
	Node *jNode = makeHFusionJoin(joinInputList,joinPopArg,inputNewStream,joinPushArg);
	
	// 对于待融合节点的数目和分支数目的不同对下端的join节点进行不同的处理
	if(nbranch == flatNodes.size())
	{//将join以及flatNodes融合成一个新的operator节点（先将flatNodes节点融合成一个新的operator然后在新的operator，然后再对输出边的数据进行重排序（为了将flatNodes下端的节点join节点处理掉）——不需要引入内部缓冲区，直接对输入缓冲区进行操作）
		//将joinflatNode节点融合掉——方法是在fusedOperatorNode的work函数中添加对输出流中的数据进行重排序
		
		//2.融合flatNodes中的节点
		Node *joinOutputNode = (Node *)FirstItem(joinflatNode->contents->decl->u.decl.type->u.operdcl.outputs);
		Node *joinOutputStream = NULL; 
		if(joinOutputNode->typ == Id) joinOutputStream = joinOutputNode->u.id.decl;
		else joinOutputStream = joinOutputNode;
		//2.1 融合flatNodes——在每一个operator的外面要做一个循环（循环的次数是最小周期数），对operator中的变量重命名,修改operator的输入输出边名称以及数据访问的偏移
		Node *fusedOperatorNode = fusingNodes(flatNodes,inputNewStream,joinOutputStream);
		
		Node *newOperatorNode = reorderFusionJoinNode(fusedOperatorNode,joinflatNode,flatNodes);
		vector<operatorNode *> newOperatorNodeVec;
		newOperatorNodeVec.push_back(&(jNode->u.operator_));
		newOperatorNodeVec.push_back(&(newOperatorNode->u.operator_));
		vector<FlatNode *>oldFlatNodes(flatNodes);
		oldFlatNodes.push_back(joinflatNode);
		vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//更新sssg了							
	}
	else if(nbranch < flatNodes.size())
	{
		
		//int oldjoinSteadyCount = sjflatNode2smallSteadyCount.find(joinflatNode)->second;
		//重新构造下端的join节点
		//根据融合的结果，收集新的构造的join节点的输入输出边以及push、pop的信息
		List *oldJoinIntputList = joinflatNode->contents->decl->u.decl.type->u.operdcl.inputs;
		//向Join的新的输入边中添加element
		List *newJoinInputList = NULL;
		List *newJoinPopArg = NULL;//（是Node *的List）
		Node *newJionPushArg = MakeConstSint(joinflatNode->outPushWeights[0]);
		List *tmp_joinInputList = oldJoinIntputList;
		int b_index;

		Node *fusedOutputNode = NULL;
		Node *fusedOutputId =NULL;
		set<FlatNode *> tmpJoinInputSet(flatNodes.begin(),flatNodes.end());
		for(b_index = 0; b_index != joinflatNode->inFlatNodes.size(); b_index++)
		{
			if(tmpJoinInputSet.count(joinflatNode->inFlatNodes[b_index]))
			{
				fusedOutputNode = (Node *)FirstItem(oldJoinIntputList);

				if(fusedOutputNode->typ == Id) fusedOutputId = fusedOutputNode;
				else fusedOutputId = MakeNewStreamId(fusedOutputNode->u.decl.name,fusedOutputNode);

				newJoinInputList = AppendItem(newJoinInputList,fusedOutputId);
				break;
			}

			Node *element = (Node *)FirstItem(oldJoinIntputList);
			newJoinInputList = AppendItem(newJoinInputList,element);
			newJoinPopArg = AppendItem(newJoinPopArg,MakeConstSint(joinflatNode->inPopWeights[b_index]));
			tmp_joinInputList = GetNextList(tmp_joinInputList);
		}
		int tmp_popvalue = 0;
		for(int i = 0; i != flatNodes.size(); i++)
		{
			tmp_joinInputList = GetNextList(tmp_joinInputList);
			tmp_popvalue += joinflatNode->inPopWeights[b_index + i];
		}
		newJoinPopArg = AppendItem(newJoinPopArg,MakeConstSint(tmp_popvalue) );
		newJoinInputList = JoinLists(newJoinInputList,tmp_joinInputList);
		for(int i = b_index + flatNodes.size(); i != joinflatNode->inPopWeights.size(); i++)
			newJoinPopArg = AppendItem(newJoinPopArg,MakeConstSint(joinflatNode->inPopWeights[i]));

		//2.融合flatNodes中的节点
		//2.1 融合flatNodes——在每一个operator的外面要做一个循环（循环的次数是最小周期数），对operator中的变量重命名,修改operator的输入输出边名称以及数据访问的偏移
		Node *fusedOperatorNode = fusingNodes(flatNodes,inputNewStream,fusedOutputId->u.id.decl);

		//对融合后的节点的输出边输出的数据根据joinflatNode的情况进行重排
		Node *newOperatorNode = reorderFusionJoinNode(fusedOperatorNode,joinflatNode,flatNodes);
		Node *newJoinNode = makeHFusionJoin(newJoinInputList,newJoinPopArg,fusedOutputId,newJionPushArg); 
		vector<operatorNode *> newOperatorNodeVec;
		newOperatorNodeVec.push_back(&(jNode->u.operator_));
		newOperatorNodeVec.push_back(&(newOperatorNode->u.operator_));
		newOperatorNodeVec.push_back(&(newJoinNode->u.operator_));
		vector<FlatNode *>oldFlatNodes(flatNodes);
		oldFlatNodes.push_back(joinflatNode);
		vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//更新sssg了
	}

}

//************************************
// Qualifier: 根据flatNodes与split的关系，将根据flatNodes融合得到的operNode的输出边的数据进行重排序
// Parameter: Node * operNode
// Parameter: FlatNode * joinFlatNode
// Parameter: vector<FlatNode * > flatNodes
//************************************
Node *HorizontalFusionSSG::reorderFusionSplitNode(Node *operNode,FlatNode *splitFlatNode,vector<FlatNode *> flatNodes)
{
	assert(splitFlatNode->contents->ot == Roundrobin_);
	//1、取split对flatNodes中的各个边数据的pop值
	int begin_index = splitFlatNode->outFlatNodes.size();//记录flatNodes的第一个节点在split的第几个分支上
	
	set<FlatNode *> tmpSplitOutputSet(flatNodes.begin(),flatNodes.end());
	
	for(int i = 0;i !=  splitFlatNode->outFlatNodes.size(); i++)
	{
		if( tmpSplitOutputSet.count(splitFlatNode->outFlatNodes[i])){ begin_index = i; break;}
	}
	assert(begin_index != splitFlatNode->outFlatNodes.size());

	vector<int > push_values; 
	push_values.push_back(0);
	int pop_value = 0;
	for(int j = 0; j!= flatNodes.size();j++)
	{
		push_values.push_back(splitFlatNode->outPushWeights[begin_index+j]);
		pop_value += splitFlatNode->outPushWeights[begin_index+j];
	}

	int splitSteadycount = _sjflatNode2smallSteadyCount.find(splitFlatNode)->second;//join节点在最小稳态的执行次数
	int popNum = pop_value * splitSteadycount;//最小稳定状态执行完后产生的数据总量

	//取operNode的输入边的流类型的成员声明列表
	Node *inputNode = (Node *)FirstItem(operNode->u.operator_.decl->u.decl.type->u.operdcl.inputs);
	Node *inputDecl = NULL;
	if(inputNode->typ == Id) inputDecl = inputNode->u.id.decl;
	else inputDecl = inputNode;
	List *stream_fields = inputDecl->u.decl.type->u.strdcl.type->fields;
	//构造重排序需要的中间变量的类型（struct）
	SUEtype* tmpStructType = NULL;
	tmpStructType = HeapNew(SUEtype);
	tmpStructType->complete = TRUE;
	tmpStructType->coord = UnknownCoord;
	char * tmpStructName = (char *) malloc(30);
	sprintf(tmpStructName, "%s_%d", "input_streamtype_struct",HFVarNum);
	tmpStructType->name = tmpStructName;
	tmpStructType->fields = stream_fields;
	tmpStructType->typ = Sdcl;
	tmpStructType->visited = FALSE;
	Node *structDecl = MakeSdclCoord(T_SUE_ELABORATED,tmpStructType, UnknownCoord);
	Node *bufferAdcl = MakeAdclCoord(EMPTY_TQ,structDecl,MakeConstSint(popNum),UnknownCoord);

	char * buffername = (char *) malloc(30);
	sprintf(buffername, "%s_%d", "input_buffer",HFVarNum);
	Node *bufferDecl = MakeDeclCoord(buffername,T_BLOCK_DECL,bufferAdcl,NULL,NULL,UnknownCoord);//将结构体数组构造完成，bufferDecl——要被添加到work函数中
	
	//利用buffer作为中转站将输出流中的数据重排序

	//构造 将operNode的输入流中的数据临时保存到buffer中 的语句——需要被添加到work中
	//构造一个循环
	char * _iname = (char *) malloc(8);
	sprintf(_iname, "%s_%d", "__i",HFVarNum);
	Node *idI = MakeIdCoord(_iname, UnknownCoord);
	Node *declI = MakeNewDecl(_iname, PrimSint, MakeConstSint(0), Redecl);//构造循环变量的定义——要加到work中
	Node *init = NULL, *cond = NULL, *next = NULL;
	idI->u.id.decl = declI;
	init = MakeBinopCoord('=', idI, MakeConstSint(0), UnknownCoord);
	cond = MakeBinopCoord('<', idI, MakeConstSint(popNum), UnknownCoord);
	next = MakeUnaryCoord(POSTINC, idI, UnknownCoord);
	//下面构造赋值语句	
	Node *inputId = MakeNewStreamId(inputDecl->u.decl.name,inputDecl);//构造输出流对应的idNode
	Node *bufferId = MakeIdCoord(buffername,UnknownCoord);//构造缓冲区对应的idnode
	bufferId->u.id.decl = bufferDecl;
	Node *bufferArray = ExtendArray(bufferId,idI,UnknownCoord);
	Node *inputArray = ExtendArray(inputId,idI,UnknownCoord);
	List *assignmentList_1 = NULL;//赋值语句list
	ListMarker maker;
	Node *field = NULL;
	IterateList(&maker,stream_fields);
	while (NextOnList(&maker , (GenericREF)&field))
	{
		assert(field->typ == Decl);
		Node *tmpId = MakeIdCoord(field->u.decl.name,UnknownCoord);
		tmpId->u.id.decl = field;
		Node *left = MakeBinopCoord('.',bufferArray,tmpId,UnknownCoord);
		Node *right = MakeBinopCoord('.',inputArray,tmpId,UnknownCoord);
		Node *assignNode = MakeBinopCoord('=',left,right,UnknownCoord);
		assignmentList_1 = AppendItem(assignmentList_1,assignNode);
	}
	Node *for_stmt = MakeBlockCoord(PrimVoid,NULL,assignmentList_1,UnknownCoord,UnknownCoord);
	Node *initBufferNode = MakeForCoord(init,cond,next,for_stmt,UnknownCoord);//构造初始化buffer的语句——要加到work中

	//将buffer中的数据调整顺序后重新写回到输入缓冲区中
	char * _jname = (char *) malloc(8);
	sprintf(_jname, "%s_%d", "__j",HFVarNum);
	char * _kname = (char *) malloc(8);
	sprintf(_kname, "%s_%d", "__k",HFVarNum);
	Node *idJ = MakeIdCoord(_jname, UnknownCoord);
	Node *idK = MakeIdCoord(_kname, UnknownCoord);
	Node *declJ = MakeNewDecl(_jname, PrimSint, MakeConstSint(0), Redecl);// 加到work中
	Node *declK = MakeNewDecl(_kname, PrimSint, MakeConstSint(0), Redecl);//加到work中
	idJ->u.id.decl = declJ;idK->u.id.decl = declK;
	Node *delt = NULL;
	Node *internForNode = NULL;
	Node *externForNode = NULL;
	HFVarNum++;
	delt = MakeBinopCoord('+',idJ,MakeBinopCoord('*',idI,MakeConstSint(pop_value), UnknownCoord),UnknownCoord);
	int offsetDate = 0;

	Node *nextK = MakeUnaryCoord(POSTINC, idK, UnknownCoord);

	Node *initJ= NULL,*condJ= NULL,*nextJ= NULL,*stmtJ= NULL;
	List *stmts = NULL;

	for(int i = 0; i != flatNodes.size(); i++)
	{	
		offsetDate += push_values[i];
		init = MakeBinopCoord('=', idI, MakeConstSint(0), UnknownCoord);
		cond = MakeBinopCoord('<', idI, MakeConstSint(splitSteadycount), UnknownCoord);
		next = MakeUnaryCoord(POSTINC, idI, UnknownCoord);

		initJ = MakeBinopCoord('=', idJ, MakeConstSint(0), UnknownCoord);
		condJ = MakeBinopCoord('<', idJ, MakeConstSint(push_values[i+1]), UnknownCoord);
		nextJ = MakeUnaryCoord(POSTINC, idJ, UnknownCoord);

		bufferArray = ExtendArray(bufferId,MakeBinopCoord('+',delt,MakeConstSint(offsetDate),UnknownCoord),UnknownCoord);
		inputArray = ExtendArray(inputId,nextK,UnknownCoord);
		List *assignmentList_2 = NULL;//赋值语句list
		IterateList(&maker,stream_fields);
		while (NextOnList(&maker , (GenericREF)&field))
		{
			assert(field->typ == Decl);
			Node *tmpId = MakeIdCoord(field->u.decl.name,UnknownCoord);
			tmpId->u.id.decl = field;
			Node *left = MakeBinopCoord('.',inputArray,tmpId,UnknownCoord);
			Node *right = MakeBinopCoord('.',bufferArray,tmpId,UnknownCoord);
			Node *assignNode = MakeBinopCoord('=',left,right,UnknownCoord);
			assignmentList_2 = AppendItem(assignmentList_2,assignNode);
		}
		stmtJ =  MakeBlockCoord(PrimVoid,NULL,assignmentList_2,UnknownCoord,UnknownCoord);

		Node *internForstmt = MakeForCoord(initJ,condJ,nextJ,stmtJ,UnknownCoord);
		Node *internForNode = MakeForCoord(init,cond,next,internForstmt,UnknownCoord);
		stmts = AppendItem(stmts, internForNode);		
	}
	Node *reorderWork = MakeBlockCoord(PrimVoid, NULL, stmts, UnknownCoord, UnknownCoord);//重排序的最外层循环节点构造完成——要加到work函数中
	//将新造的节点添加到work中

	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,declI);
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,declJ);
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,declK);
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,bufferDecl);
	List *tmp_stmts = NULL;
	tmp_stmts = AppendItem(tmp_stmts,initBufferNode);
	tmp_stmts = AppendItem(tmp_stmts,reorderWork);
	operNode->u.operator_.body->u.operBody.work->u.Block.stmts = JoinLists(tmp_stmts, operNode->u.operator_.body->u.operBody.work->u.Block.stmts);
	return operNode;
}

//************************************
// Qualifier: 如果splitflatNode的下端的分支数和flatNodes中的节点数目相同融合成一个新的节点，同时在flatNodes的下端还要构造出一个类似于split的节点
//				如果splitflatNode的下端的分支数大于flatNodes中的节点数，则将flatNode节点融合在一起，同时构造新的join节点代替joinflatNode，并且在flatNodes的上端还要构造出一个类似于join的节点
// Parameter: FlatNode * splitflatNode
// Parameter: vector<FlatNode * > flatNodes
//************************************
void HorizontalFusionSSG::fusingFlatNodesSplit(FlatNode *splitflatNode,vector<FlatNode *> flatNodes)
{
	int nbranch = splitflatNode->nOut;
	//先构造融合后节点的输出边
	operatorNode *curOperator = flatNodes[0]->contents;
	char *fusedOutputName = NULL;
	Node *outputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.outputs);
	Node *output_Decl = NULL;
	if(outputNode->typ == Id) output_Decl = outputNode->u.id.decl;
	else output_Decl = outputNode;
	Node *outputStreamType = output_Decl->u.decl.type;
	fusedOutputName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
	sprintf(fusedOutputName, "%s_%d", output_Decl->u.decl.name, ++HFStreamNum);
	Node *outputNewStream = MakeNewStream(fusedOutputName,outputStreamType);//flatNodes节点融合后形成的
	assert(outputNewStream && outputNewStream ->typ ==Decl);

	//构造flatNodes日融合后节点下端的join节点——构造成roundrobin类型
	List *down_splitOutputList = NULL;//flatNodes中的节点的输入
	List *down_splitPushArg = NULL;
	int down_splitPopValue = 0;//新建的join节点的push值

	for(int i = flatNodes.size() - 1; i >= 0;i--)
	{
		assert( ListLength(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.outputs) == 1 );
		down_splitOutputList = AppendItem(down_splitOutputList, (Node *)FirstItem(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.outputs));
		int pushValue = _sjflatNode2smallSteadyCount.find(flatNodes[i])->second * flatNodes[i]->outPushWeights[0];
		down_splitPushArg = AppendItem(down_splitPushArg, MakeConstSint(pushValue));
		down_splitPopValue += pushValue;
	}
	Node *down_splitPopArg = MakeConstSint(down_splitPopValue);
	Node *down_splitNode = makeHFusionSplit(outputNewStream,down_splitPopArg,down_splitOutputList,down_splitPushArg,S_RoundRobin);
	
	if(nbranch == flatNodes.size())
	{//将split以及flatNodes融合成一个新的operator节点（先将flatNodes节点融合成一个新的operator然后在新的operator，然后再对输出边的数据进行重排序（为了将flatNodes下端的节点join节点处理掉）——不需要引入内部缓冲区，直接对输入缓冲区进行操作）
		//2.融合flatNodes中的节点
		Node *splitInputNode = (Node *)FirstItem(splitflatNode->contents->decl->u.decl.type->u.operdcl.inputs);
		Node *splitIntputStream = NULL; 
		if(splitInputNode->typ == Id) splitIntputStream = splitInputNode->u.id.decl;
		else splitIntputStream = splitInputNode;
		//2.1 融合flatNodes——在每一个operator的外面要做一个循环（循环的次数是最小周期数），对operator中的变量重命名,修改operator的输入输出边名称以及数据访问的偏移
		Node *fusedOperatorNode = fusingNodes(flatNodes,splitIntputStream,outputNewStream);

		//2.2 对于待融合节点的数目和分支数目的不同对上端的split节点进行不同的处理

		//将joinflatNode节点融合掉——方法是在fusedOperatorNode的work函数中添加对输出流中的数据进行重排序
		Node *newOperatorNode = NULL;

		if(splitflatNode->contents->ot == Roundrobin_)newOperatorNode = reorderFusionSplitNode(fusedOperatorNode,splitflatNode,flatNodes);
		else newOperatorNode = fusedOperatorNode;
		
		vector<operatorNode *> newOperatorNodeVec;

		newOperatorNodeVec.push_back(&(newOperatorNode->u.operator_));	
		newOperatorNodeVec.push_back(&(down_splitNode->u.operator_));
		vector<FlatNode *>oldFlatNodes(flatNodes);
		oldFlatNodes.push_back(splitflatNode);
		vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//更新sssg了							
	}
	else if(nbranch > flatNodes.size())
	{
		
		//确定新的split结点的输入输出边
		List *new_splitOutputList = NULL;
		List *new_splitPushArgs = NULL;
		List *new_splitInputList = splitflatNode->contents->decl->u.decl.type->u.operdcl.inputs;

		List *old_splitOutputList= splitflatNode->contents->decl->u.decl.type->u.operdcl.outputs;

		Node *fusedInputNode = NULL;//新融合节点的输入边
		Node *fusedInputId = NULL;
		int b_index;
		set<FlatNode *> tmpSplitOutputSet(flatNodes.begin(),flatNodes.end());
		for(b_index = 0; b_index != splitflatNode->outFlatNodes.size(); b_index++)
		{
			if(tmpSplitOutputSet.count(splitflatNode->outFlatNodes[b_index]))
			{
				fusedInputNode = (Node *)FirstItem(old_splitOutputList);

				if(fusedInputNode->typ == Id) fusedInputId = fusedInputNode;
				else fusedInputId = MakeNewStreamId(fusedInputNode->u.decl.name,fusedInputNode);

				new_splitOutputList = AppendItem(new_splitOutputList,fusedInputId);
				break;
			}
			
			Node *element = (Node *)FirstItem(old_splitOutputList);
			new_splitOutputList = AppendItem(new_splitOutputList,element);
			new_splitPushArgs = AppendItem(new_splitPushArgs,MakeConstSint(splitflatNode->outPushWeights[b_index]));
			old_splitOutputList = GetNextList(old_splitOutputList);
		}
		int tmp_pushvalue = 0;
		for(int i = 0; i != flatNodes.size(); i++)
		{
			old_splitOutputList = GetNextList(old_splitOutputList);
			tmp_pushvalue += splitflatNode->outPushWeights[b_index + i];
		}
		new_splitPushArgs = AppendItem(new_splitPushArgs,MakeConstSint(tmp_pushvalue) );
		new_splitOutputList = JoinLists(new_splitOutputList,old_splitOutputList);

		for(int i = b_index + flatNodes.size(); i != splitflatNode->outPushWeights.size(); i++)
			new_splitPushArgs = AppendItem(new_splitPushArgs,MakeConstSint(splitflatNode->outPushWeights[i]));

		//int old_splitSteadyCount = sjflatNode2smallSteadyCount.find(splitflatNode)->second;//找split节点的局部稳态次数
	
		//2.融合flatNodes中的节点
		//2.1 融合flatNodes——在每一个operator的外面要做一个循环（循环的次数是最小周期数），对operator中的变量重命名,修改operator的输入输出边名称以及数据访问的偏移
		Node *fusedOperatorNode = fusingNodes(flatNodes,fusedInputId->u.id.decl,outputNewStream);

		//2.2 对于待融合节点的数目和分支数目的不同对上端的split节点进行不同的处理

		//将joinflatNode节点融合掉——方法是在fusedOperatorNode的work函数中添加对输出流中的数据进行重排序
		Node *newOperatorNode = NULL;

		if(splitflatNode->contents->ot == Roundrobin_)newOperatorNode = reorderFusionSplitNode(fusedOperatorNode,splitflatNode,flatNodes);
		else newOperatorNode = fusedOperatorNode;

		Node *newSplitNode = NULL;
		if(splitflatNode->contents->ot == Roundrobin_)newSplitNode = makeHFusionSplit((Node *)FirstItem(new_splitInputList),MakeConstSint(splitflatNode->inPopWeights[0]),new_splitOutputList,new_splitPushArgs,S_RoundRobin); 
		else newSplitNode = makeHFusionSplit((Node *)FirstItem(new_splitInputList),MakeConstSint(splitflatNode->inPopWeights[0]),new_splitOutputList,NULL,S_Duplicate); 
		
		vector<operatorNode *> newOperatorNodeVec;
		newOperatorNodeVec.push_back(&(newSplitNode->u.operator_));
		newOperatorNodeVec.push_back(&(newOperatorNode->u.operator_));
		newOperatorNodeVec.push_back(&(down_splitNode->u.operator_));
		
		vector<FlatNode *>oldFlatNodes;
		oldFlatNodes.push_back(splitflatNode);
		for(int i = 0; i != flatNodes.size(); i++)
			oldFlatNodes.push_back(flatNodes[i]);
		vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//更新sssg了		
	}
}

//************************************
// Qualifier: 上端split的分支 = flatNodesde数目 = 下端join分支，融合split，flatNodes，join
//************************************
vector<operatorNode *> HorizontalFusionSSG::fusionFlatNodesSplitJoin_FSFJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{
	vector<operatorNode *> newOperatorNodeVec;
	//取split的输入边，join的输出边
	Node *splitInputNode = (Node *)FirstItem(splitflatNode->contents->decl->u.decl.type->u.operdcl.inputs);
	Node *joinOutputNode = (Node *)FirstItem(joinflatNode->contents->decl->u.decl.type->u.operdcl.outputs);
	Node *splitInputDecl = NULL;
	Node *joinOutputDecl = NULL;
	if(splitInputNode->typ == Id) splitInputDecl = splitInputNode->u.id.decl;
	else splitInputDecl = splitInputNode;
	if(joinOutputNode->typ == Id) joinOutputDecl = joinOutputNode->u.id.decl;
	else joinOutputDecl = joinOutputNode;
	Node *fusionNode = fusingNodes(flatNodes,splitInputDecl,joinOutputDecl);
	Node *newOperatorNode = NULL;
	if(splitflatNode->contents->ot == Roundrobin_) newOperatorNode = reorderFusionSplitNode(fusionNode,splitflatNode,flatNodes);
	else newOperatorNode = fusionNode;
	Node *finalOperatorNode = reorderFusionJoinNode(newOperatorNode,joinflatNode,flatNodes);
	newOperatorNodeVec.push_back(&(finalOperatorNode->u.operator_));
	return newOperatorNodeVec;
}

//************************************
// Qualifier: 修改split，融合flatNodes和join
//************************************
vector<operatorNode *> HorizontalFusionSSG::fusionFlatNodesSplitJoin_MSFJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{//修改split，融合flatNodes，同时对融合得到的operator节点进行输入输出流中数据的reorder
	vector<operatorNode *> newOperatorNodeVec;
	//1.取join的输出边作为融合后节点的输出
	Node *old_joinOutputNode = (Node *)FirstItem(joinflatNode->contents->decl->u.decl.type->u.operdcl.outputs);
	Node *old_joinOutputDecl = NULL;
	if(old_joinOutputNode->typ == Id) old_joinOutputDecl = old_joinOutputNode->u.id.decl;
	else old_joinOutputDecl = old_joinOutputNode;
	// 2.构造融合后节点的输入边
	operatorNode *curOperator = flatNodes[0]->contents;
	char *new_fusedInputName = NULL;
	Node *inputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.inputs);
	Node *input_Decl = NULL;
	if(inputNode->typ == Id) input_Decl = inputNode->u.id.decl;
	else input_Decl = inputNode;
	Node *new_fusedInputStreamType = input_Decl->u.decl.type;
	new_fusedInputName = (char *) malloc(strlen(input_Decl->u.decl.name)+20);
	sprintf(new_fusedInputName, "%s_%d", input_Decl->u.decl.name, ++HFStreamNum);
	Node *new_fusedInputNewStream = MakeNewStream(new_fusedInputName,new_fusedInputStreamType);
	assert(new_fusedInputNewStream && new_fusedInputNewStream ->typ ==Decl);

	//3.融合flatNode节点
	Node *fusionOperatorNode = fusingNodes(flatNodes,new_fusedInputNewStream,old_joinOutputDecl);
	//4.根据join节点的类型对新融合节点的输入边重排序
	Node *reorderNode_1 = reorderFusionJoinNode(fusionOperatorNode,joinflatNode,flatNodes);
	//5.根据split节点的类型对新融合节点的输入边重排序
	Node *reorderNode_2 = NULL; 
	if(splitflatNode->contents->ot==Roundrobin_) reorderNode_2 = reorderFusionSplitNode(reorderNode_1,splitflatNode,flatNodes);
	else reorderNode_2 = reorderNode_1;
	
	//6.修改上端的split节点
	Node *fusedInputId = MakeNewStreamId(new_fusedInputNewStream->u.decl.name,new_fusedInputNewStream);//融合后节点的输入边（作为新的split的一个输出）

		//确定新的split结点的输入输出边
	List *new_splitOutputList = NULL;
	List *new_splitPushArgs = NULL;
	List *new_splitInputList = splitflatNode->contents->decl->u.decl.type->u.operdcl.inputs;

	List *old_splitOutputList= splitflatNode->contents->decl->u.decl.type->u.operdcl.outputs;

	int b_index;
	set<FlatNode *> tmpSplitOutputSet(flatNodes.begin(),flatNodes.end());
	for(b_index = 0; b_index != splitflatNode->outFlatNodes.size(); b_index++)
	{
		if(tmpSplitOutputSet.count(splitflatNode->outFlatNodes[b_index]))
		{
			new_splitOutputList = AppendItem(new_splitOutputList,fusedInputId);
			break;
		}
		
		Node *element = (Node *)FirstItem(old_splitOutputList);
		new_splitOutputList = AppendItem(new_splitOutputList,element);
		new_splitPushArgs = AppendItem(new_splitPushArgs,MakeConstSint(splitflatNode->outPushWeights[b_index]));
		old_splitOutputList = GetNextList(old_splitOutputList);
	}
	int tmp_pushvalue = 0;
	for(int i = 0; i != flatNodes.size(); i++)
	{
		old_splitOutputList = GetNextList(old_splitOutputList);
		tmp_pushvalue += splitflatNode->outPushWeights[b_index + i];
	}
	new_splitPushArgs = AppendItem(new_splitPushArgs,MakeConstSint(tmp_pushvalue) );
	new_splitOutputList = JoinLists(new_splitOutputList,old_splitOutputList);

	for(int i = b_index + flatNodes.size(); i != splitflatNode->outPushWeights.size(); i++)
		new_splitPushArgs = AppendItem(new_splitPushArgs,MakeConstSint(splitflatNode->outPushWeights[i]));

	Node *newSplitNode = NULL;
	if(splitflatNode->contents->ot == Roundrobin_)newSplitNode = makeHFusionSplit((Node *)FirstItem(new_splitInputList),MakeConstSint(splitflatNode->inPopWeights[0]),new_splitOutputList,new_splitPushArgs,S_RoundRobin); 
	else newSplitNode = makeHFusionSplit((Node *)FirstItem(new_splitInputList),MakeConstSint(splitflatNode->inPopWeights[0]),new_splitOutputList,NULL,S_Duplicate); 
	//7.确定最终的返回
	newOperatorNodeVec.push_back(&(newSplitNode->u.operator_));
	newOperatorNodeVec.push_back(&(reorderNode_2->u.operator_));
	return newOperatorNodeVec;
}

//************************************
// Qualifier: 融合split修改join
//************************************
vector<operatorNode *> HorizontalFusionSSG::fusionFlatNodesSplitJoin_FSMJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{//_ Fuse Split Modify Join //修改join，融合flatNodes，同时对融合得到的operator节点进行输入输出流中数据的reorder
	vector<operatorNode *> newOperatorNodeVec;
	//1.取split的输入边作为融合后节点的输入
	Node *old_splitInputNode = (Node *)FirstItem(splitflatNode->contents->decl->u.decl.type->u.operdcl.inputs);
	Node *old_splitInputDecl = NULL;
	if(old_splitInputNode->typ == Id) old_splitInputDecl = old_splitInputNode->u.id.decl;
	else old_splitInputDecl = old_splitInputNode;
	// 2.构造融合后节点的输出边
	operatorNode *curOperator = flatNodes[0]->contents;
	char *new_fusedOutputName = NULL;
	Node *outputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.outputs);//取第一个待融合节点的输出
	Node *output_Decl = NULL;
	if(outputNode->typ == Id) output_Decl = outputNode->u.id.decl;
	else output_Decl = outputNode;
	Node *new_fusedOutputStreamType = output_Decl->u.decl.type;
	new_fusedOutputName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
	sprintf(new_fusedOutputName, "%s_%d", output_Decl->u.decl.name, ++HFStreamNum);
	Node *new_fusedOutputNewStream = MakeNewStream(new_fusedOutputName,new_fusedOutputStreamType);
	assert(new_fusedOutputNewStream && new_fusedOutputNewStream ->typ ==Decl);

	//3.融合flatNode节点
	Node *fusionOperatorNode = fusingNodes(flatNodes,old_splitInputNode,new_fusedOutputNewStream);
	//4.根据split节点的类型对新融合节点的输入边重排序
	Node *reorderNode_1 = NULL; 
	if(splitflatNode->contents->ot==Roundrobin_) reorderNode_1 = reorderFusionSplitNode(fusionOperatorNode,splitflatNode,flatNodes);
	else reorderNode_1 = fusionOperatorNode;
	//5.根据join节点的类型对新融合节点的输入边重排序
	Node *reorderNode_2 = reorderFusionJoinNode(reorderNode_1,joinflatNode,flatNodes);
	
	//6.修改下端的join节点
	Node *fusedOutputId = MakeNewStreamId(new_fusedOutputNewStream->u.decl.name,new_fusedOutputNewStream);//融合后节点的输出边（作为新的join的一个输出）

		//确定新的join结点的输入输出边以及窗口
	List *new_joinInputList = NULL;
	List *new_joinPopArgs = NULL;

	List *new_joinOutputList= joinflatNode->contents->decl->u.decl.type->u.operdcl.outputs;
	List *old_joinInputList = joinflatNode->contents->decl->u.decl.type->u.operdcl.inputs;
	
	int b_index;
	set<FlatNode *> tmpJoinInputSet(flatNodes.begin(),flatNodes.end());
	for(b_index = 0; b_index != joinflatNode->inFlatNodes.size(); b_index++)
	{
		if(tmpJoinInputSet.count(joinflatNode->inFlatNodes[b_index]))
		{
			new_joinInputList = AppendItem(new_joinInputList,fusedOutputId);
			break;
		}
		Node *element = (Node *)FirstItem(old_joinInputList);
		new_joinInputList = AppendItem(new_joinInputList,element);
		new_joinPopArgs = AppendItem(new_joinPopArgs,MakeConstSint(joinflatNode->inPopWeights[b_index]));
		old_joinInputList = GetNextList(old_joinInputList);
	}
	int tmp_popvalue = 0;
	for(int i = 0; i != flatNodes.size(); i++)
	{
		old_joinInputList = GetNextList(old_joinInputList);
		tmp_popvalue += joinflatNode->inPopWeights[b_index + i];
	}
	new_joinPopArgs = AppendItem(new_joinPopArgs,MakeConstSint(tmp_popvalue) );
	new_joinInputList = JoinLists(new_joinInputList,old_joinInputList);

	for(int i = b_index + flatNodes.size(); i != splitflatNode->outPushWeights.size(); i++)
		new_joinPopArgs = AppendItem(new_joinPopArgs,MakeConstSint(joinflatNode->inPopWeights[i]));

	Node *newJoinNode = makeHFusionJoin(new_joinInputList,new_joinPopArgs,(Node *)FirstItem(new_joinOutputList),MakeConstSint(joinflatNode->outPushWeights[0])); 
	//7.确定最终的返回
	newOperatorNodeVec.push_back(&(newJoinNode->u.operator_));
	newOperatorNodeVec.push_back(&(reorderNode_2->u.operator_));
	return newOperatorNodeVec;
}

//************************************
// Qualifier:修改split，修改join
//************************************
vector<operatorNode *> HorizontalFusionSSG::fusionFlatNodesSplitJoin_MSMJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{//_ Modify Split Modify Join //修改split，join，融合flatNodes，同时对融合后的节点进行reorder
	vector<operatorNode *> newOperatorNodeVec;
	operatorNode *curOperator = flatNodes[0]->contents;
	
	//1.构造融合后节点的输入边
	char *new_fusedInputName = NULL;
	Node *inputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.inputs);
	Node *input_Decl = NULL;
	if(inputNode->typ == Id) input_Decl = inputNode->u.id.decl;
	else input_Decl = inputNode;
	Node *new_fusedInputStreamType = input_Decl->u.decl.type;
	new_fusedInputName = (char *) malloc(strlen(input_Decl->u.decl.name)+20);
	sprintf(new_fusedInputName, "%s_%d", input_Decl->u.decl.name, ++HFStreamNum);
	Node *new_fusedInputNewStream = MakeNewStream(new_fusedInputName,new_fusedInputStreamType);
	assert(new_fusedInputNewStream && new_fusedInputNewStream ->typ ==Decl);
	
	// 2.构造融合后节点的输出边
	char *new_fusedOutputName = NULL;
	Node *outputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.outputs);//取第一个待融合节点的输出
	Node *output_Decl = NULL;
	if(outputNode->typ == Id) output_Decl = outputNode->u.id.decl;
	else output_Decl = outputNode;
	Node *new_fusedOutputStreamType = output_Decl->u.decl.type;
	new_fusedOutputName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
	sprintf(new_fusedOutputName, "%s_%d", output_Decl->u.decl.name, ++HFStreamNum);
	Node *new_fusedOutputNewStream = MakeNewStream(new_fusedOutputName,new_fusedOutputStreamType);
	assert(new_fusedOutputNewStream && new_fusedOutputNewStream ->typ ==Decl);

	//3.融合flatNode节点
	Node *fusionOperatorNode = fusingNodes(flatNodes,new_fusedInputNewStream,new_fusedOutputNewStream);
	//4.根据split节点的类型对新融合节点的输入边重排序
	Node *reorderNode_1 = NULL; 
	if(splitflatNode->contents->ot==Roundrobin_) reorderNode_1 = reorderFusionSplitNode(fusionOperatorNode,splitflatNode,flatNodes);
	else reorderNode_1 = fusionOperatorNode;
	//5.根据join节点的类型对新融合节点的输入边重排序
	Node *reorderNode_2 = reorderFusionJoinNode(reorderNode_1,joinflatNode,flatNodes);
	//6.构造split
	Node *fusedInputId = MakeNewStreamId(new_fusedInputNewStream->u.decl.name,new_fusedInputNewStream);//融合后节点的输入边（作为新的split的一个输出）

	//确定新的split结点的输入输出边
	List *new_splitOutputList = NULL;
	List *new_splitPushArgs = NULL;
	List *new_splitInputList = splitflatNode->contents->decl->u.decl.type->u.operdcl.inputs;

	List *old_splitOutputList= splitflatNode->contents->decl->u.decl.type->u.operdcl.outputs;

	int b_index;
	set<FlatNode *> tmpSplitOutputSet(flatNodes.begin(),flatNodes.end());
	for(b_index = 0; b_index != splitflatNode->outFlatNodes.size(); b_index++)
	{
		if(tmpSplitOutputSet.count(splitflatNode->outFlatNodes[b_index] ))
		{
			new_splitOutputList = AppendItem(new_splitOutputList,fusedInputId);
			break;
		}

		Node *element = (Node *)FirstItem(old_splitOutputList);
		new_splitOutputList = AppendItem(new_splitOutputList,element);
		new_splitPushArgs = AppendItem(new_splitPushArgs,MakeConstSint(splitflatNode->outPushWeights[b_index]));
		old_splitOutputList = GetNextList(old_splitOutputList);
	}
	int tmp_pushvalue = 0;
	for(int i = 0; i != flatNodes.size(); i++)
	{
		old_splitOutputList = GetNextList(old_splitOutputList);
		tmp_pushvalue += splitflatNode->outPushWeights[b_index + i];
	}
	new_splitPushArgs = AppendItem(new_splitPushArgs,MakeConstSint(tmp_pushvalue) );
	new_splitOutputList = JoinLists(new_splitOutputList,old_splitOutputList);

	for(int i = b_index + flatNodes.size(); i != splitflatNode->outPushWeights.size(); i++)
		new_splitPushArgs = AppendItem(new_splitPushArgs,MakeConstSint(splitflatNode->outPushWeights[i]));

	Node *newSplitNode = NULL;
	if(splitflatNode->contents->ot == Roundrobin_)newSplitNode = makeHFusionSplit((Node *)FirstItem(new_splitInputList),MakeConstSint(splitflatNode->inPopWeights[0]),new_splitOutputList,new_splitPushArgs,S_RoundRobin); 
	else newSplitNode = makeHFusionSplit((Node *)FirstItem(new_splitInputList),MakeConstSint(splitflatNode->inPopWeights[0]),new_splitOutputList,NULL,S_Duplicate); 
	

	//7构造join
	Node *fusedOutputId = MakeNewStreamId(new_fusedOutputNewStream->u.decl.name,new_fusedOutputNewStream);//融合后节点的输出边（作为新的join的一个输出）

	//确定新的join结点的输入输出边以及窗口
	List *new_joinInputList = NULL;
	List *new_joinPopArgs = NULL;

	List *new_joinOutputList= joinflatNode->contents->decl->u.decl.type->u.operdcl.outputs;
	List *old_joinInputList = joinflatNode->contents->decl->u.decl.type->u.operdcl.inputs;

	set<FlatNode *> tmpJoinInputSet(flatNodes.begin(),flatNodes.end());
	for(b_index = 0; b_index != joinflatNode->inFlatNodes.size(); b_index++)
	{
		if(tmpJoinInputSet.count(joinflatNode->inFlatNodes[b_index]))
		{
			new_joinInputList = AppendItem(new_joinInputList,fusedOutputId);
			break;
		}
		Node *element = (Node *)FirstItem(old_joinInputList);
		new_joinInputList = AppendItem(new_joinInputList,element);
		new_joinPopArgs = AppendItem(new_joinPopArgs,MakeConstSint(joinflatNode->inPopWeights[b_index]));
		old_joinInputList = GetNextList(old_joinInputList);
	}
	int tmp_popvalue = 0;
	for(int i = 0; i != flatNodes.size(); i++)
	{
		old_joinInputList = GetNextList(old_joinInputList);
		tmp_popvalue += joinflatNode->inPopWeights[b_index + i];
	}
	new_joinPopArgs = AppendItem(new_joinPopArgs,MakeConstSint(tmp_popvalue) );
	new_joinInputList = JoinLists(new_joinInputList,old_joinInputList);

	for(int i = b_index + flatNodes.size(); i != splitflatNode->outPushWeights.size(); i++)
		new_joinPopArgs = AppendItem(new_joinPopArgs,MakeConstSint(joinflatNode->inPopWeights[i]));

	Node *newJoinNode = makeHFusionJoin(new_joinInputList,new_joinPopArgs,(Node *)FirstItem(new_joinOutputList),MakeConstSint(joinflatNode->outPushWeights[0])); 
	//8.确定最终的返回
	newOperatorNodeVec.push_back(&(newSplitNode->u.operator_));
	newOperatorNodeVec.push_back(&(reorderNode_2->u.operator_));
	newOperatorNodeVec.push_back(&(newJoinNode->u.operator_));
	return newOperatorNodeVec;
}
//************************************
// Qualifier: 分四种情况：1.上端split的分支 = flatNodesde数目 = 下端join分支
//						  2.上端split的分支 = flatNodesde数目 < 下端join分支
//						  3.上端split的分支 > flatNodesde数目 < 下端join分支
//						  4.上端split的分支 > flatNodesde数目 = 下端join分支
// Parameter: FlatNode * splitflatNode
// Parameter: FlatNode * joinflatNode
// Parameter: vector<FlatNode * > flatNodes
//************************************
void HorizontalFusionSSG::fusingFlatNodesSplitJoin(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{
	int split_branch = splitflatNode->nOut;
	int join_branch = joinflatNode->nIn ;
	int fusing_branch = flatNodes.size(); 
	vector<operatorNode *>newOperatorNodeVec;//记录融合新产生的operator
	if(split_branch == fusing_branch && fusing_branch == join_branch)
	{//融合split，flatNodes，join
		newOperatorNodeVec = fusionFlatNodesSplitJoin_FSFJ(splitflatNode,joinflatNode,flatNodes);
	}
	else if(split_branch > fusing_branch && fusing_branch == join_branch)
	{//修改split，融合flatNodes和join
		newOperatorNodeVec = fusionFlatNodesSplitJoin_MSFJ(splitflatNode,joinflatNode,flatNodes);
	}
	else if (split_branch == fusing_branch && fusing_branch < join_branch)
	{//修改join，融合split和flatNodes
		newOperatorNodeVec = fusionFlatNodesSplitJoin_FSMJ(splitflatNode,joinflatNode,flatNodes);
	}
	else if(split_branch > fusing_branch && fusing_branch < join_branch)
	{//修改split、join，融合flatNodes
		newOperatorNodeVec = fusionFlatNodesSplitJoin_MSMJ(splitflatNode,joinflatNode,flatNodes);
	}
	vector<FlatNode *>oldFlatNodes;
	oldFlatNodes.push_back(splitflatNode);
	for(int i = 0; i != flatNodes.size(); i++)
		oldFlatNodes.push_back(flatNodes[i]);
	oldFlatNodes.push_back(joinflatNode);
	vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//更新sssg了		
}

//************************************
// Qualifier: 将flatNode节点融合成一个节点(同时还要修改SSG中的依赖)，融合的分类见水平融合的文档
// Parameter: vector<FlatNode * > flatNodes
//************************************
void HorizontalFusionSSG::fusingFlatNodes(vector<FlatNode *> flatNodes)
{
	assert(flatNodes.size() > 1);
	//取flatNodes的上端节点和下端节点
	FlatNode *upFlatNode = flatNodes[0]->inFlatNodes[0];//flatNodes的上端节点
	FlatNode *downFlatNode = flatNodes[0]->outFlatNodes[0];//flatNodes的下端节点
	if (upFlatNode->nOut == 1 && downFlatNode->nIn == 1)
	{
		fusingFlatNodesCommon(flatNodes);//一种情况
	}
	else if(upFlatNode->nOut == 1  && downFlatNode->nIn > 1)
	{
		fusingFlatNodesJoin(downFlatNode, flatNodes);//2种情况
	}
	else if (upFlatNode->nOut > 1  && downFlatNode->nIn == 1)//((upFlatNode->contents->ot == Duplicate_ || upFlatNode->contents->ot == Roundrobin_ ) && downFlatNode->contents->ot == Common_)
	{
		fusingFlatNodesSplit(upFlatNode, flatNodes);//2中情况
	}
	else if (upFlatNode->nOut > 1  && downFlatNode->nIn > 1)//((upFlatNode->contents->ot == Duplicate_ || upFlatNode->contents->ot == Roundrobin_ ) && downFlatNode->contents->ot == Join_)
	{
		fusingFlatNodesSplitJoin(upFlatNode, downFlatNode, flatNodes);//4中情况
	}
	else
	{
		cout<<"fission error........."<<endl;
	}
}

//************************************
// Qualifier: 将flatNodes中的所有能够被水平融合的节点收集起来存放到priority2candidateFlatNodes中
//************************************
void HorizontalFusionSSG::collectFusionFlatNodesInfo(vector<FlatNode *> flatNodesTopoSequence)
{//找出当前SDF图中能够被融合的所有节点----------------要收集当前图中所有能够被融合的节点
	//提取图中所有可能被融合的节点，将最终结果保存在priority2candidateFlatNodes中
	vector<vector<FlatNode *> >splitjoinFlatNodes;//其中的每一个元素是一个splitjoin中包含的节点
	int joinFlag = 0;//标示是否已经遇到join节点了
	int splitFlag = 0;
	vector<FlatNode *>_tmpsjFlatNodes;//一个最内层的sj中的flatNode节点
	for (int i= flatNodesTopoSequence.size() - 1; i >= 0; i--)
	{//对于图是自底向上做的
		if (joinFlag && flatNodesTopoSequence[i]->contents->ot == Common_)
		{
			_tmpsjFlatNodes.push_back(flatNodesTopoSequence[i]);
		}
		else if (flatNodesTopoSequence[i]->contents->ot == Join_)
		{
			joinFlag++;
			_tmpsjFlatNodes.push_back(flatNodesTopoSequence[i]);
			
		}
		else if(joinFlag &&(flatNodesTopoSequence[i]->contents->ot == Duplicate_ || flatNodesTopoSequence[i]->contents->ot == Roundrobin_ ))
		{
			joinFlag--;
			_tmpsjFlatNodes.push_back(flatNodesTopoSequence[i]);
			if (!joinFlag)
			{
				vector<FlatNode *> _tmpflatNodes;
				for(int it =  _tmpsjFlatNodes.size()-1; it >= 0; it--)
				{
					_tmpsjFlatNodes[it]->VisitNode();
					//cout<<_tmpsjFlatNodes[it]->name<<"   "<<_tmpsjFlatNodes[it]->GetVisitTimes()<<endl;
					_tmpflatNodes.push_back(_tmpsjFlatNodes[it]);
				}
				//对此事的候选做一个稳态调度(只有joinFlag为0是才调度）
				map<FlatNode *,int>_sjflatNode2sc = hfsssg->SteadySchedulingGroup(_tmpflatNodes);
				_sjflatNode2smallSteadyCount.insert(_sjflatNode2sc.begin(),_sjflatNode2sc.end());
				_tmpsjFlatNodes.clear();
				splitjoinFlatNodes.push_back(_tmpflatNodes);
			}
			else
			{
				vector<FlatNode *> _tmpflatNodes;//第一个是split，最后一个是join
				for(int it =  _tmpsjFlatNodes.size()-1; it >= 0; it--)
				{
					if(_tmpsjFlatNodes[it]->contents->ot == Duplicate_ || _tmpsjFlatNodes[it]->contents->ot == Roundrobin_)
						splitFlag++;
					else if(_tmpsjFlatNodes[it]->contents->ot == Join_) splitFlag--;
					_tmpsjFlatNodes[it]->VisitNode();
					//cout<<_tmpsjFlatNodes[it]->name<<"   "<<_tmpsjFlatNodes[it]->GetVisitTimes()<<endl;
					_tmpflatNodes.push_back(_tmpsjFlatNodes[it]);
					if(!splitFlag)break;					
				}
				//cout<<"+++++++++++++++++++++++++++++++\n";
				splitjoinFlatNodes.push_back(_tmpflatNodes);
			}
		}
	}
	//提取分支中能够做水平融合的节点,向priority2candidateFlatNodes中添加元素，并完成局部稳态调度
	addCandidateFusionFlatNodes(splitjoinFlatNodes);
	//重置节点的访问
	for(int i = 0; i != flatNodesTopoSequence.size(); i++)
		flatNodesTopoSequence[i]->ResetVistTimes();
}

//************************************
// Returns:   void 结果放在priority2candidateFlatNodes中，对于prority2candidateFlatNodes而言节点的优先级越高priority的值越低，在map中越靠前
// Qualifier: 将sjflatNode中的能够融合的节点提出来，插入到partitonNum2FlatNode中——之融合同形分支
//				sj中能够被融合的节点的条件：1.融合节点所在的分支长度要相同，2.融合节点必须紧密相邻
// Parameter: vector<vector<FlatNode *> > 存放的是所有sj中的节点（里面的vector中的元素的第一个节点是split，最后一个节点是join）
//************************************
void HorizontalFusionSSG::addCandidateFusionFlatNodes(vector<vector<FlatNode *> > splitjoinFlatNodes)
{
	priority2candidateFlatNodes.clear();
	for(int si = 0; si != splitjoinFlatNodes.size(); si++)
	{
		vector<FlatNode *> sjFlatNodes = splitjoinFlatNodes[si];
		if(sjFlatNodes.size() <= 2) continue;
		//将各个分支的节点交给DetectHorizontalFusingEligible检测是否符合融合的条件
		//截取单个分支
		vector<vector<FlatNode *> >sjPathFlatNodes;//每一个分支存放splitjoin节点的单个分支
		FlatNode *splitFlatNode = sjFlatNodes[0];//取split节点
		FlatNode *joinFlatNode = sjFlatNodes[sjFlatNodes.size() - 1]; //取splitFlatNode对应的join节点
		vector<FlatNode *> path;
		for(int j = 1; j != sjFlatNodes.size() - 1; j++)
		{
			path.push_back(sjFlatNodes[j]);
			if(sjFlatNodes[j]->outFlatNodes[0] == joinFlatNode)//节点的下端节点是join节点，那么表明已找到一条完整的路径
			{	
				vector<FlatNode *> tmpPath(path);
				path.clear();
				sjPathFlatNodes.push_back(tmpPath);
				//for(int ci = 0; ci != tmpPath.size(); ci++)
					//cout<<tmpPath[ci]->name<<endl;
				//cout<<"++++++++++++++++++++++++++\n";
			}
		}
		if(sjPathFlatNodes.size() != sjFlatNodes[0]->nOut) continue;//验证分支数目是否完全
		assert(sjPathFlatNodes.size() > 0);

		//判断分支是否是同构的
		vector< vector< vector<FlatNode *> > > sameStylePathFlatNodes;//存放不同分支类型的分支的集合
		vector<vector<FlatNode *> >tmpStylePath;//存放只有一种类型的分支的集合
		tmpStylePath.push_back(sjPathFlatNodes[0]);

		for(int i = 1; i != sjPathFlatNodes.size();i++)
		{
			Bool flag = TRUE;
			if(sjPathFlatNodes[i].size() != sjPathFlatNodes[i-1].size())
			{//如果长度不相等则不是同一类型的分支
				vector<vector<FlatNode *> >tmpStyle(tmpStylePath);
				sameStylePathFlatNodes.push_back(tmpStyle);//将tmpStylePath放到sameStylePathFlatNodes中，并清空tmpStylePath，为存放下一个类型的分支做准备
				tmpStylePath.clear();
				tmpStylePath.push_back(sjPathFlatNodes[i]);
				continue;
			}
			//第i与第i-1条分支长度相同
			for(int j = 0; j != sjPathFlatNodes[i].size(); j++)
			{
				if(sjPathFlatNodes[i][j]->contents->ot != sjPathFlatNodes[i-1][j]->contents->ot || sjPathFlatNodes[i][j]->GetVisitTimes() != sjPathFlatNodes[i-1][j]->GetVisitTimes())
				{//对应节点的类型不相等
					vector<vector<FlatNode *> >tmpStyle(tmpStylePath);
					sameStylePathFlatNodes.push_back(tmpStyle);
					tmpStylePath.clear();
					tmpStylePath.push_back(sjPathFlatNodes[i]);
					flag = FALSE;
					break;
				}
			}
			if(!flag) continue;
			tmpStylePath.push_back(sjPathFlatNodes[i]);
		}
		sameStylePathFlatNodes.push_back(tmpStylePath);//做最后比较的收尾

		assert(sameStylePathFlatNodes.size() > 0);
		//根据上面比较的结果来确定可以被融合的节点，满足融合的条件节点加入到priority2candidateFlatNodes中,优先级采用待融合节点的总工作量/带融合节点的数目
		vector<FlatNode *> tmpflatNodes;
		for(int i = 0; i != sameStylePathFlatNodes.size(); i++)
		{//分支的种类数
			if(sameStylePathFlatNodes[i].size() == 1 ) continue;//该类型的分支只有一条，则什么也不做
			for(int j = 0; j != sameStylePathFlatNodes[i][0].size();j++)//处理第i中类型
			{//每中分支实际长度（一条分支上的flatNode数目）
				tmpflatNodes.clear();
				for(int l = 0; l != sameStylePathFlatNodes[i].size();l++)
				{//某一种类型的分支的数目（这种类型分支的宽度）
					Bool fusionCondition = detectHorizontalFusionEligible(sameStylePathFlatNodes[i][l][j]);
					//要比较处于分支同一行的operator的输入输出边的类型是否相同，如果不同则不能融合（不用比较）
					if(fusionCondition)
					{
						tmpflatNodes.push_back(sameStylePathFlatNodes[i][l][j]);
					}
					else 
					{//在处理分支的一个行时在中间出现不可融合的节点(那么就处理前面的可融合的节点)
						if(tmpflatNodes.size() <= 1) continue;//tmpflatNodes只有一个节点	
						int sum = 0;
						for(int k = 0; k != tmpflatNodes.size(); k++)
						{
							sum += hfsssg->GetSteadyWork(tmpflatNodes[k]) * _sjflatNode2smallSteadyCount.find(tmpflatNodes[k])->second;
						}
						int prority = sum/tmpflatNodes.size();
						vector<FlatNode*> tmpVec(tmpflatNodes);
						priority2candidateFlatNodes.insert(make_pair(prority, tmpVec));
						tmpflatNodes.clear();							
					}
				}
				//下面处理的是当该类型分支的一行处理完了（处理的是行尾的收尾）
				if(tmpflatNodes.size() <= 1) continue;//tmpflatNodes只有一个节点	
				int sum = 0;
				for(int k = 0; k != tmpflatNodes.size(); k++)
				{
					sum += hfsssg->GetSteadyWork(tmpflatNodes[k]) * _sjflatNode2smallSteadyCount.find(tmpflatNodes[k])->second;
				}
				int prority = sum/tmpflatNodes.size();
				vector<FlatNode*> tmpVec(tmpflatNodes);
				priority2candidateFlatNodes.insert(make_pair(prority, tmpVec));
			}		
		}
	}	
}

//************************************
// Qualifier: 检测一个filter是否满足融合的条件：是stateless，peek == pop，单入单出。如果满足返回TRUE
// Parameter: FlatNode * flatNode
//************************************
Bool HorizontalFusionSSG::detectHorizontalFusionEligible(FlatNode *flatNode)
{
	assert(flatNode);
	//if(DetectiveFilterState(flatNode)){ cout<<"stateful"<<flatNode->name<<endl;return FALSE;}//是stateful类型的filter
	if(flatNode->nIn != 1 || flatNode->nOut != 1) return FALSE;//splitjoin结点不是单入单出的
	if(flatNode->inPeekWeights[0] > flatNode->inPopWeights[0] ) {return FALSE;}//是peek>pop类型filter
	return TRUE;
}

//************************************
// Qualifier: metis划分的结果存放在mpart中，各个节点的负载存放在vwgt中，该函数的功能是确定划分完成后，伏在最大的划分的负载
//************************************
int HorizontalFusionSSG::findMaxPartitionWeight()
{
	int *partWeight = (int *)malloc(sizeof(int) * partitionNum);
	for(int i = 0; i < partitionNum; i++)
		partWeight[i] = 0;
	for(int i = 0; i < nvtxs; i++)
	{
		partWeight[mpart[i]] += vwgt[i];
	}
	int maxWeight = 0;
	for(int i =0; i != partitionNum; i++)
		if(maxWeight < partWeight[i]) maxWeight = partWeight[i];
	free(partWeight);
	return maxWeight;
}

//************************************
// Qualifier: 根据hfsssg确定metis划分的初始参数，根据拓扑初始化FlatNode2No，flatNodeNewNo2OldNo，flatNodeOldNo2NewNo
//************************************
void HorizontalFusionSSG::initMetisPartitionParamiter(vector<FlatNode *> flatNodesTopoSequence)
{
	map<FlatNode *, int>::iterator iter;
	typedef multimap<int,FlatNode *>::iterator iter1;
	map<FlatNode *, int> flatNode2Workload = hfsssg->GetSteadyWorkMap();
	hfsssg->mapSteadyCount2FlatNode.clear();
	hfsssg->mapSteadyCount2FlatNode = SteadySchedulingGroup(hfsssg->flatNodes);
	//得到初始SDF图节点的拓扑以及节点与编号的对应关系
	int k=0;//k用于记录flagnode的相邻节点数
	for(int i = 0; i != flatNodesTopoSequence.size();i++)
	{//建立flatNode与编号之间的map
		FlatNode2No.insert(make_pair(flatNodesTopoSequence[i],i));
		flatNodeNewNo2OldNo.insert(make_pair(i,i));
		flatNodeOldNo2NewNo.insert(make_pair(i,i));
	}
	for(int i = 0; i != flatNodesTopoSequence.size();i++){
		vsize[i]=0;
		int flag=0;//保证sum只加一次
		int sum=0;
		sum+=xadj[i];
		if (flatNodesTopoSequence[i]->nOut!=0){  
			flag=1;
			xadj[i+1]=sum + flatNodesTopoSequence[i]->nOut;
			for (int j = 0; j < flatNodesTopoSequence[i]->nOut;j++){
				adjncy[k]=FlatNode2No.find(flatNodesTopoSequence[i]->outFlatNodes[j])->second;
				vsize[i] +=flatNodesTopoSequence[i]->outPushWeights[j] * hfsssg->GetSteadyCount(flatNodesTopoSequence[i]);
				k++;
			}
		}
		if (flatNodesTopoSequence[i]->nIn!=0){
			if (flag==0){
				xadj[i+1]=sum+flatNodesTopoSequence[i]->nIn;
			}else{
				xadj[i+1]+=flatNodesTopoSequence[i]->nIn;
			}
			for (int j=0;j<flatNodesTopoSequence[i]->nIn;j++)
			{
				adjncy[k]=FlatNode2No.find(flatNodesTopoSequence[i]->inFlatNodes[j])->second;;
				vsize[i] += flatNodesTopoSequence[i]->inPopWeights[j]*hfsssg->GetSteadyCount(flatNodesTopoSequence[i]);//cout<<"here"<<endl;
				k++;
			}
		}
		iter=flatNode2Workload.find(flatNodesTopoSequence[i]);
		vwgt[i]=hfsssg->GetSteadyCount(flatNodesTopoSequence[i]) * iter->second;
	} 
	//初始化重置节点的访问
	for(int i = 0; i != flatNodesTopoSequence.size(); i++)
		flatNodesTopoSequence[i]->ResetVistTimes();
// 	cout<<"初始化#########################\n"<<endl;
// 	cout<<"vwgt  "<<nvtxs<<endl;;
// 	for(int i =0 ;i <nvtxs;i++)
// 		cout<<vwgt[i]<<"  ";
// 	cout<<endl<<"vsize\n";
// 	for(int i =0 ;i <nvtxs;i++)
// 		cout<<vsize[i]<<"  ";
// 	cout<<endl<<"xadj\n";
// 	for(int i =0 ;i <nvtxs+1;i++)
// 		cout<<xadj[i]<<"  ";
// 	cout<<endl<<"adjncy\n";
// 	for(int i =0 ;i <edgenum;i++)
// 		cout<<adjncy[i]<<"  ";
// 	cout<<endl;
// 	cout<<"*****"<<adjncy[xadj[nvtxs - 1]]<<endl;
}

//************************************
// Qualifier:使用metis划分数据流图
//************************************
void HorizontalFusionSSG::partitionSSG()
{
	if(partitionNum == 1)
	{//如果只有一个划分
		for(int i = 0; i != nvtxs; i++)
			mpart[i] = 0;
		return;
	}
	MetisPartiton *metispart = new MetisPartiton(0,0);
	metispart->metisPartition(nvtxs, 1,xadj,adjncy,vwgt,vsize,NULL,partitionNum,NULL,NULL,NULL,mpart);//划分的结果放在mpart中
}

//************************************
// Qualifier: 从priority2candidateFlatNodes选择优先级高的节点融合,将其中的一条记录按照工作量来分成num份(将待融合节点全部融合在一起，还是有选择的融合几个相邻的节点)
//************************************
void HorizontalFusionSSG::selectFusionFlatNodes(int num)
{//取priority2candidateFlatNodes中的第一条记录来处理
	map<int, vector<FlatNode *> > tmp_curFusingNo2FlatNodes;
	vector<FlatNode *> fusionFlatNodes = priority2candidateFlatNodes.begin()->second;//取要处理的flatNode的结合
	if(fusionFlatNodes.size() <= num) return; //当待融合节点的数目小于等于融合的份数时，不要融合
	curFusingNo2FlatNodes.clear();
	int sumWorkload = 0;
	for(int i = 0; i != fusionFlatNodes.size(); i++)
	{
		int steadyCount = _sjflatNode2smallSteadyCount.find(fusionFlatNodes[i])->second;
		int workLoad = hfsssg->GetSteadyWork(fusionFlatNodes[i]);
		sumWorkload += steadyCount * workLoad;
	}
	//int everageWorkload = sumWorkload / num;
	int curWorkload = 0;
	int fusionNo = fusingNo2FlatNodes.size();
	vector<FlatNode *> curflatNodes;
	//=================================确定有多少个节点能够被融合在一起========================================
	//20121203修改以((float)sumWorkload) / curWorkload作为分配节点的方式
	for(int i = 0; i != fusionFlatNodes.size(); i++)
	{	
		curWorkload +=  _sjflatNode2smallSteadyCount.find(fusionFlatNodes[i])->second * hfsssg->GetSteadyWork(fusionFlatNodes[i]);
		curflatNodes.push_back(fusionFlatNodes[i]);
		if (((float)sumWorkload) / curWorkload > ((float)num)) continue;
		if(((float)sumWorkload) / curWorkload > ((float)num))
		{
			vector<FlatNode *> tmpVec(curflatNodes);
			tmp_curFusingNo2FlatNodes.insert(make_pair(fusionNo++,tmpVec));
			curflatNodes.clear();
			curWorkload = 0;
		}
	}
	if( !curflatNodes.empty())
	{//将flatNodes尾部的节点添加到fusingNo2FlatNodes
		vector<FlatNode *> tmpVec(curflatNodes);
		curFusingNo2FlatNodes.insert(make_pair(fusionNo++,tmpVec));
		curflatNodes.clear();			
	}
	assert(tmp_curFusingNo2FlatNodes.size() <= num);
	for(map<int, vector<FlatNode *> >::iterator iter = tmp_curFusingNo2FlatNodes.begin();iter != tmp_curFusingNo2FlatNodes.end(); iter++)
	{
		curFusingNo2FlatNodes.insert(make_pair(iter->first,iter->second));
	}
}

//************************************
// Qualifier: 将fusingNo2FlatNodes在tmpFusingNo2FlatNodes中的元素删除，同时修改依赖的信息=========>还原一些信息
//************************************
void HorizontalFusionSSG::undoSelectFusionFlatNode()
{
	//还原metis划分中的参数，与updateMetisPartitionParamiter对应
	nvtxs = bak_nvtxs;
	edgenum = bak_edgenum;
	for(int i = 0; i != bak_nvtxs; i++)
	{
		vwgt[i] = bak_vwgt[i];
		vsize[i] = bak_vsize[i];
	}
	for(int i = 0; i <= bak_nvtxs; i++)
		xadj[i] = bak_xadj[i];
	for(int i = 0; i < edgenum; i++)
		adjncy[i] = bak_adjncy[i];
	//还原flatNodeOldNo2NewNo的值
	flatNodeOldNo2NewNo = bak_flatNodeOldNo2NewNo;
	FlatNode2No = bak_FlatNode2No;
	//清空tmpFusingNo2FlatNodes
	curFusingNo2FlatNodes.clear();
	flatNodeNewNo2OldNo.clear();
}

//************************************
// Qualifier:  备份metis划分用到的参数，在updateMetisPartitionParamiter之前使用，目的是为了再一次划分作废时能够还原到划分前的值
//************************************
void HorizontalFusionSSG::backupPartitionInfo()
{
	bak_nvtxs = nvtxs;
	bak_edgenum = edgenum;
	for(int i = 0; i != nvtxs; i++)
	{
		bak_vwgt[i] = vwgt[i];
		bak_vsize[i] = vsize[i];
	}
	for(int i = 0; i <= nvtxs; i++)
		bak_xadj[i] = xadj[i];
	for(int i = 0; i < edgenum; i++)
		bak_adjncy[i] = adjncy[i];
	bak_FlatNode2No = FlatNode2No;
	bak_flatNodeOldNo2NewNo = flatNodeOldNo2NewNo;
}

//************************************
// Method:    updateMetisPartitionParamiter(函数内部数组间的赋值可以用memcpy来实现)
// FullName:  HorizontalFusionSSG::updateMetisPartitionParamiter
// Access:    public 
// Returns:   void
// Qualifier: 根据selectFusionFlatNodes执行的结果确定那些节点将要被融合，在此处修改节点节点间的依赖关系（可以根据tmpFusingNo2FlatNodes来做处理）
//************************************
void HorizontalFusionSSG::updateMetisPartitionParamiter()
{//根据tmpFusingNo2FlatNodes中的信息来做，修改的信息主要有nvtxs，xadj，adjncy，vwgt，vsize
	//如何修改才能让metis能够接受
	int *tmp_vweight = (int *)malloc(sizeof(int) * nvtxs);
	int *tmp_vsize = (int *)malloc(sizeof(int) * nvtxs); 
	int *tmp_xadj = (int *)malloc( sizeof(int)*(nvtxs + 1) );
	int *tmp_adjncy = (int *)malloc(sizeof(int) * edgenum);

	for(map<int, vector<FlatNode *> >::iterator iter = curFusingNo2FlatNodes.begin(); iter != curFusingNo2FlatNodes.end(); iter ++)
	{
		vector<FlatNode *>curflatNodes = iter->second;
		int curFusionFlatNodeCount = curflatNodes.size();
		if(curFusionFlatNodeCount ==1) break;
		//修改flatNodeOldNo2NewNo中的信息
		int smallNo = flatNodeOldNo2NewNo.size();//用于记录待融合节点中编号最小的节点的编号
		//找待融合节点中编号最小的节点
		for(int i = 0; i != curflatNodes.size();i++)
		{
			map<FlatNode *,int>::iterator curIter = FlatNode2No.find(curflatNodes[i]);
			assert(curIter != FlatNode2No.end());
			if(curIter->second < smallNo) smallNo = curIter->second; //找待融合节点中编号最小的节点的编号
		}

		//修改节点编号flatNodeOldNo2NewNo（融合后新得的节点的编号为smallNo）
		for(int i = 0; i != curflatNodes.size();i++)
		{
			map<FlatNode *,int>::iterator curIter = FlatNode2No.find(curflatNodes[i]);
			assert(curIter != FlatNode2No.end());
			map<int,int>::iterator oldnoIter = flatNodeOldNo2NewNo.find(curIter->second);
			assert(oldnoIter != flatNodeOldNo2NewNo.end());
			oldnoIter->second = smallNo;
			curIter->second = smallNo;
		}
		//修改节点的数目nvtxs
		nvtxs = nvtxs - curflatNodes.size() + 1;
		//修改flatNodeNewNo2OldNo中的信息
		multimap<int,int>tmp_flatNodeNewNo2OldNo;//为了构建现编号与旧编号的映射关系，作为中转的
		for(map<int,int>::iterator noIter = flatNodeOldNo2NewNo.begin();noIter != flatNodeOldNo2NewNo.end(); noIter++)
		{
			tmp_flatNodeNewNo2OldNo.insert(make_pair(noIter->second,noIter->first));
		}
		assert(tmp_flatNodeNewNo2OldNo.size()== flatNodeOldNo2NewNo.size());//flatNodeNewNo2OldNo.size()记录的是融合前点的数目
		//新编号要连续（并不是只有融合的节点编号要修改，其他节点的编号也要跟着修改）
		flatNodeNewNo2OldNo.clear();
		multimap<int,int>::iterator begin_tmp_newno2oldno_index = tmp_flatNodeNewNo2OldNo.begin();
		if(begin_tmp_newno2oldno_index->first == 0) flatNodeNewNo2OldNo.insert(make_pair(0,tmp_flatNodeNewNo2OldNo.begin()->second));
		else flatNodeNewNo2OldNo.insert(make_pair(0,tmp_flatNodeNewNo2OldNo.begin()->second));
		begin_tmp_newno2oldno_index++;
		assert(begin_tmp_newno2oldno_index != tmp_flatNodeNewNo2OldNo.end());
		multimap<int,int>::iterator newno2oldno_index = flatNodeNewNo2OldNo.begin();
		for(multimap<int,int>::iterator tmp_index = begin_tmp_newno2oldno_index; tmp_index != tmp_flatNodeNewNo2OldNo.end();tmp_index++ )
		{
			if(tmp_index->first == newno2oldno_index->first) flatNodeNewNo2OldNo.insert(make_pair(newno2oldno_index->first,tmp_index->second));
			else  flatNodeNewNo2OldNo.insert(make_pair(newno2oldno_index->first + 1, tmp_index->second));
			newno2oldno_index++;
		}
		//通过flatNodeNewNo2OldNo构造flatNodeOldNo2NewNo
		flatNodeOldNo2NewNo.clear();
		for(multimap<int,int>::iterator noIter = flatNodeNewNo2OldNo.begin();noIter != flatNodeNewNo2OldNo.end(); noIter++)
		{
			flatNodeOldNo2NewNo.insert(make_pair(noIter->second,noIter->first));
		}
		//+++++++++++++++至此将节点的新旧编号问题修改完成++++++++++++++++++++++

		//根据flatNodeOldNo2NewNo修改xadj，adjncy，vwgt，vsize以及flatNodeNo2SteadyCount，sjflatNode2smallSteadyCount中的信息
		//修改节点的工作量（先将旧的工作量重新确定新节点的工作量）——vwgt，vsize

		for(int i = 0; i != nvtxs; i++)
		{
			tmp_vweight[i] = 0;
			tmp_vsize[i] = 0;
		}
		for (map<int,int>::iterator weihgt_Iter = flatNodeOldNo2NewNo.begin(); weihgt_Iter != flatNodeOldNo2NewNo.end();weihgt_Iter++)
		{
			tmp_vweight[weihgt_Iter->second] += vwgt[weihgt_Iter->first];
			tmp_vsize[weihgt_Iter->second] += vsize[weihgt_Iter->first];
		}
		//更新vwgt,vsize
		for(int i = 0; i != nvtxs; i++)
		{
			vwgt[i] = tmp_vweight[i];
			vsize[i] = tmp_vsize[i];
			mpart[i] = 0;
		}
		for(int i = nvtxs; i < flatNodeOldNo2NewNo.size(); i++)
		{
			vwgt[i] = 0;
			vsize[i] = 0;
			mpart[i] = 0;
		}

		//下面修改xadj，adjncy

		int tmp_edgenum = hfsssg->GetMapEdge2DownFlatNode().size() + hfsssg->GetMapEdge2UpFlatNode().size();

		for(int i = 0; i!= tmp_edgenum;i++)
		{
			tmp_adjncy[i] = 0;
		}
		//可以定义一个map,将所有节点与其相邻的点的关系保存在flatNodeNo2AdjFlatNodeNoSet中
		map<int ,std::set<int> > flatNodeNo2AdjFlatNodeNoSet;

		for (int i = 0;i != nvtxs;i++)
		{
			std::set<int > tmp_adjset;
			std::pair<multimap<int,int>::iterator, multimap<int,int>::iterator>range = flatNodeNewNo2OldNo.equal_range(i);//找一个新编号对应的所有的旧的编号
			for(multimap<int,int>::iterator iter = range.first;iter != range.second;iter++)
			{
				//cout<<"iter"<<iter->second <<endl;
				for(int j = xadj[iter->second]; j != xadj[iter->second + 1]; j++)
				{
					//cout<<xadj[iter->second]<<"-------------"<<xadj[iter->second + 1]<<":    "<< flatNodeOldNo2NewNo.find(adjncy[j])->second<<endl;
					tmp_adjset.insert(flatNodeOldNo2NewNo.find(adjncy[j])->second);
				}
			}
			assert(!tmp_adjset.empty());
			flatNodeNo2AdjFlatNodeNoSet.insert(make_pair(i,tmp_adjset));
		}
		//根据flatNodeNo2AdjFlatNodeNoSet中的信息重新构造xadj，adjncy
		tmp_xadj[0] = 0;
		int k = 0;
		for( int i = 0; i != nvtxs; i++)
		{
			map<int ,std::set<int> >::iterator adj_iter = flatNodeNo2AdjFlatNodeNoSet.find(i);
			assert( adj_iter!= flatNodeNo2AdjFlatNodeNoSet.end());
			tmp_xadj[adj_iter->first + 1] =  tmp_xadj[adj_iter->first] + adj_iter->second.size();
			for(std::set<int >::iterator tmp_iter = adj_iter->second.begin();tmp_iter != adj_iter->second.end(); tmp_iter++)
			{
				tmp_adjncy[k++] = *tmp_iter;
			}
			//tmp_xadj[i + 1] = tmp_xadj[i] + (adj_iter->second).size();
		}
		for (int i =0; i <= nvtxs; i++)
		{
			xadj[i] = tmp_xadj[i];
		}
		//将xadj中多余的数据清0
		for(int i = nvtxs+1; i != flatNodeOldNo2NewNo.size() + 1 ;i++)
			xadj[i] = 0;
		//修改邻接边的关系
		for(int i = 0; i!= tmp_edgenum;i++)
		{
			adjncy[i] = tmp_adjncy[i];
		}

		//根据融合后的结果重新编号
		for(map<FlatNode *,int>::iterator iter_1 = FlatNode2No.begin();iter_1 != FlatNode2No.end(); iter_1++)
		{
			iter_1->second = flatNodeOldNo2NewNo.find(iter_1->second)->second;
		}
		flatNodeOldNo2NewNo.clear();
		for(multimap<int,int>::iterator iter_2 = flatNodeNewNo2OldNo.begin(); iter_2 != flatNodeNewNo2OldNo.end();iter_2++)
		{
			flatNodeOldNo2NewNo.insert(make_pair(iter_2->first,iter_2->first));
		}

		flatNodeNewNo2OldNo.clear();

// 		cout<<"vwgt----------------------------------------"<<nvtxs<<endl;
// 		for(int i =0 ;i <nvtxs;i++)
// 			cout<<vwgt[i]<<"  ";
// 		cout<<endl<<"vsize\n";
// 		for(int i =0 ;i <nvtxs;i++)
// 			cout<<vsize[i]<<"  ";
// 		cout<<endl<<"xadj\n";
// 		for(int i =0 ;i <nvtxs+1;i++)
// 			cout<<xadj[i]<<"  ";
// 		cout<<endl<<"adjncy---------------------------"<<edgenum<<endl;
// 		for(int i =0 ;i <edgenum;i++)
// 			cout<<adjncy[i]<<"  ";
// 		cout<<endl<<endl;
// 		cout<<"*****"<<adjncy[xadj[nvtxs-1]]<<endl;
	}
	free(tmp_vsize);free(tmp_vweight);free(tmp_adjncy);free(tmp_xadj);
}


//************************************
// Method:    selectFusingSSG
// FullName:  HorizontalFusionSSG::selectFusingSSG
// Access:    public 
// Returns:   void
// Qualifier: 对整个SDF图做融合，在选择待融合节点是包裹一个划分算法（本函数使用的是metis划分算法），使得融合后划分的结果有所改善（此处主要是从负载的角度考察）
//			对候选集中的一个候选进行融合采用启发式算法，先将一个候选中的节点全部融合成一个节点，如果融合后的效果不好，则撤销当前的融合，再将候选节点融合成2份
//			再看此时的效果，如果效果不好则继续增加融合的份数。
//************************************
void HorizontalFusionSSG::selectFusingSSG()
{
	//初始划分得到融合前的初始值
	vector<FlatNode *>flatNodesTopoSequence = TopoSortFilter();//FlatNode节点的拓扑排序
	initMetisPartitionParamiter(flatNodesTopoSequence);
	partitionSSG();// 对SDF图做划分
	int initMaxWeight = findMaxPartitionWeight();
	int curMaxWeight = initMaxWeight;
	float changeRatio = THRESHOLD;

	while(1)
	{
		collectFusionFlatNodesInfo(flatNodesTopoSequence);//根据拓扑排序的结果确定能够被融合的flatNode节点的结合
		if (priority2candidateFlatNodes.empty())break;//为的是跳出外层循环（没有可融合的节点）
		int num = 1;//候选集中一个元素的节点集被融合成的数目
		std::multimap<int ,vector<FlatNode *> >::iterator iter = priority2candidateFlatNodes.end();//记录当前处理的一条记录
		while ( !priority2candidateFlatNodes.empty())
		{
			if (priority2candidateFlatNodes.empty()) break;//跳出内层循环（当前能够被融合的节点融合完了）		
			selectFusionFlatNodes(num);//选择将要被融合节点（将当前结果添加到tmpFusingNo2FlatNodes和fusingNo2FlatNodes中）
			backupPartitionInfo();//在融合前做备份，方便撤销融合
			updateMetisPartitionParamiter();//根据选择融合的节点情况更新划分要的参数((做了一个伪融合)更新的是图的结构，它是在metis的参数中体现的)
			partitionSSG();//对图重新划分
			curMaxWeight = findMaxPartitionWeight();//查找当前划分中权值最大的
			changeRatio = ((float)(curMaxWeight - initMaxWeight)/initMaxWeight);//计算融合对划分的影响
			//cout<<"init= "<<initMaxWeight<<"  cur= "<<curMaxWeight<<"  融合的改进"<<changeRatio<<endl;
			if (changeRatio >= THRESHOLD) //如果当前划分的效果更差则不融合
			{//融合的结果不好，撤销，调整融合的粒度，直到不能被融合，将将从候选中删除
				undoSelectFusionFlatNode(); //融合后结果不好，撤销当前的操作（根据tmpFusingNo2FlatNodes中的结果撤销）
				num++;//融合最终的份数增加
				//cout<<"fission....."<<num<<"  "<<priority2candidateFlatNodes.begin()->second.size()<<endl;
				if(num >= priority2candidateFlatNodes.begin()->second.size())//不能被融合
					priority2candidateFlatNodes.erase(priority2candidateFlatNodes.begin());//删除不能被处理的记录
			}
			else 
			{
				num = 1;
				for(map<int, vector<FlatNode *> >::iterator iter_0 = curFusingNo2FlatNodes.begin();iter_0 != curFusingNo2FlatNodes.end();iter_0++)
					fusingNo2FlatNodes.push_back(iter_0->second);
				priority2candidateFlatNodes.erase(priority2candidateFlatNodes.begin());	
			}
		}
		if(fusingNo2FlatNodes.empty()) break;//最终没有节点要被融合
		//做实际的融合
		for (vector<vector<FlatNode *> >::iterator iter_3 = fusingNo2FlatNodes.begin(); iter_3 != fusingNo2FlatNodes.end(); iter_3++)
			fusingFlatNodes(*iter_3);
		//reset一些变量
		flatNodesTopoSequence.clear();
		FlatNode2No.clear();
		fusingNo2FlatNodes.clear();
		_sjflatNode2smallSteadyCount.clear();
		flatNodesTopoSequence.clear();
		priority2candidateFlatNodes.clear();
		nvtxs = hfsssg->flatNodes.size();
		flatNodeOldNo2NewNo.clear();
		flatNodeNewNo2OldNo.clear();
		flatNodesTopoSequence = TopoSortFilter();//根据融合后的图做拓扑
		initMetisPartitionParamiter(flatNodesTopoSequence);
	}	
}

GLOBAL SchedulerSSG *HorizontalFusionTransform(SchedulerSSG *sssg, int clusterNum )
{
	HorizontalFusionSSG *HFSSG = new HorizontalFusionSSG(sssg,clusterNum);
	HFSSG->selectFusingSSG();
	HFSSG->hfsssg->mapSteadyCount2FlatNode.clear();
	HFSSG->hfsssg->mapInitCount2FlatNode.clear();
	HFSSG->hfsssg->ResetFlatNodeVisitTimes();
	HFSSG->hfsssg->mapSteadyCount2FlatNode = SteadySchedulingGroup(HFSSG->hfsssg->flatNodes);//对融合好的图重新进行稳态调度
	HFSSG->hfsssg->InitScheduling();//对融合好的图重新进行初态调度
	return HFSSG->hfsssg;
}