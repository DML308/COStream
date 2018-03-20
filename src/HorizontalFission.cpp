#include "HorizontalFission.h"
#include "basics.h"

using namespace std;

PRIVATE  int  gCurrentFissionCompositeNum = 0;//���ڽ������������

Bool stateful = FALSE;

Bool HorizontalFission::SInSOutOPerator(operatorNode *oper)
{//�����operator�ǵ��뵥���ķ���True
	assert(oper);
	List *inputList = oper->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = oper->decl->u.decl.type->u.operdcl.outputs;
	if((ListLength(outputList)==1)&&(ListLength(inputList)==1)) return TRUE;
	return FALSE;
}

HorizontalFission::HorizontalFission(Partition *p,float balane):partition(p),balanceFactor(balane)
{//���캯��
}
void HorizontalFission::hasMutableState(operatorNode *opNode)
{  //��state�򷵻�True
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
{ //���������state�򷵻�True
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
		{  // ����ṹ������
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
				 {//�Ը�ֵ��������ֵ���д���
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
 
//���һ��ֺ�ΪpartitionNum�е����нڵ�(�������ڵ���1��)�����ڵ㼯�Ϸ��ظ�PartitonNumSet(���->�ڵ�)
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
//�󵥸����ֵĹ������ܺ�
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
		int operatorWork = fissionNodeVec[ix]->operatorWeight; //����operator�Ĺ�����
		int operatorCount = fissionNodeVec[ix]->fissedSteadycount;	 //���ȶ�״̬��operator��ִ�д���
		sumWeight += operatorWork*operatorCount;
	} 
	return sumWeight;
}
/*����������ֵĺ�Ȩֵ��������һ��map<���ֵı�ţ�Ȩ��>*/
map<int,int> HorizontalFission::computeMapPlace2Weight()
{
	int partitions = partition->getParts();//���ֵķ���
	map<int,int>partition2weight;
	assert(partitions>=0);
	for (int npart = 0;npart<partitions;npart++)
	{
		int sumWeight = computeSumWeightofPlace(npart);
		partition2weight.insert(make_pair(npart,sumWeight));
	}
	return partition2weight;
}
/*����������С��������*/
vector<int > HorizontalFission::SortPartitionsByWeight(map<int,int>partition2weight)
{   // ����
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
/*�ҹ�������С�Ļ���*/
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
 /*partition2weight������ģ�ȡȨֵ���ģ����ص���Ȩֵ���ĵ�����*/
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
{//node��һ��Array�ڵ㣬����������һ����������decl���޸��������ڵ���±�,�����±���ɵ�List
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
	*�޸�work��������������±꣬���õݹ�����ķ�ʽ
	*��Ҫ��˼�룺1.����work���������ݣ������������������账��
				 2.���������������ж��ǲ��������������������Բ�ͬ���������޸��±���Ϣ
	 ����˵����n����ǰ����Ľڵ㡣inputlist	��outputist �������������
			   inputDelt��outputDelt ����������������±�ֵ�����ӵ�ƫ��ֵ
**********************************************************************/
void HorizontalFission::MWISD_astwalk(Node *n,List *inputlist,List *outputlist,Node *inputDelt,Node *outputDelt)
{//�޸�work�д���������ȡԪ�ص�λ��
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
						 //���崦��
						 if(n->u.array.name->u.id.decl->u.decl.type->typ == STRdcl)//�������������һ��stream
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
/*�޸�operator�е�window,��Ҫ����������peek��pop���������push��������������뵥�����operator�Ĵ���*/
List *HorizontalFission::ModifyWindow(List *inputList,List *outputList,int popValue,int peekValue,int pushValue)
{//����ԭ���Ĵ�����û�У����ڰ����µ�ֵ���¹���	 
	assert(inputList);
	assert(outputList);
	assert(ListLength(inputList) == ListLength(outputList) == 1);
	assert(ListLength(inputList)==1);
	Node *inputNode = (Node *)FirstItem(inputList);
	Node *outputNode = (Node *)FirstItem(outputList);
	assert(inputNode && inputNode->typ == Decl);
	assert(outputNode &&inputNode->typ == Decl);
	List *finalWinList = NULL;//���ڷ�������޸ĺ��window
	  //���봰��Ϊ�գ���һ������window
	List *outputs = NULL;
	outputs = AppendItem(AppendItem(outputs,MakeConstSint(peekValue)),MakeConstSint(popValue));
	Node *output = MakeCommaCoord(outputs,UnknownCoord);
	Node *sliding = MakeWindowSlidingCoord(EMPTY_TQ,output,UnknownCoord);
	Node *input_winIdNode = MakeNewStreamId(inputNode->u.decl.name,inputNode);
	Node *input_Window = MakeWindowCoord(input_winIdNode,sliding,UnknownCoord); //���봰��

	Node *tumbling = MakeWindowTumbingCoord(EMPTY_TQ, MakeConstSint(pushValue), UnknownCoord);  //push
	Node *output_winIdNode = MakeNewStreamId(outputNode->u.decl.name,outputNode);
	Node *output_Window = MakeWindowCoord(output_winIdNode,tumbling,UnknownCoord);//����ߵĴ���
	finalWinList = AppendItem(finalWinList,input_Window);
	//PrintNode(stdout,input_Window,0);

	finalWinList = AppendItem(finalWinList,output_Window);
	//PrintNode(stdout,output_Window,0);
	return finalWinList;
}
/*�޸�composite������(composite��ͷ)*/
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
	*���ݷ��Ѻ����������µ�composite,node��ԭʼ��composite��
	*inputList��outputList���²�����composite�����������
	*count���·��ѳ���composite��work��ִ�д���
	*popValue,peekValue,pushValue���²�����composite��peek��pop��push��ֵ
	*oldpopValue,oldpeekValue,oldpushValue �����ʾԭ����operator��pop��peek��pushֵ
	*���ܣ�����node�����µ�composite��composite��graph��operator��work��windowҪ���޸�
	*����������1.�����µ�������������composite��
			   2.Ϊ��ʹ�µ�operator������̬ʱִֻ��һ�Σ��޸�work��ִ�д������µ�ִ�д�����ֲ��work������
			     ͬʱҪ�޸�work��������������������ݵ��±�
			   3.�޸�window�е�peek��pop��push��ֵ
**************************************************************************/
Node *HorizontalFission::ConstructNewComposite(Node *node,List *inputList,List *outputList,int count,int popValue,int peekValue,int pushValue,int oldpopValue,int oldpeekValue,int oldpushValue,int deltpeek)
{
	Node *newComposite = NULL;
	assert(node && node->typ == Composite);

	// PrintNode(stdout,node,0);
	assert(ListLength(inputList) == ListLength(outputList) == 1);//�����compositeֻ�ǵ��뵥����
	gIsTransform = TRUE;//�������ʱ��ɱ����滻�Ŀ���
	newComposite = compositeCopy(node,inputList,outputList);//�����µ�������������µ�composite
	ResetASTSymbolTable(ToTransformDecl);
//newComposite->u.composite.decl->u.decl.name; 
	gIsTransform = FALSE;
	
	assert(newComposite);
	/*�޸�composite������*/
	//newComposite = ModifyCompositeName(newComposite);
	assert(ListLength(newComposite->u.composite.body->u.comBody.comstmts)==1);
	Node *operNode = (Node *)FirstItem(newComposite->u.composite.body->u.comBody.comstmts);//ȡ��composite�е�operator
	/*���޸�composite��window*/
	List *tmpwinList = ModifyWindow(inputList,outputList,popValue,peekValue,pushValue);

	/*�޸�work*/
	/*1.���ݵĲ����޸�work�ڶ�����������������õ��±�*/
	Node *workNode = operNode->u.operator_.body->u.operBody.work;
	assert(workNode);
	Node *workCount = MakeConstSint(count);

	/*�������workѭ��������id�ڵ�*/
	Node *workVarDecl = MakeDeclCoord("operator_work",T_BLOCK_DECL,MakePrimCoord(EMPTY_TQ,Sint,UnknownCoord),MakeConstSint(0),NULL,UnknownCoord);//��work��Ϊ�����ؼ��� 
	Node *workVarId  = MakeIdCoord("operator_work", UnknownCoord);
	workVarId->u.id.decl = workVarDecl;
	REFERENCE(workVarDecl);
	Node *inputDelt = NULL;
	inputDelt = MakeBinopCoord('*',workVarId,MakeConstSint(oldpopValue),UnknownCoord);
	if(deltpeek) inputDelt = MakeBinopCoord('+',inputDelt,MakeConstSint(deltpeek),UnknownCoord); //duplicate��ʽdeltpeekһ�㲻Ϊ0��RoundRobin��ʽdeltpeek=0
	Node *outputDelt = MakeBinopCoord('*',workVarId,MakeConstSint(oldpushValue),UnknownCoord);
	MWISD_astwalk(workNode,inputList,outputList,inputDelt,outputDelt);

	/*2.�޸�work�����ĺ������ִ�д���(��work������body���һ��ѭ��)*/
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
{/*����split������ߣ�input��split�����룬count������ߵ���Ŀ*/
	assert(input);
	assert(input->typ == Decl || input->typ == Id);
	List *outputList = NULL;
	Node *input_Decl = NULL;
	if(input->typ == Id) input_Decl = input->u.id.decl;
	else input_Decl = input;
	char *newName = NULL;//�����������
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
/*��������������Լ�pop��peek��push��ֵ�������ڣ���Ҫ��Ϊ�˹���split��join�Ĵ���*/
List *HorizontalFission::MakeDuplicateWindow(List *inputList,List *outputList,Node *pop_value)
{
	assert(ListLength(inputList) == 1 && ListLength(outputList)>1);
	List *windowList = NULL;
	ListMarker output_maker;
	ListMarker push_maker;
	Node *outputNode = NULL;
	/*�������*/
	IterateList(&output_maker,outputList);
	while (NextOnList(&output_maker,(GenericREF)&outputNode))
	{
		assert(outputNode->typ == Id ||outputNode->typ == Decl);
		Node *outputId = NULL;
		if (outputNode->typ == Id)
			outputId = MakeNewStreamId(outputNode->u.id.text,outputNode->u.id.decl);
		else 
			outputId = MakeNewStreamId(outputNode->u.decl.name,outputNode);
		windowList = AppendItem(windowList,MakeMyWindow(outputId,pop_value,1));	//����ߵĴ���
	}

	/*���봰��*/
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
	/*�������*/
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
		windowList = AppendItem(windowList,MakeMyWindow(outputId,push_value,1));	//����ߵĴ���
	}
	/*���봰��*/
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
	/*���봰��*/
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
		windowList = AppendItem(windowList,MakeMyWindow(inputId,pop_value,0));	//����ߵĴ���
	}

	/*�������*/
	Node *outputNode = (Node *)FirstItem(outputList);
	assert(outputNode->typ == Id || outputNode->typ == Decl);
	Node *outputId = NULL;

	if(outputNode->typ ==Id) outputId = MakeNewStreamId(outputNode->u.id.text,outputNode->u.id.decl);
	else outputId = MakeNewStreamId(outputNode->u.decl.name,inputNode);
	windowList =AppendItem(windowList,MakeMyWindow(outputId,MakeConstSint(push_value),1));   //���
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
	/*ʹcomposite���������ȫ����Decl���ͣ���operator���������ȫ����Id����*/
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
	/*�������*/
	Node *com_InOutNode = MakeComInOutCoord(EMPTY_TQ,composite_inputList,composite_outputList,UnknownCoord);

	/*composite��ͷ*/
	char *newName = (char *)malloc(sizeof(char)*50); // �ܱ�ʾ���������Ӧ�ò�����20λ
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
	/*ʹcomposite���������ȫ����Decl���ͣ���operator���������ȫ����Id����*/
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
	/*�������*/
	Node *com_InOutNode = MakeComInOutCoord(EMPTY_TQ,composite_inputList,composite_outputList,UnknownCoord);

	/*composite��ͷ*/
	char *newName = (char *)malloc(sizeof(char) + 50); // �ܱ�ʾ���������Ӧ�ò�����20λ
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
{//������Ľڵ�Ҳ��һ��composite,������composite�ڵ�
	assert(inputNode && ListLength(outputList)>1);//�����½���split�ڵ�������2�������	
	Node *dupOperNode = NULL;//�洢operator���͵Ľڵ�
	//Node *dupComNode = NULL;//��ŵ���Composite���͵Ľڵ㣨���յķ���ֵ��
	
	List *tmp_inputList = NULL;
	tmp_inputList = AppendItem(tmp_inputList,inputNode);
	char *operName = (char *)malloc(40);//���ֵĳ���
	sprintf(operName,"%s_%d","Duplicate_fiss",++gCurrentFissionCompositeNum);
	Node *dupOperHead = MakeOperatorHead(operName,outputList,tmp_inputList); //operator��ͷ
	dupOperNode = DefineOperator(dupOperHead);
	dupOperNode->u.operator_.ot = Duplicate_ ;
	
	/*��������������Լ�duplicate�Ĳ�������splitoperator��Body�ڵ�*/
	Node *dupWorkNode = MakeDuplicateWork(outputList,inputNode,pop_value);//����operator��work����
	List *dupWindowList = MakeDuplicateWindow(tmp_inputList,outputList,pop_value);
	Node *dupOperBodyNode = MakeOperatorBody(dupWorkNode,dupWindowList);//operator��body

	dupOperNode = SetOperatorBody(dupOperNode,dupOperBodyNode);//����operator
	/*��operatorת����Composite*/
	//dupComNode = MakeCompositeNode(dupOperNode);
	return dupOperNode;	
}

Node *HorizontalFission::MakeFissionRoundRobin(Node *inputNode,List *outputList, List *pop_arg)
{//������Ľڵ�Ҳ��һ��composite,������composite�ڵ�
	assert(inputNode && ListLength(outputList)>1);//�����½���split�ڵ�������2�������
	assert(ListLength(outputList) == ListLength(pop_arg));	
	Node *rouOperNode = NULL;//�洢operator���͵Ľڵ�
	//Node *rouComNode = NULL;//��ŵ���Composite���͵Ľڵ㣨���յķ���ֵ��
	/*��������������Լ�duplicate�Ĳ�������splitoperator�ڵ�*/
	List *tmp_inputList = NULL;
	tmp_inputList = AppendItem(tmp_inputList,inputNode);
	
	char *operName = (char *)malloc(40);//���ֵĳ���
	sprintf(operName,"%s_%d","RoundRobin_fiss",++gCurrentFissionCompositeNum);
	Node *rouOperHead = MakeOperatorHead(operName,outputList,tmp_inputList); //operator��ͷ
	rouOperNode = DefineOperator(rouOperHead);
	rouOperNode->u.operator_.ot = Roundrobin_ ;
	Node *rouWorkNode = MakeRoundrobinWork(outputList,inputNode,pop_arg);//����operator��work����
	List *rouWindowList = MakeRoundRobinWindow(tmp_inputList,outputList,pop_arg);
	Node *rouOperBodyNode = MakeOperatorBody(rouWorkNode,rouWindowList);//operator��body
	rouOperNode = SetOperatorBody(rouOperNode,rouOperBodyNode);//����operator
	/*��operatorת����Composite*/
	//rouComNode = MakeCompositeNode(rouOperNode);
	return rouOperNode;	

}
/*����join�ڵ�,��������������ߣ�������������ߣ�һ������ÿһ������ÿһ���߷������ݵ���Ŀ*/
Node *HorizontalFission::MakeFissionJoin(List *inputList,Node *outputNode, List *pop_arg)
{  //������Ľڵ�Ҳ��һ��composite
	assert(outputNode && ListLength(inputList)>1);//�����½���split�ڵ�������2�������
	assert(ListLength(inputList) == ListLength(pop_arg));	
	Node *joinOperNode = NULL;//�洢operator���͵Ľڵ�
	//Node *joinComNode = NULL;//��ŵ���Composite���͵Ľڵ㣨���յķ���ֵ��
	/*��������������Լ�duplicate�Ĳ�������splitoperator�ڵ�*/
	List *tmp_outputList = NULL;
	Node *tmp_outputNode = NULL;

	assert(outputNode && (outputNode->typ == Id || outputNode->typ == Decl) );

	if(outputNode->typ == Decl) tmp_outputNode = MakeNewStreamId(outputNode->u.decl.name,outputNode);
	else tmp_outputNode = outputNode;

	tmp_outputList = AppendItem(tmp_outputList,tmp_outputNode); 
	char *operName = (char *)malloc(40);//���ֵĳ���
	sprintf(operName,"%s_%d","Join_Fiss",++gCurrentFissionCompositeNum);
	Node *rouOperHead = MakeOperatorHead(operName,tmp_outputList,inputList); //operator��ͷ
	joinOperNode = DefineOperator(rouOperHead);
	joinOperNode->u.operator_.ot = Join_ ;		
	Node *joinWorkNode = MakeJoinWork(tmp_outputNode,inputList,pop_arg);//����operator��work����
	List *joinWindowList = MakeJoinWindow(inputList,tmp_outputList,pop_arg);
	Node *joinOperBodyNode = MakeOperatorBody(joinWorkNode,joinWindowList);//operator��body
	joinOperNode = SetOperatorBody(joinOperNode,joinOperBodyNode);//����operator
	return joinOperNode;
}


/**********************************************************************************
	* ���ܣ���һ��operator���Ʒ��ѳɶ��operator�����һ��List,List�ĳ�Ա��Node*
			���List�в�����split��join�ڵ�
			ע��������opername_fission_splijoin_��ţ�
    * flatnode����Ҫ���ѵĽڵ�
	vector<FissionNodeInfo *> splitedNodeVec ��¼���Ѻ����ɵ�ÿ���ڵ����Ϣ
	* inputList: ���Ѻ�ÿһ���ڵ��ʵ�ʵ����
    * steadyCount���ȶ�״ִ̬�д���
    * style�����ѵķ�ʽ��roundrobin��ʽ��duplicate��ʽ��
*******************************************************************************/

List *HorizontalFission::MakeReplicateOper(vector<FissionNodeInfo *> splitedNodeVec,FlatNode *flatnode, List *inputList, int steadyCount,SplitStyle style)
{
	operatorNode *oper = flatnode->contents;//��Ҫ�����ѵ�operator
	int popweight = flatnode->inPopWeights[0];//ȡ��ǰopertor��popֵ
	int pushweight = flatnode->outPushWeights[0];//ȡ��ǰopertor��pushֵ
	int peekweight = flatnode->inPeekWeights[0];//ȡ��ǰopertor��peekֵ
	
	/*��operatorNode*�����һ��Node*�ڵ�*/
	Node *operNode = NewNode(Operator_);
	operNode->u.operator_.decl= oper->decl;
	operNode->u.operator_.body= oper->body;
	operNode->u.operator_.ot = oper->ot;
	List *fissionOperList = NULL;//���Ѻ�operator��ɵ�list

	Node *compositeNode = NULL;	//��ԭʼ��operator�����һ��composite�ṹ
	compositeNode = MakeCompositeNode(operNode);//��ԭ����operNode�����һ��composite

	/*Ϊ���������ɵĽڵ㹹�������*/
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
	/*���ݲ�ͬ�ķ��ѷ�ʽ���ѣ�style = 0 ��roundrobin��ʽ��style = 1 ��Duplicate��ʽ*/
	if(style == S_Duplicate )
	{  //duplicate��ʽ����Ҫ�޸�operator��work��window�еĴ���(��Ҫ���޸�peek)
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
											//ConstructNewComposite����˵����ģ��composite���µ��������������ǰ���Ƴ���composite����̬ʱ��ִ�д�����Ȼ����ִ�д�����pop��peek��pushֵ
			fissionOperList = AppendItem(fissionOperList,ConstructNewComposite(compositeNode,new_inputList,new_outputList,splitedNodeVec[i]->fissedSteadycount,splitedNodeVec[i]->fissed_pop[0],splitedNodeVec[i]->fissed_peek[0],splitedNodeVec[i]->fissed_push[0],popweight,peekweight,pushweight,deltpeek));
			inputList = GetNextList(inputList);
			outputList = GetNextList(outputList);
		}
	}
	else
	{  //roundrobin��ʽ
		for(int i=0;i<splitedNodeVec.size();i++)
		{  //operator�ڵ��е�work�Լ�window��Ҫ����Ӧ���޸ģ���̬ʱworkִֻ��һ�Σ��������ִ̬�д�����work�����֣�
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
	operatorNode *operNode =flatNode->contents; //��Ҫ�����ѵ�operator
	int steadyCount = sssg->GetSteadyCount(flatNode);  //ȡ�ýڵ������̬�����е�ִ�д���

	List *inputList=operNode->decl->u.decl.type->u.operdcl.inputs;  //������operator������
	List *outputList = operNode->decl->u.decl.type->u.operdcl.outputs; //������operator�����

	assert(ListLength(inputList)==1 && ListLength(outputList)==1);	//��ʱֻ�Ե����뵥����Ľڵ���и��Ʒ���

	int outPushWeight =	ListLength(outputList) ? flatNode->outPushWeights[0] : 0;//operNode��pushֵ	,���û���������push = 0��
	int	inPopWeight = ListLength(inputList) ? flatNode->inPopWeights[0] : 0;//operNode��popֵ
	int inPeekWeight = ListLength(inputList) ? flatNode->inPeekWeights[0] : 0;//operNode��peekֵ

	Node *splitNode = NULL;//�²�����split�ڵ�
	Node *joinNode = NULL;	//�²�����join�ڵ�
	/*��split�ڵ�*/
	SplitStyle style = (inPeekWeight > inPopWeight)? S_Duplicate: S_RoundRobin;  //peek>popȡ1��ʾduplicate��ʽ��peek=popȡ0����ʾroundrobin��ʽ
	Node *input =(Node *)FirstItem(inputList);
	Node *output = (Node *)FirstItem(outputList);
	List *split_argument = NULL;//ȷ��split�ڵ������ߵ�pushֵ
	/*�����Ʒ��Ѻ��operator ��List*/
	List *split_outputList = NULL;//��¼split�ڵ����������ߣ���Ϊ�½���operator������
	List *replicateOperList = NULL;   //operNode���Ʒ��ѳ����µ�operator,List�е�Ԫ����operatorNode*

	/*����replicationFactor������Ϊsplit�������*/
	split_outputList = MakeSplitOutputStream(input,splitedNodeVec.size());//����split�ڵ�����������

	/*���첻ͬ���͵�split�ڵ�*/
	if(style == S_Duplicate)
	{  //����duplicate����Ĳ���
		int split_pop = inPopWeight * steadyCount;
		Node *duplicate_value =MakeConstSint(split_pop);
		split_argument = AppendItem(split_argument,duplicate_value);
		splitNode = MakeFissionSplitComposite(input,split_outputList,split_argument,style);
	}
	else 
	{  //����roundrobin����Ĳ���
		for (int i=0; i < splitedNodeVec.size(); i++)
			split_argument = AppendItem(split_argument,MakeConstSint(inPopWeight*(splitedNodeVec[i]->fissedSteadycount)));		
		splitNode = MakeFissionSplitComposite(input,split_outputList,split_argument,style);
	}
	
	List *join_inputList = NULL;
	List *join_argument = NULL;

	replicateOperList = MakeReplicateOper(splitedNodeVec,flatNode,split_outputList,steadyCount,style);

	ListMarker fissionOper_maker;
	Node *fissionOper = NULL;
	/*ȡÿһ���µ�operator�������Ϊjoin������*/
	IterateList(&fissionOper_maker,replicateOperList);   /*replicateOperList��ɵ���*/
	while (NextOnList(&fissionOper_maker,(GenericREF)&fissionOper))
	{
		join_inputList = AppendItem(join_inputList,(Node *)FirstItem(fissionOper->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.outputs));
	}
   
	/*��join��㷽ʽ��roundrobin*/ 
	for (int i=0; i < splitedNodeVec.size(); i++)
		join_argument = AppendItem(join_argument,MakeConstSint(outPushWeight*(splitedNodeVec[i]->fissedSteadycount)));
	joinNode = MakeFissionJoinComposite(join_inputList,output,join_argument);

	assert(splitNode && joinNode && replicateOperList);
	fissionOperList = AppendItem(fissionOperList,splitNode);//���²�����operatorȫ�����ӵ�һ��List��
	fissionOperList = JoinLists(fissionOperList,replicateOperList);
	fissionOperList = AppendItem(fissionOperList,joinNode);
	return fissionOperList;
}

 /*��flatNode����α���ѣ���Ҫ����PartitonNum2FissionNode��FlatNode2FissionNode�н��������Ϣ���޸�*/
 void HorizontalFission::SplitFissionNode(FissionNodeInfo *fissionNode,int replicationFactor,int max_partiotion,int min_partiotion)
 {
	 assert(fissionNode && replicationFactor>0 );
	 int steadyCount = fissionNode->fissedSteadycount;  //ȡ�ýڵ������̬�����е�ִ�д���
	 assert(steadyCount>0);

	 if(replicationFactor > steadyCount) replicationFactor = steadyCount;//������Ѵ���������ִ̬�д���������Ѵ���ȡִ�д���
	 assert(fissionNode->fissed_pop.size()<=1 && fissionNode->fissed_push.size()<=1 && fissionNode->fissed_peek.size()<=1);//ȷ���ǵ��뵥����


	 int outPushWeight =fissionNode->fissingFlatNode->outPushWeights.size()==1? fissionNode->fissingFlatNode->outPushWeights[0] : 0;//operNode��pushֵ	,���û���������push = 0��
	 int inPopWeight =fissionNode->fissingFlatNode->inPopWeights.size()==1? fissionNode->fissingFlatNode->inPopWeights[0] : 0;//operNode��popֵ
	 int inPeekWeight = fissionNode->fissingFlatNode->inPeekWeights.size()==1 ? fissionNode->fissingFlatNode->inPeekWeights[0] : 0;//operNode��peekֵ
	 /*ȷ�����õķ��ѷ�ʽ*/
	 SplitStyle style = (inPeekWeight > inPopWeight)? S_Duplicate: S_RoundRobin;  //peek>popȡ1��ʾduplicate��ʽ��peek=popȡ0����ʾroundrobin��ʽ

	 /*������Ѳ�����ÿһ������̬ʱִ�еĴ���*/
	 vector<int> newSteadyCount;//��ŷ��Ѻ��ÿһ������ִ̬�еĴ���
	 for(int i = 0;i<replicationFactor-1;i++)
		 newSteadyCount.push_back(steadyCount/replicationFactor);
	 if(steadyCount%replicationFactor==0) newSteadyCount.push_back(steadyCount/replicationFactor);
	 else newSteadyCount.push_back(steadyCount - steadyCount/replicationFactor*(replicationFactor-1));
	
	 //���ݲ�ͬ�ķ��ѷ�ʽȷ��������������ز�����ֵ
	 if(style == S_Duplicate)
	 {  //����duplicate����Ĳ���
		// int deltpeek = 0;
		 /*��һ�������ŵ���������С�Ļ�����*/
		 FissionNodeInfo * firstFissionNode = HeapNew(FissionNodeInfo);
		 firstFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
		 firstFissionNode->fissedSteadycount = newSteadyCount[0];
		 firstFissionNode->operatorWeight = fissionNode->operatorWeight;
		 firstFissionNode->fissed_peek.push_back(inPeekWeight+inPopWeight*(steadyCount-1));//����һ������һ����ִ̬���ܹ���Ҫpeek��ֵ
		 firstFissionNode->fissed_pop.push_back(inPopWeight*steadyCount);//����һ������һ����ִ̬���ܹ���Ҫpop��ֵ
		 firstFissionNode->fissed_push.push_back(outPushWeight*newSteadyCount[0]);//����һ������һ����ִ̬���ܹ���Ҫpush��ֵ
		 firstFissionNode->npart = min_partiotion;
		 /*�趨���Ѻ���ѽڵ��fiss_state��ֵ*/
		 if( firstFissionNode->fissed_pop[0]==firstFissionNode->fissed_peek[0]) firstFissionNode->fiss_state = TRUE;//����Ҫ����ִ̬�д����Ƶ�work�����ڲ�(peek=pop�ſ���)
		 else firstFissionNode->fiss_state = FALSE;
		 PartitonNum2FissionNode.insert(make_pair(min_partiotion,firstFissionNode));
		 FlatNode2FissionNode.insert(make_pair(fissionNode->fissingFlatNode,firstFissionNode));
		 /*����һ���������еĸ������ڴ����ѽڵ��������ڵĻ�����*/
		 for(int i=1; i < replicationFactor; i++)
		 {
			 FissionNodeInfo * tmpFissionNode = HeapNew(FissionNodeInfo);
			// deltpeek += inPopWeight * newSteadyCount[i-1];
			 tmpFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
			 tmpFissionNode->fissedSteadycount = newSteadyCount[i];
			 tmpFissionNode->operatorWeight = fissionNode->operatorWeight;
			 tmpFissionNode->fissed_peek.push_back(inPeekWeight+inPopWeight*(steadyCount-1));//����һ������һ����ִ̬���ܹ���Ҫpeek��ֵ
			 tmpFissionNode->fissed_pop.push_back(inPopWeight*steadyCount);//����һ������һ����ִ̬���ܹ���Ҫpop��ֵ
			 tmpFissionNode->fissed_push.push_back(outPushWeight*newSteadyCount[i]);//����һ������һ����ִ̬���ܹ���Ҫpush��ֵ
			 tmpFissionNode->npart = max_partiotion;
			 /*�趨���Ѻ���ѽڵ��fiss_state��ֵ*/
			 if(tmpFissionNode->fissed_pop[0]==tmpFissionNode->fissed_peek[0]) tmpFissionNode->fiss_state = TRUE;//����Ҫ����ִ̬�д����Ƶ�work�����ڲ�
			 else tmpFissionNode->fiss_state = FALSE;
			 PartitonNum2FissionNode.insert(make_pair(max_partiotion,tmpFissionNode));
			 FlatNode2FissionNode.insert(make_pair(fissionNode->fissingFlatNode,tmpFissionNode));
		 }
		 /*��fissionNode��PartitonNum2FissionNode��FlatNode2FissionNode��ɾ��*/
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
	 {  //����roundrobin����Ĳ���
		 /*��һ�������ŵ���������С�Ļ�����*/
		 FissionNodeInfo * firstFissionNode = HeapNew(FissionNodeInfo);
		 firstFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
		 firstFissionNode->fissedSteadycount = newSteadyCount[0];
		 firstFissionNode->operatorWeight = fissionNode->operatorWeight;
		 firstFissionNode->fissed_peek.push_back(newSteadyCount[0]*inPeekWeight);//����һ������һ����ִ̬���ܹ���Ҫpeek��ֵ
		 firstFissionNode->fissed_pop.push_back(inPopWeight*newSteadyCount[0]);//����һ������һ����ִ̬���ܹ���Ҫpop��ֵ
		 firstFissionNode->fissed_push.push_back(outPushWeight*newSteadyCount[0]);//����һ������һ����ִ̬���ܹ���Ҫpush��ֵ
		 firstFissionNode->npart = min_partiotion;
		 /*�趨���Ѻ���ѽڵ��fiss_state��ֵ*/
		 if(firstFissionNode->fissed_pop[0]==firstFissionNode->fissed_peek[0]) firstFissionNode->fiss_state = TRUE;//����Ҫ����ִ̬�д����Ƶ�work�����ڲ�
		 else firstFissionNode->fiss_state = FALSE;
		 PartitonNum2FissionNode.insert(make_pair(min_partiotion,firstFissionNode));
		 FlatNode2FissionNode.insert(make_pair(fissionNode->fissingFlatNode,firstFissionNode));
		 /*����һ���������еĸ������ڴ����ѽڵ��������ڵĻ�����*/
		 for(int i=1; i < replicationFactor; i++)
		 {
			 FissionNodeInfo * tmpFissionNode = HeapNew(FissionNodeInfo);
			 tmpFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
			 tmpFissionNode->fissedSteadycount = newSteadyCount[i];
			 tmpFissionNode->operatorWeight = fissionNode->operatorWeight;
			 tmpFissionNode->fissed_peek.push_back(newSteadyCount[i]*inPeekWeight);//����һ������һ����ִ̬���ܹ���Ҫpeek��ֵ
			 tmpFissionNode->fissed_pop.push_back(inPopWeight*newSteadyCount[i]);//����һ������һ����ִ̬���ܹ���Ҫpop��ֵ
			 tmpFissionNode->fissed_push.push_back(outPushWeight*newSteadyCount[i]);//����һ������һ����ִ̬���ܹ���Ҫpush��ֵ
			 tmpFissionNode->npart = max_partiotion;
			 /*�趨���Ѻ���ѽڵ��fiss_state��ֵ*/
			 if(tmpFissionNode->fissed_pop[0]==tmpFissionNode->fissed_peek[0]) tmpFissionNode->fiss_state = TRUE;//����Ҫ����ִ̬�д����Ƶ�work�����ڲ�
			 else tmpFissionNode->fiss_state = FALSE;
			 PartitonNum2FissionNode.insert(make_pair(max_partiotion,tmpFissionNode));
			 FlatNode2FissionNode.insert(make_pair(fissionNode->fissingFlatNode,tmpFissionNode));
		 }
		 /*��fissionNode��PartitonNum2FissionNode��FlatNode2FissionNode��ɾ��*/
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
	*��һ��operator��չ�����composite,�ú������ǽ�composite��չ��һ��composite��
	*��չ���composite�������������Ȼ��ԭ����operator�������������
	*ֻ�ǽ�ԭoperator�����Ķ����������Ϊcomposite�����������
********************************************************************************/
Node *HorizontalFission::MakeCompositeNode(Node *operNode)
{
	assert(operNode);
	Node *composite = NewNode(Composite);//���ת�����composite

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
	/*ʹcomposite���������ȫ����Decl���ͣ���operator���������ȫ����Id����*/
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
	/*�������*/
	Node *com_InOutNode = MakeComInOutCoord(EMPTY_TQ,composite_inputList,composite_outputList,UnknownCoord);
	
	/*composite��ͷ*/
	char *newName = (char *)malloc(strlen(operNode->u.operator_.decl->u.decl.name) + 50); // �ܱ�ʾ���������Ӧ�ò�����20λ
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

/*Ҫ��join������߲����flatNode�Լ���Ӧ�Ľṹ��*/
void HorizontalFission::InsertJoinOutStreamToSSG(Node *streamNode,FlatNode *topFlatNode,SchedulerSSG *sssg)
{//topFlatNode��join�ڵ�
	assert(streamNode && topFlatNode );
	assert(streamNode->typ == Id||streamNode->typ == Decl);
	Node *stream_type = NULL;
	if (streamNode->typ == Id)   //ȡ��������
		stream_type = streamNode->u.id.decl->u.decl.type;
	else 
		stream_type = streamNode->u.decl.type;
	/*�������Ѿ���mapEdge2DownFlatNode�д�����*/
	std::multimap<Node *, FlatNode *> ::iterator pos;
	FlatNode *buttomFlatNode=NULL;
	pos = sssg->mapEdge2DownFlatNode.find(stream_type);
	assert(pos != sssg->mapEdge2DownFlatNode.end());
	buttomFlatNode = pos->second;
	sssg->mapEdge2UpFlatNode.insert(make_pair(stream_type,topFlatNode));
	topFlatNode->AddOutEdges(buttomFlatNode);

}
/*��operator�ڵ�����flatNode�ڵ㲢���뵽sssg��,��ȷ��flatNode��push��peek��popֵ*/
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
		assert(out_count<=1);//��֤һ����ֻ��Ӧһ���¶˽ڵ�
		out_pos = sssg->mapEdge2DownFlatNode.find(type);
		Bool flag = TRUE;//Ҫ�����flatNode�������¶��Ѿ����ڣ��򲻲���
		if(out_pos != sssg->mapEdge2DownFlatNode.end())
		{//��ʾ�ҵ�
			FlatNode *buttomFlatNode = out_pos->second;
			int InIter =0;
			for (InIter = 0;InIter!= buttomFlatNode->inFlatNodes.size();InIter++)
			{
				if(buttomFlatNode->inFlatNodes[InIter] == oldFlatNode)break;
			}
			if(InIter< buttomFlatNode->inFlatNodes.size() && buttomFlatNode->inFlatNodes[InIter] == oldFlatNode) buttomFlatNode->inFlatNodes[InIter] = src;//�ҵ�,�޸�ֵ(split),���û�ҵ�ʲôҲ��������Ҫ��Ϊ�˴�����Ѻ������join�ڵ㣩	
		}
		sssg->mapEdge2UpFlatNode.insert(make_pair(type, src)); //��������ߡ����䡰�ϡ���operator��
	}
	sssg->AddFlatNode(src);

	dest = src; //�߱���
	IterateList(&marker, u->decl->u.decl.type->u.operdcl.inputs);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		type = item->u.id.decl->u.decl.type;
		sssg->mapEdge2DownFlatNode.insert(make_pair(type,dest));//��������ߡ����䡰�¡���operator��

		pos = sssg->mapEdge2UpFlatNode.find(type);//�ұߵ��϶˽ڵ�
		assert(pos != sssg->mapEdge2UpFlatNode.end()); //ȷ��ÿһ������߶���operator
		src = pos->second;
		/*�������϶˽ڵ��е�outFlatNodes���Ѿ��е�ǰ�߶�Ӧ���¶˵�FlatNode����ӣ�ֻ�޸����ĵ�ǰ�߶�Ӧ��ֵ*/
		int outIter =0;
		for (outIter = 0;outIter!= src->outFlatNodes.size();outIter++)
		{
			if(src->outFlatNodes[outIter] == oldFlatNode)break;
		}
		if(outIter< src->outFlatNodes.size()) 
			src->outFlatNodes[outIter] = dest;//�ҵ�,�޸�ֵ(split)	
		else src->AddOutEdges(dest);//û�ҵ�������
		dest->AddInEdges(src);
	}
}

/*��operator�ڵ�ת����flatNode,���޸������Ϣ*/
void HorizontalFission::Operator2FlatNodeModifyInfo(FlatNode *curFlatNode, List *childOperList,SchedulerSSG *sssg, vector<int >npartVec)
{//parentFlatNode ��ʾԭʼ��operator�ڵ��Ӧ��FlatNode��childOperNode������Ѻ������operator,�·��ѳ�����operator����̬�����г��ֵĴ���
	//��ط�Ҫ����pop�Ĳ�ֵ��Ҫ��FlatNode������һ���ṹ���ڱ�ʾpop�Ĳ�ֵ��
	assert(curFlatNode&& childOperList);
	assert(curFlatNode->inFlatNodes.size()==1&& curFlatNode->outFlatNodes.size()==1);//������ȶ���1 
	int npart = 0;
 	/*�޸���curFlatNode�������flatNode�е��й�����,��Ҫ����curFlatNode�����Ϊ�����flatNode�ڵ�*/
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

	/*ɾ��sssg->flatNode��mapEdge2UpFlatNode��mapEdge2DownFlatNode����parentFlatNode�󶨵ı�*/
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

	/*��childOperList�����һ��vector*/
	ListMarker childOper_Maker;
	Node *childOperNode = NULL;
	int len = sssg->GetFlatNodes().size();   //ԭʼ����
	IterateList(&childOper_Maker ,childOperList);
	while(NextOnList(&childOper_Maker,(GenericREF)&childOperNode))
	{//��childOper�����flatNode ,(���еĽڵ㶼��composite��)
		//Node *comNode = MakeCompositeNode(childOperNode);
		assert(childOperNode->typ == Composite);
		assert(ListLength(childOperNode->u.composite.body->u.comBody.comstmts)== 1);
		Node *oper = (Node *)FirstItem(childOperNode->u.composite.body->u.comBody.comstmts);
		InsertFlatNodes(&(oper->u.operator_),&(childOperNode->u.composite),&(childOperNode->u.composite),curFlatNode,sssg);	
	}
	int newlen = sssg->GetFlatNodes().size(); //����Ӻ�ĳ���
	assert(newlen - len>=3);//���������������ڵ㣨split��join���Լ����Ʒ��Ѻ����ɵĽڵ㣩
	vector<FlatNode *> newflatNode = sssg->GetFlatNodes();
	
	/*Ҫ��join������߲����flatNode�Լ���Ӧ�Ľṹ��*/
 	FlatNode *joinFlatNode = newflatNode[newlen-1];
	Node *joinOutputStream = (Node *)FirstItem(joinFlatNode->contents->decl->u.decl.type->u.operdcl.outputs);
 	InsertJoinOutStreamToSSG(joinOutputStream,joinFlatNode,sssg);


	for(int i = len;i <newflatNode.size();i++)	
		sssg->SetFlatNodesWeights(newflatNode[i]);//�޸��²���ڵ��Ȩֵ

	/*�޸��²���ڵ���ִ̬�д���(1)ɾ��ԭ�ڵ����ִ̬��(2)�����½ڵ����ִ̬�С�������mapSteadyWork2FlatNode�����ȴ���mapSteadyWork2FlatNode*/
	int oldSteadyCount = sssg->GetSteadyCount(curFlatNode);
	sssg->mapSteadyCount2FlatNode.erase(curFlatNode);	//ɾ��ԭ�ڵ����ִ̬��
	sssg->mapInitCount2FlatNode.erase(curFlatNode);	//ɾ��ԭ�ڵ�ĳ�ִ̬��
	sssg->mapInitCount2FlatNode.erase(curFlatNode);//ɾ��ԭ�ڵ�ĳ�̬����
	sssg->mapSteadyWork2FlatNode.erase(curFlatNode);//ɾ��ԭ�ڵ����̬������
	sssg->mapInitWork2FlatNode.erase(curFlatNode);//ɾ��ԭ�ڵ�ĳ�̬������

	/*���һ����е��ѷ��ѵ����ڵĻ��ֱ��*/
	partition->FlatNode2PartitionNum.erase(curFlatNode);
	multimap<int,FlatNode *>::iterator mult_iter;
	for( mult_iter = partition->PartitonNum2FlatNode.begin();mult_iter!= partition->PartitonNum2FlatNode.end();mult_iter++)
	{
		if(mult_iter->second == curFlatNode) break;
	}

	sssg->mapSteadyCount2FlatNode.insert(make_pair(newflatNode[len],1));//�²���ĵ�һ���ڵ���split   ��ִֻ��һ��
	sssg->mapInitCount2FlatNode.insert(make_pair(newflatNode[len],0));
	sssg->mapSteadyWork2FlatNode.insert(make_pair(newflatNode[len],workEstimate((newflatNode[len])->contents->body, 0)));
	sssg->mapInitWork2FlatNode.insert(make_pair(newflatNode[len],workEstimate_init((newflatNode[len])->contents->body, 0)));
	/*��split�ڵ���ӻ�����*/
	partition->FlatNode2PartitionNum.insert(make_pair(newflatNode[len],mult_iter->first));
	partition->PartitonNum2FlatNode.insert(make_pair(mult_iter->first,newflatNode[len]));

	/*�����Ѻ�Ľڵ���ӵ������У�����һ�ݷ�����С�Ļ����У�����������ԭ���Ļ����У�*/
	
	//������ִ̬�д������Լ���̬�ĵĹ�����(�½ڵ���ִֻ̬��һ��)
	sssg->mapSteadyCount2FlatNode.insert(make_pair(newflatNode[len+1],1));
	sssg->mapInitCount2FlatNode.insert(make_pair(newflatNode[len+1],0));//��ʼ����Ĭ��Ϊ0
	sssg->mapSteadyWork2FlatNode.insert(make_pair(newflatNode[len+1],workEstimate((newflatNode[len+1])->contents->body, 0)));	
	sssg->mapInitWork2FlatNode.insert(make_pair(newflatNode[len+1],workEstimate_init((newflatNode[len+1])->contents->body, 0)));	
	partition->FlatNode2PartitionNum.insert(make_pair(newflatNode[len+1],npartVec[npart]));
	partition->PartitonNum2FlatNode.insert(make_pair(npartVec[npart++],newflatNode[len+1]));

	for (int i=len+2 ;i<newlen-1;i++)
	{
		//������ִ̬�д������Լ���̬�ĵĹ�����
		sssg->mapSteadyCount2FlatNode.insert(make_pair(newflatNode[i],1));
		sssg->mapSteadyWork2FlatNode.insert(make_pair(newflatNode[i],workEstimate((newflatNode[i])->contents->body, 0)));	
		sssg->mapInitWork2FlatNode.insert(make_pair(newflatNode[i],workEstimate_init((newflatNode[i])->contents->body, 0)));	
		sssg->mapInitCount2FlatNode.insert(make_pair(newflatNode[i],0));
		partition->FlatNode2PartitionNum.insert(make_pair(newflatNode[i],npartVec[npart]));
		partition->PartitonNum2FlatNode.insert(make_pair(npartVec[npart++],newflatNode[i]));
	}
	//��join�ڵ���ӵ�sssg��
	sssg->mapSteadyCount2FlatNode.insert(make_pair(newflatNode[newlen-1],1));
	sssg->mapSteadyWork2FlatNode.insert(make_pair(newflatNode[newlen-1],workEstimate((newflatNode[newlen-1])->contents->body, 0)));
	sssg->mapInitWork2FlatNode.insert(make_pair(newflatNode[newlen-1],workEstimate_init((newflatNode[newlen-1])->contents->body, 0)));	
	sssg->mapInitCount2FlatNode.insert(make_pair(newflatNode[newlen-1],0));
	partition->FlatNode2PartitionNum.insert(make_pair(newflatNode[newlen-1],mult_iter->first));
	partition->PartitonNum2FlatNode.insert(make_pair(mult_iter->first,newflatNode[newlen-1]));
	/*ɾ�������е��ѷ���*/
	partition->PartitonNum2FlatNode.erase(mult_iter);
}

void HorizontalFission::initFissionNodeMap(SchedulerSSG *sssg)
{//��������Ҫ��ʼ��PartitonNum2FissionNode,FlatNode2FissionNode�ĳ�ʼֵ
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


 /*ͼ��ˮƽ����*/
void HorizontalFission::HorizontalFissionPRPartitions(SchedulerSSG *sssg,float balanceFactor)
{
	initFissionNodeMap(sssg);//��ʼ�����е�����map
	while(TRUE)
	{
		map<int,int > Partiotion2Weight = computeMapPlace2Weight();//���ݳ�ԱPartitonNum2FissionNode����������ֵĳ�ʼ������
		//vector<int> sortPartiotion = SortPartitionsByWeight(Partiotion2Weight);
#if 0
		cout<<"=========================================="<<endl;
		for(map<int,int >::iterator piter = Partiotion2Weight.begin(); piter != Partiotion2Weight.end();piter++ )
		{
			cout<<piter->first << "    "<< piter->second<<endl;
		}
# endif
		int maxPartition = -1;
		int cur_max_partition; //��ǰȨ�����Ļ��ֵĵ�����
		int cur_min_partition; //��ǰȨ����С�Ļ��ֵĵ�����
		FissionNodeInfo *maxWorkFlatNode = NULL;//���Ҫ�����ѵĽڵ�
		int maxWork = 0;
		Bool partiotion_flag = FALSE;

		vector<Bool> PartiotionFlag;//��ʾÿһ���������Ƿ��пɷ��ѵĽڵ� 
		for (map<int,int>::iterator iter1 = Partiotion2Weight.begin();iter1 != Partiotion2Weight.end();++iter1)
		{
			PartiotionFlag.push_back(TRUE);
		}

		for (int i = 0 ;i< partition->getParts();i++)
		{//�ҵ�ǰ�ܹ����ѵĺ�
			cur_max_partition = NextMaxWeightPartition(Partiotion2Weight,PartiotionFlag);//�ҵ���ǰȨֵ���Ļ���
			if(cur_max_partition == Partiotion2Weight.size()) break; 
#if 0
			cout<<"$$$$$$$$$$ ���ֺţ�   "<<cur_max_partition<<"    $$$$$$$$$$$$$"<<endl;
#endif
			/*ȡ��ǰȨֵ���Ļ����е����е�operator*/
			vector<FissionNodeInfo *> fissionNodeVector = findfissionNodeSetInPartition(cur_max_partition);
			/*��󻯷��й���������operator*/
			maxWorkFlatNode = NULL;
			maxWork = 0;
			for (int fissionNodeVector_iter = 0;fissionNodeVector_iter!=fissionNodeVector.size();fissionNodeVector_iter++)
			{
				stateful = FALSE;
				hasMutableState(fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->contents);
#if 0
				if(stateful && fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount==1)cout<<fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->name<<"���ɷ� "<<endl;
				else cout<<"Ҳ��ɷ�" <<fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->name<<": SteadyCount= "<< fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount <<"  ���뵥��  "<<SInSOutOPerator(fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->contents)<<"  ״̬:"<<fissionNodeVector[fissionNodeVector_iter]->fiss_state<<endl; 
#endif
				if(!stateful && fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount>1 &&fissionNodeVector[fissionNodeVector_iter]->fiss_state==TRUE&& SInSOutOPerator(fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->contents))//���ѵ�����
				{
//					cout<<"����" <<fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->name<<": SteadyCount= "<< fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount <<"  ���뵥�� " <<"    "<<SInSOutOPerator(fissionNodeVector[fissionNodeVector_iter]->fissingFlatNode->contents)<<endl; 
					int operatorWork = fissionNodeVector[fissionNodeVector_iter]->operatorWeight; // ����operator�Ĺ�����
					int operatorCount = fissionNodeVector[fissionNodeVector_iter]->fissedSteadycount;	 //���ȶ�״̬��operator��ִ�д���
					int work_Weight= operatorWork*operatorCount;
					if(maxWork<work_Weight)	
					{
						maxWorkFlatNode = fissionNodeVector[fissionNodeVector_iter];//�ҹ��������ĺ��й���������operator
						maxWork = work_Weight;
					}
					partiotion_flag = TRUE;//�ҵ����ѵĺ˺Ϳɷ��ѵ�operator,�޸ı�־λ
				}
			}
			if(partiotion_flag ) break;
			else PartiotionFlag[cur_max_partition] = FALSE;	//�û�����û�пɷ��ѽڵ�
		}
		/*û�����������ķ��ѽڵ�*/
		if(!partiotion_flag || maxWorkFlatNode == NULL)
			break;
		assert(maxWorkFlatNode);
		/*�ҹ�������С�Ļ���*/
		cur_min_partition = MinPartitionWeight(Partiotion2Weight);
		/*��������˳�����*/
		if(Partiotion2Weight.find(cur_max_partition)->second < Partiotion2Weight.find(cur_min_partition)->second * balanceFactor)
			break;
		/*������Ѵ���*/
		int replicationFactor = maxWork /((Partiotion2Weight.find(cur_max_partition)->second) - (Partiotion2Weight.find(cur_min_partition)->second));
		replicationFactor = replicationFactor>2 ? replicationFactor:2;
#if 0
		cout<<"�����ֵܷĺˣ� "<<cur_max_partition<<"    ��С�ˣ� "<<cur_min_partition<<endl;
#endif

		/*������Ҫ����������Աmap�޸���Ϣ��ɾ����Ҫ���ѽڵ����Ϣ����ӷ��Ѻ�ĸ�������Ϣ����α����*/
		SplitFissionNode(maxWorkFlatNode,replicationFactor,cur_max_partition,cur_min_partition);
	}
	/******ȡ��FlatNode2FissionNode�и���flatNode��Ӧ�ĸ�����������Ϣ���
	 ***һ��Vector�����ݸ�horizontalFissOperator�������ķ��ѣ����һ��List��Ȼ��
	 ***�ٽ�List���ݸ�Operator2FlatNodeModifyInfo���޸�sssgͼ����Ϣ*************/
	multimap<FlatNode* ,FissionNodeInfo *>::iterator flatNodeIter ;
	while (!FlatNode2FissionNode.empty())
	{
		flatNodeIter = FlatNode2FissionNode.begin();
		vector<FissionNodeInfo *> splitedNodeVec;
		vector<int >npartVec;//��¼ÿ�����������䵽���ֵı��
		typedef multimap<FlatNode* ,FissionNodeInfo *>::iterator F2FIter;
		pair<F2FIter,F2FIter>range=FlatNode2FissionNode.equal_range(flatNodeIter->first);
		for(F2FIter iter=range.first;iter!=range.second;++iter)
		{
			splitedNodeVec.push_back(iter->second);
			npartVec.push_back((iter->second)->npart);
		}
		//����splitedNodeVec�е����нڵ㲢���split��join�ڵ�
		List *oper_list = horizontalFissOperator(splitedNodeVec,sssg);
		/*���ݷ��Ѻ������޸���Ӧ�ṹ����Ϣ*/
		Operator2FlatNodeModifyInfo(flatNodeIter->first,oper_list,sssg,npartVec);//����oper_list�޸�sssgͼ����Ϣ
		/*��FlatNode2FissionNode���Ѵ�����Ľڵ�ɾ��*/
		FlatNode2FissionNode.erase(flatNodeIter->first);
	}
	/*�ȵ�����ͼ�ȶ����ڽ��������ķ��ѣ�����ĺ�����Ҫ�޸ģ�*/

#if 0
	PrintList(stdout,oper_list,-1);
#endif

	/*�����µ�ͼ���г�̬����*/
	/*�������֮��ɾ����ʼ�������е����нڵ㣬�Ա��ܽ����µĳ�̬����*/
	//map<int,int > Partiotion2Weight1= computeMapPlace2Weight();//���ݳ�ԱPartitonNum2FissionNode����������ֵĳ�ʼ������
	sssg->mapInitCount2FlatNode.erase(sssg->mapInitCount2FlatNode.begin(),sssg->mapInitCount2FlatNode.end());
}