#include "HorizontalFusionSSG.h"
#include "MetisPartiton.h"
#include "ActorStateDetector.h"
#include <set>
#include "rename.h"


#define THRESHOLD 0.05

PRIVATE int HFStreamNum = 0;//����¹����stream��������
PRIVATE int HFOperatorNum = 0;//����¹����operator��������
PRIVATE int HFVarNum = 0;



//************************************
// Qualifier: ���캯��
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
// Qualifier: ��������
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
// Qualifier: ��SDFͼ����������
//************************************
vector<FlatNode *> HorizontalFusionSSG::TopoSortFilter()
{
	vector<FlatNode *>original = hfsssg->GetFlatNodes();
	vector<FlatNode *> 	actorTopoOrder;
	vector<int> nInDegree;//���ڱ�����ڵ�����
	vector<FlatNode *> flatNodeStack;
	int nsize=original.size();
	for (int i = 0;i != nsize;++i)
	{
		nInDegree.push_back(original[i]->nIn);//�����ڵ���ȱ���
	}
	for (int i = 0; i != nInDegree.size();i++)
	{
		if(!nInDegree[i]) flatNodeStack.push_back(original[i]);
	}
	while (!flatNodeStack.empty())
	{
		FlatNode *tmpFlatNode = flatNodeStack.back();// ȡ��Ҫ��ջ�Ľڵ�
		actorTopoOrder.push_back(tmpFlatNode);
		flatNodeStack.pop_back();
		for(int i= 0; i != tmpFlatNode->nOut;i++)
		{
			for (int j =0;j != original.size(); j++)
			{
				if(original[j] == tmpFlatNode->outFlatNodes[i])
				{
					if(!(--nInDegree[j])) flatNodeStack.push_back(original[j]);//���Ϊ0���ջ
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
	/*���봰��*/
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
		windowList = AppendItem(windowList,MakeWindowNode(inputId,inputDecl,pop_value,0));	//����ߵĴ���
	}

	/*�������*/
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
	windowList =AppendItem(windowList,MakeWindowNode(outputId,outputDecl,pushArg,1));   //���
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
	/*�������*/
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
		windowList = AppendItem(windowList,MakeWindowNode(outputId,outputDecl,push_value,1));	//����ߵĴ���
	}
	/*���봰��*/
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
	/*�������*/
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
		windowList = AppendItem(windowList,MakeWindowNode(outputId,outputDecl,pop_value,1));	//����ߵĴ���
	}

	/*���봰��*/
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
{//����join�ڵ�
	char *operatorName = (char *)malloc(60);
	sprintf(operatorName,"%s_%s_%d","HFusion","join",HFOperatorNum++);
	//�µ�filter�������List
	assert(joinOutputNode->typ== Decl || joinOutputNode->typ== Id);
	Node *joinOutputDecl = NULL;
	if(joinOutputNode->typ== Decl) joinOutputDecl = joinOutputNode;
	else joinOutputDecl = joinOutputNode->u.id.decl;
	
	Node *tmp_outputNode = MakeNewStreamId(joinOutputNode->u.decl.name,joinOutputNode);
	List *outputList = NULL;
	outputList = AppendItem(outputList,tmp_outputNode); 

	gIsUnfold=TRUE;
	Node *newOperatorHead = MakeOperatorHead(operatorName,outputList,joinInputList);//�½ڵ��ͷ
	Node *newOperator = DefineOperator(newOperatorHead);
	newOperator->u.operator_.ot = Join_;
	Node *newOperatorWork = MakeJoinWork(tmp_outputNode,joinInputList,joinPopArg);
	//�µ�filter����window
	List *newOperatorWindow = makeHFusionJoinWindow(joinInputList,outputList,joinPopArg,joinPushArg);
	Node *newOperatorBody = MakeOperatorBody(newOperatorWork,newOperatorWindow);
	newOperator = SetOperatorBody(newOperator,newOperatorBody);//newoperator�������
	gIsUnfold=FALSE;
	return newOperator;
}

Node *HorizontalFusionSSG::makeHFusionSplit(Node *splitInputNode,Node *splitPopArg,List *splitOutputList, List *splitPushArg, SplitStyle style)
{//����split�ڵ�(Ҫȷ�����õ�������split�ķ�ʽ����duplicate��roundrobin)
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
		Node *rouOperHead = MakeOperatorHead(operatorName,splitOutputList,tmp_inputList); //operator��ͷ
		newOperator = DefineOperator(rouOperHead);
		newOperator->u.operator_.ot = Roundrobin_;
		Node *rouWorkNode = MakeRoundrobinWork(splitOutputList,tmp_inputNode,splitPushArg);//����operator��work����
		List *rouWindowList = makeHFusionRoundRobinWindow(tmp_inputList,splitOutputList,splitPushArg,splitPopArg);
		Node *rouOperBodyNode = MakeOperatorBody(rouWorkNode,rouWindowList);//operator��body
		newOperator = SetOperatorBody(newOperator,rouOperBodyNode);//����operator

	}
	else 
	{//style == S_Duplicate
		sprintf(operatorName,"%s_%s_%d","HFusion","Duplicate",HFOperatorNum++);
		Node *rouOperHead = MakeOperatorHead(operatorName,splitOutputList,tmp_inputList); //operator��ͷ
		newOperator = DefineOperator(rouOperHead);
		newOperator->u.operator_.ot = Duplicate_;
		Node *rouWorkNode = MakeDuplicateWork(splitOutputList,splitInputNode,splitPopArg);//����operator��work����
		List *rouWindowList = makeHFusionDuplicateWindow(tmp_inputList,splitOutputList,splitPopArg);
		Node *rouOperBodyNode = MakeOperatorBody(rouWorkNode,rouWindowList);//operator��body
		newOperator = SetOperatorBody(newOperator,rouOperBodyNode);//����operator
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
		//���崦��
		if(n->u.array.name->u.id.decl->u.decl.type->typ == STRdcl)//�������������һ��stream
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
	//����ԭ���Ĵ�����û�У����ڰ����µ�ֵ���¹���	 
	assert(inputNode && inputNode->typ == Decl);
	assert(outputNode &&inputNode->typ == Decl);
	List *finalWinList = NULL;//���ڷ�������޸ĺ��window
	//���봰��Ϊ�գ���һ������window
	List *outputs = AppendItem(AppendItem(outputs,MakeConstSint(peekValue)),MakeConstSint(popValue));
	Node *output = MakeCommaCoord(outputs,UnknownCoord);
	Node *sliding = MakeWindowSlidingCoord(EMPTY_TQ,output,UnknownCoord);
	Node *input_winIdNode = MakeNewStreamId(inputNode->u.decl.name,inputNode);
	Node *input_Window = MakeWindowCoord(input_winIdNode,sliding,UnknownCoord); //���봰��

	Node *tumbling = MakeWindowTumbingCoord(EMPTY_TQ, MakeConstSint(pushValue), UnknownCoord);  //push
	Node *output_winIdNode = MakeNewStreamId(outputNode->u.decl.name,outputNode);
	Node *output_Window = MakeWindowCoord(output_winIdNode,tumbling,UnknownCoord);//����ߵĴ���
	finalWinList = AppendItem(finalWinList,input_Window);

	finalWinList = AppendItem(finalWinList,output_Window);

	return finalWinList;

}

//************************************
// Qualifier: �޸���ͨ��operator�����ݣ���������ߣ��Լ�������С�ȶ��ĵ���ѭ�����޸����ݷ��ʵ�ƫ��ֵ��
// Parameter: operatorNode * operNode ����Ҫ�޸ĵ�operator(���뵥����)
// Parameter: int popvalue	����ԭoperator��popֵ
// Parameter: int pushvalue����ԭoperator��pushֵ
// Parameter: Node * newInputStream  �����޸ĺ���µ�operator�������
// Parameter: Node * newOutputStream �����޸ĺ���µ�operator�������
// Parameter: int popOffset	��������������ȡ����ƫ����
// Parameter: int pushOffset�������������ȡ����ƫ����
// Parameter: int steadyCount ������С��̬��ִ�д��������ڹ����µ�operator��������ѭ���������operator��work���ԣ�
// Parameter: int index������Ҫ���ڹ������ѭ����ѭ���������������� 
//************************************
void HorizontalFusionSSG::commonOperatorTransform(operatorNode *operNode,int popvalue, int pushvalue, Node *newInputStream,Node *newOutputStream,int popOffset,int pushOffset,int steadyCount,int index)
{
	//1.ȡԭ��operator����������ߵ�decl
	List *inputList = operNode->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = operNode->decl->u.decl.type->u.operdcl.outputs;
	assert(ListLength(inputList) == ListLength(outputList) == 1);
	Node *inputNode = (Node *)FirstItem(inputList);
	Node *outputNode = (Node *)FirstItem(outputList);
	Node *inputDecl = NULL;
	Node *outputDecl = NULL;
		//ȡ����������Ķ���
	if(inputNode->typ == Id) inputDecl =  inputNode->u.id.decl;
	else inputDecl = inputNode;
	if(outputNode->typ == Id)outputDecl = outputNode->u.id.decl;
	else outputDecl = outputNode;
	//2.��������ѭ����forѭ��,ѭ����ѭ�������޸ĺ͵�operator��work������
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

	//3.�޸�operator������operator��decl���Լ�work��
	List *newInputList = NULL;
	List *newOutputList = NULL;
	assert(newOutputStream->typ == Decl && newInputStream->typ == Decl);
	Node *newInputId = MakeNewStreamId(newInputStream->u.decl.name,newInputStream);
	Node *newOutputId = MakeNewStreamId(newOutputStream->u.decl.name,newOutputStream);
	newInputList = AppendItem(newInputList,newInputId);
	newOutputList = AppendItem(newOutputList,newOutputId);
		//�޸�operator����������ߣ�operator��decl�У�
	operNode->decl->u.decl.type->u.operdcl.inputs = newInputList;
	operNode->decl->u.decl.type->u.operdcl.outputs = newOutputList;
	assert(ListLength(operNode->decl->u.decl.type->u.operdcl.inputs) == ListLength(operNode->decl->u.decl.type->u.operdcl.outputs) == 1);
		//�޸�operator��work�����е��������
	Node *workNode = operNode->body->u.operBody.work;
	assert(workNode);
	MWIOS_astwalk(workNode,inputDecl,outputDecl,newInputStream,newOutputStream,forVarId,pushvalue,popvalue,pushOffset,popOffset);
		//��work��Ϊforѭ����stmt
	Node *tmpForNode = MakeForCoord(init,cond,next,workNode,UnknownCoord);
	stmts = AppendItem(stmts, tmpForNode);//��work�����
	Node *newWorkNode = MakeBlockCoord(PrimVoid, decls, stmts, UnknownCoord, UnknownCoord);//�����µ�work����
		//�޸�operator��work
	operNode->body->u.operBody.work = newWorkNode;
	//4.�޸�operator��window���������¹����µ�window���Ǿɵ�window��
	operNode->body->u.operBody.window = makeHFusionSInSOutWindow(newInputStream,newOutputStream,popvalue*steadyCount,popvalue*steadyCount,pushvalue*steadyCount);
}

Node *HorizontalFusionSSG::fusingOperators(vector<operatorNode *>operNodes,int popvalue,int pushvalue)
{//���ù����µ�operator��ֻҪ��operNodes������operator��init��work�Լ�state�����һ�𣬽������ַŵ�operNodes�ĵ�һ��Ԫ������Ϊ���յķ���ֵ
	assert(operNodes.size() >1 );
	List *stateList =NULL;
	List *initStmtsList = NULL;
	List *workStmtsList = NULL;
	for(int i = 0; i != operNodes.size();i++)
	{
		//1.����operator��stateƴ�ӵ�һ��
		if(operNodes[i]->body->u.operBody.state)stateList= JoinLists(stateList, operNodes[i]->body->u.operBody.state);
		//2.����operator��initƴ�ӵ�һ��
		if(operNodes[i]->body->u.operBody.init)initStmtsList = AppendItem(initStmtsList,operNodes[i]->body->u.operBody.init);
		//3.����operator��workƴ�ӵ�һ��
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
	//ȡ����������Ķ���
	if(inputNode->typ == Id) inputDecl =  inputNode->u.id.decl;
	else inputDecl = inputNode;
	if(outputNode->typ == Id)outputDecl = outputNode->u.id.decl;
	else outputDecl = outputNode;
	tmpOper->body->u.operBody.window = makeHFusionSInSOutWindow(inputDecl,outputDecl,popvalue,popvalue,pushvalue);
	
	char *operatorName = (char *)malloc(60);
	sprintf(operatorName,"%s_%s_%d","HFusion",tmpOper->decl->u.decl.name,HFOperatorNum++);
	tmpOper->decl->u.decl.name = operatorName;
	Node *finalOperatorNode = MakeOperator(tmpOper->decl,tmpOper->body);//finalOperatorNode ��Ϊ���յķ���
	return finalOperatorNode;
}

//************************************
// Returns:   Node *�����ص����ںϺ��γɵ��µĽڵ㣩
// Qualifier: ��flatNode�е����е�operator�ڵ��ںϳ�һ���ڵ㣬inputNode��outputNode���ںϺ�ڵ�����������
// Parameter: vector<FlatNode * >flatNodes
// Parameter: Node * inputNode
// Parameter: Node * outputNode
//************************************
Node *HorizontalFusionSSG::fusingNodes(vector<FlatNode *>flatNodes,Node *inputNode,Node *outputNode)
{//��ÿһ��operator������Ҫ��һ��ѭ����ѭ���Ĵ�������С������������operator�еı���������,�޸�operator����������������Լ����ݷ��ʵ�ƫ��
	assert(inputNode->typ == Decl && outputNode->typ == Decl);
	vector<operatorNode *>fusionOperNodes;//��������������flatNode��contents�����ڸ�vector��
	int popOffset = 0;
	int pushOffset = 0;
	for(int i = flatNodes.size()-1; i >= 0; i--)
	{//����ͼ�нڵ���������
		//1.������operator�еĸ�������
		operatorNode *tmpOperNode = flatNodes[i]->contents;
		//2.�޸�����������Լ�����ƫ�Ƶ�λ�ò���work���������һ����С�ȶ�״̬��ѭ��
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
		//3.���޸ĺ��operator��ӵ�fusionOperNodes
		fusionOperNodes.push_back(tmpOperNode);
	}
	//��fusionOperNodes�е�����operator�ںϳ�һ��operator
	Node *finalOperatorNode =  fusingOperators(fusionOperNodes,popOffset,pushOffset);
	assert(finalOperatorNode);
	return finalOperatorNode;
}

void HorizontalFusionSSG::InsertExternalOutputStreamToSSG(FlatNode *flatNode)
{
	ListMarker marker;
	Node *streamNode = NULL;
	Node *stream_type = NULL;
	IterateList(&marker, flatNode->contents->decl->u.decl.type->u.operdcl.outputs ); //u�������
	while (NextOnList(&marker, (GenericREF) &streamNode)) 
	{
		if (streamNode->typ == Id)   //ȡ��������
			stream_type = streamNode->u.id.decl->u.decl.type;
		else 
			stream_type = streamNode->u.decl.type;
		/*�������Ѿ���mapEdge2DownFlatNode�д�����*/
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
// Returns:   vector<FlatNode *> �������ص����µ�operatorת���ɵ�FlatNode�Ľ��
// Qualifier: ��ssg����newFlatNodes����oldFlatNodes,(oldflatNodes��newflatNodes��һ����ͼ��������ssg������ͬ���϶˺��¶˽ڵ�)
// Parameter: vector<FlatNode * >oldFlatNodes��Ҫ��ssg��ɾ����flatNode�ļ��ϣ�
// Parameter: vector<operatorNode * >newNodes �ڸýṹ�����нڵ������ҽ���һ���ڵ�����벻��newNodes,���ҽ���һ���ڵ���������newNodes��(��newNode�еĽڵ㹹���FlatNode�������뵽ssg�У�
//************************************
vector<FlatNode *> HorizontalFusionSSG::replaceHfusionFlatNodes(vector<FlatNode *>oldFlatNodes,vector<operatorNode *>newNodes)
{
	assert(newNodes.size() > 0);
	vector<FlatNode *>fusedResult;
	//ɾ���ɵ���Ϣ
	for(int i = 0; i!= oldFlatNodes.size(); i++)
	{
		//ɾ��flatNodes�е�����
		vector<FlatNode *>::iterator erase_iter = hfsssg->flatNodes.end();
		for(vector<FlatNode *>::iterator iter = hfsssg->flatNodes.begin(); iter != hfsssg->flatNodes.end(); iter++)
		{
			if (oldFlatNodes[i] == *iter) { erase_iter = iter; break;}
		}
		assert(erase_iter != hfsssg->flatNodes.end());
		hfsssg->flatNodes.erase(erase_iter);
		//ɾ��mapEdge2UpFlatNode�е�����
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
		//ɾ��mapEdge2DownFlatNode�е�Ԫ��
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
		//ɾ���ڹ����������е�ֵ
		hfsssg->mapInitWork2FlatNode.erase(oldFlatNodes[i]);
		hfsssg->mapSteadyWork2FlatNode.erase(oldFlatNodes[i]);
	}
	//����µ���Ϣ
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

		/*Ҫ������߲���newNodes�еĽڵ������߲����flatNode�Լ���Ӧ�Ľṹ��*/
		//InsertExternalOutputStreamToSSG(lastFlatNode);

		hfsssg->SetFlatNodesWeights(firstFlatNode);
		hfsssg->SetFlatNodesWeights(lastFlatNode);

	}
	else
	{
		FlatNode *firstFlatNode = hfsssg->InsertFlatNodes(newNodes[0],NULL,NULL,oldFlatNodeSet);
		fusedResult.push_back(firstFlatNode);
		//��Node *�����FlatNode*�ڵ�ͬʱ���뵽hfssg��
		vector<FlatNode *>tmpNewFlatNode;
		for(int i = 1; i != newNodes.size()-1; i++)
		{
			FlatNode * tmpflatNode = hfsssg->InsertFlatNodes(newNodes[i],NULL,	NULL,oldFlatNodeSet);
			//stringstream newName;
			//newName<<"HFusion_"<<tmpflatNode->name<<"_"<<HFOperatorNum++;
			//tmpflatNode->name = newName.str();
			tmpNewFlatNode.push_back(tmpflatNode);
			fusedResult.push_back(tmpflatNode);//���½ڵ�ת����FlatNode�����뵽ssg��	
		}
		FlatNode *lastFlatNode = hfsssg->InsertFlatNodes(newNodes[newNodes.size()-1],NULL,NULL,oldFlatNodeSet);
		fusedResult.push_back(lastFlatNode);

		/*Ҫ������߲���newNodes�еĽڵ������߲����flatNode�Լ���Ӧ�Ľṹ��*/
		//InsertExternalOutputStreamToSSG(lastFlatNode);

		hfsssg->SetFlatNodesWeights(firstFlatNode);
		for(int i = 0; i != tmpNewFlatNode.size(); i++)
		{
			hfsssg->SetFlatNodesWeights(tmpNewFlatNode[i]);
		}
		hfsssg->SetFlatNodesWeights(lastFlatNode);
	}
	//���µĽ�������������
	for(int i = 0; i != fusedResult.size(); i++)
	{
		hfsssg->AddSteadyWork(fusedResult[i], workEstimate(fusedResult[i]->contents->body, 0));
		hfsssg->AddInitWork(fusedResult[i], workEstimate_init(fusedResult[i]->contents->body, 0));
		_sjflatNode2smallSteadyCount.insert(make_pair(fusedResult[i],1));
	}
	return fusedResult;
}

//************************************
// Qualifier: ���ںϽڵ���϶˲���split���¶˲���join,�����ںϽڵ��ںϳ�һ���ڵ�ͬʱ���϶˹���һ��join�����¶˹���һ��split�ڵ�
// Parameter: vector<FlatNode * > flatNodes
//************************************
void HorizontalFusionSSG::fusingFlatNodesCommon(vector<FlatNode *> flatNodes)
{//��operator�еı������������޸�operator����������ߣ��������������ȡ���ݵ�λ�õ�ƫ�ƣ��϶˹���join�¶˹���split����
	//0.�����ںϺ��½ڵ����������ߣ���һ�����������ںϺ��½ڵ���Ե��������
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
	
	//1.�ȹ���join�ڵ�(�����¹����������Լ�flatNodes�����нڵ������߹���join�ڵ�)
	List *joinInputList = NULL;//flatNodes�еĽڵ������
	List *joinPopArg = NULL;
	int joinPushValue = 0;
	for(int i = flatNodes.size() - 1; i >= 0;i--)//Ҫע��ߵ����˳��
	{
		assert( ListLength(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.inputs) == 1 );
		joinInputList = AppendItem(joinInputList, (Node *)FirstItem(flatNodes[i]->contents->decl->u.decl.type->u.operdcl.inputs));
		int popValue = _sjflatNode2smallSteadyCount.find(flatNodes[i])->second * flatNodes[i]->inPopWeights[0];
		joinPopArg = AppendItem(joinPopArg, MakeConstSint(popValue));
		joinPushValue += popValue;
	}
	Node *joinPushArg = MakeConstSint(joinPushValue);
	Node *jNode = makeHFusionJoin(joinInputList,joinPopArg,inputNewStream,joinPushArg);
	
	//2.����split�ڵ�(�����¹����������Լ�flatNodes�����нڵ������߹���join�ڵ�)
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
	Node *sNode = makeHFusionSplit(outputNewStream,splitPopArg,splitOutputList,splitPushArg, S_RoundRobin);//�˴������splitӦ����roundrobin��ʽ

	//3.�������ںϡ�����ÿһ��operator������Ҫ��һ��ѭ����ѭ���Ĵ�������С������������operator�еı���������,�޸�operator����������������Լ����ݷ��ʵ�ƫ��
	Node *fusedOperatorNode = fusingNodes(flatNodes,inputNewStream,outputNewStream);

	//4.�޸�SSG����Ϣ����ɾ���ںϵ���operator��flatNode�ڵ㣬ͬʱ��������ӵ�operator(sNode,jNode)
	vector<operatorNode *> newOperatorNodes;//�����������������е�
	newOperatorNodes.push_back(&(jNode->u.operator_));
	newOperatorNodes.push_back(&(fusedOperatorNode->u.operator_));
	newOperatorNodes.push_back(&(sNode->u.operator_));
	vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(flatNodes, newOperatorNodes);
}

//************************************
// Qualifier: /*����flatNodes��join�Ĺ�ϵ��������flatNodes�ںϵõ���operNode������ߵ����ݽ�����������work��������Ӹù��̣� */
// Parameter: Node * operNode ������flatNodes�ڵ��ںϵõ����µ�operNode
// Parameter: FlatNode * joinFlatNode����flatNodes���¶˵�join�ڵ�
// Parameter: vector<FlatNode * > flatNodes
//************************************
Node *HorizontalFusionSSG::reorderFusionJoinNode(Node *operNode,FlatNode *joinFlatNode,vector<FlatNode *> flatNodes)
{
	//1��ȡjoin��flatNodes�еĸ��������ݵ�popֵ
	int begin_index = joinFlatNode->inFlatNodes.size();//��¼flatNodes�ĵ�һ���ڵ���join��֧��λ��
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
	int joinSteadycount = _sjflatNode2smallSteadyCount.find(joinFlatNode)->second;//join�ڵ�����С��̬��ִ�д���
	int pushNum = push_value * joinSteadycount;//��С�ȶ�״ִ̬������������������
	//ȡoperNode������ߵ������͵ĳ�Ա�����б�
	Node *outputNode = (Node *)FirstItem(operNode->u.operator_.decl->u.decl.type->u.operdcl.outputs);
	Node *outputDecl = NULL;
	if(outputNode->typ == Id) outputDecl = outputNode->u.id.decl;
	else outputDecl = outputNode;
	List *stream_fields = outputDecl->u.decl.type->u.strdcl.type->fields;
	//������������Ҫ���м���������ͣ�struct��
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
	Node *bufferDecl = MakeDeclCoord(buffername,T_BLOCK_DECL,bufferAdcl,NULL,NULL,UnknownCoord);//���ṹ�幹����ɣ�bufferDecl����Ҫ����ӵ�work������
	
	//���� ��operNode��������е�������ʱ���浽buffer�� ����䡪����Ҫ����ӵ�work��
		//����һ��ѭ��
	char * _iname = (char *) malloc(8);
	sprintf(_iname, "%s_%d", "__i",HFVarNum);
	Node *idI = MakeIdCoord(_iname, UnknownCoord);
	Node *declI = MakeNewDecl(_iname, PrimSint, MakeConstSint(0), Redecl);//����ѭ�������Ķ��塪��Ҫ�ӵ�work��
	idI->u.id.decl = declI;
	Node *init = NULL, *cond = NULL, *next = NULL;
	init = MakeBinopCoord('=', idI, MakeConstSint(0), UnknownCoord);
	cond = MakeBinopCoord('<', idI, MakeConstSint(pushNum), UnknownCoord);
	next = MakeUnaryCoord(PREINC, idI, UnknownCoord);
	//���湹�츳ֵ���	
	Node *outputId = MakeNewStreamId(outputDecl->u.decl.name,outputDecl);//�����������Ӧ��idNode
	Node *bufferId = MakeIdCoord(buffername,UnknownCoord);//���컺������Ӧ��idnode
	bufferId->u.id.decl = bufferDecl;
	Node *bufferArray = ExtendArray(bufferId,idI,UnknownCoord);
	Node *outputArray = ExtendArray(outputId,idI,UnknownCoord);
	List *assignmentList_1 = NULL;//��ֵ���list

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
	Node *initBufferNode = MakeForCoord(init,cond,next,for_stmt,UnknownCoord);//�����ʼ��buffer����䡪��Ҫ�ӵ�work��
	//����buffer��Ϊ��תվ��������е�����������
	char * _jname = (char *) malloc(8);
	sprintf(_jname, "%s_%d", "__j",HFVarNum);
	char * _kname = (char *) malloc(8);
	sprintf(_kname, "%s_%d", "__k",HFVarNum);
	Node *idJ = MakeIdCoord(_jname, UnknownCoord);
	Node *idK = MakeIdCoord(_kname, UnknownCoord);
	Node *declJ = MakeNewDecl(_jname, PrimSint, MakeConstSint(0), Redecl);// �ӵ�work��
	Node *declK = MakeNewDecl(_kname, PrimSint, MakeConstSint(0), Redecl);//�ӵ�work��
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
		List *assignmentList_2 = NULL;//��ֵ���list

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
	Node *reorderBufferNode = MakeForCoord(initI,condI,nextI,externForBody,UnknownCoord);//������������ѭ���ڵ㹹����ɡ���Ҫ�ӵ�work������
	//������Ľڵ���ӵ�work��
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,declI);
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,declJ);
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,declK);
	operNode->u.operator_.body->u.operBody.work->u.Block.decl = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.decl,bufferDecl);
	operNode->u.operator_.body->u.operBody.work->u.Block.stmts = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.stmts,initBufferNode);
	operNode->u.operator_.body->u.operBody.work->u.Block.stmts = AppendItem(operNode->u.operator_.body->u.operBody.work->u.Block.stmts,reorderBufferNode);
	return operNode;
}

