#include "workEstimate.h"
void WEST_listwalk(List *l)
{	
	{ ListMarker _listwalk_marker; Node *_listwalk_ref; 
		IterateList(&_listwalk_marker, l); 
		while (NextOnList(&_listwalk_marker, (GenericREF)&_listwalk_ref)) { 
			if (_listwalk_ref) {rWorkCompute(_listwalk_ref);}                     
				SetCurrentOnList(&_listwalk_marker, (Generic *)_listwalk_ref); 
		}
	}
}

void WEST_astwalk(Node *n)
{
	switch ((n)->typ) { 
	 case Const:         if ((n)->u.Const.type) {rWorkCompute((n)->u.Const.type);}
						 break; 
	 case Id:            break; 
	 case Ternary:       if ((n)->u.ternary.cond) 
						 {rWorkCompute((n)->u.ternary.cond);} 
						 if ((n)->u.ternary.true_) 
							{rWorkCompute((n)->u.ternary.true_);} 
						 if ((n)->u.ternary.false_) 
							{rWorkCompute((n)->u.ternary.false_);} 
						 break;
	 case Binop:         if ((n)->u.binop.left) 
							{rWorkCompute((n)->u.binop.left);} 
						 if ((n)->u.binop.right) 
							{rWorkCompute((n)->u.binop.right);} 
						 break; 
	 case Unary:         if ((n)->u.unary.expr) 
							{rWorkCompute((n)->u.unary.expr);}
						 break; 
	 case Cast:          if ((n)->u.cast.expr) 
							{rWorkCompute((n)->u.cast.expr);}
						 break; 
	 case Comma:         if ((n)->u.comma.exprs)
							{WEST_listwalk((n)->u.comma.exprs);} 
						 break; 
	 case Array:         rWorkCompute(n);
						 break; 
	 case Call:          //if ((n)->u.call.name) {rWorkCompute((n)->u.call.name);} 
						 if ((n)->u.call.args) 
							{WEST_listwalk((n)->u.call.args);}
						 break; 
	 case Initializer:   if ((n)->u.initializer.exprs) 
							{WEST_listwalk((n)->u.initializer.exprs);} 
						 break; 
	 case ImplicitCast:  if ((n)->u.implicitcast.expr) 
							{rWorkCompute((n)->u.implicitcast.expr);} 
						 break; 
	 case Label:         if ((n)->u.label.stmt) 
							{rWorkCompute((n)->u.label.stmt);} 
						 break; 
	 case Switch:        if ((n)->u.Switch.expr) 
							{rWorkCompute((n)->u.Switch.expr);} 
						 if ((n)->u.Switch.stmt) 
							{rWorkCompute((n)->u.Switch.stmt);} 
						 break; 
	 case Case:          if ((n)->u.Case.expr) 
							{rWorkCompute((n)->u.Case.expr);} 
						 if ((n)->u.Case.stmt) 
							{rWorkCompute((n)->u.Case.stmt);} 
						 break; 
	 case Default:       if ((n)->u.Default.stmt) 
							{rWorkCompute((n)->u.Default.stmt);} 
						 break; 
	 case If:            if ((n)->u.If.expr) 
							{rWorkCompute((n)->u.If.expr);} 
						 if ((n)->u.If.stmt) 
							{rWorkCompute((n)->u.If.stmt);} 
						 break; 
	 case IfElse:        if ((n)->u.IfElse.expr) 
							{rWorkCompute((n)->u.IfElse.expr);} 
						 if ((n)->u.IfElse.true_) 
							{rWorkCompute((n)->u.IfElse.true_);} 
						 if ((n)->u.IfElse.false_) 
							{rWorkCompute((n)->u.IfElse.false_);} 
						 break; 
	 case While:         if ((n)->u.While.expr) 
							{rWorkCompute((n)->u.While.expr);}
						 if ((n)->u.While.stmt) 
							{rWorkCompute((n)->u.While.stmt);} 
						 break; 
	 case Do:            if ((n)->u.Do.stmt)
							{rWorkCompute((n)->u.Do.stmt);} 
						 if ((n)->u.Do.expr) 
							{rWorkCompute((n)->u.Do.expr);} 
						 break; 
	 case For:           if ((n)->u.For.cond) 
							{rWorkCompute((n)->u.For.cond);}
						 if ((n)->u.For.next) 
							{rWorkCompute((n)->u.For.next);} 
						 if ((n)->u.For.stmt) 
							{rWorkCompute((n)->u.For.stmt);} 
						 break; 
	 case Goto:          break; 
	 case Continue:      break; 
	 case Break:         break; 
	 case Return:        if ((n)->u.Return.expr) {rWorkCompute((n)->u.Return.expr);} break; //spl少有
	 case Block:         if ((n)->u.Block.decl)
							{WEST_listwalk((n)->u.Block.decl);} 
						 if ((n)->u.Block.stmts) 
							{WEST_listwalk((n)->u.Block.stmts);} 
						 break; 
	 case Prim:          break; 
	 case Adcl:          break; 
	 case Decl:          //if ((n)->u.decl.init) {rWorkCompute((n)->u.decl.init);} 
						 break; 
	 case Text:          break; 
	 case OperBody:		
		 if(state == INIT){
							if ((n)->u.operBody.init) 
								{rWorkCompute((n)->u.operBody.init);}
						 }else if(state == STEADY){
							 if ((n)->u.operBody.state) 
								{WEST_listwalk((n)->u.operBody.state);} 
							 if ((n)->u.operBody.work) 
								{rWorkCompute((n)->u.operBody.work);} 
						 }
						 break;    
	 default:            FAIL("Unrecognized node type"); break; 
	}
}

