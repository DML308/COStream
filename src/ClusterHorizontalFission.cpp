#include "ClusterHorizontalFission.h"
#include "basics.h"
#include "ActorStateDetector.h"

PRIVATE int gCurrentFissionStreamNum = 0;
PRIVATE int HFOperatorNum = 0;//解决新构造的operator的重命名

//构造函数
ClusterHorizontialFission::ClusterHorizontialFission(map<FlatNode *,int>flatNode2partitionNum,map<FlatNode *,int> flatNode2steadycount,SchedulerSSG *tmpsssg,int np,float bf)
{
	_flatNode2partitionNum = flatNode2partitionNum;
	_flatNode2steadycount = flatNode2steadycount;
	sssg = tmpsssg;
	balanceFactor = bf;
	npartition = np;
}

/*计算各个划分的和权值并保存在一个map<划分的编号，权重>*/
map<int,int> ClusterHorizontialFission::computeMapPlace2Weight()
{
	assert(npartition>0);
	vector<int> weights(npartition,0);
	map<int,int>partition2weight;
	for(multimap<int ,FissionNodeInfo *>::iterator iter = PartitonNum2FissionNode.begin();iter != PartitonNum2FissionNode.end(); ++iter)
	{
		weights[iter->first] += iter->second->operatorWeight * iter->second->fissedSteadycount;
	}
	for (int npart = 0;npart<npartition;npart++)
	{
		partition2weight.insert(make_pair(npart,weights[npart]));
	}
	return partition2weight;
}

void ClusterHorizontialFission::initFissionNodeMap()
{//在这里面要初始化PartitonNum2FissionNode的初始值
	map<FlatNode*,int>::iterator iter;
	for(iter = _flatNode2partitionNum.begin();iter != _flatNode2partitionNum.end();++iter)
	{
		FissionNodeInfo* fissionNode = HeapNew(FissionNodeInfo);
		fissionNode->fissingFlatNode = iter->first;
		fissionNode->operatorWeight = sssg->GetSteadyWork(iter->first);
		fissionNode->fissedSteadycount = sssg->GetSteadyCount(iter->first);
		for(int i=0;i < fissionNode->fissingFlatNode->inPopWeights.size();i++)
		{
			fissionNode->fissed_pop.push_back(fissionNode->fissingFlatNode->inPopWeights[i]);
		}
		for(int i=0;i < fissionNode->fissingFlatNode->inPeekWeights.size();i++)
		{
			fissionNode->fissed_peek.push_back(fissionNode->fissingFlatNode->inPeekWeights[i]);
		}
		for(int i=0;i < fissionNode->fissingFlatNode->outPushWeights.size();i++)
		{
			fissionNode->fissed_push.push_back(fissionNode->fissingFlatNode->outPushWeights[i]);
		}
		fissionNode->npart = iter->second;
		fissionNode->fiss_state = TRUE;
		PartitonNum2FissionNode.insert(make_pair(iter->second,fissionNode));
		if(iter->first->contents->ot==Join_ || iter->first->contents->ot==Roundrobin_ ||iter->first->contents->ot==Duplicate_) continue;
	}
}

/*按工作量大小进行排序--从小到大排*/
vector<int> ClusterHorizontialFission::SortPartitionbyWeight(map<int,int>partition2weight)
{
	vector<int> sortPartitions;
	vector<Bool> PartiotionTag; 
	for (map<int,int>::iterator iter1 = partition2weight.begin();iter1 != partition2weight.end();++iter1)
	{
		PartiotionTag.push_back(FALSE);
	}
	for (map<int,int>::iterator iter1 = partition2weight.begin();iter1 != partition2weight.end();++iter1)
	{
		map<int,int>::iterator ix = partition2weight.begin();
		while (PartiotionTag[ix->first]) ix++;
		for(map<int,int>::iterator iter2 = partition2weight.begin();iter2 != partition2weight.end();++iter2)
		{
			if(!PartiotionTag[iter2->first]&&iter2->second < ix->second) ix = iter2;
		}
		sortPartitions.push_back(ix->first);
		PartiotionTag[ix->first] = TRUE;
	}
	return sortPartitions;
}

Bool ClusterHorizontialFission::SInSOutOPerator(FlatNode *operFlatNode)// 20130119 zww 将参数修改为flatNode，检测单入单出且push，pop不为0的节点
{//如果该operator是单入单出的返回True 
	operatorNode *oper = operFlatNode->contents;
	assert(oper);
	List *inputList = oper->decl->u.decl.type->u.operdcl.inputs;
	List *outputList = oper->decl->u.decl.type->u.operdcl.outputs;
	if((ListLength(outputList)==1)&&(ListLength(inputList)==1)&&operFlatNode->inPopWeights[0] && operFlatNode->outPushWeights[0]) return TRUE;// 20130119 zww 将参数修改为flatNode，检测单入单出且push，pop不为0的节点
	else return FALSE;
}

vector<ClusterHorizontialFission::FissionNodeInfo *> ClusterHorizontialFission::findFissionflatNodesInPartition(int npart)
{	//取当前划分中可被分裂的所有flatNode节点
	vector<FissionNodeInfo *>tmpfissionFlatNodes;
	tmpfissionFlatNodes.clear();
	typedef multimap<int,FissionNodeInfo*>::iterator Num2NodeIter;
	pair<Num2NodeIter,Num2NodeIter>range=PartitonNum2FissionNode.equal_range(npart);
	for(Num2NodeIter iter=range.first;iter!=range.second;++iter)
	{
		//cout<<iter->second->name<<" ";
		assert(npart == iter->second->npart);
		if(SInSOutOPerator( iter->second->fissingFlatNode) && ( iter->second->fissedSteadycount > 1) 
			&& (!DetectiveActorState(iter->second->fissingFlatNode))) 
			tmpfissionFlatNodes.push_back(iter->second);
	}
	return tmpfissionFlatNodes;
}

/*将flatNode进行伪分裂，主要是在PartitonNum2FissionNode和FlatNode2FissionNode中进行相关信息的修改*/
void ClusterHorizontialFission::SplitFissionNode(FissionNodeInfo *fissionNode,int replicationFactor,int max_partiotion,int min_partiotion)
{
	//cout<<"分裂"<<endl;
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

	/*计算分裂产生的每一份在稳态时执行的次数*/
	vector<int> newSteadyCount;//存放分裂后的每一份在稳态执行的次数
	for(int i = 0;i<replicationFactor-1;i++)
		newSteadyCount.push_back(steadyCount/replicationFactor);
	if(steadyCount%replicationFactor==0) newSteadyCount.push_back(steadyCount/replicationFactor);
	else newSteadyCount.push_back(steadyCount - steadyCount/replicationFactor*(replicationFactor-1));
	//先确定怎么分，然后再考虑各个副本的偏移，以及各个的peek，pop和push的值
	set<FissionNodeInfo *>tmpfissionNodeInfoSet;
	//将第一个副本放在min_partiotion中
	FissionNodeInfo * tmpFissionNode = HeapNew(FissionNodeInfo);
	tmpFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
	tmpFissionNode->fissedSteadycount = newSteadyCount[0];
	tmpFissionNode->operatorWeight = fissionNode->operatorWeight;
	tmpFissionNode->npart = min_partiotion;//第一个副本留在min_partiotion中
	//tmpFissionNode->style = fissionNode->style;
	PartitonNum2FissionNode.insert(make_pair(min_partiotion,tmpFissionNode));
	tmpfissionNodeInfoSet.insert(tmpFissionNode);

	for (int i = 1; i < replicationFactor; i++)
	{//将分裂的副本构造成一个set（确定fissedSteadycount，operatorWeight，npart，style）
		FissionNodeInfo * _tmpFissionNode = HeapNew(FissionNodeInfo);
		_tmpFissionNode->fissingFlatNode = fissionNode->fissingFlatNode;
		_tmpFissionNode->fissedSteadycount = newSteadyCount[i];
		_tmpFissionNode->operatorWeight = fissionNode->operatorWeight;
		_tmpFissionNode->npart = max_partiotion;//replicationFactor-1个副本留在max_partiotion中
		//_tmpFissionNode->style = fissionNode->style;
		PartitonNum2FissionNode.insert(make_pair(max_partiotion,_tmpFissionNode));
		tmpfissionNodeInfoSet.insert(_tmpFissionNode);
	}
	assert(tmpfissionNodeInfoSet.size() == replicationFactor);
	//1.先看flatNode在不在，(不在直接插入）//2.flatNode中的fissionNode在不在flatNode对应的set中（在先删除fissionNode，再插入set）
	pair<map<FlatNode *,set<FissionNodeInfo *> >::iterator,bool> ret = FlatNode2FissionNode.insert(make_pair(fissionNode->fissingFlatNode, tmpfissionNodeInfoSet));
	if(!ret.second)
	{
		(ret.first->second).erase(fissionNode);
		(ret.first->second).insert(tmpfissionNodeInfoSet.begin(),tmpfissionNodeInfoSet.end());
	}
	/*将fissionNode从PartitonNum2FissionNode*/
	multimap<int,FissionNodeInfo*>::iterator iter;
	for(iter = PartitonNum2FissionNode.begin();iter != PartitonNum2FissionNode.end();++iter)
	{
		if(iter->second == fissionNode) break;
	}
	assert(iter!=  PartitonNum2FissionNode.end());
	PartitonNum2FissionNode.erase(iter);
}

