#include <string.h>
#include <stdio.h>
#include "rename.h"
int renameNumber[6];//定义符号表中各结构的项数
//GLOBAL char tempName[300][2000];
void New_astwalk(Node *n);
/***********************************************************************\
* ly:LookupVariableRenameSymbol
\***********************************************************************/
/* returns TRUE if symbol found, *var set to the found oldID
   (*oldID valid only if return value is TRUE */

int My_rename(const char *name,const char **newName,int *number,TypeQual tq,NodeType typ)
{
	char sn[40];
	int t,i,k;
	char ch;
	//char s[2000];
	char *s = (char*)malloc(sizeof(char)*256);
	char *prefix;
	int n,j=0;
	switch(typ){
		case Fdcl:
			prefix = "Fun_";
			n = ++number[0];
			break;
		case Comdcl:
			prefix = "Cmp_";
			n = ++number[1];
			break;
		case Operdcl:
			prefix = "Opt_";
			n = ++number[2];
			break;
		case STRdcl:
			prefix = "Str_";
			n = ++number[3];
			break;
		default:
			switch(tq){
				case T_TOP_DECL:
					prefix = "Glb_";
					n = ++number[4];
					break;
				default:
					prefix = "Var_";
					n = ++number[5];
					break;
			}

	}
	strcpy(s,prefix);
	strcat(s,name);	
	while(n){
		sn[j++] = n%10+'0';
		n /= 10;
	}
	sn[j++] = '_';
	sn[j] = '\0';
	//strrev(sn);
	t = !(j%2)?1:0;
	for(i=j-1,k=0;i>(j/2-t);i--){
		ch = sn[i];
		sn[i] = sn[k];
		sn[k++] = ch;
	}
	
	strcat(s,sn);
	*newName = s;
	//printf("%s\n",s);
	return 0;
	
}

