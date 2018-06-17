#include "ActorStateDetector.h"
#include <iostream>
using namespace std;

void ActorStateDetector::hasMutableState()
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
	FSD_astwalk(workNode,stateList);
}

void ActorStateDetector::IsMutableVar(List *list,Node *node)
{  //如果变量是state则返回True
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
			FSD_astwalk(tmpNode,list);
		}
	}
}

void ActorStateDetector::FSD_listwalk(List *l,List *list)
{
	assert(list && l);
	if(stateful) return;
	ListMarker _listwalk_marker; Node *_listwalk_ref; 
	IterateList(&_listwalk_marker, l); 
	while (NextOnList(&_listwalk_marker, (GenericREF)&_listwalk_ref)) { 
		if (_listwalk_ref) 
			FSD_astwalk(_listwalk_ref,list);
		SetCurrentOnList(&_listwalk_marker, (Generic *)_listwalk_ref); 
	}
}

void ActorStateDetector::FSD_astwalk(Node *n,List *list)
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
				FSD_astwalk((n)->u.binop.left,list);
			} 
			if ((n)->u.binop.right) { FSD_astwalk((n)->u.binop.right,list); }
			break; 
		}
	case Unary:         
		{
			if ((n)->u.unary.expr) 
			{
				IsMutableVar(list,n->u.unary.expr);
				FSD_astwalk((n)->u.unary.expr,list);
			}
			break; 
		}
	case Cast:          if ((n)->u.cast.type) { FSD_astwalk((n)->u.cast.type,list);} if ((n)->u.cast.expr) { FSD_astwalk((n)->u.cast.expr,list);} break; 
	case Comma:         if ((n)->u.comma.exprs) { FSD_listwalk((n)->u.comma.exprs, list);} break; 
	case Ternary:       if ((n)->u.ternary.cond) { FSD_astwalk((n)->u.ternary.cond,list);} if ((n)->u.ternary.true_) { FSD_astwalk((n)->u.ternary.true_,list);} if ((n)->u.ternary.false_) { FSD_astwalk((n)->u.ternary.false_,list);} break;
	case Array:         if ((n)->u.array.name) { FSD_astwalk((n)->u.array.name,list);} if ((n)->u.array.dims) { FSD_listwalk((n)->u.array.dims, list);} break; 
	case Call:          if ((n)->u.call.args) { FSD_listwalk((n)->u.call.args, list);} break; 
	case Initializer:   if ((n)->u.initializer.exprs) { FSD_listwalk((n)->u.initializer.exprs, list);} break; 
	case ImplicitCast:  if ((n)->u.implicitcast.expr) { FSD_astwalk((n)->u.implicitcast.expr, list);} break; 
	case Label:         if ((n)->u.label.stmt) { FSD_astwalk((n)->u.label.stmt, list);} break; 
	case Switch:        if ((n)->u.Switch.expr) { FSD_astwalk((n)->u.Switch.expr , list);} if ((n)->u.Switch.stmt) { FSD_astwalk((n)->u.Switch.stmt, list);} break; 
	case Case:          if ((n)->u.Case.expr) { FSD_astwalk((n)->u.Case.expr , list);} if ((n)->u.Case.stmt) { FSD_astwalk((n)->u.Case.stmt, list);} break; 
	case Default:       if ((n)->u.Default.stmt) { FSD_astwalk((n)->u.Default.stmt,list);} break; 
	case If:            if ((n)->u.If.expr) { FSD_astwalk((n)->u.If.expr, list);} if ((n)->u.If.stmt) { FSD_astwalk((n)->u.If.stmt,list);} break; 
	case IfElse:        if ((n)->u.IfElse.expr) { FSD_astwalk((n)->u.IfElse.expr,list);} if ((n)->u.IfElse.true_) { FSD_astwalk((n)->u.IfElse.true_, list);} if ((n)->u.IfElse.false_) { FSD_astwalk((n)->u.IfElse.false_,list);} break;
	case While:         if ((n)->u.While.expr) { FSD_astwalk((n)->u.While.expr,list);} if ((n)->u.While.stmt) { FSD_astwalk((n)->u.While.stmt,list);} break; 
	case Do:            if ((n)->u.Do.stmt) { FSD_astwalk((n)->u.Do.stmt, list);} if ((n)->u.Do.expr) { FSD_astwalk((n)->u.Do.expr, list);} break; 
	case For:           if ((n)->u.For.init) { FSD_astwalk((n)->u.For.init,list);}if ((n)->u.For.cond) { FSD_astwalk((n)->u.For.cond,list);}if ((n)->u.For.next) { FSD_astwalk((n)->u.For.next,list);}if ((n)->u.For.stmt) { FSD_astwalk((n)->u.For.stmt,list);} break; 
	case Goto:          break; 
	case Continue:      break; 
	case Break:         break; 
	case Return:        if ((n)->u.Return.expr) { FSD_astwalk((n)->u.Return.expr,list);} break; 
	case Block:         if ((n)->u.Block.decl) { FSD_listwalk((n)->u.Block.decl, list);}if ((n)->u.Block.stmts) { FSD_listwalk((n)->u.Block.stmts, list);} break; 
	case Prim:          break; 
	case Tdef:          break; 
	case Ptr:           if ((n)->u.ptr.type) { FSD_astwalk((n)->u.ptr.type,list);} break; 
	case Adcl:          if ((n)->u.adcl.type) { FSD_astwalk((n)->u.adcl.type,list);} if ((n)->u.adcl.dim) { FSD_astwalk((n)->u.adcl.dim,list);} break; 
	case Fdcl:          break; 
	case Sdcl:          break; 
	case Udcl:          break; 
	case Edcl:          break; 
	case Decl:          if ((n)->u.decl.type) { FSD_astwalk((n)->u.decl.type,list);} if ((n)->u.decl.init) { FSD_astwalk((n)->u.decl.init,list);} if ((n)->u.decl.bitsize) { FSD_astwalk((n)->u.decl.bitsize,list);} break; 
	case Attrib:        if (n->u.attrib.arg) { FSD_astwalk(n->u.attrib.arg,list);} break; 
	case Proc:          if ((n)->u.proc.decl) { FSD_astwalk((n)->u.proc.decl,list);} if ((n)->u.proc.body) { FSD_astwalk((n)->u.proc.body,list);} break; 
	case Text:          break; 
	default:            FAIL("Unrecognized node type"); break; 
	}

}

//************************************
// Method:    DetectiveFilterState
// FullName:  DetectiveFilterState
// Access:    public 
// Returns:   GLOBAL Bool
// Qualifier: 检测flatNode中的operator的状态,TRUE表示是stateful类型
// Parameter: FlatNode * flatNode
//************************************
GLOBAL Bool DetectiveActorState(FlatNode *flatNode)
{
	ActorStateDetector* detector = new ActorStateDetector(flatNode->contents);
	detector->hasMutableState();
	return detector->GetOperatorState();
}