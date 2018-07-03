#include "HorizontalFission.h"
#include "basics.h"

using namespace std;

PRIVATE  int  gCurrentFissionCompositeNum = 0;//由于解决重命名问题

Bool stateful = FALSE;

Bool HorizontalFission::SInSOutOPerator(operatorNode *oper)
{//如果该operator是单入单出的返回True
	assert(oper);
	List *inputList = oper->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = oper->decl->u.decl.type->u.operdcl.outputs;
	if((ListLength(outputList)==1)&&(ListLength(inputList)==1)) return TRUE;
	return FALSE;
}

HorizontalFission::HorizontalFission(Partition *p,float balane):partition(p),balanceFactor(balane)
{//构造函数
}
void HorizontalFission::hasMutableState(operatorNode *opNode)
{  //是state则返回True
	assert(opNode->body!=NULL);
	List *stateList = opNode->body->u.operBody.state;
	Node *workNode = opNode->body->u.operBody.work;
	assert(workNode&&workNode->typ==Block);
	if(stateList == NULL) 
	{
		stateful = FALSE;
		return;
	}
	MS_astwalk(workNode,stateList);
}

void HorizontalFission::IsMutableVar(List *list,Node *node)
{ //如果变量是state则返回True
	assert(list);
	assert(node);
	ListMarker m;
	Node *stateDecl = NULL;
	IterateList(&m,list);
	while (NextOnList(&m , (GenericREF)&stateDecl))
	{
		assert(stateDecl);
		if(node->typ == Id)
		{
			if (node->u.id.decl == stateDecl)
			{
				//cout<<"*********  " << node->u.id.text<<"  ************"<<endl;
				stateful = TRUE;
				return;
			}
		}
		else if (node->typ == Array)
		{
			if(node->u.array.name->u.id.decl == stateDecl)
			{
				//cout<<"*********  " <<node->u.array.name->u.id.text<<"  ************"<<endl;
				stateful = TRUE;
				return;
			}
		}
		else if(node->typ==Binop&&node->u.binop.op == '.')
		{  // 处理结构体类型
			Node *tmpNode = node->u.binop.left;
			assert(tmpNode);
			while (tmpNode->typ==Binop)
			{
				assert(tmpNode->u.binop.op == '.');
				tmpNode = tmpNode->u.binop.left; 
			}
			assert(tmpNode->typ == Id || tmpNode->typ == Array);
			MS_astwalk(tmpNode,list);
		}
	}
}

void HorizontalFission::MS_listwalk(List *l,List *list)
{
	assert(list && l);
	if(stateful) return;
	ListMarker _listwalk_marker; Node *_listwalk_ref; 
	IterateList(&_listwalk_marker, l); 
	while (NextOnList(&_listwalk_marker, (GenericREF)&_listwalk_ref)) { 
		if (_listwalk_ref) 
			MS_astwalk(_listwalk_ref,list);
		SetCurrentOnList(&_listwalk_marker, (Generic *)_listwalk_ref); 
	}
}

void HorizontalFission::MS_astwalk(Node *n,List *list)
{
	assert(list && n);
	if(stateful) return;
	switch ((n)->typ) { 
	 case Const:         break;
	 case Id:            break;
	 case Binop:
		 {
			 if ((n)->u.binop.left) 
			 {
				 if(IsAssignmentOp(n->u.binop.op))
				 {//对赋值操作的左值进行处理
					 IsMutableVar(list,n->u.binop.left);
				 }
				 MS_astwalk((n)->u.binop.left,list);
			 } 
			 if ((n)->u.binop.right) { MS_astwalk((n)->u.binop.right,list); }
			 break; 
		 }
	 case Unary:         
		 {
			 if ((n)->u.unary.expr) 
			 {
				 IsMutableVar(list,n->u.unary.expr);
				  MS_astwalk((n)->u.unary.expr,list);
			 }
			 break; 
		 }
	 case Cast:          if ((n)->u.cast.type) { MS_astwalk((n)->u.cast.type,list);} if ((n)->u.cast.expr) { MS_astwalk((n)->u.cast.expr,list);} break; 
	 case Comma:         if ((n)->u.comma.exprs) { MS_listwalk((n)->u.comma.exprs, list);} break; 
	 case Ternary:       if ((n)->u.ternary.cond) { MS_astwalk((n)->u.ternary.cond,list);} if ((n)->u.ternary.true_) { MS_astwalk((n)->u.ternary.true_,list);} if ((n)->u.ternary.false_) { MS_astwalk((n)->u.ternary.false_,list);} break;
	 case Array:         if ((n)->u.array.name) { MS_astwalk((n)->u.array.name,list);} if ((n)->u.array.dims) { MS_listwalk((n)->u.array.dims, list);} break; 
	 case Call:          if ((n)->u.call.args) { MS_listwalk((n)->u.call.args, list);} break; 
	 case Initializer:   if ((n)->u.initializer.exprs) { MS_listwalk((n)->u.initializer.exprs, list);} break; 
	 case ImplicitCast:  if ((n)->u.implicitcast.expr) { MS_astwalk((n)->u.implicitcast.expr, list);} break; 
	 case Label:         if ((n)->u.label.stmt) { MS_astwalk((n)->u.label.stmt, list);} break; 
	 case Switch:        if ((n)->u.Switch.expr) { MS_astwalk((n)->u.Switch.expr , list);} if ((n)->u.Switch.stmt) { MS_astwalk((n)->u.Switch.stmt, list);} break; 
	 case Case:          if ((n)->u.Case.expr) { MS_astwalk((n)->u.Case.expr , list);} if ((n)->u.Case.stmt) { MS_astwalk((n)->u.Case.stmt, list);} break; 
	 case Default:       if ((n)->u.Default.stmt) { MS_astwalk((n)->u.Default.stmt,list);} break; 
	 case If:            if ((n)->u.If.expr) { MS_astwalk((n)->u.If.expr, list);} if ((n)->u.If.stmt) { MS_astwalk((n)->u.If.stmt,list);} break; 
	 case IfElse:        if ((n)->u.IfElse.expr) { MS_astwalk((n)->u.IfElse.expr,list);} if ((n)->u.IfElse.true_) { MS_astwalk((n)->u.IfElse.true_, list);} if ((n)->u.IfElse.false_) { MS_astwalk((n)->u.IfElse.false_,list);} break;
	 case While:         if ((n)->u.While.expr) { MS_astwalk((n)->u.While.expr,list);} if ((n)->u.While.stmt) { MS_astwalk((n)->u.While.stmt,list);} break; 
	 case Do:            if ((n)->u.Do.stmt) { MS_astwalk((n)->u.Do.stmt, list);} if ((n)->u.Do.expr) { MS_astwalk((n)->u.Do.expr, list);} break; 
	 case For:           if ((n)->u.For.init) { MS_astwalk((n)->u.For.init,list);} if ((n)->u.For.cond) { MS_astwalk((n)->u.For.cond,list);} if ((n)->u.For.next) { MS_astwalk((n)->u.For.next,list);} if ((n)->u.For.stmt) { MS_astwalk((n)->u.For.stmt,list);} break; 
	 case Goto:          break; 
	 case Continue:      break; 
	 case Break:         break; 
	 case Return:        if ((n)->u.Return.expr) { MS_astwalk((n)->u.Return.expr,list);} break; 
	 case Block:         if ((n)->u.Block.decl) { MS_listwalk((n)->u.Block.decl, list);} if ((n)->u.Block.stmts) { MS_listwalk((n)->u.Block.stmts, list);} break; 
	 case Prim:          break; 
	 case Tdef:          break; 
	 case Ptr:           if ((n)->u.ptr.type) { MS_astwalk((n)->u.ptr.type,list);} break; 
	 case Adcl:          if ((n)->u.adcl.type) { MS_astwalk((n)->u.adcl.type,list);} if ((n)->u.adcl.dim) { MS_astwalk((n)->u.adcl.dim,list);} break; 
	 case Fdcl:          break; 
	 case Sdcl:          break; 
	 case Udcl:          break; 
	 case Edcl:          break; 
	 case Decl:          if ((n)->u.decl.type) { MS_astwalk((n)->u.decl.type,list);} if ((n)->u.decl.init) { MS_astwalk((n)->u.decl.init,list);} if ((n)->u.decl.bitsize) { MS_astwalk((n)->u.decl.bitsize,list);} break; 
	 case Attrib:        if (n->u.attrib.arg) { MS_astwalk(n->u.attrib.arg,list);} break; 
	 case Proc:          if ((n)->u.proc.decl) { MS_astwalk((n)->u.proc.decl,list);} if ((n)->u.proc.body) { MS_astwalk((n)->u.proc.body,list);} break; 
	 case Text:          break; 
	 default:            FAIL("Unrecognized node type"); break; 
	}

}
 
//查找划分号为partitionNum中的所有节点(个数大于等于1个)，将节点集合返回给PartitonNumSet(编号->节点)
vector<HorizontalFission::FissionNodeInfo *> HorizontalFission::findfissionNodeSetInPartition(int partitionNum)
{
	vector<FissionNodeInfo *> PartitonNumSet;
	typedef multimap<int,FissionNodeInfo*>::iterator Num2NodeIter;
	pair<Num2NodeIter,Num2NodeIter>range=PartitonNum2FissionNode.equal_range(partitionNum);
	for(Num2NodeIter iter=range.first;iter!=range.second;++iter)
	{
		//cout<<iter->second->name<<" ";
		PartitonNumSet.push_back(iter->second);
	}
	return PartitonNumSet;
}
//求单个划分的工作量总和
int HorizontalFission::computeSumWeightofPlace(int partitionNum)
{
	vector<FissionNodeInfo *> fissionNodeVec;
	typedef multimap<int,FissionNodeInfo*>::iterator Num2NodeIter;
	pair<Num2NodeIter,Num2NodeIter>range=PartitonNum2FissionNode.equal_range(partitionNum);
	for(Num2NodeIter iter=range.first;iter!=range.second;++iter)
	{
		//cout<<iter->second->name<<" ";
		fissionNodeVec.push_back(iter->second);
	}
	int sumWeight = 0;
	for(vector<FissionNodeInfo *>::size_type ix=0;ix!=fissionNodeVec.size();++ix)
	{
		int operatorWork = fissionNodeVec[ix]->operatorWeight; //单个operator的工作量
		int operatorCount = fissionNodeVec[ix]->fissedSteadycount;	 //在稳定状态下operator的执行次数
		sumWeight += operatorWork*operatorCount;
	} 
	return sumWeight;
}
/*计算各个划分的和权值并保存在一个map<划分的编号，权重>*/
map<int,int> HorizontalFission::computeMapPlace2Weight()
{
	int partitions = partition->getParts();//划分的分数
	map<int,int>partition2weight;
	assert(partitions>=0);
	for (int npart = 0;npart<partitions;npart++)
	{
		int sumWeight = computeSumWeightofPlace(npart);
		partition2weight.insert(make_pair(npart,sumWeight));
	}
	return partition2weight;
}
/*按工作量大小进行排序*/
vector<int > HorizontalFission::SortPartitionsByWeight(map<int,int>partition2weight)
{   // 排序
	vector<int> sortPartitions;
	vector<Bool> PartiotionFlag; 
	for (map<int,int>::iterator iter1 = partition2weight.begin();iter1 != partition2weight.end();++iter1)
	{
		PartiotionFlag.push_back(FALSE);
	}
	for (map<int,int>::iterator iter1 = partition2weight.begin();iter1 != partition2weight.end();++iter1)
	{
		map<int,int>::iterator ix = partition2weight.begin();
		while (PartiotionFlag[ix->first]) ix++;
		for(map<int,int>::iterator iter2 = partition2weight.begin();iter2 != partition2weight.end();++iter2)
		{
			if(!PartiotionFlag[iter2->first]&&iter2->second > ix->second) ix = iter2;
		}
		sortPartitions.push_back(ix->first);
		PartiotionFlag[ix->first] = TRUE;
	}
	return sortPartitions;
}
/*找工作量最小的划分*/
int HorizontalFission::MinPartitionWeight(map<int,int>partition2weight)
{
	int minWeight = partition2weight.begin()->second;
	map<int,int>::iterator min_iter = partition2weight.begin();
	for (map<int,int>::iterator iter = partition2weight.begin();iter != partition2weight.end();++iter)
	{
		if(iter->second<minWeight) 
		{
			minWeight = iter->second;
			min_iter = iter;
		}		
	}
	return min_iter->first;

}
 /*partition2weight是有序的，取权值最大的，返回的是权值最大的迭代器*/