void ClusterHorizontialFission::modifyFissionNodeInfo()
{	//根据各个flatNode副本的信息确定pop，peek以及push的值,同时确定要不要在副本的上端造一个split节点
	for(map<FlatNode *,set<FissionNodeInfo*> >::iterator iter = FlatNode2FissionNode.begin(); iter != FlatNode2FissionNode.end();iter++ )
	{
		int original_steadycount = _flatNode2steadycount.find(iter->first)->second;
		int original_peek = iter->first->inPeekWeights[0];
		int original_pop = iter->first->inPopWeights[0];
		int original_push = iter->first->outPushWeights[0];
		for(set<FissionNodeInfo *>::iterator _copy_iter = iter->second.begin();_copy_iter != iter->second.end();_copy_iter++)
		{
			//确定peek，pop,push的值，以及work产生的方式
			if(original_peek == original_pop)
			{//在实际构造节点是要加一个偏移量;该种情况节点的稳定状态不需要移入work内
				(*_copy_iter)->fissed_peek.push_back(original_peek);
				(*_copy_iter)->fissed_pop.push_back(original_pop);
				(*_copy_iter)->fissed_push.push_back(original_push);
				(*_copy_iter)->fiss_state = TRUE;//稳定状态不需要移入work内
				//cout<<" 1#  "<<(*_copy_iter)->fissed_peek[0]<<"   "<<(*_copy_iter)->fissed_pop[0]<<"   "<<(*_copy_iter)->fissed_push[0]<<endl;
			}
			else
			{//original_peek > original_pop
				(*_copy_iter)->fissed_peek.push_back(original_peek + original_pop *original_steadycount - 1);
				(*_copy_iter)->fissed_pop.push_back(original_pop *original_steadycount);
				(*_copy_iter)->fissed_push.push_back(original_push * (*_copy_iter)->fissedSteadycount);
				(*_copy_iter)->fiss_state = FALSE;//稳定状态需要移入work内，然后稳态执行1次
				//cout<<" 2#  "<<(*_copy_iter)->fissed_peek[0]<<"   "<<(*_copy_iter)->fissed_pop[0]<<"   "<<(*_copy_iter)->fissed_push[0]<<endl;
			}
		}
		//确定节点的分裂方式
		if(original_peek > original_pop) _FlatNode2splitStyle.insert(make_pair(iter->first, S_Duplicate));
		else  _FlatNode2splitStyle.insert(make_pair(iter->first, S_RoundRobin));
		//根据条件不同确定_FlatNode2makeUpNodeTag和_FlatNode2makeDownNodeTag
		vector<FlatNode *>upflatNodes = iter->first->inFlatNodes;
		assert(upflatNodes.size() == 1);
		if(upflatNodes[0]->contents->ot == Common_ || upflatNodes[0]->contents->ot == Join_) _FlatNode2makeUpNodeTag.insert(make_pair(iter->first, TRUE));
		else if(upflatNodes[0]->contents->ot == Duplicate_)
		{
			map<FlatNode *,int>::iterator tmpIter = _flatNode2partitionNum.find(iter->first);
			if(tmpIter != _flatNode2partitionNum.end())_FlatNode2makeUpNodeTag.insert(make_pair(iter->first, FALSE));//上端节点在该划分中
			else _FlatNode2makeUpNodeTag.insert(make_pair(iter->first, TRUE));//上端节点不在该划分中（造什么样的split待定）
		}
		else //对于普通的上端节点则为S_RoundRobin
		{
			map<FlatNode *,int>::iterator tmpIter = _flatNode2partitionNum.find(iter->first);
			if(tmpIter != _flatNode2partitionNum.end() && original_peek == original_pop)_FlatNode2makeUpNodeTag.insert(make_pair(iter->first, FALSE));//上端节点在该划分中，且peek==pop，不造
			else _FlatNode2makeUpNodeTag.insert(make_pair(iter->first, TRUE));//上端节点不在该划分中，要造一个duplicate的节点
		}
		vector<FlatNode *>downflatNodes = iter->first->outFlatNodes;
		assert(downflatNodes.size() == 1);
		if(downflatNodes[0]->contents->ot != Join_ ) _FlatNode2makeDownNodeTag.insert(make_pair(iter->first, TRUE));
		else
		{
			map<FlatNode *,int>::iterator tmpIter = _flatNode2partitionNum.find(iter->first);
			if(tmpIter != _flatNode2partitionNum.end())_FlatNode2makeDownNodeTag.insert(make_pair(iter->first, FALSE));//下端节点在该划分中
			else _FlatNode2makeDownNodeTag.insert(make_pair(iter->first, TRUE));//下端节点不在该划分中
		}
	}
	assert(_FlatNode2makeUpNodeTag.size() == FlatNode2FissionNode.size() && _FlatNode2makeDownNodeTag.size() == FlatNode2FissionNode.size() );
	assert(_FlatNode2splitStyle.size() == FlatNode2FissionNode.size());
}