//************************************
// Qualifier: ���joinflatNode���϶˵ķ�֧����flatNodes�еĽڵ���Ŀ��ͬ�ںϳ�һ���µĽڵ㣬ͬʱ��flatNodes���϶˻�Ҫ�����һ��������join�Ľڵ�
//				���joinflatNode���϶˵ķ�֧������flatNode����flatNode�ڵ��ں���һ��ͬʱ�����µ�join�ڵ����joinflatNode��������flatNodes���϶˻�Ҫ�����һ��������join�Ľڵ�
// Parameter: FlatNode * joinflatNode 
// Parameter: vector<FlatNode * > flatNodes join�϶˵Ľڵ㣨���ںϵĽڵ㼯��
//************************************
void HorizontalFusionSSG::fusingFlatNodesJoin(FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{
	int nbranch = joinflatNode->nIn;

	operatorNode *curOperator = flatNodes[0]->contents;
		//�ȹ�������� �������ںϺ�ĵĽڵ����
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

	//1.�ȹ����µ�join�ڵ㣨����flatNodes�еĽڵ���϶ˣ�
	List *joinInputList = NULL;//flatNodes�еĽڵ������
	List *joinPopArg = NULL;
	int newjoinPushValue = 0;//�½���join�ڵ��pushֵ
	
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
	
	// ���ڴ��ںϽڵ����Ŀ�ͷ�֧��Ŀ�Ĳ�ͬ���¶˵�join�ڵ���в�ͬ�Ĵ���
	if(nbranch == flatNodes.size())
	{//��join�Լ�flatNodes�ںϳ�һ���µ�operator�ڵ㣨�Ƚ�flatNodes�ڵ��ںϳ�һ���µ�operatorȻ�����µ�operator��Ȼ���ٶ�����ߵ����ݽ���������Ϊ�˽�flatNodes�¶˵Ľڵ�join�ڵ㴦�������������Ҫ�����ڲ���������ֱ�Ӷ����뻺�������в�����
		//��joinflatNode�ڵ��ںϵ�������������fusedOperatorNode��work��������Ӷ�������е����ݽ���������
		
		//2.�ں�flatNodes�еĽڵ�
		Node *joinOutputNode = (Node *)FirstItem(joinflatNode->contents->decl->u.decl.type->u.operdcl.outputs);
		Node *joinOutputStream = NULL; 
		if(joinOutputNode->typ == Id) joinOutputStream = joinOutputNode->u.id.decl;
		else joinOutputStream = joinOutputNode;
		//2.1 �ں�flatNodes������ÿһ��operator������Ҫ��һ��ѭ����ѭ���Ĵ�������С������������operator�еı���������,�޸�operator����������������Լ����ݷ��ʵ�ƫ��
		Node *fusedOperatorNode = fusingNodes(flatNodes,inputNewStream,joinOutputStream);
		
		Node *newOperatorNode = reorderFusionJoinNode(fusedOperatorNode,joinflatNode,flatNodes);
		vector<operatorNode *> newOperatorNodeVec;
		newOperatorNodeVec.push_back(&(jNode->u.operator_));
		newOperatorNodeVec.push_back(&(newOperatorNode->u.operator_));
		vector<FlatNode *>oldFlatNodes(flatNodes);
		oldFlatNodes.push_back(joinflatNode);
		vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//����sssg��							
	}
	else if(nbranch < flatNodes.size())
	{
		
		//int oldjoinSteadyCount = sjflatNode2smallSteadyCount.find(joinflatNode)->second;
		//���¹����¶˵�join�ڵ�
		//�����ںϵĽ�����ռ��µĹ����join�ڵ������������Լ�push��pop����Ϣ
		List *oldJoinIntputList = joinflatNode->contents->decl->u.decl.type->u.operdcl.inputs;
		//��Join���µ�����������element
		List *newJoinInputList = NULL;
		List *newJoinPopArg = NULL;//����Node *��List��
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

		//2.�ں�flatNodes�еĽڵ�
		//2.1 �ں�flatNodes������ÿһ��operator������Ҫ��һ��ѭ����ѭ���Ĵ�������С������������operator�еı���������,�޸�operator����������������Լ����ݷ��ʵ�ƫ��
		Node *fusedOperatorNode = fusingNodes(flatNodes,inputNewStream,fusedOutputId->u.id.decl);

		//���ںϺ�Ľڵ���������������ݸ���joinflatNode�������������
		Node *newOperatorNode = reorderFusionJoinNode(fusedOperatorNode,joinflatNode,flatNodes);
		Node *newJoinNode = makeHFusionJoin(newJoinInputList,newJoinPopArg,fusedOutputId,newJionPushArg); 
		vector<operatorNode *> newOperatorNodeVec;
		newOperatorNodeVec.push_back(&(jNode->u.operator_));
		newOperatorNodeVec.push_back(&(newOperatorNode->u.operator_));
		newOperatorNodeVec.push_back(&(newJoinNode->u.operator_));
		vector<FlatNode *>oldFlatNodes(flatNodes);
		oldFlatNodes.push_back(joinflatNode);
		vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//����sssg��
	}

}

//************************************
// Qualifier: ����flatNodes��split�Ĺ�ϵ��������flatNodes�ںϵõ���operNode������ߵ����ݽ���������
// Parameter: Node * operNode
// Parameter: FlatNode * joinFlatNode
// Parameter: vector<FlatNode * > flatNodes
//************************************
Node *HorizontalFusionSSG::reorderFusionSplitNode(Node *operNode,FlatNode *splitFlatNode,vector<FlatNode *> flatNodes)
{
	assert(splitFlatNode->contents->ot == Roundrobin_);
	//1��ȡsplit��flatNodes�еĸ��������ݵ�popֵ
	int begin_index = splitFlatNode->outFlatNodes.size();//��¼flatNodes�ĵ�һ���ڵ���split�ĵڼ�����֧��
	
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

	int splitSteadycount = _sjflatNode2smallSteadyCount.find(splitFlatNode)->second;//join�ڵ�����С��̬��ִ�д���
	int popNum = pop_value * splitSteadycount;//��С�ȶ�״ִ̬������������������

	//ȡoperNode������ߵ������͵ĳ�Ա�����б�
	Node *inputNode = (Node *)FirstItem(operNode->u.operator_.decl->u.decl.type->u.operdcl.inputs);
	Node *inputDecl = NULL;
	if(inputNode->typ == Id) inputDecl = inputNode->u.id.decl;
	else inputDecl = inputNode;
	List *stream_fields = inputDecl->u.decl.type->u.strdcl.type->fields;
	//������������Ҫ���м���������ͣ�struct��
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
	Node *bufferDecl = MakeDeclCoord(buffername,T_BLOCK_DECL,bufferAdcl,NULL,NULL,UnknownCoord);//���ṹ�����鹹����ɣ�bufferDecl����Ҫ����ӵ�work������
	
	//����buffer��Ϊ��תվ��������е�����������

	//���� ��operNode���������е�������ʱ���浽buffer�� ����䡪����Ҫ����ӵ�work��
	//����һ��ѭ��
	char * _iname = (char *) malloc(8);
	sprintf(_iname, "%s_%d", "__i",HFVarNum);
	Node *idI = MakeIdCoord(_iname, UnknownCoord);
	Node *declI = MakeNewDecl(_iname, PrimSint, MakeConstSint(0), Redecl);//����ѭ�������Ķ��塪��Ҫ�ӵ�work��
	Node *init = NULL, *cond = NULL, *next = NULL;
	idI->u.id.decl = declI;
	init = MakeBinopCoord('=', idI, MakeConstSint(0), UnknownCoord);
	cond = MakeBinopCoord('<', idI, MakeConstSint(popNum), UnknownCoord);
	next = MakeUnaryCoord(POSTINC, idI, UnknownCoord);
	//���湹�츳ֵ���	
	Node *inputId = MakeNewStreamId(inputDecl->u.decl.name,inputDecl);//�����������Ӧ��idNode
	Node *bufferId = MakeIdCoord(buffername,UnknownCoord);//���컺������Ӧ��idnode
	bufferId->u.id.decl = bufferDecl;
	Node *bufferArray = ExtendArray(bufferId,idI,UnknownCoord);
	Node *inputArray = ExtendArray(inputId,idI,UnknownCoord);
	List *assignmentList_1 = NULL;//��ֵ���list
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
	Node *initBufferNode = MakeForCoord(init,cond,next,for_stmt,UnknownCoord);//�����ʼ��buffer����䡪��Ҫ�ӵ�work��

	//��buffer�е����ݵ���˳�������д�ص����뻺������
	char * _jname = (char *) malloc(8);
	sprintf(_jname, "%s_%d", "__j",HFVarNum);
	char * _kname = (char *) malloc(8);
	sprintf(_kname, "%s_%d", "__k",HFVarNum);
	Node *idJ = MakeIdCoord(_jname, UnknownCoord);
	Node *idK = MakeIdCoord(_kname, UnknownCoord);
	Node *declJ = MakeNewDecl(_jname, PrimSint, MakeConstSint(0), Redecl);// �ӵ�work��
	Node *declK = MakeNewDecl(_kname, PrimSint, MakeConstSint(0), Redecl);//�ӵ�work��
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
		List *assignmentList_2 = NULL;//��ֵ���list
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
	Node *reorderWork = MakeBlockCoord(PrimVoid, NULL, stmts, UnknownCoord, UnknownCoord);//������������ѭ���ڵ㹹����ɡ���Ҫ�ӵ�work������
	//������Ľڵ���ӵ�work��

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
// Qualifier: ���splitflatNode���¶˵ķ�֧����flatNodes�еĽڵ���Ŀ��ͬ�ںϳ�һ���µĽڵ㣬ͬʱ��flatNodes���¶˻�Ҫ�����һ��������split�Ľڵ�
//				���splitflatNode���¶˵ķ�֧������flatNodes�еĽڵ�������flatNode�ڵ��ں���һ��ͬʱ�����µ�join�ڵ����joinflatNode��������flatNodes���϶˻�Ҫ�����һ��������join�Ľڵ�
// Parameter: FlatNode * splitflatNode
// Parameter: vector<FlatNode * > flatNodes
//************************************
void HorizontalFusionSSG::fusingFlatNodesSplit(FlatNode *splitflatNode,vector<FlatNode *> flatNodes)
{
	int nbranch = splitflatNode->nOut;
	//�ȹ����ںϺ�ڵ�������
	operatorNode *curOperator = flatNodes[0]->contents;
	char *fusedOutputName = NULL;
	Node *outputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.outputs);
	Node *output_Decl = NULL;
	if(outputNode->typ == Id) output_Decl = outputNode->u.id.decl;
	else output_Decl = outputNode;
	Node *outputStreamType = output_Decl->u.decl.type;
	fusedOutputName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
	sprintf(fusedOutputName, "%s_%d", output_Decl->u.decl.name, ++HFStreamNum);
	Node *outputNewStream = MakeNewStream(fusedOutputName,outputStreamType);//flatNodes�ڵ��ںϺ��γɵ�
	assert(outputNewStream && outputNewStream ->typ ==Decl);

	//����flatNodes���ںϺ�ڵ��¶˵�join�ڵ㡪�������roundrobin����
	List *down_splitOutputList = NULL;//flatNodes�еĽڵ������
	List *down_splitPushArg = NULL;
	int down_splitPopValue = 0;//�½���join�ڵ��pushֵ

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
	{//��split�Լ�flatNodes�ںϳ�һ���µ�operator�ڵ㣨�Ƚ�flatNodes�ڵ��ںϳ�һ���µ�operatorȻ�����µ�operator��Ȼ���ٶ�����ߵ����ݽ���������Ϊ�˽�flatNodes�¶˵Ľڵ�join�ڵ㴦�������������Ҫ�����ڲ���������ֱ�Ӷ����뻺�������в�����
		//2.�ں�flatNodes�еĽڵ�
		Node *splitInputNode = (Node *)FirstItem(splitflatNode->contents->decl->u.decl.type->u.operdcl.inputs);
		Node *splitIntputStream = NULL; 
		if(splitInputNode->typ == Id) splitIntputStream = splitInputNode->u.id.decl;
		else splitIntputStream = splitInputNode;
		//2.1 �ں�flatNodes������ÿһ��operator������Ҫ��һ��ѭ����ѭ���Ĵ�������С������������operator�еı���������,�޸�operator����������������Լ����ݷ��ʵ�ƫ��
		Node *fusedOperatorNode = fusingNodes(flatNodes,splitIntputStream,outputNewStream);

		//2.2 ���ڴ��ںϽڵ����Ŀ�ͷ�֧��Ŀ�Ĳ�ͬ���϶˵�split�ڵ���в�ͬ�Ĵ���

		//��joinflatNode�ڵ��ںϵ�������������fusedOperatorNode��work��������Ӷ�������е����ݽ���������
		Node *newOperatorNode = NULL;

		if(splitflatNode->contents->ot == Roundrobin_)newOperatorNode = reorderFusionSplitNode(fusedOperatorNode,splitflatNode,flatNodes);
		else newOperatorNode = fusedOperatorNode;
		
		vector<operatorNode *> newOperatorNodeVec;

		newOperatorNodeVec.push_back(&(newOperatorNode->u.operator_));	
		newOperatorNodeVec.push_back(&(down_splitNode->u.operator_));
		vector<FlatNode *>oldFlatNodes(flatNodes);
		oldFlatNodes.push_back(splitflatNode);
		vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//����sssg��							
	}
	else if(nbranch > flatNodes.size())
	{
		
		//ȷ���µ�split�������������
		List *new_splitOutputList = NULL;
		List *new_splitPushArgs = NULL;
		List *new_splitInputList = splitflatNode->contents->decl->u.decl.type->u.operdcl.inputs;

		List *old_splitOutputList= splitflatNode->contents->decl->u.decl.type->u.operdcl.outputs;

		Node *fusedInputNode = NULL;//���ںϽڵ�������
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

		//int old_splitSteadyCount = sjflatNode2smallSteadyCount.find(splitflatNode)->second;//��split�ڵ�ľֲ���̬����
	
		//2.�ں�flatNodes�еĽڵ�
		//2.1 �ں�flatNodes������ÿһ��operator������Ҫ��һ��ѭ����ѭ���Ĵ�������С������������operator�еı���������,�޸�operator����������������Լ����ݷ��ʵ�ƫ��
		Node *fusedOperatorNode = fusingNodes(flatNodes,fusedInputId->u.id.decl,outputNewStream);

		//2.2 ���ڴ��ںϽڵ����Ŀ�ͷ�֧��Ŀ�Ĳ�ͬ���϶˵�split�ڵ���в�ͬ�Ĵ���

		//��joinflatNode�ڵ��ںϵ�������������fusedOperatorNode��work��������Ӷ�������е����ݽ���������
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
		vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//����sssg��		
	}
}

//************************************
// Qualifier: �϶�split�ķ�֧ = flatNodesde��Ŀ = �¶�join��֧���ں�split��flatNodes��join
//************************************
vector<operatorNode *> HorizontalFusionSSG::fusionFlatNodesSplitJoin_FSFJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{
	vector<operatorNode *> newOperatorNodeVec;
	//ȡsplit������ߣ�join�������
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
// Qualifier: �޸�split���ں�flatNodes��join
//************************************
vector<operatorNode *> HorizontalFusionSSG::fusionFlatNodesSplitJoin_MSFJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{//�޸�split���ں�flatNodes��ͬʱ���ںϵõ���operator�ڵ������������������ݵ�reorder
	vector<operatorNode *> newOperatorNodeVec;
	//1.ȡjoin���������Ϊ�ںϺ�ڵ�����
	Node *old_joinOutputNode = (Node *)FirstItem(joinflatNode->contents->decl->u.decl.type->u.operdcl.outputs);
	Node *old_joinOutputDecl = NULL;
	if(old_joinOutputNode->typ == Id) old_joinOutputDecl = old_joinOutputNode->u.id.decl;
	else old_joinOutputDecl = old_joinOutputNode;
	// 2.�����ںϺ�ڵ�������
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

	//3.�ں�flatNode�ڵ�
	Node *fusionOperatorNode = fusingNodes(flatNodes,new_fusedInputNewStream,old_joinOutputDecl);
	//4.����join�ڵ�����Ͷ����ںϽڵ�������������
	Node *reorderNode_1 = reorderFusionJoinNode(fusionOperatorNode,joinflatNode,flatNodes);
	//5.����split�ڵ�����Ͷ����ںϽڵ�������������
	Node *reorderNode_2 = NULL; 
	if(splitflatNode->contents->ot==Roundrobin_) reorderNode_2 = reorderFusionSplitNode(reorderNode_1,splitflatNode,flatNodes);
	else reorderNode_2 = reorderNode_1;
	
	//6.�޸��϶˵�split�ڵ�
	Node *fusedInputId = MakeNewStreamId(new_fusedInputNewStream->u.decl.name,new_fusedInputNewStream);//�ںϺ�ڵ������ߣ���Ϊ�µ�split��һ�������

		//ȷ���µ�split�������������
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
	//7.ȷ�����յķ���
	newOperatorNodeVec.push_back(&(newSplitNode->u.operator_));
	newOperatorNodeVec.push_back(&(reorderNode_2->u.operator_));
	return newOperatorNodeVec;
}

//************************************
// Qualifier: �ں�split�޸�join
//************************************
vector<operatorNode *> HorizontalFusionSSG::fusionFlatNodesSplitJoin_FSMJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{//_ Fuse Split Modify Join //�޸�join���ں�flatNodes��ͬʱ���ںϵõ���operator�ڵ������������������ݵ�reorder
	vector<operatorNode *> newOperatorNodeVec;
	//1.ȡsplit���������Ϊ�ںϺ�ڵ������
	Node *old_splitInputNode = (Node *)FirstItem(splitflatNode->contents->decl->u.decl.type->u.operdcl.inputs);
	Node *old_splitInputDecl = NULL;
	if(old_splitInputNode->typ == Id) old_splitInputDecl = old_splitInputNode->u.id.decl;
	else old_splitInputDecl = old_splitInputNode;
	// 2.�����ںϺ�ڵ�������
	operatorNode *curOperator = flatNodes[0]->contents;
	char *new_fusedOutputName = NULL;
	Node *outputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.outputs);//ȡ��һ�����ںϽڵ�����
	Node *output_Decl = NULL;
	if(outputNode->typ == Id) output_Decl = outputNode->u.id.decl;
	else output_Decl = outputNode;
	Node *new_fusedOutputStreamType = output_Decl->u.decl.type;
	new_fusedOutputName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
	sprintf(new_fusedOutputName, "%s_%d", output_Decl->u.decl.name, ++HFStreamNum);
	Node *new_fusedOutputNewStream = MakeNewStream(new_fusedOutputName,new_fusedOutputStreamType);
	assert(new_fusedOutputNewStream && new_fusedOutputNewStream ->typ ==Decl);

	//3.�ں�flatNode�ڵ�
	Node *fusionOperatorNode = fusingNodes(flatNodes,old_splitInputNode,new_fusedOutputNewStream);
	//4.����split�ڵ�����Ͷ����ںϽڵ�������������
	Node *reorderNode_1 = NULL; 
	if(splitflatNode->contents->ot==Roundrobin_) reorderNode_1 = reorderFusionSplitNode(fusionOperatorNode,splitflatNode,flatNodes);
	else reorderNode_1 = fusionOperatorNode;
	//5.����join�ڵ�����Ͷ����ںϽڵ�������������
	Node *reorderNode_2 = reorderFusionJoinNode(reorderNode_1,joinflatNode,flatNodes);
	
	//6.�޸��¶˵�join�ڵ�
	Node *fusedOutputId = MakeNewStreamId(new_fusedOutputNewStream->u.decl.name,new_fusedOutputNewStream);//�ںϺ�ڵ������ߣ���Ϊ�µ�join��һ�������

		//ȷ���µ�join��������������Լ�����
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
	//7.ȷ�����յķ���
	newOperatorNodeVec.push_back(&(newJoinNode->u.operator_));
	newOperatorNodeVec.push_back(&(reorderNode_2->u.operator_));
	return newOperatorNodeVec;
}

//************************************
// Qualifier:�޸�split���޸�join
//************************************
vector<operatorNode *> HorizontalFusionSSG::fusionFlatNodesSplitJoin_MSMJ(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{//_ Modify Split Modify Join //�޸�split��join���ں�flatNodes��ͬʱ���ںϺ�Ľڵ����reorder
	vector<operatorNode *> newOperatorNodeVec;
	operatorNode *curOperator = flatNodes[0]->contents;
	
	//1.�����ںϺ�ڵ�������
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
	
	// 2.�����ںϺ�ڵ�������
	char *new_fusedOutputName = NULL;
	Node *outputNode = (Node *)FirstItem(curOperator->decl->u.decl.type->u.operdcl.outputs);//ȡ��һ�����ںϽڵ�����
	Node *output_Decl = NULL;
	if(outputNode->typ == Id) output_Decl = outputNode->u.id.decl;
	else output_Decl = outputNode;
	Node *new_fusedOutputStreamType = output_Decl->u.decl.type;
	new_fusedOutputName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
	sprintf(new_fusedOutputName, "%s_%d", output_Decl->u.decl.name, ++HFStreamNum);
	Node *new_fusedOutputNewStream = MakeNewStream(new_fusedOutputName,new_fusedOutputStreamType);
	assert(new_fusedOutputNewStream && new_fusedOutputNewStream ->typ ==Decl);

	//3.�ں�flatNode�ڵ�
	Node *fusionOperatorNode = fusingNodes(flatNodes,new_fusedInputNewStream,new_fusedOutputNewStream);
	//4.����split�ڵ�����Ͷ����ںϽڵ�������������
	Node *reorderNode_1 = NULL; 
	if(splitflatNode->contents->ot==Roundrobin_) reorderNode_1 = reorderFusionSplitNode(fusionOperatorNode,splitflatNode,flatNodes);
	else reorderNode_1 = fusionOperatorNode;
	//5.����join�ڵ�����Ͷ����ںϽڵ�������������
	Node *reorderNode_2 = reorderFusionJoinNode(reorderNode_1,joinflatNode,flatNodes);
	//6.����split
	Node *fusedInputId = MakeNewStreamId(new_fusedInputNewStream->u.decl.name,new_fusedInputNewStream);//�ںϺ�ڵ������ߣ���Ϊ�µ�split��һ�������

	//ȷ���µ�split�������������
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
	

	//7����join
	Node *fusedOutputId = MakeNewStreamId(new_fusedOutputNewStream->u.decl.name,new_fusedOutputNewStream);//�ںϺ�ڵ������ߣ���Ϊ�µ�join��һ�������

	//ȷ���µ�join��������������Լ�����
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
	//8.ȷ�����յķ���
	newOperatorNodeVec.push_back(&(newSplitNode->u.operator_));
	newOperatorNodeVec.push_back(&(reorderNode_2->u.operator_));
	newOperatorNodeVec.push_back(&(newJoinNode->u.operator_));
	return newOperatorNodeVec;
}
//************************************
// Qualifier: �����������1.�϶�split�ķ�֧ = flatNodesde��Ŀ = �¶�join��֧
//						  2.�϶�split�ķ�֧ = flatNodesde��Ŀ < �¶�join��֧
//						  3.�϶�split�ķ�֧ > flatNodesde��Ŀ < �¶�join��֧
//						  4.�϶�split�ķ�֧ > flatNodesde��Ŀ = �¶�join��֧
// Parameter: FlatNode * splitflatNode
// Parameter: FlatNode * joinflatNode
// Parameter: vector<FlatNode * > flatNodes
//************************************
void HorizontalFusionSSG::fusingFlatNodesSplitJoin(FlatNode *splitflatNode,FlatNode *joinflatNode,vector<FlatNode *> flatNodes)
{
	int split_branch = splitflatNode->nOut;
	int join_branch = joinflatNode->nIn ;
	int fusing_branch = flatNodes.size(); 
	vector<operatorNode *>newOperatorNodeVec;//��¼�ں��²�����operator
	if(split_branch == fusing_branch && fusing_branch == join_branch)
	{//�ں�split��flatNodes��join
		newOperatorNodeVec = fusionFlatNodesSplitJoin_FSFJ(splitflatNode,joinflatNode,flatNodes);
	}
	else if(split_branch > fusing_branch && fusing_branch == join_branch)
	{//�޸�split���ں�flatNodes��join
		newOperatorNodeVec = fusionFlatNodesSplitJoin_MSFJ(splitflatNode,joinflatNode,flatNodes);
	}
	else if (split_branch == fusing_branch && fusing_branch < join_branch)
	{//�޸�join���ں�split��flatNodes
		newOperatorNodeVec = fusionFlatNodesSplitJoin_FSMJ(splitflatNode,joinflatNode,flatNodes);
	}
	else if(split_branch > fusing_branch && fusing_branch < join_branch)
	{//�޸�split��join���ں�flatNodes
		newOperatorNodeVec = fusionFlatNodesSplitJoin_MSMJ(splitflatNode,joinflatNode,flatNodes);
	}
	vector<FlatNode *>oldFlatNodes;
	oldFlatNodes.push_back(splitflatNode);
	for(int i = 0; i != flatNodes.size(); i++)
		oldFlatNodes.push_back(flatNodes[i]);
	oldFlatNodes.push_back(joinflatNode);
	vector<FlatNode *> newFlatNodes = replaceHfusionFlatNodes(oldFlatNodes, newOperatorNodeVec);//����sssg��		
}

//************************************
// Qualifier: ��flatNode�ڵ��ںϳ�һ���ڵ�(ͬʱ��Ҫ�޸�SSG�е�����)���ںϵķ����ˮƽ�ںϵ��ĵ�
// Parameter: vector<FlatNode * > flatNodes
//************************************
void HorizontalFusionSSG::fusingFlatNodes(vector<FlatNode *> flatNodes)
{
	assert(flatNodes.size() > 1);
	//ȡflatNodes���϶˽ڵ���¶˽ڵ�
	FlatNode *upFlatNode = flatNodes[0]->inFlatNodes[0];//flatNodes���϶˽ڵ�
	FlatNode *downFlatNode = flatNodes[0]->outFlatNodes[0];//flatNodes���¶˽ڵ�
	if (upFlatNode->nOut == 1 && downFlatNode->nIn == 1)
	{
		fusingFlatNodesCommon(flatNodes);//һ�����
	}
	else if(upFlatNode->nOut == 1  && downFlatNode->nIn > 1)
	{
		fusingFlatNodesJoin(downFlatNode, flatNodes);//2�����
	}
	else if (upFlatNode->nOut > 1  && downFlatNode->nIn == 1)//((upFlatNode->contents->ot == Duplicate_ || upFlatNode->contents->ot == Roundrobin_ ) && downFlatNode->contents->ot == Common_)
	{
		fusingFlatNodesSplit(upFlatNode, flatNodes);//2�����
	}
	else if (upFlatNode->nOut > 1  && downFlatNode->nIn > 1)//((upFlatNode->contents->ot == Duplicate_ || upFlatNode->contents->ot == Roundrobin_ ) && downFlatNode->contents->ot == Join_)
	{
		fusingFlatNodesSplitJoin(upFlatNode, downFlatNode, flatNodes);//4�����
	}
	else
	{
		cout<<"fission error........."<<endl;
	}
}

//************************************
// Qualifier: ��flatNodes�е������ܹ���ˮƽ�ںϵĽڵ��ռ�������ŵ�priority2candidateFlatNodes��
//************************************
void HorizontalFusionSSG::collectFusionFlatNodesInfo(vector<FlatNode *> flatNodesTopoSequence)
{//�ҳ���ǰSDFͼ���ܹ����ںϵ����нڵ�----------------Ҫ�ռ���ǰͼ�������ܹ����ںϵĽڵ�
	//��ȡͼ�����п��ܱ��ںϵĽڵ㣬�����ս��������priority2candidateFlatNodes��
	vector<vector<FlatNode *> >splitjoinFlatNodes;//���е�ÿһ��Ԫ����һ��splitjoin�а����Ľڵ�
	int joinFlag = 0;//��ʾ�Ƿ��Ѿ�����join�ڵ���
	int splitFlag = 0;
	vector<FlatNode *>_tmpsjFlatNodes;//һ�����ڲ��sj�е�flatNode�ڵ�
	for (int i= flatNodesTopoSequence.size() - 1; i >= 0; i--)
	{//����ͼ���Ե���������
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
				//�Դ��µĺ�ѡ��һ����̬����(ֻ��joinFlagΪ0�ǲŵ��ȣ�
				map<FlatNode *,int>_sjflatNode2sc = hfsssg->SteadySchedulingGroup(_tmpflatNodes);
				_sjflatNode2smallSteadyCount.insert(_sjflatNode2sc.begin(),_sjflatNode2sc.end());
				_tmpsjFlatNodes.clear();
				splitjoinFlatNodes.push_back(_tmpflatNodes);
			}
			else
			{
				vector<FlatNode *> _tmpflatNodes;//��һ����split�����һ����join
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
	//��ȡ��֧���ܹ���ˮƽ�ںϵĽڵ�,��priority2candidateFlatNodes�����Ԫ�أ�����ɾֲ���̬����
	addCandidateFusionFlatNodes(splitjoinFlatNodes);
	//���ýڵ�ķ���
	for(int i = 0; i != flatNodesTopoSequence.size(); i++)
		flatNodesTopoSequence[i]->ResetVistTimes();
}

//************************************
// Returns:   void �������priority2candidateFlatNodes�У�����prority2candidateFlatNodes���Խڵ�����ȼ�Խ��priority��ֵԽ�ͣ���map��Խ��ǰ
// Qualifier: ��sjflatNode�е��ܹ��ںϵĽڵ�����������뵽partitonNum2FlatNode�С���֮�ں�ͬ�η�֧
//				sj���ܹ����ںϵĽڵ��������1.�ںϽڵ����ڵķ�֧����Ҫ��ͬ��2.�ںϽڵ�����������
// Parameter: vector<vector<FlatNode *> > ��ŵ�������sj�еĽڵ㣨�����vector�е�Ԫ�صĵ�һ���ڵ���split�����һ���ڵ���join��
//************************************
void HorizontalFusionSSG::addCandidateFusionFlatNodes(vector<vector<FlatNode *> > splitjoinFlatNodes)
{
	priority2candidateFlatNodes.clear();
	for(int si = 0; si != splitjoinFlatNodes.size(); si++)
	{
		vector<FlatNode *> sjFlatNodes = splitjoinFlatNodes[si];
		if(sjFlatNodes.size() <= 2) continue;
		//��������֧�Ľڵ㽻��DetectHorizontalFusingEligible����Ƿ�����ںϵ�����
		//��ȡ������֧
		vector<vector<FlatNode *> >sjPathFlatNodes;//ÿһ����֧���splitjoin�ڵ�ĵ�����֧
		FlatNode *splitFlatNode = sjFlatNodes[0];//ȡsplit�ڵ�
		FlatNode *joinFlatNode = sjFlatNodes[sjFlatNodes.size() - 1]; //ȡsplitFlatNode��Ӧ��join�ڵ�
		vector<FlatNode *> path;
		for(int j = 1; j != sjFlatNodes.size() - 1; j++)
		{
			path.push_back(sjFlatNodes[j]);
			if(sjFlatNodes[j]->outFlatNodes[0] == joinFlatNode)//�ڵ���¶˽ڵ���join�ڵ㣬��ô�������ҵ�һ��������·��
			{	
				vector<FlatNode *> tmpPath(path);
				path.clear();
				sjPathFlatNodes.push_back(tmpPath);
				//for(int ci = 0; ci != tmpPath.size(); ci++)
					//cout<<tmpPath[ci]->name<<endl;
				//cout<<"++++++++++++++++++++++++++\n";
			}
		}
		if(sjPathFlatNodes.size() != sjFlatNodes[0]->nOut) continue;//��֤��֧��Ŀ�Ƿ���ȫ
		assert(sjPathFlatNodes.size() > 0);

		//�жϷ�֧�Ƿ���ͬ����
		vector< vector< vector<FlatNode *> > > sameStylePathFlatNodes;//��Ų�ͬ��֧���͵ķ�֧�ļ���
		vector<vector<FlatNode *> >tmpStylePath;//���ֻ��һ�����͵ķ�֧�ļ���
		tmpStylePath.push_back(sjPathFlatNodes[0]);

		for(int i = 1; i != sjPathFlatNodes.size();i++)
		{
			Bool flag = TRUE;
			if(sjPathFlatNodes[i].size() != sjPathFlatNodes[i-1].size())
			{//������Ȳ��������ͬһ���͵ķ�֧
				vector<vector<FlatNode *> >tmpStyle(tmpStylePath);
				sameStylePathFlatNodes.push_back(tmpStyle);//��tmpStylePath�ŵ�sameStylePathFlatNodes�У������tmpStylePath��Ϊ�����һ�����͵ķ�֧��׼��
				tmpStylePath.clear();
				tmpStylePath.push_back(sjPathFlatNodes[i]);
				continue;
			}
			//��i���i-1����֧������ͬ
			for(int j = 0; j != sjPathFlatNodes[i].size(); j++)
			{
				if(sjPathFlatNodes[i][j]->contents->ot != sjPathFlatNodes[i-1][j]->contents->ot || sjPathFlatNodes[i][j]->GetVisitTimes() != sjPathFlatNodes[i-1][j]->GetVisitTimes())
				{//��Ӧ�ڵ�����Ͳ����
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
		sameStylePathFlatNodes.push_back(tmpStylePath);//�����Ƚϵ���β

		assert(sameStylePathFlatNodes.size() > 0);
		//��������ȽϵĽ����ȷ�����Ա��ںϵĽڵ㣬�����ںϵ������ڵ���뵽priority2candidateFlatNodes��,���ȼ����ô��ںϽڵ���ܹ�����/���ںϽڵ����Ŀ
		vector<FlatNode *> tmpflatNodes;
		for(int i = 0; i != sameStylePathFlatNodes.size(); i++)
		{//��֧��������
			if(sameStylePathFlatNodes[i].size() == 1 ) continue;//�����͵ķ�ֻ֧��һ������ʲôҲ����
			for(int j = 0; j != sameStylePathFlatNodes[i][0].size();j++)//�����i������
			{//ÿ�з�֧ʵ�ʳ��ȣ�һ����֧�ϵ�flatNode��Ŀ��
				tmpflatNodes.clear();
				for(int l = 0; l != sameStylePathFlatNodes[i].size();l++)
				{//ĳһ�����͵ķ�֧����Ŀ���������ͷ�֧�Ŀ�ȣ�
					Bool fusionCondition = detectHorizontalFusionEligible(sameStylePathFlatNodes[i][l][j]);
					//Ҫ�Ƚϴ��ڷ�֧ͬһ�е�operator����������ߵ������Ƿ���ͬ�������ͬ�����ںϣ����ñȽϣ�
					if(fusionCondition)
					{
						tmpflatNodes.push_back(sameStylePathFlatNodes[i][l][j]);
					}
					else 
					{//�ڴ����֧��һ����ʱ���м���ֲ����ںϵĽڵ�(��ô�ʹ���ǰ��Ŀ��ںϵĽڵ�)
						if(tmpflatNodes.size() <= 1) continue;//tmpflatNodesֻ��һ���ڵ�	
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
				//���洦����ǵ������ͷ�֧��һ�д������ˣ����������β����β��
				if(tmpflatNodes.size() <= 1) continue;//tmpflatNodesֻ��һ���ڵ�	
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
// Qualifier: ���һ��filter�Ƿ������ںϵ���������stateless��peek == pop�����뵥����������㷵��TRUE
// Parameter: FlatNode * flatNode
//************************************
Bool HorizontalFusionSSG::detectHorizontalFusionEligible(FlatNode *flatNode)
{
	assert(flatNode);
	//if(DetectiveFilterState(flatNode)){ cout<<"stateful"<<flatNode->name<<endl;return FALSE;}//��stateful���͵�filter
	if(flatNode->nIn != 1 || flatNode->nOut != 1) return FALSE;//splitjoin��㲻�ǵ��뵥����
	if(flatNode->inPeekWeights[0] > flatNode->inPopWeights[0] ) {return FALSE;}//��peek>pop����filter
	return TRUE;
}

//************************************
// Qualifier: metis���ֵĽ�������mpart�У������ڵ�ĸ��ش����vwgt�У��ú����Ĺ�����ȷ��������ɺ󣬷������Ļ��ֵĸ���
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
// Qualifier: ����hfsssgȷ��metis���ֵĳ�ʼ�������������˳�ʼ��FlatNode2No��flatNodeNewNo2OldNo��flatNodeOldNo2NewNo
//************************************
void HorizontalFusionSSG::initMetisPartitionParamiter(vector<FlatNode *> flatNodesTopoSequence)
{
	map<FlatNode *, int>::iterator iter;
	typedef multimap<int,FlatNode *>::iterator iter1;
	map<FlatNode *, int> flatNode2Workload = hfsssg->GetSteadyWorkMap();
	hfsssg->mapSteadyCount2FlatNode.clear();
	hfsssg->mapSteadyCount2FlatNode = SteadySchedulingGroup(hfsssg->flatNodes);
	//�õ���ʼSDFͼ�ڵ�������Լ��ڵ����ŵĶ�Ӧ��ϵ
	int k=0;//k���ڼ�¼flagnode�����ڽڵ���
	for(int i = 0; i != flatNodesTopoSequence.size();i++)
	{//����flatNode����֮���map
		FlatNode2No.insert(make_pair(flatNodesTopoSequence[i],i));
		flatNodeNewNo2OldNo.insert(make_pair(i,i));
		flatNodeOldNo2NewNo.insert(make_pair(i,i));
	}
	for(int i = 0; i != flatNodesTopoSequence.size();i++){
		vsize[i]=0;
		int flag=0;//��֤sumֻ��һ��
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
	//��ʼ�����ýڵ�ķ���
	for(int i = 0; i != flatNodesTopoSequence.size(); i++)
		flatNodesTopoSequence[i]->ResetVistTimes();
// 	cout<<"��ʼ��#########################\n"<<endl;
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
// Qualifier:ʹ��metis����������ͼ
//************************************
void HorizontalFusionSSG::partitionSSG()
{
	if(partitionNum == 1)
	{//���ֻ��һ������
		for(int i = 0; i != nvtxs; i++)
			mpart[i] = 0;
		return;
	}
	MetisPartiton *metispart = new MetisPartiton(0,0);
	metispart->metisPartition(nvtxs, 1,xadj,adjncy,vwgt,vsize,NULL,partitionNum,NULL,NULL,NULL,mpart);//���ֵĽ������mpart��
}

//************************************
// Qualifier: ��priority2candidateFlatNodesѡ�����ȼ��ߵĽڵ��ں�,�����е�һ����¼���չ��������ֳ�num��(�����ںϽڵ�ȫ���ں���һ�𣬻�����ѡ����ںϼ������ڵĽڵ�)
//************************************
void HorizontalFusionSSG::selectFusionFlatNodes(int num)
{//ȡpriority2candidateFlatNodes�еĵ�һ����¼������
	map<int, vector<FlatNode *> > tmp_curFusingNo2FlatNodes;
	vector<FlatNode *> fusionFlatNodes = priority2candidateFlatNodes.begin()->second;//ȡҪ�����flatNode�Ľ��
	if(fusionFlatNodes.size() <= num) return; //�����ںϽڵ����ĿС�ڵ����ںϵķ���ʱ����Ҫ�ں�
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
	//=================================ȷ���ж��ٸ��ڵ��ܹ����ں���һ��========================================
	//20121203�޸���((float)sumWorkload) / curWorkload��Ϊ����ڵ�ķ�ʽ
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
	{//��flatNodesβ���Ľڵ���ӵ�fusingNo2FlatNodes
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
// Qualifier: ��fusingNo2FlatNodes��tmpFusingNo2FlatNodes�е�Ԫ��ɾ����ͬʱ�޸���������Ϣ=========>��ԭһЩ��Ϣ
//************************************
void HorizontalFusionSSG::undoSelectFusionFlatNode()
{
	//��ԭmetis�����еĲ�������updateMetisPartitionParamiter��Ӧ
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
	//��ԭflatNodeOldNo2NewNo��ֵ
	flatNodeOldNo2NewNo = bak_flatNodeOldNo2NewNo;
	FlatNode2No = bak_FlatNode2No;
	//���tmpFusingNo2FlatNodes
	curFusingNo2FlatNodes.clear();
	flatNodeNewNo2OldNo.clear();
}

//************************************
// Qualifier:  ����metis�����õ��Ĳ�������updateMetisPartitionParamiter֮ǰʹ�ã�Ŀ����Ϊ����һ�λ�������ʱ�ܹ���ԭ������ǰ��ֵ
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
// Method:    updateMetisPartitionParamiter(�����ڲ������ĸ�ֵ������memcpy��ʵ��)
// FullName:  HorizontalFusionSSG::updateMetisPartitionParamiter
// Access:    public 
// Returns:   void
// Qualifier: ����selectFusionFlatNodesִ�еĽ��ȷ����Щ�ڵ㽫Ҫ���ںϣ��ڴ˴��޸Ľڵ�ڵ���������ϵ�����Ը���tmpFusingNo2FlatNodes��������
//************************************
void HorizontalFusionSSG::updateMetisPartitionParamiter()
{//����tmpFusingNo2FlatNodes�е���Ϣ�������޸ĵ���Ϣ��Ҫ��nvtxs��xadj��adjncy��vwgt��vsize
	//����޸Ĳ�����metis�ܹ�����
	int *tmp_vweight = (int *)malloc(sizeof(int) * nvtxs);
	int *tmp_vsize = (int *)malloc(sizeof(int) * nvtxs); 
	int *tmp_xadj = (int *)malloc( sizeof(int)*(nvtxs + 1) );
	int *tmp_adjncy = (int *)malloc(sizeof(int) * edgenum);

	for(map<int, vector<FlatNode *> >::iterator iter = curFusingNo2FlatNodes.begin(); iter != curFusingNo2FlatNodes.end(); iter ++)
	{
		vector<FlatNode *>curflatNodes = iter->second;
		int curFusionFlatNodeCount = curflatNodes.size();
		if(curFusionFlatNodeCount ==1) break;
		//�޸�flatNodeOldNo2NewNo�е���Ϣ
		int smallNo = flatNodeOldNo2NewNo.size();//���ڼ�¼���ںϽڵ��б����С�Ľڵ�ı��
		//�Ҵ��ںϽڵ��б����С�Ľڵ�
		for(int i = 0; i != curflatNodes.size();i++)
		{
			map<FlatNode *,int>::iterator curIter = FlatNode2No.find(curflatNodes[i]);
			assert(curIter != FlatNode2No.end());
			if(curIter->second < smallNo) smallNo = curIter->second; //�Ҵ��ںϽڵ��б����С�Ľڵ�ı��
		}

		//�޸Ľڵ���flatNodeOldNo2NewNo���ںϺ��µõĽڵ�ı��ΪsmallNo��
		for(int i = 0; i != curflatNodes.size();i++)
		{
			map<FlatNode *,int>::iterator curIter = FlatNode2No.find(curflatNodes[i]);
			assert(curIter != FlatNode2No.end());
			map<int,int>::iterator oldnoIter = flatNodeOldNo2NewNo.find(curIter->second);
			assert(oldnoIter != flatNodeOldNo2NewNo.end());
			oldnoIter->second = smallNo;
			curIter->second = smallNo;
		}
		//�޸Ľڵ����Ŀnvtxs
		nvtxs = nvtxs - curflatNodes.size() + 1;
		//�޸�flatNodeNewNo2OldNo�е���Ϣ
		multimap<int,int>tmp_flatNodeNewNo2OldNo;//Ϊ�˹����ֱ����ɱ�ŵ�ӳ���ϵ����Ϊ��ת��
		for(map<int,int>::iterator noIter = flatNodeOldNo2NewNo.begin();noIter != flatNodeOldNo2NewNo.end(); noIter++)
		{
			tmp_flatNodeNewNo2OldNo.insert(make_pair(noIter->second,noIter->first));
		}
		assert(tmp_flatNodeNewNo2OldNo.size()== flatNodeOldNo2NewNo.size());//flatNodeNewNo2OldNo.size()��¼�����ں�ǰ�����Ŀ
		//�±��Ҫ������������ֻ���ںϵĽڵ���Ҫ�޸ģ������ڵ�ı��ҲҪ�����޸ģ�
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
		//ͨ��flatNodeNewNo2OldNo����flatNodeOldNo2NewNo
		flatNodeOldNo2NewNo.clear();
		for(multimap<int,int>::iterator noIter = flatNodeNewNo2OldNo.begin();noIter != flatNodeNewNo2OldNo.end(); noIter++)
		{
			flatNodeOldNo2NewNo.insert(make_pair(noIter->second,noIter->first));
		}
		//+++++++++++++++���˽��ڵ���¾ɱ�������޸����++++++++++++++++++++++

		//����flatNodeOldNo2NewNo�޸�xadj��adjncy��vwgt��vsize�Լ�flatNodeNo2SteadyCount��sjflatNode2smallSteadyCount�е���Ϣ
		//�޸Ľڵ�Ĺ��������Ƚ��ɵĹ���������ȷ���½ڵ�Ĺ�����������vwgt��vsize

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
		//����vwgt,vsize
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

		//�����޸�xadj��adjncy

		int tmp_edgenum = hfsssg->GetMapEdge2DownFlatNode().size() + hfsssg->GetMapEdge2UpFlatNode().size();

		for(int i = 0; i!= tmp_edgenum;i++)
		{
			tmp_adjncy[i] = 0;
		}
		//���Զ���һ��map,�����нڵ��������ڵĵ�Ĺ�ϵ������flatNodeNo2AdjFlatNodeNoSet��
		map<int ,std::set<int> > flatNodeNo2AdjFlatNodeNoSet;

		for (int i = 0;i != nvtxs;i++)
		{
			std::set<int > tmp_adjset;
			std::pair<multimap<int,int>::iterator, multimap<int,int>::iterator>range = flatNodeNewNo2OldNo.equal_range(i);//��һ���±�Ŷ�Ӧ�����еľɵı��
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
		//����flatNodeNo2AdjFlatNodeNoSet�е���Ϣ���¹���xadj��adjncy
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
		//��xadj�ж����������0
		for(int i = nvtxs+1; i != flatNodeOldNo2NewNo.size() + 1 ;i++)
			xadj[i] = 0;
		//�޸��ڽӱߵĹ�ϵ
		for(int i = 0; i!= tmp_edgenum;i++)
		{
			adjncy[i] = tmp_adjncy[i];
		}

		//�����ںϺ�Ľ�����±��
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
// Qualifier: ������SDFͼ���ںϣ���ѡ����ںϽڵ��ǰ���һ�������㷨��������ʹ�õ���metis�����㷨����ʹ���ںϺ󻮷ֵĽ���������ƣ��˴���Ҫ�ǴӸ��صĽǶȿ��죩
//			�Ժ�ѡ���е�һ����ѡ�����ںϲ�������ʽ�㷨���Ƚ�һ����ѡ�еĽڵ�ȫ���ںϳ�һ���ڵ㣬����ںϺ��Ч�����ã�������ǰ���ںϣ��ٽ���ѡ�ڵ��ںϳ�2��
//			�ٿ���ʱ��Ч�������Ч����������������ںϵķ�����
//************************************
void HorizontalFusionSSG::selectFusingSSG()
{
	//��ʼ���ֵõ��ں�ǰ�ĳ�ʼֵ
	vector<FlatNode *>flatNodesTopoSequence = TopoSortFilter();//FlatNode�ڵ����������
	initMetisPartitionParamiter(flatNodesTopoSequence);
	partitionSSG();// ��SDFͼ������
	int initMaxWeight = findMaxPartitionWeight();
	int curMaxWeight = initMaxWeight;
	float changeRatio = THRESHOLD;

	while(1)
	{
		collectFusionFlatNodesInfo(flatNodesTopoSequence);//������������Ľ��ȷ���ܹ����ںϵ�flatNode�ڵ�Ľ��
		if (priority2candidateFlatNodes.empty())break;//Ϊ�����������ѭ����û�п��ںϵĽڵ㣩
		int num = 1;//��ѡ����һ��Ԫ�صĽڵ㼯���ںϳɵ���Ŀ
		std::multimap<int ,vector<FlatNode *> >::iterator iter = priority2candidateFlatNodes.end();//��¼��ǰ�����һ����¼
		while ( !priority2candidateFlatNodes.empty())
		{
			if (priority2candidateFlatNodes.empty()) break;//�����ڲ�ѭ������ǰ�ܹ����ںϵĽڵ��ں����ˣ�		
			selectFusionFlatNodes(num);//ѡ��Ҫ���ںϽڵ㣨����ǰ�����ӵ�tmpFusingNo2FlatNodes��fusingNo2FlatNodes�У�
			backupPartitionInfo();//���ں�ǰ�����ݣ����㳷���ں�
			updateMetisPartitionParamiter();//����ѡ���ںϵĽڵ�������»���Ҫ�Ĳ���((����һ��α�ں�)���µ���ͼ�Ľṹ��������metis�Ĳ��������ֵ�)
			partitionSSG();//��ͼ���»���
			curMaxWeight = findMaxPartitionWeight();//���ҵ�ǰ������Ȩֵ����
			changeRatio = ((float)(curMaxWeight - initMaxWeight)/initMaxWeight);//�����ں϶Ի��ֵ�Ӱ��
			//cout<<"init= "<<initMaxWeight<<"  cur= "<<curMaxWeight<<"  �ںϵĸĽ�"<<changeRatio<<endl;
			if (changeRatio >= THRESHOLD) //�����ǰ���ֵ�Ч���������ں�
			{//�ںϵĽ�����ã������������ںϵ����ȣ�ֱ�����ܱ��ںϣ������Ӻ�ѡ��ɾ��
				undoSelectFusionFlatNode(); //�ںϺ������ã�������ǰ�Ĳ���������tmpFusingNo2FlatNodes�еĽ��������
				num++;//�ں����յķ�������
				//cout<<"fission....."<<num<<"  "<<priority2candidateFlatNodes.begin()->second.size()<<endl;
				if(num >= priority2candidateFlatNodes.begin()->second.size())//���ܱ��ں�
					priority2candidateFlatNodes.erase(priority2candidateFlatNodes.begin());//ɾ�����ܱ�����ļ�¼
			}
			else 
			{
				num = 1;
				for(map<int, vector<FlatNode *> >::iterator iter_0 = curFusingNo2FlatNodes.begin();iter_0 != curFusingNo2FlatNodes.end();iter_0++)
					fusingNo2FlatNodes.push_back(iter_0->second);
				priority2candidateFlatNodes.erase(priority2candidateFlatNodes.begin());	
			}
		}
		if(fusingNo2FlatNodes.empty()) break;//����û�нڵ�Ҫ���ں�
		//��ʵ�ʵ��ں�
		for (vector<vector<FlatNode *> >::iterator iter_3 = fusingNo2FlatNodes.begin(); iter_3 != fusingNo2FlatNodes.end(); iter_3++)
			fusingFlatNodes(*iter_3);
		//resetһЩ����
		flatNodesTopoSequence.clear();
		FlatNode2No.clear();
		fusingNo2FlatNodes.clear();
		_sjflatNode2smallSteadyCount.clear();
		flatNodesTopoSequence.clear();
		priority2candidateFlatNodes.clear();
		nvtxs = hfsssg->flatNodes.size();
		flatNodeOldNo2NewNo.clear();
		flatNodeNewNo2OldNo.clear();
		flatNodesTopoSequence = TopoSortFilter();//�����ںϺ��ͼ������
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
	HFSSG->hfsssg->mapSteadyCount2FlatNode = SteadySchedulingGroup(HFSSG->hfsssg->flatNodes);//���ںϺõ�ͼ���½�����̬����
	HFSSG->hfsssg->InitScheduling();//���ںϺõ�ͼ���½��г�̬����
	return HFSSG->hfsssg;
}