int HorizontalFission::NextMaxWeightPartition(map<int,int>partition2weight,vector<Bool> PartiotionFlag)
{
	int maxWeight = 0;
	map<int,int>::iterator max_iter = partition2weight.end();
	for (map<int,int>::iterator iter = partition2weight.begin();iter != partition2weight.end();++iter)
	{
		if(iter->second>maxWeight && PartiotionFlag[iter->first]) 
		{
			maxWeight = iter->second;
			max_iter = iter;
		}		
	}
	if (max_iter !=partition2weight.end())
		return max_iter->first;
	else return partition2weight.size();
}


List *HorizontalFission::ModifyWorkInOutStreamDim(Node *node,List *inputList,List *outputList,Node* inputdelt,Node* outputdelt)
{//node是一个Array节点，他的名字是一个输入流的decl，修改这个数组节点的下标,返回下标组成的List
	assert(node && node->typ == Array );
	assert(node->u.array.name->u.id.decl->u.decl.type->typ == STRdcl);
	Node *inputNode = NULL;
	Node *outputNode = NULL;
	List *dimList = NULL;
	ListMarker input_Maker;
	ListMarker output_Maker;
	Node *decl = node->u.array.name->u.id.decl;
	assert(decl);
	IterateList(&input_Maker,inputList);
	while (NextOnList(&input_Maker,(GenericREF)&inputNode))
	{
		assert(inputNode && inputNode->typ == Decl);
		if(decl == inputNode)
		{
			assert(ListLength(node->u.array.dims)==1);
			Node *dim = (Node *)FirstItem(node->u.array.dims);
			Node *binNode = MakeBinopCoord('+',dim,inputdelt,UnknownCoord);
			dimList = AppendItem(dimList,binNode);
		}
	}
	if(dimList !=NULL) return dimList;
	IterateList(&output_Maker,outputList);
	while (NextOnList(&output_Maker,(GenericREF)&outputNode))
	{
		assert(outputNode && outputNode->typ == Decl);
		if(decl == outputNode)
		{
			assert(ListLength(node->u.array.dims)==1);
			Node *dim = (Node *)FirstItem(node->u.array.dims);
			Node *binNode = MakeBinopCoord('+',dim,outputdelt,UnknownCoord);
			dimList = AppendItem(dimList,binNode);
		}
	}
	assert(dimList);
	return dimList; 
}

void HorizontalFission::MWISD_List(List *l,List *inputlist,List *outputlist,Node *inputDelt,Node *outputDelt)
{
	ListMarker _listwalk_marker; Node *_listwalk_ref; 
	IterateList(&_listwalk_marker, l); 
	while (NextOnList(&_listwalk_marker, (GenericREF)&_listwalk_ref)) { 
		if (_listwalk_ref) 
		{ 
			MWISD_astwalk(_listwalk_ref,inputlist,outputlist,inputDelt,outputDelt);
		}                     
		SetCurrentOnList(&_listwalk_marker, (Generic *)_listwalk_ref); 
	}
}