void New_listwalk(List *l)
{
	ListMarker _listwalk_marker; Node *_listwalk_ref; 
	assert(l);
	IterateList(&_listwalk_marker, l); 
	while (NextOnList(&_listwalk_marker, (GenericREF)&_listwalk_ref)) { 
		if (_listwalk_ref) 
			New_astwalk(_listwalk_ref);
		SetCurrentOnList(&_listwalk_marker, (Generic *)_listwalk_ref); 
	}
}
void New_astwalk(Node *n)
{	
	const char *newName;
	//const char *tempName;
	switch((n)->typ){
		case Const:         if ((n)->u.Const.type) {New_astwalk((n)->u.Const.type);} break; 
		 case Id:            
			 LookupVariableRenameSymbol(VariableRenameTable,&((n)->u.id.text),(n)->u.id.decl);
				
			 break; 
		 case Binop:         if ((n)->u.binop.left) {New_astwalk((n)->u.binop.left);} 
							 if ((n)->u.binop.right) {New_astwalk((n)->u.binop.right);} break; 
		 case Unary:         if ((n)->u.unary.expr) {New_astwalk((n)->u.unary.expr);} break; 
		 case Cast:          if ((n)->u.cast.type) {New_astwalk((n)->u.cast.type);} 
							 if ((n)->u.cast.expr) {New_astwalk((n)->u.cast.expr);} break; 
		 case Comma:         if ((n)->u.comma.exprs) {New_listwalk((n)->u.comma.exprs);} break; 
		 case Ternary:       if ((n)->u.ternary.cond) {New_astwalk((n)->u.ternary.cond);} if ((n)->u.ternary.true_) {New_astwalk((n)->u.ternary.true_);} if ((n)->u.ternary.false_) {New_astwalk((n)->u.ternary.false_);} break; 
		 case Array:         if ((n)->u.array.name) {New_astwalk((n)->u.array.name);} if ((n)->u.array.dims) {New_listwalk((n)->u.array.dims);} break; 
		 case Call:          //if ((n)->u.call.name) {New_astwalk((n)->u.call.name);} 
							 if ((n)->u.call.args) {New_listwalk((n)->u.call.args);} 
			 break; 
		 case Initializer:   if ((n)->u.initializer.exprs) {New_listwalk((n)->u.initializer.exprs);} break; 
		 case ImplicitCast:  if ((n)->u.implicitcast.expr) {New_astwalk((n)->u.implicitcast.expr);} break; 
		 case Label:         if ((n)->u.label.stmt) {New_astwalk((n)->u.label.stmt);} break; 
		 case Switch:        if ((n)->u.Switch.expr) {New_astwalk((n)->u.Switch.expr);} if ((n)->u.Switch.stmt) {New_astwalk((n)->u.Switch.stmt);} break; 
		 case Case:          if ((n)->u.Case.expr) {New_astwalk((n)->u.Case.expr);} if ((n)->u.Case.stmt) {New_astwalk((n)->u.Case.stmt);} break; 
		 case Default:       if ((n)->u.Default.stmt) {New_astwalk((n)->u.Default.stmt);} break; 
		 case If:            if ((n)->u.If.expr) {New_astwalk((n)->u.If.expr);} if ((n)->u.If.stmt) {New_astwalk((n)->u.If.stmt);} break; 
		 case IfElse:        if ((n)->u.IfElse.expr) {New_astwalk((n)->u.IfElse.expr);} if ((n)->u.IfElse.true_) {New_astwalk((n)->u.IfElse.true_);} if ((n)->u.IfElse.false_) {New_astwalk((n)->u.IfElse.false_);} break; 
		 case While:         if ((n)->u.While.expr) {New_astwalk((n)->u.While.expr);} if ((n)->u.While.stmt) {New_astwalk((n)->u.While.stmt);} break; 
		 case Do:            if ((n)->u.Do.stmt) {New_astwalk((n)->u.Do.stmt);} if ((n)->u.Do.expr) {New_astwalk((n)->u.Do.expr);} break; 
		 case For:           if ((n)->u.For.init) {New_astwalk((n)->u.For.init);} if ((n)->u.For.cond) {New_astwalk((n)->u.For.cond);} if ((n)->u.For.next) {New_astwalk((n)->u.For.next);} if ((n)->u.For.stmt) {New_astwalk((n)->u.For.stmt);} break; 
		 case Goto:          break; 
		 case Continue:      break; 
		 case Break:         break; 
		 case Return:        if ((n)->u.Return.expr) {New_astwalk((n)->u.Return.expr);} break; 
		 case Block:         if ((n)->u.Block.decl) {New_listwalk((n)->u.Block.decl);} if ((n)->u.Block.stmts) {New_listwalk((n)->u.Block.stmts);} break; 
		 case Prim:          break;
		 case Tdef:          break; 
		 case Ptr:           if ((n)->u.ptr.type) {New_astwalk((n)->u.ptr.type);} break; 
		 case Adcl:          if ((n)->u.adcl.type) {New_astwalk((n)->u.adcl.type);} if ((n)->u.adcl.dim) {New_astwalk((n)->u.adcl.dim);} break; 
		 case Fdcl:          if ((n)->u.fdcl.args) {New_listwalk((n)->u.fdcl.args);} if ((n)->u.fdcl.returns) {New_astwalk((n)->u.fdcl.returns);} break; 
		 case Sdcl:          if (SUE_ELABORATED((n)->u.sdcl.tq) && (n)->u.sdcl.type->fields) {New_listwalk((n)->u.sdcl.type->fields);} break;
		 case Udcl:          if (SUE_ELABORATED((n)->u.udcl.tq) && (n)->u.udcl.type->fields) {New_listwalk((n)->u.udcl.type->fields);} break; 
		 case Edcl:          if (SUE_ELABORATED((n)->u.edcl.tq) && (n)->u.edcl.type->fields) {New_listwalk((n)->u.edcl.type->fields);} break; 
		 case Decl:        //if(n->u.decl.type->typ == Adcl||n->u.decl.type->typ == Prim||n->u.decl.type->typ == Ptr){
							if((strcmp(n->u.decl.name,"Main")!=0)&&(strcmp(n->u.decl.name,"FileReader")!=0)&&(strcmp(n->u.decl.name,"FileWriter")!=0)){
							 if((!LookupVariableRenameSymbol(VariableRenameTable,&((n)->u.decl.name),n))){
								 if(strlen((n)->u.decl.name)>4&&strncmp((n)->u.decl.name,"Var_",4)==0)
									 break;
								 My_rename((n)->u.decl.name,&newName,renameNumber,n->u.decl.tq,n->u.decl.type->typ);
								
								 ASTInsertSymbol(VariableRenameTable,(n)->u.decl.name,newName,n,n);
								 
								 LookupExternalsSymbol(Externals,n->u.decl.name,newName,n);
																		 								 
								 n->u.decl.name = newName;
								// printf("%s\n",newName);
							 }
						
							 if ((n)->u.decl.type) {New_astwalk((n)->u.decl.type);} if ((n)->u.decl.init) {New_astwalk((n)->u.decl.init);} if ((n)->u.decl.bitsize) {New_astwalk((n)->u.decl.bitsize);}

							}
							 break; 
							 
		 case Attrib:     if (n->u.attrib.arg) {New_astwalk(n->u.attrib.arg);} break; \
		 case Proc:          if ((n)->u.proc.decl) {New_astwalk((n)->u.proc.decl);} if ((n)->u.proc.body) {New_astwalk((n)->u.proc.body);} break; 
		 case Text:          break; 
		 case STRdcl:       
			 if (STREAM_ELABORATED((n)->u.strdcl.tq) && (n)->u.strdcl.type->fields) {New_listwalk((n)->u.strdcl.type->fields);} 
			 break;
		 case Comdcl:        if ((n)->u.comdcl.inout){New_astwalk((n)->u.comdcl.inout);}break; 
		 case Composite:     if ((n)->u.composite.decl){New_astwalk((n)->u.composite.decl);}if ((n)->u.composite.body){New_astwalk((n)->u.composite.body);} break;
		 case ComInOut:      if ((n)->u.comInOut.inputs){New_listwalk((n)->u.comInOut.inputs);}if ((n)->u.comInOut.outputs){New_listwalk((n)->u.comInOut.outputs);} break; 
		 case ComBody:       if ((n)->u.comBody.param){New_astwalk((n)->u.comBody.param);} if ((n)->u.comBody.decl){New_listwalk((n)->u.comBody.decl);} if((n)->u.comBody.comstmts){New_listwalk((n)->u.comBody.comstmts);} break;
		 case Param:         if ((n)->u.param.parameters){New_listwalk((n)->u.param.parameters);} break;
		 case OperBody:      if ((n)->u.operBody.state) {New_listwalk((n)->u.operBody.state);}if ((n)->u.operBody.init) {New_astwalk((n)->u.operBody.init);}if ((n)->u.operBody.work) {New_astwalk((n)->u.operBody.work);}if ((n)->u.operBody.window){New_listwalk((n)->u.operBody.window);} break;
		 case Operdcl:       if ((n)->u.operdcl.inputs){New_listwalk((n)->u.operdcl.inputs );}if ((n)->u.operdcl.outputs){New_listwalk((n)->u.operdcl.outputs);}
							 if ((n)->u.operdcl.arguments){New_listwalk((n)->u.operdcl.arguments);}break; 
		 case Operator_:     if ((n)->u.operator_.decl) {New_astwalk((n)->u.operator_.decl);} if ((n)->u.operator_.body) {New_astwalk((n)->u.operator_.body);}break; 
		 case Window:        if ((n)->u.window.id) {New_astwalk((n)->u.window.id);} if ((n)->u.window.wtype){New_astwalk((n)->u.window.wtype);} break;
		 case Sliding:       if ((n)->u.sliding.sliding_value) {New_astwalk((n)->u.sliding.sliding_value);}
		 case Tumbling:      if ((n)->u.tumbling.tumbling_value) {New_astwalk((n)->u.tumbling.tumbling_value);} break; 
		 case CompositeCall: //if ((n)->u.comCall.call) {New_astwalk((n)->u.comCall.call);} 
							 if ((n)->u.comCall.operdcl) {New_astwalk((n)->u.comCall.operdcl);}
			 break;
		 case Pipeline:		 if ((n)->u.pipeline.output) {New_astwalk((n)->u.pipeline.output);} if ((n)->u.pipeline.input) {New_astwalk((n)->u.pipeline.input);} if ((n)->u.pipeline.decl){New_listwalk((n)->u.pipeline.decl);}if ((n)->u.pipeline.stmts){New_listwalk((n)->u.pipeline.stmts);}break;
		 case SplitJoin:	 if ((n)->u.splitJoin.output) {New_astwalk((n)->u.splitJoin.output);}if ((n)->u.splitJoin.input) {New_astwalk((n)->u.splitJoin.input);}if ((n)->u.splitJoin.decl) {New_listwalk((n)->u.splitJoin.decl);}if ((n)->u.splitJoin.initstmts) {New_listwalk((n)->u.splitJoin.initstmts);}if ((n)->u.splitJoin.split) {New_astwalk((n)->u.splitJoin.split);}if ((n)->u.splitJoin.stmts) {New_listwalk((n)->u.splitJoin.stmts);}if ((n)->u.splitJoin.join) {New_astwalk((n)->u.splitJoin.join);} break; 
		 case Split:		 if ((n)->u.split.type) {New_astwalk((n)->u.split.type);} break; 
		 case Join:			 if ((n)->u.join.type) {New_astwalk((n)->u.join.type);} break;
		 case RoundRobin:	 if ((n)->u.roundrobin.arguments){New_listwalk((n)->u.roundrobin.arguments);}break; 
		 case Duplicate:	 if ((n)->u.duplicate.expr) {New_astwalk((n)->u.duplicate.expr);} break; 
		 case Add:			 if(n->u.add.content){New_astwalk(n->u.add.content);} break;
		 default:   break; 
	}
}

GLOBAL List *VariableRenameProgram(List *program)
{
	ListMarker marker;
	Node *item;
	int i;
	for(i=0;i<6;i++){
		renameNumber[i] = 0;
	}
	IterateList(&marker, program);
	while (NextOnList(&marker, (GenericREF) &item)) 
	{
		assert(item);
		if(!item->coord.includedp)
			New_astwalk(item);

	}
	
	return program;
}	