void rWorkCompute(Node *from)
{
	int tmp=0;
	int newWork=0;
	int oldWork=0;
	Node *tmpNode;
	List *tmpList;
	switch (from->typ) {
		/* for nodes with sub-lists, make new copy of sub-list */
	case ImplicitCast: {rWorkCompute(from->u.implicitcast.expr); WEST_astwalk(from->u.implicitcast.expr);}break;
	  case Comma:         WEST_astwalk(from);break;
	  case Array:         
		  {if(isSTREAM)
			totalWork += STREAM_OP;
		  else
			  totalWork+=MEMORY_OP;}
		  break;
	  case Ternary:		  WEST_astwalk(from);break;
	  case Initializer:   //目前不会到这一步
	  case Block:         WEST_astwalk(from);break;
	  case Fdcl:          break;//目前不会到这一步
	  case Decl:		  
		  if (from->u.decl.init != NULL)//必须是已经初始化才计算work
		  {
			  tmpNode = from->u.decl.type;
			  if (tmpNode->typ == Adcl)//如果是数组声明
			  {
				  int tmpw=0;
				  if (tmpNode->u.adcl.dim->typ == Const)
				  {
				    tmpw = tmpNode->u.adcl.dim->u.Const.value.i;
					totalWork+=MEMORY_OP*tmpw;
				  } else if(tmpNode->u.adcl.dim->typ == ImplicitCast){
					tmpw = tmpNode->u.adcl.dim->u.implicitcast.value->u.Const.value.i;
					totalWork+=MEMORY_OP*tmpw;
				  }
				  else 
					FAIL("Unrecognized array type");
					
			  }
			  else
				  totalWork+=MEMORY_OP;
		  }
		  break;
	  case Const:         break;
	  case Id:            totalWork+=MEMORY_OP;break;
	  case Unary:	
		  WEST_astwalk(from);
		  if (from->u.unary.opType == op_float)
		  {
			  tmp=FLOAT_ARITH_OP;
			  if (from->u.unary.op == '/')//仅当浮点的除法需要x16
				  tmp*=16;
		  }else if(from->u.unary.opType == op_int){
			  tmp=INT_ARITH_OP;
		  }else if(from->u.unary.opType == op_unkonwn){
			  tmp=FLOAT_ARITH_OP;//未知的二元操作暂算浮点大小
		  }
		  totalWork+=tmp;
		  break;
	  case Binop:
		  if(from->u.binop.op != '=' && from->u.binop.op != '.'){//赋值表达式不参与计算,memory_op足够表示
			  if (from->u.binop.opType == op_float)
			  {
				  tmp=FLOAT_ARITH_OP;
				  if (from->u.binop.op == '/')//仅当浮点的除法需要x16
					  tmp*=16;
			  }else if(from->u.binop.opType == op_int){
				  tmp=INT_ARITH_OP;
			  }else if(from->u.binop.opType == op_unkonwn){
				  tmp=FLOAT_ARITH_OP;//未知的二元操作暂算浮点大小
			  }
		  }else if(from->u.binop.op == '='){
			  tmp = 0;
		  }else if(from->u.binop.op == '.'){
			  isSTREAM = 1;
			  rWorkCompute(from->u.binop.left);//如果是点操作，则仅取左边表达式计算
			  isSTREAM = 0;
			  break;
		  }
		  WEST_astwalk(from);
		  totalWork+=tmp;
		  break;
	  case For:
		  if(0){
		  rWorkCompute(from->u.For.init);
		  oldWork=totalWork;
		  if ((GetValue(from->u.For.cond->u.binop.right)==NULL)) 
			  tmp = LOOP_COUNT;
		  else if (GetValue(from->u.For.cond->u.binop.right)->typ == Const)//获取for 循环的次数,for条件表达式必须已完成常量传播
			tmp =GetValue(from->u.For.cond->u.binop.right)->u.Const.value.i ;
		  else
			  tmp = LOOP_COUNT;
		  WEST_astwalk(from);
		  newWork=totalWork;
		  totalWork=oldWork+tmp*(newWork-oldWork);
		  break;
		  }
		  else{
		  rWorkCompute(from->u.For.init);
		  oldWork=totalWork;
		  if ((GetValue(from->u.For.cond->u.binop.right)==NULL)) 
			  tmp = LOOP_COUNT;
		  else if (GetValue(from->u.For.cond->u.binop.right)->typ == Const)//获取for 循环的次数,for条件表达式必须已完成常量传播
			{
				int condition = 0,init = 0,step =0;
				tmp = 0;
				condition=GetValue(from->u.For.cond->u.binop.right)->u.Const.value.i;//condition
				if(GetValue(from->u.For.init->u.binop.right))
					init = GetValue(from->u.For.init->u.binop.right)->u.Const.value.i;//condition
				else
					init = condition/2;
				if(from->u.For.cond->u.binop.op == '<' || from->u.For.cond->u.binop.op == '>')
					tmp = condition - init;
				else 
					tmp = condition -init + 1;
				if(from->u.For.next->typ  == Unary)		//一元操作符
				{
					if(from->u.For.next->u.unary.op == DECR)		
						tmp *= -1;
				}
				else if(from->u.For.next->typ  == Binop)
				{
					if(from->u.For.next->u.binop.op == MINUSassign)
						tmp *= -1;
					step = GetValue(from->u.For.next->u.binop.right)->u.Const.value.i;
					tmp = (tmp + step-1)/step; 
				}
			}
		  else
			  tmp = LOOP_COUNT;
		  WEST_astwalk(from);
		  newWork=totalWork;
		  totalWork=oldWork+tmp*(newWork-oldWork);
		  break;
		  }
	  case If:		
	  case IfElse:		  
		  oldWork = totalWork;
		  WEST_astwalk(from);
		  newWork = totalWork;
		  totalWork += (newWork - oldWork) / 2;
		  totalWork += IF;
		  break;
	  case Do:
	  case While:		  
		  oldWork = totalWork;
		  WEST_astwalk(from);
		  newWork = totalWork;
		  totalWork = oldWork + LOOP_COUNT*(newWork - oldWork);
		  break;
	  case Switch:	
		  WEST_astwalk(from);
		  totalWork += SWITCH;
		  break;
	  case Case:
	  case Default: WEST_astwalk(from); break;
	  case Break:         
		  WEST_astwalk(from);
		  totalWork += BREAK;
		  break;
	  case Continue:	  
		  WEST_astwalk(from);
		  totalWork += CONTINUE;
		  break;
	  case Call:         
		  WEST_astwalk(from);
		  assert(from->u.call.name->typ == Id);
		  {
			  const char *ident = from->u.call.name->u.id.text;
			  if (strcmp(ident,"acos") == 0) 
				  totalWork += 515/1;
			  else if (strcmp(ident,"acosh")==0) 
				  totalWork += 665/1;
			  else if (strcmp(ident,"acosh")==0) 
				  totalWork += 665/1;
			  else if (strcmp(ident,"asin")==0) 
				  totalWork += 536/1;
			  else if (strcmp(ident,"asinh")==0) 
				  totalWork += 578/1;
			  else if (strcmp(ident,"atan")==0) 
				  totalWork += 195/1;
			  else if (strcmp(ident,"atan2")==0) 
				  totalWork += 272/1;
			  else if (strcmp(ident,"atanh")==0) 
				  totalWork += 304/1;
			  else if (strcmp(ident,"ceil")==0) 
				  totalWork += 47/1;
			  else if (strcmp(ident,"cos")==0) 
				  totalWork += 120/1;
			  else if (strcmp(ident,"cosh")==0) 
				  totalWork += 368/1;
			  else if (strcmp(ident,"exp")==0) 
				  totalWork += 162/1;
			  else if (strcmp(ident,"expm1")==0) 
				  totalWork += 220/1;
			  else if (strcmp(ident,"floor")==0) 
				  totalWork += 58/1;
			  else if (strcmp(ident,"fmod")==0) 
				  totalWork += 147/1;
			  else if (strcmp(ident,"frexp")==0) 
				  totalWork += 60/1;
			  else if (strcmp(ident,"log")==0) 
				  totalWork += 146/1;
			  else if (strcmp(ident,"log10")==0) 
				  totalWork += 212/1;
			  else if (strcmp(ident,"log1p")==0) 
				  totalWork += 233/1;
			  else if (strcmp(ident,"modf")==0) 
				  totalWork += 41/1;
			  else if (strcmp(ident,"pow")==0) 
				  totalWork += 554/1;
			  else if (strcmp(ident,"sin")==0) 
				  totalWork += 97/1;
			  else if (strcmp(ident,"sinh")==0) 
				  totalWork += 303/1;
			  else if (strcmp(ident,"sqrt")==0) 
				  totalWork += 297/1;
			  else if (strcmp(ident,"tan")==0) 
				  totalWork += 224/1;
			  else if (strcmp(ident,"tanh")==0) 
				  totalWork += 288/1;
			  // not from profiling: round(x) is currently macro for floor((x)+0.5)
			  else if (strcmp(ident,"round")==0) 
				  totalWork += (58 + FLOAT_ARITH_OP)/1;
			  // not from profiling: just stuck in here to keep compilation of gmti 
			  // from spewing warnings.
			  else if (strcmp(ident,"abs")==0) 
				  totalWork += 60/1;
			  else if (strcmp(ident,"max")==0) 
				  totalWork += 60/1;
			  else if (strcmp(ident,"min")==0) 
				  totalWork += 60/1;
			  else if(strcmp(ident,"println")==0)
				  totalWork += PRINTLN_OP/1;			//修改		
			  else
			  {
				 /* JMethodDeclaration target = findMethod(ident);
				  if (target != null)
					  target.accept(this);
				  else {
					  System.err.println("Warning:  Work estimator couldn't find target method \"" + ident + "\"" + "\n" + 
						  "   Will assume constant totalWork overhead of " + WorkConstants.UNKNOWN_METHOD_CALL);
					  totalWork += UNKNOWN_METHOD_CALL;
				  }*/
				  totalWork += UNKNOWN_METHOD_CALL/1;
			  }
			  totalWork += METHOD_CALL_OVERHEAD/1;
		  }
		  break;
	  case OperBody:		//入口
		  WEST_astwalk(from);
		  break;
	  case Cast:
		  WEST_astwalk(from->u.cast.expr);
		  break;
	  default:
		  break;
	}
}

GLOBAL int workEstimate_init(Node *from,int w_init)
{
	state = INIT;
	totalWork = w_init;
	rWorkCompute(from);
	return totalWork;
}

GLOBAL int workEstimate(Node *from,int w)
{
	state =STEADY;
	totalWork = w;
	if(from->coord.line == 0)
		isSTREAM = 1;
	rWorkCompute(from);
	isSTREAM = 0;
	return totalWork;
}