/*********************************************************************
	*修改work中输入输出流的下标，采用递归遍历的方式
	*主要的思想：1.遍历work函数的内容，当遇到数组类型是需处理
				 2.对于数组类型先判断是不是输入流或输出流，针对不同的流类型修改下标信息
	 参数说明：n：当前处理的节点。inputlist	，outputist 是输入输出流。
			   inputDelt，outputDelt 引用输入输出流是下标值的增加的偏移值
**********************************************************************/
void HorizontalFission::MWISD_astwalk(Node *n,List *inputlist,List *outputlist,Node *inputDelt,Node *outputDelt)
{//修改work中从输入流中取元素的位置
	switch ((n)->typ) { 
	 case Const:         break;
	 case Id:            break;
	 case Binop:		 if ((n)->u.binop.left) {MWISD_astwalk((n)->u.binop.left,inputlist,outputlist,inputDelt,outputDelt);}if ((n)->u.binop.right) {MWISD_astwalk((n)->u.binop.right,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Unary:         if ((n)->u.unary.expr)  {MWISD_astwalk((n)->u.unary.expr,inputlist,outputlist,inputDelt,outputDelt);}	 break; 
	 case Cast:          if ((n)->u.cast.type) {MWISD_astwalk((n)->u.cast.type,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.cast.expr) {MWISD_astwalk((n)->u.cast.expr,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Comma:         if ((n)->u.comma.exprs) {MWISD_List((n)->u.comma.exprs, inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Ternary:       if ((n)->u.ternary.cond) {MWISD_astwalk((n)->u.ternary.cond,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.ternary.true_) {MWISD_astwalk((n)->u.ternary.true_,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.ternary.false_) {MWISD_astwalk((n)->u.ternary.false_,inputlist,outputlist,inputDelt,outputDelt);} break;
	 case Array:        {
						 if ((n)->u.array.name) {MWISD_astwalk((n)->u.array.name,inputlist,outputlist,inputDelt,outputDelt);}
						 if ((n)->u.array.dims) {MWISD_List((n)->u.array.dims, inputlist,outputlist,inputDelt,outputDelt);} 
						 //具体处理
						 if(n->u.array.name->u.id.decl->u.decl.type->typ == STRdcl)//表明数组的名是一个stream
							n->u.array.dims = ModifyWorkInOutStreamDim(n,inputlist,outputlist,inputDelt,outputDelt);
						 break;
						}
	 case Call:          if ((n)->u.call.args) {MWISD_List((n)->u.call.args, inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Initializer:   if ((n)->u.initializer.exprs) {MWISD_List((n)->u.initializer.exprs,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case ImplicitCast:  if ((n)->u.implicitcast.expr) {MWISD_astwalk((n)->u.implicitcast.expr, inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Label:         if ((n)->u.label.stmt) {MWISD_astwalk((n)->u.label.stmt, inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Switch:        if ((n)->u.Switch.expr) {MWISD_astwalk((n)->u.Switch.expr , inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.Switch.stmt) {MWISD_astwalk((n)->u.Switch.stmt, inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Case:          if ((n)->u.Case.expr) {MWISD_astwalk((n)->u.Case.expr ,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.Case.stmt) {MWISD_astwalk((n)->u.Case.stmt, inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Default:       if ((n)->u.Default.stmt) {MWISD_astwalk((n)->u.Default.stmt,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case If:            if ((n)->u.If.expr) {MWISD_astwalk((n)->u.If.expr, inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.If.stmt) {MWISD_astwalk((n)->u.If.stmt,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case IfElse:        if ((n)->u.IfElse.expr) {MWISD_astwalk((n)->u.IfElse.expr,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.IfElse.true_) {MWISD_astwalk((n)->u.IfElse.true_, inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.IfElse.false_) {MWISD_astwalk((n)->u.IfElse.false_,inputlist,outputlist,inputDelt,outputDelt);} break;
	 case While:         if ((n)->u.While.expr) {MWISD_astwalk((n)->u.While.expr,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.While.stmt) {MWISD_astwalk((n)->u.While.stmt,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Do:            if ((n)->u.Do.stmt) {MWISD_astwalk((n)->u.Do.stmt, inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.Do.expr) {MWISD_astwalk((n)->u.Do.expr, inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case For:           if ((n)->u.For.init) {MWISD_astwalk((n)->u.For.init,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.For.cond) {MWISD_astwalk((n)->u.For.cond,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.For.next) {MWISD_astwalk((n)->u.For.next,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.For.stmt) {MWISD_astwalk((n)->u.For.stmt,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Goto:          break; 
	 case Continue:      break; 
	 case Break:         break; 
	 case Return:        if ((n)->u.Return.expr) {MWISD_astwalk((n)->u.Return.expr,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Block:         if ((n)->u.Block.decl) {MWISD_List((n)->u.Block.decl,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.Block.stmts) {MWISD_List((n)->u.Block.stmts,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Prim:          break; 
	 case Tdef:          break; 
	 case Ptr:           if ((n)->u.ptr.type) {MWISD_astwalk((n)->u.ptr.type,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Adcl:          if ((n)->u.adcl.type) {MWISD_astwalk((n)->u.adcl.type,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.adcl.dim) {MWISD_astwalk((n)->u.adcl.dim,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Fdcl:          break; 
	 case Sdcl:          break; 
	 case Udcl:          break; 
	 case Edcl:          break; 
	 case Decl:          if ((n)->u.decl.type) {MWISD_astwalk((n)->u.decl.type,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.decl.init) {MWISD_astwalk((n)->u.decl.init,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.decl.bitsize) {MWISD_astwalk((n)->u.decl.bitsize,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Attrib:        if (n->u.attrib.arg) {MWISD_astwalk(n->u.attrib.arg,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Proc:          if ((n)->u.proc.decl) {MWISD_astwalk((n)->u.proc.decl,inputlist,outputlist,inputDelt,outputDelt);} if ((n)->u.proc.body) {MWISD_astwalk((n)->u.proc.body,inputlist,outputlist,inputDelt,outputDelt);} break; 
	 case Text:          break; 
	 default:            FAIL("Unrecognized node type"); break; 
	}
}
/*修改operator中的window,主要是输入流的peek，pop，输出流的push，这里仅处理单输入单输出的operator的窗口*/
List *HorizontalFission::ModifyWindow(List *inputList,List *outputList,int popValue,int peekValue,int pushValue)
{//不管原来的窗口有没有，现在按照新的值重新构造	 
	assert(inputList);
	assert(outputList);
	assert(ListLength(inputList) == ListLength(outputList) == 1);
	assert(ListLength(inputList)==1);
	Node *inputNode = (Node *)FirstItem(inputList);
	Node *outputNode = (Node *)FirstItem(outputList);
	assert(inputNode && inputNode->typ == Decl);
	assert(outputNode &&inputNode->typ == Decl);
	List *finalWinList = NULL;//用于返回最后修改后的window
	  //输入窗口为空，建一个输入window
	List *outputs = NULL;
	outputs = AppendItem(AppendItem(outputs,MakeConstSint(peekValue)),MakeConstSint(popValue));
	Node *output = MakeCommaCoord(outputs,UnknownCoord);
	Node *sliding = MakeWindowSlidingCoord(EMPTY_TQ,output,UnknownCoord);
	Node *input_winIdNode = MakeNewStreamId(inputNode->u.decl.name,inputNode);
	Node *input_Window = MakeWindowCoord(input_winIdNode,sliding,UnknownCoord); //输入窗口

	Node *tumbling = MakeWindowTumbingCoord(EMPTY_TQ, MakeConstSint(pushValue), UnknownCoord);  //push
	Node *output_winIdNode = MakeNewStreamId(outputNode->u.decl.name,outputNode);
	Node *output_Window = MakeWindowCoord(output_winIdNode,tumbling,UnknownCoord);//输出边的窗口
	finalWinList = AppendItem(finalWinList,input_Window);
	//PrintNode(stdout,input_Window,0);

	finalWinList = AppendItem(finalWinList,output_Window);
	//PrintNode(stdout,output_Window,0);
	return finalWinList;
}
/*修改composite的名字(composite的头)*/
Node *HorizontalFission::ModifyCompositeName(Node *composite)
{
	assert(composite);
	gIsUnfold = TRUE;
	char *compositeName = (char *)malloc(strlen(composite->u.composite.decl->u.decl.name)+20);
	sprintf(compositeName,"%s_%d",composite->u.composite.decl->u.decl.name,++gCurrentFissionCompositeNum);
	Node *compositeNameId = MakeIdCoord(compositeName,UnknownCoord);
	Node *compositeHead =  SetDeclType(ModifyDeclType(ConvertIdToDecl(compositeNameId, EMPTY_TQ, NULL, NULL, NULL), 
		MakeComdclCoord(EMPTY_TQ, composite->u.composite.decl->u.decl.type->u.comdcl.inout, UnknownCoord)),
		MakeDefaultPrimType(EMPTY_TQ, UnknownCoord), 
		Redecl);
	Node *newComposite = DefineComposite(compositeHead);
	newComposite = SetCompositeBody(newComposite,composite->u.composite.body);
	gIsUnfold = FALSE;
	return newComposite;
}

/*************************************************************************
	*根据分裂后的情况构造新的composite,node是原始的composite，
	*inputList，outputList是新产生的composite的输入与输出
	*count是新分裂出的composite的work的执行次数
	*popValue,peekValue,pushValue是新产生的composite的peek，pop，push的值
	*oldpopValue,oldpeekValue,oldpushValue 这个表示原来的operator的pop，peek，push值
	*功能：根据node构造新的composite，composite中graph的operator的work和window要被修改
	*具体做法：1.根据新的输入输出流深拷贝composite。
			   2.为了使新的operator的在稳态时只执行一次，修改work的执行次数将新的执行次数移植到work函数内
			     同时要修改work中引用输入输出流中数据的下标
			   3.修改window中的peek，pop，push的值
**************************************************************************/
Node *HorizontalFission::ConstructNewComposite(Node *node,List *inputList,List *outputList,int count,int popValue,int peekValue,int pushValue,int oldpopValue,int oldpeekValue,int oldpushValue,int deltpeek)
{
	Node *newComposite = NULL;
	assert(node && node->typ == Composite);

	// PrintNode(stdout,node,0);
	assert(ListLength(inputList) == ListLength(outputList) == 1);//这里的composite只是单入单出的
	gIsTransform = TRUE;//进行深拷贝时完成变量替换的开关
	newComposite = compositeCopy(node,inputList,outputList);//根据新的输入输出构造新的composite
	ResetASTSymbolTable(ToTransformDecl);
//newComposite->u.composite.decl->u.decl.name; 
	gIsTransform = FALSE;
	
	assert(newComposite);
	/*修改composite的名字*/
	//newComposite = ModifyCompositeName(newComposite);
	assert(ListLength(newComposite->u.composite.body->u.comBody.comstmts)==1);
	Node *operNode = (Node *)FirstItem(newComposite->u.composite.body->u.comBody.comstmts);//取出composite中的operator
	/*先修改composite中window*/
	List *tmpwinList = ModifyWindow(inputList,outputList,popValue,peekValue,pushValue);

	/*修改work*/
	/*1.根据的参数修改work内对输入输出流数据引用的下标*/
	Node *workNode = operNode->u.operator_.body->u.operBody.work;
	assert(workNode);
	Node *workCount = MakeConstSint(count);

	/*构造控制work循环变量的id节点*/
	Node *workVarDecl = MakeDeclCoord("operator_work",T_BLOCK_DECL,MakePrimCoord(EMPTY_TQ,Sint,UnknownCoord),MakeConstSint(0),NULL,UnknownCoord);//用work作为次数关键字 
	Node *workVarId  = MakeIdCoord("operator_work", UnknownCoord);
	workVarId->u.id.decl = workVarDecl;
	REFERENCE(workVarDecl);
	Node *inputDelt = NULL;
	inputDelt = MakeBinopCoord('*',workVarId,MakeConstSint(oldpopValue),UnknownCoord);
	if(deltpeek) inputDelt = MakeBinopCoord('+',inputDelt,MakeConstSint(deltpeek),UnknownCoord); //duplicate方式deltpeek一般不为0，RoundRobin方式deltpeek=0
	Node *outputDelt = MakeBinopCoord('*',workVarId,MakeConstSint(oldpushValue),UnknownCoord);
	MWISD_astwalk(workNode,inputList,outputList,inputDelt,outputDelt);

	/*2.修改work函数的函数体的执行次数(在work函数的body外包一个循环)*/
	Node *forNode = NULL;
	Node *forInit = MakeBinopCoord('=',workVarId,MakeConstSint(0),UnknownCoord);
	Node *forCond = MakeBinopCoord('<',workVarId,MakeConstSint(count),UnknownCoord);
	Node *forNext = MakeUnaryCoord(POSTINC,workVarId,UnknownCoord);
	Node *forStmt = workNode;
	forNode = MakeForCoord(forInit,forCond,forNext,forStmt,UnknownCoord);
	List *newWorkDeclList = NULL;
	newWorkDeclList = AppendItem(newWorkDeclList,workVarDecl);
	List *newWorkStmtsList = NULL;
	newWorkStmtsList = AppendItem(newWorkStmtsList,forNode);
	Node *newWorkBlock = MakeBlockCoord(PrimVoid,newWorkDeclList,newWorkStmtsList,UnknownCoord,UnknownCoord);

	operNode->u.operator_.body->u.operBody.work = newWorkBlock;
	operNode->u.operator_.body->u.operBody.window = tmpwinList;
	newComposite->u.composite.body->u.comBody.comstmts = MakeNewList(operNode);
	
	//PrintNode(stdout,newComposite,0);	
	// cout<<endl<<"###########################################"<<endl; 
	return newComposite;  
}


List *HorizontalFission::MakeSplitOutputStream(Node *input,int count)
{/*构造split的输出边，input是split的输入，count是输入边的数目*/
	assert(input);
	assert(input->typ == Decl || input->typ == Id);
	List *outputList = NULL;
	Node *input_Decl = NULL;
	if(input->typ == Id) input_Decl = input->u.id.decl;
	else input_Decl = input;
	char *newName = NULL;//输出流的名字
	assert(input_Decl&&input_Decl->typ == Decl);
	Node *streamType = input_Decl->u.decl.type;
	for (int i =0;i < count ; i++)
	{
		newName = (char *) malloc(strlen(input_Decl->u.decl.name)+20);
		sprintf(newName, "%s%d_%d", input_Decl->u.decl.name, ++gCurrentFissionCompositeNum, i);
		Node *newStream = MakeNewStream(newName,streamType);
		assert(newStream && newStream ->typ ==Decl);
		outputList = AppendItem(outputList, newStream);
	}
	return outputList;
}
/*根据输入输出流以及pop，peek，push的值构建窗口，主要是为了构建split和join的窗口*/
List *HorizontalFission::MakeDuplicateWindow(List *inputList,List *outputList,Node *pop_value)
{
	assert(ListLength(inputList) == 1 && ListLength(outputList)>1);
	List *windowList = NULL;
	ListMarker output_maker;
	ListMarker push_maker;
	Node *outputNode = NULL;
	/*输出窗口*/
	IterateList(&output_maker,outputList);
	while (NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		assert(outputNode->typ == Id ||outputNode->typ == Decl);
		Node *outputId = NULL;
		if (outputNode->typ == Id)
			outputId = MakeNewStreamId(outputNode->u.id.text,outputNode->u.id.decl);
		else 
			outputId = MakeNewStreamId(outputNode->u.decl.name,outputNode);
		windowList = AppendItem(windowList,MakeMyWindow(outputId,pop_value,1));	//输出边的窗口
	}

	/*输入窗口*/
	Node *inputNode = (Node *)FirstItem(inputList);
	assert(inputNode->typ == Id || inputNode->typ == Decl);
	Node *inputId = NULL;

	if(inputNode->typ ==Id) inputId = MakeNewStreamId(inputNode->u.id.text,inputNode->u.id.decl);
	else inputId = MakeNewStreamId(inputNode->u.decl.name,inputNode);
	windowList =AppendItem(windowList,MakeMyWindow(inputId,pop_value,0));
	return windowList;
}

List *HorizontalFission::MakeRoundRobinWindow(List *inputList,List *outputList,List *push_arg)
{
	assert(ListLength(inputList) == 1 && ListLength(outputList)>1&&ListLength(outputList) == ListLength(push_arg));
	List *windowList = NULL;
	ListMarker output_maker;
	ListMarker push_maker;
	Node *outputNode = NULL;
	Node *push_value = NULL;
	int pop_value =0;
	/*输出窗口*/
	IterateList(&output_maker,outputList);
	IterateList(&push_maker,push_arg);
	while (NextOnList(&output_maker,(GenericREF)&outputNode) && NextOnList(&push_maker,(GenericREF)&push_value))
	{
		assert(outputNode&&push_value);
		assert(outputNode->typ == Id ||outputNode->typ == Decl);
		pop_value += NodeConstantIntegralValue(push_value);
		Node *outputId = NULL;
		if (outputNode->typ == Id)
			outputId = MakeNewStreamId(outputNode->u.id.text,outputNode->u.id.decl);
		else 
			outputId = MakeNewStreamId(outputNode->u.decl.name,outputNode);
		windowList = AppendItem(windowList,MakeMyWindow(outputId,push_value,1));	//输出边的窗口
	}
	/*输入窗口*/
	Node *inputNode = (Node *)FirstItem(inputList);
	assert(inputNode->typ == Id || inputNode->typ == Decl);
	Node *inputId = NULL;

	if(inputNode->typ ==Id) inputId = MakeNewStreamId(inputNode->u.id.text,inputNode->u.id.decl);
	else inputId = MakeNewStreamId(inputNode->u.decl.name,inputNode);
	windowList =AppendItem(windowList,MakeMyWindow(inputId,MakeConstSint(pop_value),0));
	return windowList;
}

List *HorizontalFission::MakeJoinWindow(List *inputList,List *outputList,List *pop_arg)
{
	assert(ListLength(outputList) == 1 && ListLength(inputList)>1&&ListLength(inputList) == ListLength(pop_arg));
	List *windowList = NULL;
	ListMarker input_maker;
	ListMarker pop_maker;
	Node *inputNode = NULL;
	Node *pop_value = NULL;
	int push_value =0;
	/*输入窗口*/
	IterateList(&input_maker,inputList);
	IterateList(&pop_maker,pop_arg);
	while (NextOnList(&input_maker,(GenericREF)&inputNode) && NextOnList(&pop_maker,(GenericREF)&pop_value))
	{
		assert(inputNode&&pop_value);
		assert(inputNode->typ == Id ||inputNode->typ == Decl);
		push_value += NodeConstantIntegralValue(pop_value);
		Node *inputId = NULL;
		if (inputNode->typ == Id)
			inputId = MakeNewStreamId(inputNode->u.id.text,inputNode->u.id.decl);
		else 
			inputId = MakeNewStreamId(inputNode->u.decl.name,inputNode);
		windowList = AppendItem(windowList,MakeMyWindow(inputId,pop_value,0));	//输入边的窗口
	}

	/*输出窗口*/
	Node *outputNode = (Node *)FirstItem(outputList);
	assert(outputNode->typ == Id || outputNode->typ == Decl);
	Node *outputId = NULL;

	if(outputNode->typ ==Id) outputId = MakeNewStreamId(outputNode->u.id.text,outputNode->u.id.decl);
	else outputId = MakeNewStreamId(outputNode->u.decl.name,inputNode);
	windowList =AppendItem(windowList,MakeMyWindow(outputId,MakeConstSint(push_value),1));   //输出
	return windowList;
}

Node *HorizontalFission::MakeFissionSplitComposite(Node *inputNode,List *outputList, List *pop_arg,SplitStyle style)
{
	Node *composite = NULL;

	List *composite_inputList = NULL;
	List *composite_outputList = NULL;
	List *oper_inputList = NULL;
	List *oper_outputList = NULL;
	ListMarker output_maker;
	Node *outputNode;
	/*使composite的输入输出全部是Decl类型，而operator的输入输出全部是Id类型*/
	assert(inputNode);
	assert(inputNode->typ == Decl || inputNode->typ == Id);
	if (inputNode->typ == Decl)
	{
		composite_inputList = AppendItem(composite_inputList,inputNode);
		oper_inputList = AppendItem(oper_inputList,MakeNewStreamId(inputNode->u.decl.name,inputNode));
	}
	else 
	{
		composite_inputList = AppendItem(composite_inputList,inputNode->u.id.decl);
		oper_inputList = AppendItem(oper_inputList , inputNode);
	}
	IterateList(&output_maker,outputList);
	while (NextOnList(&output_maker ,(GenericREF)&outputNode))
	{
		assert(outputNode);
		assert(outputNode->typ == Decl || outputNode->typ == Id);
		if(outputNode->typ == Decl)
		{
			composite_outputList = AppendItem(composite_outputList,outputNode);
			oper_outputList = AppendItem(oper_outputList,MakeNewStreamId(outputNode->u.decl.name,outputNode));
		}
		else
		{
			composite_outputList = AppendItem(composite_outputList,outputNode->u.id.decl);
			oper_outputList = AppendItem(oper_outputList , outputNode);
		}
	}
	/*输入输出*/
	Node *com_InOutNode = MakeComInOutCoord(EMPTY_TQ,composite_inputList,composite_outputList,UnknownCoord);

	/*composite的头*/
	char *newName = (char *)malloc(sizeof(char)*50); // 能表示的最大整数应该不超过20位
	if(style == S_Duplicate) sprintf(newName, "%s_%s_%d", "Duplicate","fission",++gCurrentFissionCompositeNum);
	else sprintf(newName, "%s_%s_%d", "RoundRobin","fission",++gCurrentFissionCompositeNum);
	Node *id = MakeIdCoord(newName, UnknownCoord);
	Node *composite_Decl = SetDeclType(ModifyDeclType(ConvertIdToDecl(id, EMPTY_TQ, NULL, NULL, NULL), MakeComdclCoord(EMPTY_TQ, com_InOutNode, UnknownCoord)),
		MakeDefaultPrimType(EMPTY_TQ, UnknownCoord), 
		Redecl);
	
	gIsUnfold = TRUE;
	composite = DefineComposite(composite_Decl);

	Node *operNode = NULL;
	if(style == S_Duplicate) operNode = MakeFissionDuplicate((Node *)FirstItem(oper_inputList),oper_outputList,(Node *)FirstItem(pop_arg));
	else operNode = MakeFissionRoundRobin((Node *)FirstItem(oper_inputList),oper_outputList,pop_arg);
	List *operatorList = NULL;
	operatorList = AppendItem(operatorList,operNode);
	Node *composite_Body = MakeComBodyCoord(PrimVoid, NULL, NULL, operatorList, UnknownCoord, UnknownCoord);
	composite = SetCompositeBody(composite, composite_Body); 
	gIsUnfold = FALSE;
	return composite;
}

Node *HorizontalFission::MakeFissionJoinComposite(List *inputList,Node *outputNode, List *pop_arg)
{
	Node *composite = NULL;

	List *composite_inputList = NULL;
	List *composite_outputList = NULL;
	List *oper_inputList = NULL;
	List *oper_outputList = NULL;
	ListMarker input_maker;
	Node *inputNode;
	/*使composite的输入输出全部是Decl类型，而operator的输入输出全部是Id类型*/
	assert(outputNode);
	assert(outputNode->typ == Decl || outputNode->typ == Id);
	if (outputNode->typ == Decl)
	{
		composite_outputList = AppendItem(composite_outputList,outputNode);
		oper_outputList = AppendItem(oper_outputList,MakeNewStreamId(outputNode->u.decl.name,outputNode));
	}
	else 
	{
		composite_outputList = AppendItem(composite_outputList,outputNode->u.id.decl);
		oper_outputList = AppendItem(oper_outputList , outputNode);
	}
	IterateList(&input_maker,inputList);
	while (NextOnList(&input_maker ,(GenericREF)&inputNode))
	{
		assert(inputNode);
		assert(inputNode->typ == Decl || inputNode->typ == Id);
		if(inputNode->typ == Decl)
		{
			composite_inputList = AppendItem(composite_inputList,inputNode);
			oper_inputList = AppendItem(oper_inputList,MakeNewStreamId(inputNode->u.decl.name,inputNode));
		}
		else
		{
			composite_inputList = AppendItem(composite_inputList,inputNode->u.id.decl);
			oper_inputList = AppendItem(oper_inputList , inputNode);
		}
	}
	/*输入输出*/
	Node *com_InOutNode = MakeComInOutCoord(EMPTY_TQ,composite_inputList,composite_outputList,UnknownCoord);

	/*composite的头*/
	char *newName = (char *)malloc(sizeof(char) + 50); // 能表示的最大整数应该不超过20位
	sprintf(newName, "%s_%s_%d", "Join","fission",++gCurrentFissionCompositeNum);
	Node *id = MakeIdCoord(newName, UnknownCoord);
	Node *composite_Decl = SetDeclType(ModifyDeclType(ConvertIdToDecl(id, EMPTY_TQ, NULL, NULL, NULL), MakeComdclCoord(EMPTY_TQ, com_InOutNode, UnknownCoord)),
		MakeDefaultPrimType(EMPTY_TQ, UnknownCoord), 
		Redecl);
	gIsUnfold = TRUE;
	composite = DefineComposite(composite_Decl);

	Node *operNode = NULL;
	operNode = MakeFissionJoin(oper_inputList,outputNode,pop_arg);
	List *operatorList = NULL;
	operatorList = AppendItem(operatorList,operNode);
	Node *composite_Body = MakeComBodyCoord(PrimVoid, NULL, NULL, operatorList, UnknownCoord, UnknownCoord);
	composite = SetCompositeBody(composite, composite_Body);
	gIsUnfold = FALSE;
	return composite;

}


Node *HorizontalFission::MakeFissionDuplicate(Node *inputNode,List *outputList, Node *pop_value)
{//构造出的节点也是一个composite,返回是composite节点
	assert(inputNode && ListLength(outputList)>1);//表明新建的split节点至少有2个输出边	
	Node *dupOperNode = NULL;//存储operator类型的节点
	//Node *dupComNode = NULL;//存放的是Composite类型的节点（最终的返回值）
	
	List *tmp_inputList = NULL;
	tmp_inputList = AppendItem(tmp_inputList,inputNode);
	char *operName = (char *)malloc(40);//名字的长度
	sprintf(operName,"%s_%d","Duplicate_fiss",++gCurrentFissionCompositeNum);
	Node *dupOperHead = MakeOperatorHead(operName,outputList,tmp_inputList); //operator的头
	dupOperNode = DefineOperator(dupOperHead);
	dupOperNode->u.operator_.ot = Duplicate_ ;
	
	/*根据输入输出，以及duplicate的参数构造splitoperator的Body节点*/
	Node *dupWorkNode = MakeDuplicateWork(outputList,inputNode,pop_value);//构造operator的work函数
	List *dupWindowList = MakeDuplicateWindow(tmp_inputList,outputList,pop_value);
	Node *dupOperBodyNode = MakeOperatorBody(dupWorkNode,dupWindowList);//operator的body

	dupOperNode = SetOperatorBody(dupOperNode,dupOperBodyNode);//构造operator
	/*将operator转换成Composite*/
	//dupComNode = MakeCompositeNode(dupOperNode);
	return dupOperNode;	
}

Node *HorizontalFission::MakeFissionRoundRobin(Node *inputNode,List *outputList, List *pop_arg)
{//构造出的节点也是一个composite,返回是composite节点
	assert(inputNode && ListLength(outputList)>1);//表明新建的split节点至少有2个输入边
	assert(ListLength(outputList) == ListLength(pop_arg));	
	Node *rouOperNode = NULL;//存储operator类型的节点
	//Node *rouComNode = NULL;//存放的是Composite类型的节点（最终的返回值）
	/*根据输入输出，以及duplicate的参数构造splitoperator节点*/
	List *tmp_inputList = NULL;
	tmp_inputList = AppendItem(tmp_inputList,inputNode);
	
	char *operName = (char *)malloc(40);//名字的长度
	sprintf(operName,"%s_%d","RoundRobin_fiss",++gCurrentFissionCompositeNum);
	Node *rouOperHead = MakeOperatorHead(operName,outputList,tmp_inputList); //operator的头
	rouOperNode = DefineOperator(rouOperHead);
	rouOperNode->u.operator_.ot = Roundrobin_ ;
	Node *rouWorkNode = MakeRoundrobinWork(outputList,inputNode,pop_arg);//构造operator的work函数
	List *rouWindowList = MakeRoundRobinWindow(tmp_inputList,outputList,pop_arg);
	Node *rouOperBodyNode = MakeOperatorBody(rouWorkNode,rouWindowList);//operator的body
	rouOperNode = SetOperatorBody(rouOperNode,rouOperBodyNode);//构造operator
	/*将operator转换成Composite*/
	//rouComNode = MakeCompositeNode(rouOperNode);
	return rouOperNode;	

}
/*构造join节点,输入内容是输入边（多条），输出边（一条），每一条边向每一条边发送数据的数目*/
Node *HorizontalFission::MakeFissionJoin(List *inputList,Node *outputNode, List *pop_arg)
{  //构造出的节点也是一个composite
	assert(outputNode && ListLength(inputList)>1);//表明新建的split节点至少有2个输入边
	assert(ListLength(inputList) == ListLength(pop_arg));	
	Node *joinOperNode = NULL;//存储operator类型的节点
	//Node *joinComNode = NULL;//存放的是Composite类型的节点（最终的返回值）
	/*根据输入输出，以及duplicate的参数构造splitoperator节点*/
	List *tmp_outputList = NULL;
	Node *tmp_outputNode = NULL;

	assert(outputNode && (outputNode->typ == Id || outputNode->typ == Decl) );

	if(outputNode->typ == Decl) tmp_outputNode = MakeNewStreamId(outputNode->u.decl.name,outputNode);
	else tmp_outputNode = outputNode;

	tmp_outputList = AppendItem(tmp_outputList,tmp_outputNode); 
	char *operName = (char *)malloc(40);//名字的长度
	sprintf(operName,"%s_%d","Join_Fiss",++gCurrentFissionCompositeNum);
	Node *rouOperHead = MakeOperatorHead(operName,tmp_outputList,inputList); //operator的头
	joinOperNode = DefineOperator(rouOperHead);
	joinOperNode->u.operator_.ot = Join_ ;		
	Node *joinWorkNode = MakeJoinWork(tmp_outputNode,inputList,pop_arg);//构造operator的work函数
	List *joinWindowList = MakeJoinWindow(inputList,tmp_outputList,pop_arg);
	Node *joinOperBodyNode = MakeOperatorBody(joinWorkNode,joinWindowList);//operator的body
	joinOperNode = SetOperatorBody(joinOperNode,joinOperBodyNode);//构造operator
	return joinOperNode;
}


/**********************************************************************************
	* 功能：将一个operator复制分裂成多个operator并组成一个List,List的成员是Node*
			这个List中不包含split和join节点
			注意命名（opername_fission_splijoin_编号）
    * flatnode：需要分裂的节点
	vector<FissionNodeInfo *> splitedNodeVec 记录分裂后生成的每个节点的信息
	* inputList: 分裂后每一个节点的实际的输出
    * steadyCount：稳定状态执行次数
    * style：分裂的方式（roundrobin方式，duplicate方式）
*******************************************************************************/

List *HorizontalFission::MakeReplicateOper(vector<FissionNodeInfo *> splitedNodeVec,FlatNode *flatnode, List *inputList, int steadyCount,SplitStyle style)
{
	operatorNode *oper = flatnode->contents;//将要被分裂的operator
	int popweight = flatnode->inPopWeights[0];//取当前opertor的pop值
	int pushweight = flatnode->outPushWeights[0];//取当前opertor的push值
	int peekweight = flatnode->inPeekWeights[0];//取当前opertor的peek值
	
	/*将operatorNode*改造成一个Node*节点*/
	Node *operNode = NewNode(Operator_);
	operNode->u.operator_.decl= oper->decl;
	operNode->u.operator_.body= oper->body;
	operNode->u.operator_.ot = oper->ot;
	List *fissionOperList = NULL;//分裂后operator组成的list

	Node *compositeNode = NULL;	//将原始的operator构造成一个composite结构
	compositeNode = MakeCompositeNode(operNode);//将原来的operNode构造成一个composite

	/*为所有新生成的节点构造输出边*/
	List *comp_outputDecl = compositeNode->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.outputs;
	assert(ListLength(comp_outputDecl) == 1);
	Node *output_Decl = (Node *)FirstItem(comp_outputDecl);
	List *outputList = NULL;
	Node *streamType = output_Decl->u.decl.type;
	char *newName = NULL;
	for (int i =0;i < splitedNodeVec.size() ; i++)
	{
		newName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
		sprintf(newName, "%s%d_%d", output_Decl->u.decl.name, ++gCurrentFissionCompositeNum, i);
		Node *newStream = MakeNewStream(newName,streamType);
		assert(newStream && newStream->typ == Decl);
		outputList = AppendItem(outputList, newStream);
	}
	assert(ListLength(outputList) == ListLength(inputList));
	
	int deltpeek = 0;
	/*根据不同的分裂方式分裂，style = 0 是roundrobin方式，style = 1 是Duplicate方式*/
	if(style == S_Duplicate )
	{  //duplicate方式——要修改operator中work和window中的代码(主要是修改peek)
		for (int i= 0;i < splitedNodeVec.size(); i++)
		{
			if(inputList == NULL && outputList ==NULL) break;
			assert(inputList && outputList);
			List *new_outputList =NULL;
			List *new_inputList = NULL;
			new_inputList = AppendItem(new_inputList,FirstItem(inputList));
			new_outputList= AppendItem(new_outputList,FirstItem(outputList));
			if(i!=0)deltpeek += popweight * splitedNodeVec[i-1]->fissedSteadycount;
			else deltpeek=0;
											//ConstructNewComposite参数说明：模板composite，新的输入输出流，当前复制出的composite在稳态时的执行次数，然后是执行次数，pop，peek，push值
			fissionOperList = AppendItem(fissionOperList,ConstructNewComposite(compositeNode,new_inputList,new_outputList,splitedNodeVec[i]->fissedSteadycount,splitedNodeVec[i]->fissed_pop[0],splitedNodeVec[i]->fissed_peek[0],splitedNodeVec[i]->fissed_push[0],popweight,peekweight,pushweight,deltpeek));
			inputList = GetNextList(inputList);
			outputList = GetNextList(outputList);
		}
	}
	else
	{  //roundrobin方式
		for(int i=0;i<splitedNodeVec.size();i++)
		{  //operator节点中的work以及window均要做相应的修改（稳态时work只执行一次，具体的稳态执行次数在work内体现）
			if(inputList == NULL && outputList ==NULL) break;
			assert(inputList && outputList);
			List *new_outputList =NULL;
			List *new_inputList = NULL;
			deltpeek = 0;
			new_inputList = AppendItem(new_inputList,FirstItem(inputList));
			new_outputList= AppendItem(new_outputList,FirstItem(outputList));
			fissionOperList = AppendItem(fissionOperList,ConstructNewComposite(compositeNode,new_inputList,new_outputList,splitedNodeVec[i]->fissedSteadycount,splitedNodeVec[i]->fissed_pop[0],splitedNodeVec[i]->fissed_peek[0],splitedNodeVec[i]->fissed_push[0],popweight,peekweight,pushweight,deltpeek));
			inputList = GetNextList(inputList);
			outputList = GetNextList(outputList);
		}
	}	
	return fissionOperList;
}


 List *HorizontalFission::horizontalFissOperator(vector<FissionNodeInfo *> splitedNodeVec,SchedulerSSG *sssg)
{ 
	List *fissionOperList = NULL;
	assert(splitedNodeVec.size()>=2);
	FlatNode * flatNode = splitedNodeVec[0]->fissingFlatNode;
	assert(flatNode);
	operatorNode *operNode =flatNode->contents; //将要被分裂的operator
	int steadyCount = sssg->GetSteadyCount(flatNode);  //取该节点的在稳态调度中的执行次数

	List *inputList=operNode->decl->u.decl.type->u.operdcl.inputs;  //待分裂operator的输入
	List *outputList = operNode->decl->u.decl.type->u.operdcl.outputs; //待分裂operator的输出

	assert(ListLength(inputList)==1 && ListLength(outputList)==1);	//暂时只对单输入单输出的节点进行复制分裂

	int outPushWeight =	ListLength(outputList) ? flatNode->outPushWeights[0] : 0;//operNode的push值	,如果没有输出边则push = 0；
	int	inPopWeight = ListLength(inputList) ? flatNode->inPopWeights[0] : 0;//operNode的pop值
	int inPeekWeight = ListLength(inputList) ? flatNode->inPeekWeights[0] : 0;//operNode的peek值

	Node *splitNode = NULL;//新产生的split节点
	Node *joinNode = NULL;	//新产生的join节点
	/*做split节点*/
	SplitStyle style = (inPeekWeight > inPopWeight)? S_Duplicate: S_RoundRobin;  //peek>pop取1表示duplicate方式，peek=pop取0，表示roundrobin方式
	Node *input =(Node *)FirstItem(inputList);
	Node *output = (Node *)FirstItem(outputList);
	List *split_argument = NULL;//确定split节点的输出边的push值
	/*做复制分裂后的operator 的List*/
	List *split_outputList = NULL;//记录split节点的所有输出边，作为新建的operator的输入
	List *replicateOperList = NULL;   //operNode复制分裂出的新的operator,List中的元素是operatorNode*

	/*构造replicationFactor条边作为split的输出边*/
	split_outputList = MakeSplitOutputStream(input,splitedNodeVec.size());//构造split节点的所有输出边

	/*构造不同类型的split节点*/
	if(style == S_Duplicate)
	{  //计算duplicate后面的参数
		int split_pop = inPopWeight * steadyCount;
		Node *duplicate_value =MakeConstSint(split_pop);
		split_argument = AppendItem(split_argument,duplicate_value);
		splitNode = MakeFissionSplitComposite(input,split_outputList,split_argument,style);
	}
	else 
	{  //计算roundrobin后面的参数
		for (int i=0; i < splitedNodeVec.size(); i++)
			split_argument = AppendItem(split_argument,MakeConstSint(inPopWeight*(splitedNodeVec[i]->fissedSteadycount)));		
		splitNode = MakeFissionSplitComposite(input,split_outputList,split_argument,style);
	}
	
	List *join_inputList = NULL;
	List *join_argument = NULL;

	replicateOperList = MakeReplicateOper(splitedNodeVec,flatNode,split_outputList,steadyCount,style);

	ListMarker fissionOper_maker;
	Node *fissionOper = NULL;
	/*取每一个新的operator的输出作为join的输入*/
	IterateList(&fissionOper_maker,replicateOperList);   /*replicateOperList组成的链*/
	while (NextOnList(&fissionOper_maker,(GenericREF)&fissionOper))
	{
		join_inputList = AppendItem(join_inputList,(Node *)FirstItem(fissionOper->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.outputs));
	}
   
	/*做join结点方式是roundrobin*/ 
	for (int i=0; i < splitedNodeVec.size(); i++)
		join_argument = AppendItem(join_argument,MakeConstSint(outPushWeight*(splitedNodeVec[i]->fissedSteadycount)));
	joinNode = MakeFissionJoinComposite(join_inputList,output,join_argument);

	assert(splitNode && joinNode && replicateOperList);
	fissionOperList = AppendItem(fissionOperList,splitNode);//将新产生的operator全部链接到一个List中
	fissionOperList = JoinLists(fissionOperList,replicateOperList);
	fissionOperList = AppendItem(fissionOperList,joinNode);
	return fissionOperList;
}

 /*将flatNode进行伪分裂，主要是在PartitonNum2FissionNode和FlatNode2FissionNode中进行相关信息的修改*/
 void HorizontalFission::SplitFissionNode(FissionNodeInfo *fissionNode,int replicationFactor,int max_partiotion,int min_partiotion)
 {
	 assert(fissionNode && replicationFactor>0 );
	 int steadyCount = fissionNode->fissedSteadycount;  //取该节点的在稳态调度中的执行次数
	 assert(steadyCount>0);

	 if(replicationFactor > steadyCount) replicationFactor = steadyCount;//如果分裂次数大于稳态执行次数，则分裂次数取执行次数
	 assert(fissionNode->fissed_pop.size()<=1 && fissionNode->fissed_push.size()<=1 && fissionNode->fissed_peek.size()<=1);//确定是单入单出的


	 int outPushWeight =fissionNode->fissingFlatNode->outPushWeights.size()==1? fissionNode->fissingFlatNode->outPushWeights[0] : 0;//operNode的push值	,如果没有输出边则push = 0；
	 int inPopWeight =fissionNode->fissingFlatNode->inPopWeights.size()==1? fissionNode->fissingFlatNode->inPopWeights[0] : 0;//operNode的pop值
	 int inPeekWeight = fissionNode->fissingFlatNode->inPeekWeights.size()==1 ? fissionNode->fissingFlatNode->inPeekWeights[0] : 0;//operNode的peek值
	 /*确定采用的分裂方式*/
	 SplitStyle style = (inPeekWeight > inPopWeight)? S_Duplicate: S_RoundRobin;  //peek>pop取1表示duplicate方式，peek=pop取0，表示roundrobin方式

	 /*计算分裂产生的每一分在稳态时执行的次数*/
	 vector<int> newSteadyCount;//存放分裂后的每一份在稳态执行的次数
	 for(int i = 0;i<replicationFactor-1;i++)
		 newSteadyCount.push_back(steadyCount/replicationFactor);
	 if(steadyCount%replicationFactor==0) newSteadyCount.push_back(steadyCount/replicationFactor);
	 else newSteadyCount.push_back(steadyCount - steadyCount/replicationFactor*(replicationFactor-1));
	
	 //根据不同的分裂方式确定各个副本的相关参数的值
	 if(style == S_Duplicate)
	 {  //计算duplicate后面的参数
		// int deltpeek = 0;
		 /*第一个副本放到工作量最小的划分中*/
		 FissionNodeInfo * firstFissionNode = HeapNew(FissionNodeInfo);
		 firstFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
		 firstFissionNode->fissedSteadycount = newSteadyCount[0];
		 firstFissionNode->operatorWeight = fissionNode->operatorWeight;
		 firstFissionNode->fissed_peek.push_back(inPeekWeight+inPopWeight*(steadyCount-1));//计算一个副本一次稳态执行总共需要peek的值
		 firstFissionNode->fissed_pop.push_back(inPopWeight*steadyCount);//计算一个副本一次稳态执行总共需要pop的值
		 firstFissionNode->fissed_push.push_back(outPushWeight*newSteadyCount[0]);//计算一个副本一次稳态执行总共需要push的值
		 firstFissionNode->npart = min_partiotion;
		 /*设定分裂后分裂节点的fiss_state的值*/
		 if( firstFissionNode->fissed_pop[0]==firstFissionNode->fissed_peek[0]) firstFissionNode->fiss_state = TRUE;//不需要将稳态执行次数移到work函数内部(peek=pop才可以)
		 else firstFissionNode->fiss_state = FALSE;
		 PartitonNum2FissionNode.insert(make_pair(min_partiotion,firstFissionNode));
		 FlatNode2FissionNode.insert(make_pair(fissionNode->fissingFlatNode,firstFissionNode));
		 /*除第一个以外所有的副本放在待分裂节点自身所在的划分中*/
		 for(int i=1; i < replicationFactor; i++)
		 {
			 FissionNodeInfo * tmpFissionNode = HeapNew(FissionNodeInfo);
			// deltpeek += inPopWeight * newSteadyCount[i-1];
			 tmpFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
			 tmpFissionNode->fissedSteadycount = newSteadyCount[i];
			 tmpFissionNode->operatorWeight = fissionNode->operatorWeight;
			 tmpFissionNode->fissed_peek.push_back(inPeekWeight+inPopWeight*(steadyCount-1));//计算一个副本一次稳态执行总共需要peek的值
			 tmpFissionNode->fissed_pop.push_back(inPopWeight*steadyCount);//计算一个副本一次稳态执行总共需要pop的值
			 tmpFissionNode->fissed_push.push_back(outPushWeight*newSteadyCount[i]);//计算一个副本一次稳态执行总共需要push的值
			 tmpFissionNode->npart = max_partiotion;
			 /*设定分裂后分裂节点的fiss_state的值*/
			 if(tmpFissionNode->fissed_pop[0]==tmpFissionNode->fissed_peek[0]) tmpFissionNode->fiss_state = TRUE;//不需要将稳态执行次数移到work函数内部
			 else tmpFissionNode->fiss_state = FALSE;
			 PartitonNum2FissionNode.insert(make_pair(max_partiotion,tmpFissionNode));
			 FlatNode2FissionNode.insert(make_pair(fissionNode->fissingFlatNode,tmpFissionNode));
		 }
		 /*将fissionNode从PartitonNum2FissionNode，FlatNode2FissionNode中删除*/
		 multimap<int,FissionNodeInfo*>::iterator iter;
		 for(iter = PartitonNum2FissionNode.begin();iter != PartitonNum2FissionNode.end();++iter)
		 {
			 if(iter->second == fissionNode) break;
		 }
		 assert(iter!=  PartitonNum2FissionNode.end());
		 PartitonNum2FissionNode.erase(iter);
		 multimap<FlatNode* ,FissionNodeInfo *>::iterator iter2;
		 for(iter2 = FlatNode2FissionNode.begin();iter2 != FlatNode2FissionNode.end();++iter2)
		 {
			 if(iter2->second == fissionNode) break;
		 }
		 if(iter2!=FlatNode2FissionNode.end()) FlatNode2FissionNode.erase(iter2);
	 }
	 else 
	 {  //计算roundrobin后面的参数
		 /*第一个副本放到工作量最小的划分中*/
		 FissionNodeInfo * firstFissionNode = HeapNew(FissionNodeInfo);
		 firstFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
		 firstFissionNode->fissedSteadycount = newSteadyCount[0];
		 firstFissionNode->operatorWeight = fissionNode->operatorWeight;
		 firstFissionNode->fissed_peek.push_back(newSteadyCount[0]*inPeekWeight);//计算一个副本一次稳态执行总共需要peek的值
		 firstFissionNode->fissed_pop.push_back(inPopWeight*newSteadyCount[0]);//计算一个副本一次稳态执行总共需要pop的值
		 firstFissionNode->fissed_push.push_back(outPushWeight*newSteadyCount[0]);//计算一个副本一次稳态执行总共需要push的值
		 firstFissionNode->npart = min_partiotion;
		 /*设定分裂后分裂节点的fiss_state的值*/
		 if(firstFissionNode->fissed_pop[0]==firstFissionNode->fissed_peek[0]) firstFissionNode->fiss_state = TRUE;//不需要将稳态执行次数移到work函数内部
		 else firstFissionNode->fiss_state = FALSE;
		 PartitonNum2FissionNode.insert(make_pair(min_partiotion,firstFissionNode));
		 FlatNode2FissionNode.insert(make_pair(fissionNode->fissingFlatNode,firstFissionNode));
		 /*除第一个以外所有的副本放在待分裂节点自身所在的划分中*/
		 for(int i=1; i < replicationFactor; i++)
		 {
			 FissionNodeInfo * tmpFissionNode = HeapNew(FissionNodeInfo);
			 tmpFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
			 tmpFissionNode->fissedSteadycount = newSteadyCount[i];
			 tmpFissionNode->operatorWeight = fissionNode->operatorWeight;
			 tmpFissionNode->fissed_peek.push_back(newSteadyCount[i]*inPeekWeight);//计算一个副本一次稳态执行总共需要peek的值
			 tmpFissionNode->fissed_pop.push_back(inPopWeight*newSteadyCount[i]);//计算一个副本一次稳态执行总共需要pop的值
			 tmpFissionNode->fissed_push.push_back(outPushWeight*newSteadyCount[i]);//计算一个副本一次稳态执行总共需要push的值
			 tmpFissionNode->npart = max_partiotion;
			 /*设定分裂后分裂节点的fiss_state的值*/
			 if(tmpFissionNode->fissed_pop[0]==tmpFissionNode->fissed_peek[0]) tmpFissionNode->fiss_state = TRUE;//不需要将稳态执行次数移到work函数内部
			 else tmpFissionNode->fiss_state = FALSE;
			 PartitonNum2FissionNode.insert(make_pair(max_partiotion,tmpFissionNode));
			 FlatNode2FissionNode.insert(make_pair(fissionNode->fissingFlatNode,tmpFissionNode));
		 }
		 /*将fissionNode从PartitonNum2FissionNode，FlatNode2FissionNode中删除*/
		 multimap<int,FissionNodeInfo*>::iterator iter;
		 for(iter = PartitonNum2FissionNode.begin();iter != PartitonNum2FissionNode.end();++iter)
		 {
			 if(iter->second == fissionNode) break;
		 }
		 assert(iter!=  PartitonNum2FissionNode.end());
		 PartitonNum2FissionNode.erase(iter);
		 multimap<FlatNode* ,FissionNodeInfo *>::iterator iter2;
		 for(iter2 = FlatNode2FissionNode.begin();iter2 != FlatNode2FissionNode.end();++iter2)
		 {
			 if(iter2->second == fissionNode) break;
		 }
		 if(iter2!=FlatNode2FissionNode.end()) FlatNode2FissionNode.erase(iter2);
	 }
 }


/*******************************************************************************
	*将一个operator扩展改造成composite,该函数仅是将composite扩展成一个composite，
	*扩展后的composite的输入输出流任然是原来的operator的输入输出流，
	*只是将原operator中流的定义提出来作为composite的输入输出流
********************************************************************************/
Node *HorizontalFission::MakeCompositeNode(Node *operNode)
{
	assert(operNode);
	Node *composite = NewNode(Composite);//存放转换后的composite

	List *inputList = operNode->u.operator_.decl->u.decl.type->u.operdcl.inputs;
	List *outputList = operNode->u.operator_.decl->u.decl.type->u.operdcl.outputs;
	List *composite_inputList = NULL;
	List *composite_outputList = NULL;
	List *oper_inputList = NULL;
	List *oper_outputList = NULL;
	ListMarker input_maker;
	ListMarker output_maker;
	Node *inputNode;
	Node *outputNode;
	/*使composite的输入输出全部是Decl类型，而operator的输入输出全部是Id类型*/
	IterateList(&input_maker,inputList);
	while (NextOnList(&input_maker ,(GenericREF)&inputNode))
	{
		assert(inputNode);
		assert(inputNode->typ == Decl || inputNode->typ == Id);
		if(inputNode->typ == Decl)
		{
			composite_inputList = AppendItem(composite_inputList,inputNode);
			oper_inputList = AppendItem(oper_inputList,MakeNewStreamId(inputNode->u.decl.name,inputNode));
		}
		else
		{
			composite_inputList = AppendItem(composite_inputList,inputNode->u.id.decl);
			oper_inputList = AppendItem(oper_inputList , inputNode);
		}
	}
	IterateList(&output_maker,outputList);
	while (NextOnList(&output_maker ,(GenericREF)&outputNode))
	{
		assert(outputNode);
		assert(outputNode->typ == Decl || outputNode->typ == Id);
		if(outputNode->typ == Decl)
		{
			composite_outputList = AppendItem(composite_outputList,outputNode);
			oper_outputList = AppendItem(oper_outputList,MakeNewStreamId(outputNode->u.decl.name,outputNode));
		}
		else
		{
			composite_outputList = AppendItem(composite_outputList,outputNode->u.id.decl);
			oper_outputList = AppendItem(oper_outputList , outputNode);
		}
	}
	/*输入输出*/
	Node *com_InOutNode = MakeComInOutCoord(EMPTY_TQ,composite_inputList,composite_outputList,UnknownCoord);
	
	/*composite的头*/
	char *newName = (char *)malloc(strlen(operNode->u.operator_.decl->u.decl.name) + 50); // 能表示的最大整数应该不超过20位
	sprintf(newName, "%s_%s_%d", operNode->u.operator_.decl->u.decl.name,"splitjoin_fission",++gCurrentFissionCompositeNum);
	Node *id = MakeIdCoord(newName, UnknownCoord);
	Node *composite_Decl = SetDeclType(ModifyDeclType(ConvertIdToDecl(id, EMPTY_TQ, NULL, NULL, NULL), MakeComdclCoord(EMPTY_TQ, com_InOutNode, UnknownCoord)),
										MakeDefaultPrimType(EMPTY_TQ, UnknownCoord), 
										Redecl);
	gIsUnfold = TRUE;
	composite = DefineComposite(composite_Decl);

	operNode->u.operator_.decl->u.decl.type->u.operdcl.inputs = oper_inputList;
	operNode->u.operator_.decl->u.decl.type->u.operdcl.outputs = oper_outputList;
	List *operatorList = NULL;
	operatorList = AppendItem(operatorList,operNode);
	Node *composite_Body = MakeComBodyCoord(PrimVoid, NULL, NULL, operatorList, UnknownCoord, UnknownCoord);
	composite = SetCompositeBody(composite, composite_Body);
	gIsUnfold = FALSE;
	return composite;
}

/*要将join的输出边插入的flatNode以及相应的结构中*/
void HorizontalFission::InsertJoinOutStreamToSSG(Node *streamNode,FlatNode *topFlatNode,SchedulerSSG *sssg)
{//topFlatNode是join节点
	assert(streamNode && topFlatNode );
	assert(streamNode->typ == Id||streamNode->typ == Decl);
	Node *stream_type = NULL;
	if (streamNode->typ == Id)   //取流的类型
		stream_type = streamNode->u.id.decl->u.decl.type;
	else 
		stream_type = streamNode->u.decl.type;
	/*这条边已经在mapEdge2DownFlatNode中存在了*/
	std::multimap<Node *, FlatNode *> ::iterator pos;
	FlatNode *buttomFlatNode=NULL;
	pos = sssg->mapEdge2DownFlatNode.find(stream_type);
	assert(pos != sssg->mapEdge2DownFlatNode.end());
	buttomFlatNode = pos->second;
	sssg->mapEdge2UpFlatNode.insert(make_pair(stream_type,topFlatNode));
	topFlatNode->AddOutEdges(buttomFlatNode);

}
/*将operator节点改造成flatNode节点并插入到sssg中,并确定flatNode的push，peek，pop值*/
void HorizontalFission::InsertFlatNodes(operatorNode *u, compositeNode *oldComposite, compositeNode *newComposite,FlatNode *oldFlatNode,SchedulerSSG *sssg)
{
	FlatNode *src = NULL, *dest = NULL;
	std::vector<FlatNode *> tmp;
	std::map<Node *, FlatNode *> ::iterator pos;
	ListMarker marker;
	Node *item = NULL, *type = NULL;
	assert(u && oldComposite);
	src = new FlatNode(u, oldComposite, newComposite);
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.outputs ); 
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		if (item->typ == Id)
			type = item->u.id.decl->u.decl.type;
		else 
			type = item->u.decl.type;
		multimap<Node *, FlatNode *> ::iterator out_pos;
		int out_count = sssg->mapEdge2DownFlatNode.count(type);
		assert(out_count<=1);//保证一条边只对应一个下端节点
		out_pos = sssg->mapEdge2DownFlatNode.find(type);
		Bool flag = TRUE;//要插入的flatNode在他的下端已经存在，则不插入
		if(out_pos != sssg->mapEdge2DownFlatNode.end())
		{//表示找到
			FlatNode *buttomFlatNode = out_pos->second;
			int InIter =0;
			for (InIter = 0;InIter!= buttomFlatNode->inFlatNodes.size();InIter++)
			{
				if(buttomFlatNode->inFlatNodes[InIter] == oldFlatNode)break;
			}
			if(InIter< buttomFlatNode->inFlatNodes.size() && buttomFlatNode->inFlatNodes[InIter] == oldFlatNode) buttomFlatNode->inFlatNodes[InIter] = src;//找到,修改值(split),如果没找到什么也不做（主要是为了处理分裂后产生的join节点）	
		}
		sssg->mapEdge2UpFlatNode.insert(make_pair(type, src)); //将“有向边”与其“上”端operator绑定
	}
	sssg->AddFlatNode(src);

	dest = src; //边变了
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.inputs);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		type = item->u.id.decl->u.decl.type;
		sssg->mapEdge2DownFlatNode.insert(make_pair(type,dest));//将“有向边”与其“下”端operator绑定

		pos = sssg->mapEdge2UpFlatNode.find(type);//找边的上端节点
		assert(pos != sssg->mapEdge2UpFlatNode.end()); //确保每一条输入边都有operator
		src = pos->second;
		/*如果变得上端节点中的outFlatNodes中已经有当前边对应的下端的FlatNode则不添加，只修改它的当前边对应的值*/
		int outIter =0;
		for (outIter = 0;outIter!= src->outFlatNodes.size();outIter++)
		{
			if(src->outFlatNodes[outIter] == oldFlatNode)break;
		}
		if(outIter< src->outFlatNodes.size()) 
			src->outFlatNodes[outIter] = dest;//找到,修改值(split)	
		else src->AddOutEdges(dest);//没找到，插入
		dest->AddInEdges(src);
	}
}

/*将operator节点转换成flatNode,并修改相关信息*/
void HorizontalFission::Operator2FlatNodeModifyInfo(FlatNode *curFlatNode, List *childOperList,SchedulerSSG *sssg, vector<int >npartVec)
{//parentFlatNode 表示原始的operator节点对应的FlatNode，childOperNode代表分裂后产生的operator,新分裂出来的operator在稳态调度中出现的次数
	//这地方要考虑pop的差值（要在FlatNode中增加一个结构用于表示pop的差值）
	assert(curFlatNode&& childOperList);
	assert(curFlatNode->inFlatNodes.size()==1&& curFlatNode->outFlatNodes.size()==1);//出度入度都是1 
	int npart = 0;
 	/*修改与curFlatNode相关联的flatNode中的有关数据,主要是以curFlatNode输出作为输入的flatNode节点*/
	vector<FlatNode *>::iterator pos;
	for(vector<FlatNode *>::iterator iter = sssg->flatNodes.begin();iter !=sssg->flatNodes.end();iter++)
	{
		if(*iter == curFlatNode) 
		{
			pos = iter;
			break;
		}
	}
	assert(pos!= sssg->flatNodes.end());
	sssg->flatNodes.erase(pos);

	/*删除sssg->flatNode中mapEdge2UpFlatNode和mapEdge2DownFlatNode中与parentFlatNode绑定的边*/
	Node *tmp_Instream = (Node *)FirstItem(curFlatNode->contents->decl->u.decl.type->u.operdcl.inputs);
	Node *tmp_Outstream = (Node *)FirstItem(curFlatNode->contents->decl->u.decl.type->u.operdcl.outputs);
	Node *curOperInstream = NULL;
	Node *curOperOutstream = NULL;
	if (tmp_Instream->typ == Id)
		curOperInstream = tmp_Instream->u.id.decl->u.decl.type;
	else 
		curOperInstream = tmp_Instream->u.decl.type;

	sssg->mapEdge2DownFlatNode.erase(curOperInstream);

	if (tmp_Outstream->typ == Id)
		curOperOutstream = tmp_Outstream->u.id.decl->u.decl.type;
	else 
		curOperOutstream = tmp_Outstream->u.decl.type;
	sssg->mapEdge2UpFlatNode.erase(curOperOutstream);	

	/*将childOperList改造成一个vector*/
	ListMarker childOper_Maker;
	Node *childOperNode = NULL;
	int len = sssg->GetFlatNodes().size();   //原始长度
	IterateList(&childOper_Maker ,childOperList);
	while(NextOnList(&childOper_Maker,(GenericREF)&childOperNode))
	{//将childOper改造成flatNode ,(所有的节点都是composite了)
		//Node *comNode = MakeCompositeNode(childOperNode);
		assert(childOperNode->typ == Composite);
		assert(ListLength(childOperNode->u.composite.body->u.comBody.comstmts)== 1);
		Node *oper = (Node *)FirstItem(childOperNode->u.composite.body->u.comBody.comstmts);
		InsertFlatNodes(&(oper->u.operator_),&(childOperNode->u.composite),&(childOperNode->u.composite),curFlatNode,sssg);	
	}
	int newlen = sssg->GetFlatNodes().size(); //新添加后的长度
	assert(newlen - len>=3);//至少增加了三个节点（split，join，以及复制分裂后生成的节点）
	vector<FlatNode *> newflatNode = sssg->GetFlatNodes();
	
	/*要将join的输出边插入的flatNode以及相应的结构中*/
 	FlatNode *joinFlatNode = newflatNode[newlen-1];
	Node *joinOutputStream = (Node *)FirstItem(joinFlatNode->contents->decl->u.decl.type->u.operdcl.outputs);
 	InsertJoinOutStreamToSSG(joinOutputStream,joinFlatNode,sssg);


	for(int i = len;i <newflatNode.size();i++)	
		sssg->SetFlatNodesWeights(newflatNode[i]);//修改新插入节点的权值

	/*修改新插入节点稳态执行次数(1)删除原节点的稳态执行(2)插入新节点的稳态执行。工作量mapSteadyWork2FlatNode，调度次数mapSteadyWork2FlatNode*/
	int oldSteadyCount = sssg->GetSteadyCount(curFlatNode);
	sssg->mapSteadyCount2FlatNode.erase(curFlatNode);	//删除原节点的稳态执行
	sssg->mapInitCount2FlatNode.erase(curFlatNode);	//删除原节点的初态执行
	sssg->mapInitCount2FlatNode.erase(curFlatNode);//删除原节点的初态调度
	sssg->mapSteadyWork2FlatNode.erase(curFlatNode);//删除原节点的稳态工作量
	sssg->mapInitWork2FlatNode.erase(curFlatNode);//删除原节点的初态工作量

	/*查找划分中的已分裂的所在的划分编号*/
	partition->FlatNode2PartitionNum.erase(curFlatNode);
	multimap<int,FlatNode *>::iterator mult_iter;
	for( mult_iter = partition->PartitonNum2FlatNode.begin();mult_iter!= partition->PartitonNum2FlatNode.end();mult_iter++)
	{
		if(mult_iter->second == curFlatNode) break;
	}

	sssg->mapSteadyCount2FlatNode.insert(make_pair(newflatNode[len],1));//新插入的第一个节点是split   ，只执行一次
	sssg->mapInitCount2FlatNode.insert(make_pair(newflatNode[len],0));
	sssg->mapSteadyWork2FlatNode.insert(make_pair(newflatNode[len],workEstimate((newflatNode[len])->contents->body, 0)));
	sssg->mapInitWork2FlatNode.insert(make_pair(newflatNode[len],workEstimate_init((newflatNode[len])->contents->body, 0)));
	/*将split节点添加划分中*/
	partition->FlatNode2PartitionNum.insert(make_pair(newflatNode[len],mult_iter->first));
	partition->PartitonNum2FlatNode.insert(make_pair(mult_iter->first,newflatNode[len]));

	/*将分裂后的节点添加到划分中（将第一份放在最小的划分中，其他的留在原来的划分中）*/
	
	//更新稳态执行次数，以及稳态的的工作量(新节点稳态只执行一次)
	sssg->mapSteadyCount2FlatNode.insert(make_pair(newflatNode[len+1],1));
	sssg->mapInitCount2FlatNode.insert(make_pair(newflatNode[len+1],0));//初始化是默认为0
	sssg->mapSteadyWork2FlatNode.insert(make_pair(newflatNode[len+1],workEstimate((newflatNode[len+1])->contents->body, 0)));	
	sssg->mapInitWork2FlatNode.insert(make_pair(newflatNode[len+1],workEstimate_init((newflatNode[len+1])->contents->body, 0)));	
	partition->FlatNode2PartitionNum.insert(make_pair(newflatNode[len+1],npartVec[npart]));
	partition->PartitonNum2FlatNode.insert(make_pair(npartVec[npart++],newflatNode[len+1]));

	for (int i=len+2 ;i<newlen-1;i++)
	{
		//更新稳态执行次数，以及稳态的的工作量
		sssg->mapSteadyCount2FlatNode.insert(make_pair(newflatNode[i],1));
		sssg->mapSteadyWork2FlatNode.insert(make_pair(newflatNode[i],workEstimate((newflatNode[i])->contents->body, 0)));	
		sssg->mapInitWork2FlatNode.insert(make_pair(newflatNode[i],workEstimate_init((newflatNode[i])->contents->body, 0)));	
		sssg->mapInitCount2FlatNode.insert(make_pair(newflatNode[i],0));
		partition->FlatNode2PartitionNum.insert(make_pair(newflatNode[i],npartVec[npart]));
		partition->PartitonNum2FlatNode.insert(make_pair(npartVec[npart++],newflatNode[i]));
	}
	//将join节点添加到sssg中
	sssg->mapSteadyCount2FlatNode.insert(make_pair(newflatNode[newlen-1],1));
	sssg->mapSteadyWork2FlatNode.insert(make_pair(newflatNode[newlen-1],workEstimate((newflatNode[newlen-1])->contents->body, 0)));
	sssg->mapInitWork2FlatNode.insert(make_pair(newflatNode[newlen-1],workEstimate_init((newflatNode[newlen-1])->contents->body, 0)));	
	sssg->mapInitCount2FlatNode.insert(make_pair(newflatNode[newlen-1],0));
	partition->FlatNode2PartitionNum.insert(make_pair(newflatNode[newlen-1],mult_iter->first));
	partition->PartitonNum2FlatNode.insert(make_pair(mult_iter->first,newflatNode[newlen-1]));
	/*删除划分中的已分裂*/
	partition->PartitonNum2FlatNode.erase(mult_iter);
}

void HorizontalFission::initFissionNodeMap(SchedulerSSG *sssg)
{//在这里面要初始化PartitonNum2FissionNode,FlatNode2FissionNode的初始值
	multimap<int,FlatNode*>::iterator iter;
	for(iter = partition->PartitonNum2FlatNode.begin();iter != partition->PartitonNum2FlatNode.end();++iter)
	{
		FissionNodeInfo* fissionNode = HeapNew(FissionNodeInfo);
		fissionNode->fissingFlatNode = iter->second;
		fissionNode->operatorWeight = sssg->GetSteadyWork(iter->second);
		fissionNode->fissedSteadycount = sssg->GetSteadyCount(iter->second);
		for(int i=0;i < fissionNode->fissingFlatNode->inPopWeights.size();i++)
		{
			fissionNode->fissed_pop.push_back(fissionNode->fissingFlatNode->inPopWeights[i]*fissionNode->fissedSteadycount);
		}
		for(int i=0;i < fissionNode->fissingFlatNode->inPeekWeights.size();i++)
		{
			fissionNode->fissed_peek.push_back(fissionNode->fissingFlatNode->inPeekWeights[i]*fissionNode->fissedSteadycount);
		}
		for(int i=0;i < fissionNode->fissingFlatNode->outPushWeights.size();i++)
		{
			fissionNode->fissed_push.push_back(fissionNode->fissingFlatNode->outPushWeights[i]*fissionNode->fissedSteadycount);
		}
		fissionNode->npart = iter->first;
		fissionNode->fiss_state = TRUE;
		PartitonNum2FissionNode.insert(make_pair(iter->first,fissionNode));
		//FlatNode2FissionNode.insert(make_pair(iter->second,fissionNode));
	}
}


 /*图的水平分裂*/
void HorizontalFission::HorizontalFissionPRPartitions(SchedulerSSG *sssg,float balanceFactor)
{
	initFissionNodeMap(sssg);//初始化类中的两个map
	while(TRUE)
	{
		map<int,int > Partiotion2Weight = computeMapPlace2Weight();//根据成员PartitonNum2FissionNode计算各个划分的初始工作量
		//vector<int> sortPartiotion = SortPartitionsByWeight(Partiotion2Weight);
#if 0
		cout<<"=========================================="<<endl;
		for(map<int,int >::iterator piter = Partiotion2Weight.begin(); piter != Partiotion2Weight.end();piter++ )
		{
			cout<<piter->first << "    "<< piter->second<<endl;
		}
# endif
		int maxPartition = -1;
		int cur_max_partition; //当前权重最大的划分的迭代器
		int cur_min_partition; //当前权重最小的划分的迭代器
		bool maxNodeInMaxPartition = true;
		FissionNodeInfo *maxWorkFlatNode = NULL;//存放要被分裂的节点
		int maxWork = 0;
		Bool partiotion_flag = FALSE;

		vector<Bool> PartiotionFlag;//表示每一个划分中是否有可分裂的节点 
		for (map<int,int>::iterator iter1 = Partiotion2Weight.begin();iter1 != Partiotion2Weight.end();++iter1)
		{
			PartiotionFlag.push_back(TRUE);
		}

		for (int i = 0 ;i< partition->getParts();i++)
		{//找当前能够分裂的核
			cur_max_partition = NextMaxWeightPartition(Partiotion2Weight,PartiotionFlag);//找到当前权值最大的划分
			if(cur_max_partition == Partiotion2Weight.size()) break; 
#if 0
			cout<<"$$$$$$$$$$ 划分号：   "<<cur_max_partition<<"    $$$$$$$$$$$$$"<<endl;
#endif
			/*取当前权值最大的划分中的所有的operator*/
			vector<FissionNodeInfo *> fissionNodeVector = findfissionNodeSetInPartition(cur_max_partition);
			/*最大化分中工作量最大的operator*/
			maxWorkFlatNode = NULL;
			maxWork = 0;
			for (int fissionNodeVector_iter = 0;fissionNodeVector_iter!=fissionNodeVector.size();fissionNodeVector_iter++)
			{
				stateful = FALSE;
				hasMutableState(fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->contents);
#if 0
				if(stateful && fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount==1)cout<<fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->name<<"不可分 "<<endl;
				else cout<<"也许可分" <<fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->name<<": SteadyCount= "<< fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount <<"  单入单出  "<<SInSOutOPerator(fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->contents)<<"  状态:"<<fissionNodeVector[fissionNodeVector_iter]->fiss_state<<endl; 
#endif
				if(maxNodeInMaxPartition){
					for (int fissionNodeVector_iter = 0;fissionNodeVector_iter!=fissionNodeVector.size();fissionNodeVector_iter++){
						int operatorWork = fissionNodeVector[fissionNodeVector_iter]->operatorWeight; // 单个operator的工作量
						int operatorCount = fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount;	 //在稳定状态下operator的执行次数
						int work_Weight= operatorWork*operatorCount;
						if(maxWork<work_Weight)	
						{
							maxWorkFlatNode = fissionNodeVector[fissionNodeVector_iter];//找工作量最大的核中工作量最大的operator
							maxWork = work_Weight;
						}
					}
					if(maxWork>10000&&maxWorkFlatNode->fissingFlatNode->inPeekWeights[0]<=64&&maxWorkFlatNode->fissingFlatNode->outPushWeights[0]<=64)
					//if(maxWork>10000)
					maxWorkFlatNode->fissingFlatNode->memorizedNode = true;
					maxNodeInMaxPartition = false;
				}
				maxWork = 0;
				maxWorkFlatNode = NULL;
				if(!stateful&&fissionNodeVector[fissionNodeVector_iter]->fiss_state==TRUE&& SInSOutOPerator(fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->contents))
					if(fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount>1)//分裂的条件
					{
						//						cout<<"条件" <<fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->name<<": SteadyCount= "<< fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount <<"  单入单出 " <<"    "<<SInSOutOPerator(fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->contents)<<endl; 
						int operatorWork = fissionNodeVector[fissionNodeVector_iter]->operatorWeight; // 单个operator的工作量
						int operatorCount = fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount;	 //在稳定状态下operator的执行次数
						int work_Weight= operatorWork*operatorCount;
						if(maxWork<work_Weight)	
						{
							maxWorkFlatNode = fissionNodeVector[fissionNodeVector_iter];//找工作量最大的核中工作量最大的operator
							maxWork = work_Weight;
						}
						partiotion_flag = TRUE;//找到分裂的核和可分裂的operator,修改标志位
					}
			}
			
			if(partiotion_flag ) break;
			else{ 
				PartiotionFlag[cur_max_partition] = FALSE;	//该划分中没有可分裂节点
				if(maxWork>10000&&maxWorkFlatNode->fissingFlatNode->inPeekWeights[0]<=64&&maxWorkFlatNode->fissingFlatNode->outPushWeights[0]<=64)
					maxWorkFlatNode->fissingFlatNode->memorizedNode = true;
			}
		}
		/*没有满足条件的分裂节点*/
		if(!partiotion_flag || maxWorkFlatNode == NULL)
			break;
		assert(maxWorkFlatNode);
		/*找工作量最小的划分*/
		cur_min_partition = MinPartitionWeight(Partiotion2Weight);
		/*任务均衡退出划分*/
		if(Partiotion2Weight.find(cur_max_partition)->second < Partiotion2Weight.find(cur_min_partition)->second * balanceFactor)
			break;
		/*计算分裂次数*/
		int replicationFactor = maxWork /((Partiotion2Weight.find(cur_max_partition)->second) - (Partiotion2Weight.find(cur_min_partition)->second));
		replicationFactor = replicationFactor>2 ? replicationFactor:2;
#if 0
		cout<<"最大核能分的核： "<<cur_max_partition<<"    最小核： "<<cur_min_partition<<endl;
#endif

		/*下面主要是在两个成员map修改信息，删除将要分裂节点的信息，添加分裂后的副本的信息——伪分裂*/
		SplitFissionNode(maxWorkFlatNode,replicationFactor,cur_max_partition,cur_min_partition);
	}
	/******取出FlatNode2FissionNode中各个flatNode对应的各个副本的信息组成
	 ***一个Vector，传递给horizontalFissOperator函数精心分裂，组成一个List，然后
	 ***再将List传递给Operator2FlatNodeModifyInfo，修改sssg图的信息*************/
	multimap<FlatNode* ,FissionNodeInfo *>::iterator flatNodeIter ;
	while (!FlatNode2FissionNode.empty())
	{
		flatNodeIter = FlatNode2FissionNode.begin();
		vector<FissionNodeInfo *> splitedNodeVec;
		vector<int >npartVec;//记录每个副本被分配到划分的编号
		typedef multimap<FlatNode* ,FissionNodeInfo *>::iterator F2FIter;
		pair<F2FIter,F2FIter>range=FlatNode2FissionNode.equal_range(flatNodeIter->first);
		for(F2FIter iter=range.first;iter!=range.second;++iter)
		{
			splitedNodeVec.push_back(iter->second);
			npartVec.push_back((iter->second)->npart);
		}
		//构造splitedNodeVec中的所有节点并添加split和join节点
		List *oper_list = horizontalFissOperator(splitedNodeVec,sssg);
		/*根据分裂后的情况修改相应结构的信息*/
		Operator2FlatNodeModifyInfo(flatNodeIter->first,oper_list,sssg,npartVec);//根据oper_list修改sssg图的信息
		/*将FlatNode2FissionNode中已处理过的节点删除*/
		FlatNode2FissionNode.erase(flatNodeIter->first);
	}
	/*等到整个图稳定后在进行真正的分裂（下面的函数需要修改）*/

#if 0
	PrintList(stdout,oper_list,-1);
#endif

	/*根据新的图进行初态调度*/
	/*划分完成之后删除初始化调度中的所有节点，以便能进行新的初态调度*/
	//map<int,int > Partiotion2Weight1= computeMapPlace2Weight();//根据成员PartitonNum2FissionNode计算各个划分的初始工作量
	sssg->mapInitCount2FlatNode.erase(sssg->mapInitCount2FlatNode.begin(),sssg->mapInitCount2FlatNode.end());
}