void ClusterHorizontialFission::MWISD_List(List *l,Node *old_inputDecl,Node *old_outputDecl,Node *inputId,Node *outputId,Node *inputDelt,Node *outputDelt)
{
	ListMarker _listwalk_marker; Node *_listwalk_ref; 
	IterateList(&_listwalk_marker, l); 
	while (NextOnList(&_listwalk_marker, (GenericREF)&_listwalk_ref)) { 
		if (_listwalk_ref) 
		{ 
			MWISD_astwalk(_listwalk_ref,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);
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
void ClusterHorizontialFission::MWISD_astwalk(Node *n,Node *old_inputDecl,Node *old_outputDecl,Node *inputId,Node *outputId,Node *inputDelt,Node *outputDelt)
{//修改work中从输入流中取元素的位置
	switch ((n)->typ) { 
	 case Const:         break;
	 case Id:            break;
	 case Binop:		 if ((n)->u.binop.left) {MWISD_astwalk((n)->u.binop.left,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);}if ((n)->u.binop.right) {MWISD_astwalk((n)->u.binop.right,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Unary:         if ((n)->u.unary.expr)  {MWISD_astwalk((n)->u.unary.expr,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);}	 break; 
	 case Cast:          if ((n)->u.cast.type) {MWISD_astwalk((n)->u.cast.type,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.cast.expr) {MWISD_astwalk((n)->u.cast.expr,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Comma:         if ((n)->u.comma.exprs) {MWISD_List((n)->u.comma.exprs,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Ternary:       if ((n)->u.ternary.cond) {MWISD_astwalk((n)->u.ternary.cond,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.ternary.true_) {MWISD_astwalk((n)->u.ternary.true_,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.ternary.false_) {MWISD_astwalk((n)->u.ternary.false_,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break;
	 case Array:        {
						 if ((n)->u.array.name) {MWISD_astwalk((n)->u.array.name,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);}
						 if ((n)->u.array.dims) {MWISD_List((n)->u.array.dims, old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} 
						 //具体处理
						 if(n->u.array.name->u.id.decl->u.decl.type->typ == STRdcl)//表明数组的名是一个stream
						 {
							 Node *tmpDecl = n->u.array.name->u.id.decl;
							 if(tmpDecl->u.decl.name == old_inputDecl->u.decl.name)
							 {
								 n->u.array.name = inputId; 
								 List *dimList = NULL;
								 assert(ListLength(n->u.array.dims)==1);
								 Node *dim = (Node *)FirstItem(n->u.array.dims);
								 Node *binNode = MakeBinopCoord('+',dim,inputDelt,UnknownCoord);
								 dimList = AppendItem(dimList,binNode);
								 n->u.array.dims = dimList;
							 }
							 else if(tmpDecl->u.decl.name == old_outputDecl->u.decl.name)
							 {
								 n->u.array.name = outputId;
								 assert(ListLength(n->u.array.dims)==1);
								 Node *dim = (Node *)FirstItem(n->u.array.dims);
								 Node *binNode = MakeBinopCoord('+',dim,outputDelt,UnknownCoord);
								 List *dimList = NULL;
								 dimList = AppendItem(dimList,binNode);
								 n->u.array.dims = dimList;
							 }
							 else {cout<<"fissed error...";UNREACHABLE;}
						 }
						 break;
						}
	 case Call:          if ((n)->u.call.args) {MWISD_List((n)->u.call.args, old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Initializer:   if ((n)->u.initializer.exprs) {MWISD_List((n)->u.initializer.exprs,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case ImplicitCast:  if ((n)->u.implicitcast.expr) {MWISD_astwalk((n)->u.implicitcast.expr, old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Label:         if ((n)->u.label.stmt) {MWISD_astwalk((n)->u.label.stmt,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Switch:        if ((n)->u.Switch.expr) {MWISD_astwalk((n)->u.Switch.expr , old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.Switch.stmt) {MWISD_astwalk((n)->u.Switch.stmt,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Case:          if ((n)->u.Case.expr) {MWISD_astwalk((n)->u.Case.expr ,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.Case.stmt) {MWISD_astwalk((n)->u.Case.stmt, old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Default:       if ((n)->u.Default.stmt) {MWISD_astwalk((n)->u.Default.stmt,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case If:            if ((n)->u.If.expr) {MWISD_astwalk((n)->u.If.expr,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.If.stmt) {MWISD_astwalk((n)->u.If.stmt,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case IfElse:        if ((n)->u.IfElse.expr) {MWISD_astwalk((n)->u.IfElse.expr,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.IfElse.true_) {MWISD_astwalk((n)->u.IfElse.true_, old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.IfElse.false_) {MWISD_astwalk((n)->u.IfElse.false_,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break;
	 case While:         if ((n)->u.While.expr) {MWISD_astwalk((n)->u.While.expr,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.While.stmt) {MWISD_astwalk((n)->u.While.stmt,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Do:            if ((n)->u.Do.stmt) {MWISD_astwalk((n)->u.Do.stmt, old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.Do.expr) {MWISD_astwalk((n)->u.Do.expr, old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case For:           if ((n)->u.For.init) {MWISD_astwalk((n)->u.For.init,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.For.cond) {MWISD_astwalk((n)->u.For.cond,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.For.next) {MWISD_astwalk((n)->u.For.next,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.For.stmt) {MWISD_astwalk((n)->u.For.stmt,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Goto:          break; 
	 case Continue:      break; 
	 case Break:         break; 
	 case Return:        if ((n)->u.Return.expr) {MWISD_astwalk((n)->u.Return.expr,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Block:         if ((n)->u.Block.decl) {MWISD_List((n)->u.Block.decl,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.Block.stmts) {MWISD_List((n)->u.Block.stmts,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Prim:          break; 
	 case Tdef:          break; 
	 case Ptr:           if ((n)->u.ptr.type) {MWISD_astwalk((n)->u.ptr.type,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Adcl:          if ((n)->u.adcl.type) {MWISD_astwalk((n)->u.adcl.type,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.adcl.dim) {MWISD_astwalk((n)->u.adcl.dim,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Fdcl:          break; 
	 case Sdcl:          break; 
	 case Udcl:          break; 
	 case Edcl:          break; 
	 case Decl:          if ((n)->u.decl.type) {MWISD_astwalk((n)->u.decl.type,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.decl.init) {MWISD_astwalk((n)->u.decl.init,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.decl.bitsize) {MWISD_astwalk((n)->u.decl.bitsize,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Attrib:        if (n->u.attrib.arg) {MWISD_astwalk(n->u.attrib.arg,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Proc:          if ((n)->u.proc.decl) {MWISD_astwalk((n)->u.proc.decl,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} if ((n)->u.proc.body) {MWISD_astwalk((n)->u.proc.body,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);} break; 
	 case Text:          break; 
	 default:            FAIL("Unrecognized node type"); break; 
	}
}

/*修改operator中的window,主要是输入流的peek，pop，输出流的push，这里仅处理单输入单输出的operator的窗口*/
List *ClusterHorizontialFission::ModifyWindow(List *inputList,List *outputList,int popValue,int peekValue,int pushValue)
{//不管原来的窗口有没有，现在按照新的值重新构造	 
	assert(inputList);
	assert(outputList);
	assert(ListLength(inputList) == ListLength(outputList) == 1);
	assert(ListLength(inputList)==1);
	Node *inputNode = (Node *)FirstItem(inputList);
	Node *outputNode = (Node *)FirstItem(outputList);
	Node *inputDecl = NULL;
	Node *outputDecl = NULL;
	if(inputNode->typ == Id) inputDecl = inputNode->u.id.decl;
	else inputDecl = inputNode;
	if(outputNode->typ == Id) outputDecl = outputNode->u.id.decl;
	else outputDecl = outputNode;
	assert(inputDecl && inputDecl->typ == Decl);
	assert(outputDecl &&outputDecl->typ == Decl);
	List *finalWinList = NULL;//用于返回最后修改后的window
	//输入窗口为空，建一个输入window
	List *slidings = NULL;
	slidings = AppendItem(AppendItem(slidings,MakeConstSint(peekValue)),MakeConstSint(popValue));
	Node *sliding_value = MakeCommaCoord(slidings,UnknownCoord);
	Node *sliding = MakeWindowSlidingCoord(EMPTY_TQ,sliding_value,UnknownCoord);
	Node *input_winIdNode = MakeNewStreamId(inputDecl->u.decl.name,inputDecl);
	Node *input_Window = MakeWindowCoord(input_winIdNode,sliding,UnknownCoord); //输入窗口

	Node *tumbling = MakeWindowTumbingCoord(EMPTY_TQ, MakeConstSint(pushValue), UnknownCoord);  //push
	Node *output_winIdNode = MakeNewStreamId(outputDecl->u.decl.name,outputDecl);
	Node *output_Window = MakeWindowCoord(output_winIdNode,tumbling,UnknownCoord);//输出边的窗口
	finalWinList = AppendItem(finalWinList,input_Window);
	finalWinList = AppendItem(finalWinList,output_Window);
	return finalWinList;
}

Node *ClusterHorizontialFission::MakeCopyOperatorNode_Duplicate(FissionNodeInfo *copyNodeInfo, Node *inputNode,Node *outputNode,int oldpopValue, int oldpeekValue,int oldpushValue,int deltpeek)
{//制造以duplicate方式分裂的副本operator节点，同时替换完输入输出边，以及peek的偏移
	assert(copyNodeInfo && copyNodeInfo->fiss_state==FALSE);
	assert(inputNode->typ == Decl && outputNode->typ == Decl);
	Node *form = MakeOperator(copyNodeInfo->fissingFlatNode->contents->decl,copyNodeInfo->fissingFlatNode->contents->body);
	Node *copyNode = NodeCopy(form,Subtree);
	//修改copyNode的输出输出以及以及偏移
	Node *inputId = MakeNewStreamId(inputNode->u.decl.name,inputNode);//构造输入流对应的idNode
	Node *outputId = MakeNewStreamId(outputNode->u.decl.name,outputNode);//构造输出流对应的idNode
	Node *old_inputNode =(Node *)FirstItem(copyNode->u.operator_.decl->u.decl.type->u.operdcl.inputs);
	Node *old_outputNode = (Node *)FirstItem(copyNode->u.operator_.decl->u.decl.type->u.operdcl.outputs);
	Node *old_inputDecl = NULL;
	Node *old_outputDecl = NULL;
	if(old_inputNode->typ == Id) old_inputDecl = old_inputNode->u.id.decl;
	else  old_inputDecl = old_inputNode;
	if(old_outputNode->typ == Id) old_outputDecl = old_outputNode->u.id.decl;
	else old_outputDecl = old_outputNode;
	
	List *inputList = NULL;
	List *outputList = NULL;
	inputList = AppendItem(inputList, inputId);
	outputList = AppendItem(outputList,outputId);
	//修改副本的头的输入输出——新的输入输出
	copyNode->u.operator_.decl->u.decl.type->u.operdcl.inputs = inputList;
	copyNode->u.operator_.decl->u.decl.type->u.operdcl.outputs = outputList;
	//修改work的体

	/*构造控制work循环变量的id节点*/
	Node *workVarDecl = MakeDeclCoord("__operator_work",T_BLOCK_DECL,MakePrimCoord(EMPTY_TQ,Sint,UnknownCoord),MakeConstSint(0),NULL,UnknownCoord);//用work作为次数关键字 
	Node *workVarId  = MakeIdCoord("__operator_work", UnknownCoord);
	workVarId->u.id.decl = workVarDecl;
	REFERENCE(workVarDecl);

	Node *inputDelt = NULL;
	inputDelt = MakeBinopCoord('*',workVarId,MakeConstSint(oldpopValue),UnknownCoord);
	if(deltpeek) inputDelt = MakeBinopCoord('+',inputDelt,MakeConstSint(deltpeek),UnknownCoord); //duplicate方式deltpeek一般不为0，RoundRobin方式deltpeek=0
	Node *outputDelt = MakeBinopCoord('*',workVarId,MakeConstSint(oldpushValue),UnknownCoord);
	Node *workNode = copyNode->u.operator_.body->u.operBody.work;
	assert(workNode);
	MWISD_astwalk(workNode,old_inputDecl,old_outputDecl,inputId,outputId,inputDelt,outputDelt);
	//在work外面构造一个循环
	Node *forNode = NULL;
	Node *forInit = MakeBinopCoord('=',workVarId,MakeConstSint(0),UnknownCoord);
	Node *forCond = MakeBinopCoord('<',workVarId,MakeConstSint(copyNodeInfo->fissedSteadycount),UnknownCoord);
	Node *forNext = MakeUnaryCoord(POSTINC,workVarId,UnknownCoord);
	Node *forStmt = workNode;
	forNode = MakeForCoord(forInit,forCond,forNext,forStmt,UnknownCoord);
	List *newWorkDeclList = NULL;
	newWorkDeclList = AppendItem(newWorkDeclList,workVarDecl);
	List *newWorkStmtsList = NULL;
	newWorkStmtsList = AppendItem(newWorkStmtsList,forNode);
	Node *newWorkBlock = MakeBlockCoord(PrimVoid,newWorkDeclList,newWorkStmtsList,UnknownCoord,UnknownCoord);
	
	//构建新的window
	List *tmpwinList = ModifyWindow(inputList,outputList,copyNodeInfo->fissed_pop[0],copyNodeInfo->fissed_peek[0],copyNodeInfo->fissed_push[0]);
	//组成新的operator
	copyNode->u.operator_.body->u.operBody.work = newWorkBlock;
	copyNode->u.operator_.body->u.operBody.window = tmpwinList;

	//修改composite的名字
	char *operatorName = (char *)malloc(60);
	sprintf(operatorName,"%s_%s_%d","HFission",copyNode->u.operator_.decl->u.decl.name,HFOperatorNum++);
	copyNode->u.operator_.decl->u.decl.name = operatorName;

	return copyNode;
}

Node *ClusterHorizontialFission::MakeCopyOperatorNode_RoundRobin(FissionNodeInfo *copyNodeInfo, Node *inputNode,Node *outputNode,int oldpopValue, int oldpeekValue,int oldpushValue,int deltpeek)
{//制造以duplicate方式分裂的副本operator节点，同时替换完输入输出边，以及peek的偏移
	assert(copyNodeInfo && copyNodeInfo->fiss_state==TRUE);
	assert(inputNode->typ == Decl && outputNode->typ == Decl);
	Node *form = MakeOperator(copyNodeInfo->fissingFlatNode->contents->decl,copyNodeInfo->fissingFlatNode->contents->body);
	Node *copyNode = NodeCopy(form,Subtree);
	//PrintNode(stdout,form,0);
	//printf("++++++++++++++++++++++++++++++++++++++++++\n");
	//PrintNode(stdout,copyNode,0);
	//修改copyNode的输出输出以及以及偏移
	Node *inputId = MakeNewStreamId(inputNode->u.decl.name,inputNode);//构造输入流对应的idNode
	Node *outputId = MakeNewStreamId(outputNode->u.decl.name,outputNode);//构造输出流对应的idNode
	assert(ListLength(copyNode->u.operator_.decl->u.decl.type->u.operdcl.inputs)==1 && ListLength(copyNode->u.operator_.decl->u.decl.type->u.operdcl.outputs) == 1);
	Node *old_inputNode =(Node *)FirstItem(copyNode->u.operator_.decl->u.decl.type->u.operdcl.inputs);
	Node *old_outputNode = (Node *)FirstItem(copyNode->u.operator_.decl->u.decl.type->u.operdcl.outputs);
	Node *old_inputDecl = NULL;
	Node *old_outputDecl = NULL;
	if(old_inputNode->typ == Id) old_inputDecl = old_inputNode->u.id.decl;
	else  old_inputDecl = old_inputNode;
	if(old_outputNode->typ == Id) old_outputDecl = old_outputNode->u.id.decl;
	else old_outputDecl = old_outputNode;

	List *inputList = NULL;
	List *outputList = NULL;
	inputList = AppendItem(inputList, inputId);
	outputList = AppendItem(outputList,outputId);
	//修改副本的头的输入输出
	copyNode->u.operator_.decl->u.decl.type->u.operdcl.inputs = inputList;
	copyNode->u.operator_.decl->u.decl.type->u.operdcl.outputs = outputList;
	//修改work的体
	Node *workNode = copyNode->u.operator_.body->u.operBody.work;
	assert(workNode);

	MWISD_astwalk(workNode,old_inputDecl,old_outputDecl,inputId,outputId,MakeConstSint(0),MakeConstSint(0));

	//构建新的window
	List *tmpwinList = ModifyWindow(inputList,outputList,copyNodeInfo->fissed_pop[0],copyNodeInfo->fissed_peek[0],copyNodeInfo->fissed_push[0]);
	//组成新的operator
	copyNode->u.operator_.body->u.operBody.work = workNode;
	copyNode->u.operator_.body->u.operBody.window = tmpwinList;

	//修改composite的名字
	char *operatorName = (char *)malloc(60);
	sprintf(operatorName,"%s_%s_%d","HFission",copyNode->u.operator_.decl->u.decl.name,HFOperatorNum++);
	copyNode->u.operator_.decl->u.decl.name = operatorName;

	return copyNode;
}

Node *ClusterHorizontialFission::MakeWindowNode(Node *id,Node *decl, Node *count, int style)
{
	Node *window = NULL;
	Node *sliding = NULL, *tumbling = NULL;
	Node *trigger = NULL;
	Node *sliding_value = NULL;
	List *inputs;

	assert(id && id->typ == Id && count && decl && decl->typ == Decl);
	id->u.id.decl = decl;

	inputs = AppendItem(AppendItem(inputs,count),count);
	sliding_value = MakeCommaCoord(inputs,UnknownCoord);

	if (style == 0) // sliding window
	{

		sliding = MakeWindowSlidingCoord(EMPTY_TQ, sliding_value, UnknownCoord);

		window = MakeWindowCoord(id, sliding, UnknownCoord); 
	}
	else // tumbling window
	{
		tumbling = MakeWindowTumbingCoord(EMPTY_TQ, count, UnknownCoord);
		window = MakeWindowCoord(id, tumbling, UnknownCoord); 
	}

	return window;
}

List *ClusterHorizontialFission::makeHFissionJoinWindow(List *inputList,List *outputList,List *pop_arg, Node *pushArg)
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

List *ClusterHorizontialFission::makeHFissionRoundRobinWindow(List *inputList,List *outputList,List *push_arg, Node *popArg)
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

List *ClusterHorizontialFission::makeHFissionDuplicateWindow(List *inputList,List *outputList,Node *pop_value)
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


void ClusterHorizontialFission::horizontalFissOperator(FlatNode* flatNode,set<FissionNodeInfo *> &splitedNodeSet,SchedulerSSG *sssg)
{ 
	assert(splitedNodeSet.size()>=2);
	assert(flatNode);
	operatorNode *operNode =flatNode->contents; //将要被分裂的operator
	int steadyCount = sssg->GetSteadyCount(flatNode);  //取该节点的在稳态调度中的执行次数

	List *inputList=operNode->decl->u.decl.type->u.operdcl.inputs;  //待分裂operator的输入
	List *outputList = operNode->decl->u.decl.type->u.operdcl.outputs; //待分裂operator的输出

	assert(ListLength(inputList)==1 && ListLength(outputList)==1);	//暂时只对单输入单输出的节点进行复制分裂

	Node *splitNode = NULL;//新产生的split节点
	Node *joinNode = NULL;	//新产生的join节点
	SplitStyle style = _FlatNode2splitStyle.find(flatNode)->second; //取节点的分裂方式

	Node *inputNode =(Node *)FirstItem(inputList);
	Node *outputNode = (Node *)FirstItem(outputList);

	Node *input_Decl = NULL;
	Node *output_Decl = NULL;

	if(inputNode->typ == Id) input_Decl = inputNode->u.id.decl;
	else input_Decl = inputNode;
	if(outputNode->typ == Id) output_Decl = outputNode->u.id.decl;
	else output_Decl = outputNode;
	Node *output_streamType = output_Decl->u.decl.type;
	Node *input_streamType = input_Decl->u.decl.type;

	int popweight = flatNode->inPopWeights[0];//取当前opertor的pop值
	int pushweight = flatNode->outPushWeights[0];//取当前opertor的push值
	int peekweight = flatNode->inPeekWeights[0];//取当前opertor的peek值

	int deltpeek = 0;
	//vector<Node *>_copy_flatNode;//根据待分裂节点的信息将复制好的副本存放在该vector中
	if(style == S_Duplicate)
		for(set<FissionNodeInfo *>::iterator iter = splitedNodeSet.begin(); iter != splitedNodeSet.end(); iter++)
		{
			char *output_newName = NULL;
			output_newName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
			sprintf(output_newName, "%s_%d", output_Decl->u.decl.name, gCurrentFissionStreamNum++);//新的输出流名称
			Node *output_newStream = MakeNewStream(output_newName,output_streamType);
			assert(output_newStream && output_newStream->typ == Decl);

			char *input_newName = NULL;
			input_newName = (char *) malloc(strlen(input_Decl->u.decl.name)+20);
			sprintf(input_newName, "%s_%d", input_Decl->u.decl.name, gCurrentFissionStreamNum++);//新的输入流
			Node *input_newStream = MakeNewStream(input_newName,input_streamType);
			assert(input_newStream && input_newStream->typ == Decl);

			Node *tmpOperNode = MakeCopyOperatorNode_Duplicate(*iter,input_newStream,output_newStream,popweight,peekweight,pushweight,deltpeek);
			(*iter)->actualOperatorNode = tmpOperNode;
			newOperNode2steadycount.insert(make_pair(tmpOperNode,1));
			newOperNode2partitionNum.insert(make_pair(tmpOperNode,(*iter)->npart));
			deltpeek += popweight * (*iter)->fissedSteadycount;
			(*iter)->fissedSteadycount = 1;
		}
	else
		for(set<FissionNodeInfo *>::iterator iter = splitedNodeSet.begin(); iter != splitedNodeSet.end(); iter++)
		{
			char *output_newName = NULL;
			output_newName = (char *) malloc(strlen(output_Decl->u.decl.name)+20);
			sprintf(output_newName, "%s_%d", output_Decl->u.decl.name, gCurrentFissionStreamNum++);
			Node *output_newStream = MakeNewStream(output_newName,output_streamType);
			assert(output_newStream && output_newStream->typ == Decl);
			//deltpeek = 0;
			char *input_newName = NULL;
			input_newName = (char *) malloc(strlen(input_Decl->u.decl.name)+20);
			sprintf(input_newName, "%s_%d", input_Decl->u.decl.name, gCurrentFissionStreamNum++);
			Node *input_newStream = MakeNewStream(input_newName,input_streamType);
			assert(input_newStream && input_newStream->typ == Decl);
			Node *tmpOperNode = MakeCopyOperatorNode_RoundRobin(*iter,input_newStream,output_newStream,popweight,peekweight,pushweight,0);
			//_copy_flatNode.push_back(MakeCopyOperatorNode_RoundRobin(*iter,input_newStream,output_newStream,popweight,peekweight,pushweight,0));	
			(*iter)->actualOperatorNode = tmpOperNode;
			newOperNode2steadycount.insert(make_pair(tmpOperNode,(*iter)->fissedSteadycount));
			newOperNode2partitionNum.insert(make_pair(tmpOperNode,(*iter)->npart));
		}
}

Node *ClusterHorizontialFission::makeHFissionSplit(Node *splitInputNode,Node *splitPopArg,List *splitOutputList, List *splitPushArg, SplitStyle style)
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
		sprintf(operatorName,"%s_%s_%d","HFission","RoundRobin",HFOperatorNum++);
		assert(operatorName);
		assert(splitOutputList);
		assert(tmp_inputList);
		Node *rouOperHead = MakeOperatorHead(operatorName,splitOutputList,tmp_inputList); //operator的头
		newOperator = DefineOperator(rouOperHead);
		newOperator->u.operator_.ot = Roundrobin_;
		Node *rouWorkNode = MakeRoundrobinWork(splitOutputList,tmp_inputNode,splitPushArg);//构造operator的work函数
		List *rouWindowList = makeHFissionRoundRobinWindow(tmp_inputList,splitOutputList,splitPushArg,splitPopArg);
		Node *rouOperBodyNode = MakeOperatorBody(rouWorkNode,rouWindowList);//operator的body
		newOperator = SetOperatorBody(newOperator,rouOperBodyNode);//构造operator

	}
	else 
	{//style == S_Duplicate
		sprintf(operatorName,"%s_%s_%d","HFission","Duplicate",HFOperatorNum++);
		Node *rouOperHead = MakeOperatorHead(operatorName,splitOutputList,tmp_inputList); //operator的头
		newOperator = DefineOperator(rouOperHead);
		newOperator->u.operator_.ot = Duplicate_;
		Node *rouWorkNode = MakeDuplicateWork(splitOutputList,splitInputNode,splitPopArg);//构造operator的work函数
		List *rouWindowList = makeHFissionDuplicateWindow(tmp_inputList,splitOutputList,splitPopArg);
		Node *rouOperBodyNode = MakeOperatorBody(rouWorkNode,rouWindowList);//operator的body
		newOperator = SetOperatorBody(newOperator,rouOperBodyNode);//构造operator
	}
	gIsUnfold=FALSE;
	return newOperator;
}

Node *ClusterHorizontialFission::makeHFissionJoin(List *joinInputList, List *joinPopArg, Node *joinOutputNode, Node *joinPushArg)
{//构造join节点
	char *operatorName = (char *)malloc(60);
	sprintf(operatorName,"%s_%s_%d","HFission","join",HFOperatorNum++);
	//新的filter的输出边List
	Node *joinOutputDecl = NULL;
	if(joinOutputNode->typ == Id) joinOutputDecl = joinOutputNode->u.id.decl;
	else joinOutputDecl = joinOutputNode;
	Node *tmp_outputNode = MakeNewStreamId(joinOutputDecl->u.decl.name,joinOutputDecl);
	List *tmp_outputList = NULL;
	tmp_outputList = AppendItem(tmp_outputList,tmp_outputNode); 

	gIsUnfold=TRUE;
	Node *newOperatorHead = MakeOperatorHead(operatorName,tmp_outputList,joinInputList);//新节点的头
	Node *newOperator = DefineOperator(newOperatorHead);
	newOperator->u.operator_.ot = Join_;
	Node *newOperatorWork = MakeJoinWork(tmp_outputNode,joinInputList,joinPopArg);
	//新的filter构造window
	List *newOperatorWindow = makeHFissionJoinWindow(joinInputList,tmp_outputList,joinPopArg,joinPushArg);
	Node *newOperatorBody = MakeOperatorBody(newOperatorWork,newOperatorWindow);
	newOperator = SetOperatorBody(newOperator,newOperatorBody);//newoperator构造完成
	gIsUnfold=FALSE;
	return newOperator;
}

map<FlatNode *,vector<Node *> > ClusterHorizontialFission::makeSplitJoinExtractOperators()
{//对要分裂的结点构造上下端节点

	map<FlatNode *,vector<Node *> > flatNode2_copyNodes;

	for (map<FlatNode *,set<FissionNodeInfo *> >::iterator iter = FlatNode2FissionNode.begin(); iter != FlatNode2FissionNode.end(); iter++)
	{
		vector<Node *>_copyNodes;
		//构造上端节点
		Node *inSplitStream = (Node *)FirstItem(iter->first->contents->decl->u.decl.type->u.operdcl.inputs);
		Node *inSplitStreamPopvalue = MakeConstSint(iter->first->inPopWeights[0] * _flatNode2steadycount.find(iter->first)->second);
		List *outSplitStreamList = NULL;
		List *outSplitStreamPushvalueList = NULL;
		SplitStyle style = _FlatNode2splitStyle.find(iter->first)->second;
		for(set<FissionNodeInfo *>::iterator _copy_iter = (iter->second).begin(); _copy_iter != (iter->second).end();_copy_iter++)
		{

			cout<<"   pop: "<<(*_copy_iter)->fissed_pop[0]<<"   peek: "<<(*_copy_iter)->fissed_peek[0]<<"   push:" <<(*_copy_iter)->fissed_push[0]<<"   steady:"<< (*_copy_iter)->fissedSteadycount<<endl;
			assert((*_copy_iter)->actualOperatorNode);
			//新建输出节点++++++++++++++++++++++++++
			Node *S_outputNode = (Node *)FirstItem((*_copy_iter)->actualOperatorNode->u.operator_.decl->u.decl.type->u.operdcl.inputs);
			Node *S_outputDecl = NULL;
			if(S_outputNode->typ == Id) S_outputDecl = S_outputNode->u.id.decl;
			else S_outputDecl = S_outputNode;
			Node *newS_outputId = MakeNewStreamId(S_outputDecl->u.decl.name,S_outputDecl);

			outSplitStreamList = AppendItem(outSplitStreamList,newS_outputId);
			outSplitStreamPushvalueList = AppendItem(outSplitStreamPushvalueList,MakeConstSint((*_copy_iter)->fissed_pop[0] * (*_copy_iter)->fissedSteadycount));
			//cout<<"push value"<<(*_copy_iter)->fissed_pop[0] * (*_copy_iter)->fissedSteadycount<<endl;
		}
		Node *newSplitNode = makeHFissionSplit(inSplitStream,inSplitStreamPopvalue,outSplitStreamList,outSplitStreamPushvalueList,style);

		newOperNode2partitionNum.insert(make_pair(newSplitNode,_flatNode2partitionNum.find(iter->first)->second));
		newOperNode2steadycount.insert(make_pair(newSplitNode,1));
		_copyNodes.push_back(newSplitNode);
		//构造下端节点

		List *inJoinStreamList = NULL;
		List *inJoinStreamPopvalueList = NULL;
		Node *outJoinStream = (Node *)FirstItem(iter->first->contents->decl->u.decl.type->u.operdcl.outputs);
		Node *outJoinPushvalue = MakeConstSint(iter->first->outPushWeights[0] * _flatNode2steadycount.find(iter->first)->second);
		for(set<FissionNodeInfo *>::iterator _copy_iter = (iter->second).begin(); _copy_iter != (iter->second).end();_copy_iter++)
		{
			assert((*_copy_iter)->actualOperatorNode);
			Node *J_inputNode = (Node *)FirstItem((*_copy_iter)->actualOperatorNode->u.operator_.decl->u.decl.type->u.operdcl.outputs);
			Node *J_inputDecl = NULL;
			if(J_inputNode->typ == Id) J_inputDecl =  J_inputNode->u.id.decl;
			else J_inputDecl = J_inputNode;
			Node *newJ_inputId = MakeNewStreamId(J_inputDecl->u.decl.name, J_inputDecl);

			inJoinStreamList = AppendItem(inJoinStreamList,newJ_inputId);
			inJoinStreamPopvalueList = AppendItem(inJoinStreamPopvalueList,MakeConstSint((*_copy_iter)->fissed_push[0] * (*_copy_iter)->fissedSteadycount));
		}
		Node *newJionNode = makeHFissionJoin(inJoinStreamList,inJoinStreamPopvalueList,outJoinStream,outJoinPushvalue);
		newOperNode2partitionNum.insert(make_pair(newJionNode,_flatNode2partitionNum.find(iter->first)->second));
		newOperNode2steadycount.insert(make_pair(newJionNode,1));

		for(set<FissionNodeInfo *>::iterator _copy_iter = (iter->second).begin(); _copy_iter != (iter->second).end();_copy_iter++)
		{
			_copyNodes.push_back((*_copy_iter)->actualOperatorNode);
		}
		_copyNodes.push_back(newJionNode);
		flatNode2_copyNodes.insert(make_pair(iter->first,_copyNodes));
	}
	return flatNode2_copyNodes;
}

map<FlatNode *,vector<FlatNode *> > ClusterHorizontialFission::transOperatorNodetoFlatNode(map<FlatNode *,vector<Node *> > flatNode2FissionOperatorNode)
{
	map<FlatNode *,vector<FlatNode *> > flatNode2_copyFlatNodes;
	//删除旧结点在sssg中的信息
	for(map<FlatNode *,vector<Node *> >::iterator tran_iter = flatNode2FissionOperatorNode.begin(); tran_iter != flatNode2FissionOperatorNode.end(); tran_iter++)
	{
		//删除flatNodes中的内容
		vector<FlatNode *>::iterator erase_iter = sssg->flatNodes.end();
		for(vector<FlatNode *>::iterator iter = sssg->flatNodes.begin(); iter != sssg->flatNodes.end(); iter++)
		{
			if (tran_iter->first == *iter) { erase_iter = iter; break;}
		}
		assert(erase_iter != sssg->flatNodes.end());
		sssg->flatNodes.erase(erase_iter);
		//删除mapEdge2UpFlatNode中的内容
		map<Node *, FlatNode *>::iterator mapEdge2UpFlatNode_iter = sssg->mapEdge2UpFlatNode.begin();
		while (mapEdge2UpFlatNode_iter != sssg->mapEdge2UpFlatNode.end())
		{
			if(mapEdge2UpFlatNode_iter->second == tran_iter->first)
			{
				map<Node *, FlatNode *>::iterator mapEdge2UpFlatNode_eraseIter = mapEdge2UpFlatNode_iter;
				mapEdge2UpFlatNode_iter++;
				sssg->mapEdge2UpFlatNode.erase(mapEdge2UpFlatNode_eraseIter);
			}
			else mapEdge2UpFlatNode_iter++;
		}
		//删除mapEdge2DownFlatNode中的元素
		multimap<Node *, FlatNode *>::iterator mapEdge2DownFlatNode_iter = sssg->mapEdge2DownFlatNode.begin();
		while (mapEdge2DownFlatNode_iter != sssg->mapEdge2DownFlatNode.end())
		{
			if(mapEdge2DownFlatNode_iter->second == tran_iter->first)
			{
				multimap<Node *, FlatNode *>::iterator mapEdge2DownFlatNode_eraseIter = mapEdge2DownFlatNode_iter;
				mapEdge2DownFlatNode_iter++;
				sssg->mapEdge2DownFlatNode.erase(mapEdge2DownFlatNode_eraseIter);
			}
			else mapEdge2DownFlatNode_iter++;
		}
		//删除在工作量估计中的值
		sssg->mapInitWork2FlatNode.erase(tran_iter->first );
		sssg->mapSteadyWork2FlatNode.erase(tran_iter->first);

		//删除_flatNode2partitionNum和_flatNode2steadycount 中旧结点
		_flatNode2partitionNum.erase(tran_iter->first);
		_flatNode2steadycount.erase(tran_iter->first);

		set<FlatNode *> oldFlatNodeSet;
		oldFlatNodeSet.insert(tran_iter->first);
		//插入新节点
		vector<FlatNode *> newFlatNodes;
		vector<Node *> __tmpFlatNodes = tran_iter->second;
		for(vector<Node *>::iterator new_iter  = (tran_iter->second).begin(); new_iter != (tran_iter->second).end(); new_iter++)
		{
			FlatNode *nflatNode = sssg->InsertFlatNodes(&((*new_iter)->u.operator_),NULL,NULL,oldFlatNodeSet); 
			newFlatNodes.push_back(nflatNode);
			//插入_flatNode2partitionNum和_flatNode2steadycount 中
			_flatNode2partitionNum.insert(make_pair(nflatNode,newOperNode2partitionNum.find(*new_iter)->second));
			_flatNode2steadycount.insert(make_pair(nflatNode,newOperNode2steadycount.find(*new_iter)->second));
		}
		for(vector<FlatNode *>::iterator n_iter  = newFlatNodes.begin(); n_iter != newFlatNodes.end(); n_iter++)
		{
			sssg->SetFlatNodesWeights(*n_iter);
			sssg->AddSteadyWork(*n_iter, workEstimate((*n_iter)->contents->body, 0));
			sssg->AddInitWork(*n_iter, workEstimate_init((*n_iter)->contents->body, 0));
		}
		flatNode2_copyFlatNodes.insert(make_pair(tran_iter->first,newFlatNodes));
		newFlatNodes.clear();
	}
	return flatNode2_copyFlatNodes;
}

void ClusterHorizontialFission::modifySplitFlatNode(map<FlatNode*,set<FlatNode *> > upflatNode2flatNodes,map<FlatNode *,vector<FlatNode *> > flatNode2copyFlatNodes)
{
	for(map<FlatNode*,set<FlatNode *> >::iterator mod_iter = upflatNode2flatNodes.begin(); mod_iter != upflatNode2flatNodes.end(); mod_iter++)
	{
		assert(mod_iter->first->contents->ot == Duplicate_ || mod_iter->first->contents->ot == Roundrobin_);
		FlatNode *curSplitNode = mod_iter->first;
		map<FlatNode *,int>::iterator _split_iter = _flatNode2steadycount.find(curSplitNode);
		if(_split_iter == _flatNode2steadycount.end())continue;//上端节点不在该划分中
		int split_steadycount = _split_iter->second;
		List *outputList = NULL;//输出边
		List *output_pushvaluse = NULL;//push的值
		Node *inputNode = (Node *)FirstItem(curSplitNode->contents->decl->u.decl.type->u.operdcl.inputs);
		Node *pop_value = MakeConstSint(curSplitNode->inPopWeights[0] * split_steadycount);
		vector<int>_newpeek(1,curSplitNode->inPopWeights[0] * split_steadycount);
		vector<int>_newpop(1,curSplitNode->inPopWeights[0] * split_steadycount);
		vector<int>_newpush;
		int nOut = 0; 
		vector<FlatNode *> new_outFlatNodes; // 输 出 边各operator
			//取split节点的所有的下端节点
		vector<FlatNode *> outFlatNodes = curSplitNode->outFlatNodes;
		List *oldoutputList = curSplitNode->contents->decl->u.decl.type->u.operdcl.outputs;
		assert(ListLength(oldoutputList) == outFlatNodes.size());
		for(int down_iter = 0; down_iter != outFlatNodes.size(); down_iter++)
		{
			Node *oldoutputNode = (Node *)FirstItem(oldoutputList);
			Node *oldoutputDecl = NULL;
			Node *oldoutputId = NULL;
			if(oldoutputNode->typ == Decl) oldoutputDecl = oldoutputNode;
			else oldoutputDecl = oldoutputNode->u.id.decl;
			oldoutputId = MakeNewStreamId(oldoutputDecl->u.decl.name, oldoutputDecl);
			oldoutputList = GetNextList(oldoutputList);
			if((mod_iter->second).count(outFlatNodes[down_iter]))
			{
				vector<FlatNode *> _copyFlatNode = flatNode2copyFlatNodes.find(outFlatNodes[down_iter])->second;//取出所有副本
				assert(_copyFlatNode[0]->contents->ot == Duplicate_ || _copyFlatNode[0]->contents->ot ==Roundrobin_);
				//删除在中的信息
				_flatNode2partitionNum.erase(_copyFlatNode[0]);
				_flatNode2steadycount.erase(_copyFlatNode[0]);
				
				assert( _copyFlatNode[_copyFlatNode.size()-1]->contents->ot ==Join_);
				assert(curSplitNode->outPushWeights[down_iter] * split_steadycount == _copyFlatNode[0]->inPopWeights[0]);
				outputList = JoinLists(outputList,_copyFlatNode[0]->contents->decl->u.decl.type->u.operdcl.outputs);
				for(int i = 1; i!= _copyFlatNode.size()-1;i++)
				{
					new_outFlatNodes.push_back(_copyFlatNode[i]);
					nOut++;
				}
				for(int j = 0; j!= _copyFlatNode[0]->nOut;j++)
				{
					output_pushvaluse= AppendItem(output_pushvaluse,MakeConstSint(_copyFlatNode[0]->outPushWeights[j]));
					_newpush.push_back(_copyFlatNode[0]->outPushWeights[j]);
				}
			}
			else
			{
				outputList = AppendItem(outputList,oldoutputId);
				new_outFlatNodes.push_back(outFlatNodes[down_iter]);
				nOut++;
				output_pushvaluse = AppendItem(output_pushvaluse,MakeConstSint(curSplitNode->outPushWeights[down_iter]*split_steadycount));
				_newpush.push_back(curSplitNode->outPushWeights[down_iter]*split_steadycount);
			}
		}	
		SplitStyle style;
		if(curSplitNode->contents->ot == Duplicate_) style = S_Duplicate;
		else style = S_RoundRobin;
		Node *newSplit = makeHFissionSplit(inputNode,pop_value,outputList,output_pushvaluse,style);
		//用新的split的flatNode替换旧的flatNode节点
		replaceHfisionFlatNodes(curSplitNode,&(newSplit->u.operator_));
	}
}

void ClusterHorizontialFission::modifyJoinFlatNode(map<FlatNode*,set<FlatNode *> > upflatNode2flatNodes,map<FlatNode *,vector<FlatNode *> > flatNode2copyFlatNodes)
{
	for(map<FlatNode*,set<FlatNode *> >::iterator mod_iter = upflatNode2flatNodes.begin(); mod_iter != upflatNode2flatNodes.end(); mod_iter++)
	{
		assert(mod_iter->first->contents->ot == Join_);
		FlatNode *curJoinNode = mod_iter->first;
		map<FlatNode *,int>::iterator _join_iter = _flatNode2steadycount.find(curJoinNode);
		if(_join_iter == _flatNode2steadycount.end())continue;//join节点不在该划分中
		int join_steadycount = _join_iter->second;
		List *inputList = NULL;//输入边
		List *input_popvaluse = NULL;//pop的值
		Node *outputNode = (Node *)FirstItem(curJoinNode->contents->decl->u.decl.type->u.operdcl.outputs);
		Node *push_value = MakeConstSint(curJoinNode->outPushWeights[0] * join_steadycount);
		vector<int>_newpeek;
		vector<int>_newpop;
		vector<int>_newpush(1,curJoinNode->outPushWeights[0] * join_steadycount);
		int nIn = 0; 
		vector<FlatNode *> new_inFlatNodes; // 输 入 边各operator
		//取join节点的所有的下端节点
		vector<FlatNode *> inFlatNodes = curJoinNode->inFlatNodes;
		List *oldinputList = curJoinNode->contents->decl->u.decl.type->u.operdcl.inputs;
		assert(ListLength(oldinputList) == inFlatNodes.size());
		for(int up_iter = 0; up_iter != inFlatNodes.size(); up_iter++)
		{
			Node *oldinputNode = (Node *)FirstItem(oldinputList);
			oldinputList = GetNextList(oldinputList);
			if((mod_iter->second).count(inFlatNodes[up_iter]))
			{
				vector<FlatNode *> _copyFlatNode = flatNode2copyFlatNodes.find(inFlatNodes[up_iter])->second;//取出所有副本
				assert(_copyFlatNode[0]->contents->ot == Duplicate_ || _copyFlatNode[0]->contents->ot ==Roundrobin_);
				assert( _copyFlatNode[_copyFlatNode.size()-1]->contents->ot ==Join_);
				_flatNode2partitionNum.erase(_copyFlatNode[_copyFlatNode.size()-1]);
				_flatNode2steadycount.erase(_copyFlatNode[_copyFlatNode.size()-1]);
				assert(curJoinNode->inPopWeights[up_iter] * join_steadycount == _copyFlatNode[_copyFlatNode.size()-1]->outPushWeights[0]);
				inputList = JoinLists(inputList,_copyFlatNode[0]->contents->decl->u.decl.type->u.operdcl.inputs);
				for(int i = 1; i!= _copyFlatNode.size()-1;i++)
				{
					new_inFlatNodes.push_back(_copyFlatNode[i]);
					nIn++;
				}
				for(int j = 0; j!= _copyFlatNode[0]->nOut;j++)
				{
					input_popvaluse= AppendItem(input_popvaluse,MakeConstSint(_copyFlatNode[0]->inPopWeights[j]));
					_newpop.push_back(_copyFlatNode[0]->inPopWeights[j]);
					_newpeek.push_back(_copyFlatNode[0]->inPopWeights[j]);
				}
			}
			else
			{
				inputList = AppendItem(inputList,oldinputNode);
				new_inFlatNodes.push_back(inFlatNodes[up_iter]);
				nIn++;
				input_popvaluse = AppendItem(input_popvaluse,MakeConstSint(curJoinNode->inPopWeights[up_iter]*join_steadycount));
				_newpop.push_back(curJoinNode->inPopWeights[up_iter]*join_steadycount);
				_newpeek.push_back(curJoinNode->inPopWeights[up_iter]*join_steadycount);
			}
		}	

		Node *newJoin = makeHFissionJoin(inputList,input_popvaluse,outputNode,push_value);
		FlatNode *newflatNode = replaceHfisionFlatNodes(curJoinNode,&(newJoin->u.operator_));
	}

}

FlatNode * ClusterHorizontialFission::replaceHfisionFlatNodes(FlatNode *oldFlatNodes,operatorNode *newNodes)
{
	assert(newNodes);
	vector<FlatNode *>fusedResult;
	//删除旧的信息
	//删除flatNodes中的内容
	vector<FlatNode *>::iterator erase_iter = sssg->flatNodes.end();
	for(vector<FlatNode *>::iterator iter = sssg->flatNodes.begin(); iter != sssg->flatNodes.end(); iter++)
	{
		if (oldFlatNodes == *iter) { erase_iter = iter; break;}
	}
	assert(erase_iter != sssg->flatNodes.end());
	sssg->flatNodes.erase(erase_iter);
	//删除mapEdge2UpFlatNode中的内容
	map<Node *, FlatNode *>::iterator mapEdge2UpFlatNode_iter = sssg->mapEdge2UpFlatNode.begin();
	while (mapEdge2UpFlatNode_iter != sssg->mapEdge2UpFlatNode.end())
	{
		if(mapEdge2UpFlatNode_iter->second == oldFlatNodes)
		{
			map<Node *, FlatNode *>::iterator mapEdge2UpFlatNode_eraseIter = mapEdge2UpFlatNode_iter;
			mapEdge2UpFlatNode_iter++;
			sssg->mapEdge2UpFlatNode.erase(mapEdge2UpFlatNode_eraseIter);
		}
		else mapEdge2UpFlatNode_iter++;
	}
	//删除mapEdge2DownFlatNode中的元素
	multimap<Node *, FlatNode *>::iterator mapEdge2DownFlatNode_iter = sssg->mapEdge2DownFlatNode.begin();
	while (mapEdge2DownFlatNode_iter != sssg->mapEdge2DownFlatNode.end())
	{
		if(mapEdge2DownFlatNode_iter->second == oldFlatNodes)
		{
			multimap<Node *, FlatNode *>::iterator mapEdge2DownFlatNode_eraseIter = mapEdge2DownFlatNode_iter;
			mapEdge2DownFlatNode_iter++;
			sssg->mapEdge2DownFlatNode.erase(mapEdge2DownFlatNode_eraseIter);
		}
		else mapEdge2DownFlatNode_iter++;
	}
	//删除在工作量估计中的值
	sssg->mapInitWork2FlatNode.erase(oldFlatNodes);
	sssg->mapSteadyWork2FlatNode.erase(oldFlatNodes);
	//添加新的信息
	set<FlatNode *> oldFlatNodeSet;
	oldFlatNodeSet.insert(oldFlatNodes);
	FlatNode *firstFlatNode = sssg->InsertFlatNodes(newNodes,NULL,NULL,oldFlatNodeSet);
	sssg->SetFlatNodesWeights(firstFlatNode);	
	//对新的节做工作量估计
	sssg->AddSteadyWork(firstFlatNode, workEstimate(firstFlatNode->contents->body, 0));
	sssg->AddInitWork(firstFlatNode, workEstimate_init(firstFlatNode->contents->body, 0));
	_flatNode2steadycount.insert(make_pair(firstFlatNode,1));
	_flatNode2steadycount.erase(oldFlatNodes);
	_flatNode2partitionNum.insert(make_pair(firstFlatNode,_flatNode2partitionNum.find(oldFlatNodes)->second));
	_flatNode2partitionNum.erase(oldFlatNodes);
	return firstFlatNode;
}


//根据FlatNode2FissionNode中的信息进行实际的分裂
void ClusterHorizontialFission::ReplicationFissing()
{
	map<FlatNode*,set<FlatNode *> >upflatNode2flatNodes;//上端节点与其待分裂节点的map——分裂flatNode的上端split节点是被修改的而不是新添加的split
	map<FlatNode*,set<FlatNode *> >downflatNode2flatNodes;//下端节点与其待分裂节点的map——分裂flatNode的下端join节点是被修改的而不是新添加的join

	//下面造节点要根据副本的信息，要利用到前面确定的所有信息
	/******取出FlatNode2FissionNode中各个flatNode对应的各个副本的信息组成一个Vector，传递给horizontalFissOperator函数进行分裂，组成一个List，
	然后再将List传递给Operator2FlatNodeModifyInfo，修改sssg图的信息*************/
	
	//map<FlatNode *,vector<Node *> > flatNode2FissionOperatorNode;
	map<FlatNode* ,set<FissionNodeInfo *> >::iterator fissing_iter;
	for(fissing_iter = FlatNode2FissionNode.begin();fissing_iter != FlatNode2FissionNode.end();fissing_iter++)
	{
		horizontalFissOperator(fissing_iter->first,fissing_iter->second,sssg);
		//与flatNode的上端节点和下端节点相结合，判断是构造新的上下端，还是修改上下端
		if(!_FlatNode2makeUpNodeTag.find(fissing_iter->first)->second)
		{//不需要做上端节点——修改上端的split节点，要将split的稳态考虑进去，修改完成后节点在稳态只做一次
			FlatNode *upFlatNode = fissing_iter->first->inFlatNodes[0];
			assert(upFlatNode->contents->ot == Duplicate_ || upFlatNode->contents->ot == Roundrobin_);
			set<FlatNode *>tmpVec;
			tmpVec.insert((fissing_iter->first));
			pair<map<FlatNode*,set<FlatNode *> >::iterator ,bool> ret = upflatNode2flatNodes.insert(make_pair(upFlatNode, tmpVec));
			if (!ret.second) (ret.first->second).insert(fissing_iter->first);
		}
		if(!_FlatNode2makeDownNodeTag.find(fissing_iter->first)->second)
		{//不需要做下端节点——修改下端的join节点，要将join的稳态考虑进去，修改完成后节点在稳态只做一次
			FlatNode *downFlatNode = fissing_iter->first->outFlatNodes[0];
			assert(downFlatNode->contents->ot == Join_);
			set<FlatNode *>tmpVec;
			tmpVec.insert(fissing_iter->first);
			pair<map<FlatNode*,set<FlatNode *> >::iterator ,bool> ret = downflatNode2flatNodes.insert(make_pair(downFlatNode, tmpVec));
			if (!ret.second) (ret.first->second).insert(fissing_iter->first);
		}
	}
	map<FlatNode *,vector<Node *> > flatNode2FissionOperatorNode = makeSplitJoinExtractOperators();//对于构造上端以及下端节点即split和join节点
	map<FlatNode *,vector<FlatNode *> > flatNode2copyFlatNodes = transOperatorNodetoFlatNode(flatNode2FissionOperatorNode);//将分裂后的节点装换成FlatNode并插入到ssg中，同时修改相应的信息
	//20130114取消下面的代码——原代码的功能是尽可能的消除多余的split和join节点，但在实际做的时候破坏了split和join的逻辑，出现错误
	//if(upflatNode2flatNodes.size() > 1)modifySplitFlatNode(upflatNode2flatNodes,flatNode2copyFlatNodes);//处理具有相同的上端节点的待分裂——修改上端flatNode的work，window等信息，主要是修改split对应的flatNode的信息，并且删除新构造的split对应的flatNode节点
	//if(downflatNode2flatNodes.size() > 1)modifyJoinFlatNode(downflatNode2flatNodes,flatNode2copyFlatNodes);//处理具有相同的下端节点的待分裂——修改上端flatNode的work，window等信息， 主要是修改join对应的flatNode的信息，并且删除新构造的join对应的flatNode节点
}


float ClusterHorizontialFission::fakementHorizontalFission()
{
	float bf = 0;
	initFissionNodeMap();//初始化类中的两个map
	while(TRUE)
	{
		map<int,int > Partiotion2Weight = computeMapPlace2Weight();//根据成员PartitonNum2FissionNode计算各个划分的初始工作量
		vector<int> partitionSort = SortPartitionbyWeight(Partiotion2Weight);//对划分按工作量从大到小排序
		int cur_max_partition; //当前权重最大的划分的迭代器
		int cur_min_partition; //当前权重最小的划分的迭代器
		FissionNodeInfo *curFissionFlatNode = NULL;//存放要被分裂的节点
		int maxWork = 0;
		Bool partiotion_flag = FALSE;

		vector<Bool> PartiotionFlag(npartition,TRUE);//表示每一个划分中是否有可分裂的节点 

		for (int i = 0 ;i< npartition; i++)
		{//找当前能够分裂的核
			partiotion_flag = TRUE;
			cur_max_partition = partitionSort.back();//找到当前权值最大的划分 
			/*取当前权值最大的划分中的所有的可被分裂的flatNode*/
			vector<FissionNodeInfo *> fissionFlatNodes = findFissionflatNodesInPartition(cur_max_partition);
			if(fissionFlatNodes.size()==0)
			{
				partitionSort.pop_back();//从排好序的vector删除刚刚取出的元素
				partiotion_flag = FALSE;continue;
			}//该划分中没有可分裂的元素
			if(partitionSort.empty()) break;//如果所有的划分全被访问则退出
			/*最大化分中工作量最大的operator*/
			curFissionFlatNode = NULL;
			maxWork = 0;
			//找权值最大的flatNode作为将要被分裂的节点
			for(int j = 0; j != fissionFlatNodes.size(); j++)
			{
				if(fissionFlatNodes[j]->fissedSteadycount * fissionFlatNodes[j]->operatorWeight > maxWork)
				{
					curFissionFlatNode = fissionFlatNodes[j];
					maxWork = fissionFlatNodes[j]->fissedSteadycount * fissionFlatNodes[j]->operatorWeight;
				}
			}
			if(partiotion_flag)break;//找到可分裂元素跳出循环
		}
		/*没有满足条件的分裂节点*/
		if(!partiotion_flag || curFissionFlatNode == NULL)
			break;
		assert(curFissionFlatNode && partiotion_flag);
		/*找工作量最小的划分*/
		cur_min_partition = partitionSort.front();//取工作量最小的划分
		/*任务均衡退出划分*/
		if(Partiotion2Weight.find(cur_max_partition)->second < Partiotion2Weight.find(cur_min_partition)->second * balanceFactor)
			break;
		/*计算分裂次数*/
		int delt_weight = ((Partiotion2Weight.find(cur_max_partition)->second) - (Partiotion2Weight.find(cur_min_partition)->second));
		if(!delt_weight) break;//整个图相对比较平衡不用再分
		int replicationFactor = maxWork / delt_weight;
		replicationFactor = replicationFactor>2 ? replicationFactor:2;
		/*下面主要是在两个成员map修改信息，删除将要分裂节点的信息，添加分裂后的副本的信息——伪分裂*/
		SplitFissionNode(curFissionFlatNode,replicationFactor,cur_max_partition,cur_min_partition);
	}
	//计算当前图的平衡情况
	map<int,int > _partiotion2Weight = computeMapPlace2Weight();//根据成员PartitonNum2FissionNode计算各个划分的初始工作量
	vector<int> _partitionSort = SortPartitionbyWeight(_partiotion2Weight);//对划分按工作量从大到小排序
	int max_partition = _partitionSort.back(); //权重最大的划分的迭代器
	int min_partition = _partitionSort.front(); //权重最小的划分的迭代器
	bf = ((float)(_partiotion2Weight.find(max_partition)->second))/(_partiotion2Weight.find(min_partition)->second);
	return bf;
}

 /*图的水平分裂*/
void ClusterHorizontialFission::HorizontalFissionPartitions()
{
	fakementHorizontalFission();//先做伪分裂，确定相关分裂的信息，然后进行正真的分裂
	//根据分裂的结果确定副本的peek，pop以及push的值
	modifyFissionNodeInfo();
	//根据FlatNode2FissionNode中的信息进行实际的分裂
	ReplicationFissing();
}

GLOBAL map<FlatNode *,int> ClusterHFissing(map<FlatNode *,int>flatNode2partitionNum,SchedulerSSG *sssg, int _nplace, float bf)
{
	map<FlatNode *,int> flatNode2steadycount = sssg->mapSteadyCount2FlatNode;
	ClusterHorizontialFission *chfp = new ClusterHorizontialFission(flatNode2partitionNum,flatNode2steadycount,sssg,_nplace,1.5);
	chfp->HorizontalFissionPartitions();
	sssg->mapSteadyCount2FlatNode .clear();
	sssg->mapSteadyCount2FlatNode = chfp->GetFlatNode2steadycount();
	sssg->mapInitCount2FlatNode.clear();
	sssg->InitScheduling();
	return chfp->GetFlatNode2partitionNum();//返回分裂后节点与核之间的map
}