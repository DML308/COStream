/*************************************************************************
*
*  C-to-C Translator
*
*  Adapted from Clean ANSI C Parser
*  Eric A. Brewer, Michael D. Noakes
*  
*  ast.c,v
* Revision 1.14  1995/05/11  18:54:05  rcm
* Added gcc extension __attribute__.
*
* Revision 1.13  1995/05/05  19:18:21  randall
* Added #include reconstruction.
*
* Revision 1.12  1995/04/21  05:44:01  rcm
* Cleaned up data-flow analysis, and separated into two files, dataflow.c
* and analyze.c.  Fixed void pointer arithmetic bug (void *p; p+=5).
* Moved CVS Id after comment header of each file.
*
* Revision 1.11  1995/03/23  15:30:46  rcm
* Dataflow analysis; removed IsCompatible; replaced SUN4 compile-time symbol
* with more specific symbols; minor bug fixes.
*
* Revision 1.10  1995/03/01  16:23:03  rcm
* Various type-checking bug fixes; added T_REDUNDANT_EXTERNAL_DECL.
*
* Revision 1.9  1995/02/13  18:14:51  rcm
* Fixed LISTWALK to skip non-null list members.
*
* Revision 1.8  1995/02/13  01:59:57  rcm
* Added ASTWALK macro; fixed some small bugs.
*
* Revision 1.7  1995/02/01  23:03:40  rcm
* Added Text node and #pragma collection
*
* Revision 1.6  1995/02/01  21:07:05  rcm
* New AST constructors convention: MakeFoo makes a foo with unknown coordinates,
* whereas MakeFooCoord takes an explicit Coord argument.
*
* Revision 1.5  1995/01/27  01:38:50  rcm
* Redesigned type qualifiers and storage classes;  introduced "declaration
* qualifier."
*
* Revision 1.4  1995/01/20  03:37:57  rcm
* Added some GNU extensions (long long, zero-length arrays, cast to union).
* Moved all scope manipulation out of lexer.
*
* Revision 1.3  1995/01/06  16:48:30  rcm
* added copyright message
*
* Revision 1.2  1994/12/23  09:18:14  rcm
* Added struct packing rules from wchsieh.  Fixed some initializer problems.
*
* Revision 1.1  1994/12/20  09:20:20  rcm
* Created
*
* Revision 1.5  1994/11/22  01:54:32  rcm
* No longer folds constant expressions.
*
* Revision 1.4  1994/11/10  03:13:14  rcm
* Fixed line numbers on AST nodes.
*
* Revision 1.3  1994/11/03  07:38:43  rcm
* Added code to output C from the parse tree.
*
* Revision 1.2  1994/10/28  18:52:32  rcm
* Removed ALEWIFE-isms.
*
*
*  Created: Mon Apr 26 12:48:52 EDT 1993
*
*
*
* Copyright (c) 1994 MIT Laboratory for Computer Science
* 
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
* 
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE MIT LABORATORY FOR COMPUTER SCIENCE BE LIABLE
* FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
* Except as contained in this notice, the name of the MIT Laboratory for
* Computer Science shall not be used in advertising or otherwise to
* promote the sale, use or other dealings in this Software without prior
* written authorization from the MIT Laboratory for Computer Science.
* 
*************************************************************************/
#pragma ident "ast.c,v 1.14 1995/05/11 18:54:05 rcm Exp Copyright 1994 Massachusetts Institute of Technology"

#include "ast.h"


GLOBAL Coord UnknownCoord = { 
	/* line:   */ 0,
	/* offset: */ 0,
	/* file:   */	0,
	/* includedp: */ FALSE
};

/***********************--------------Define For SPL----------****************************/
typedef struct{//zww
	Node *old_container;
	Node *new_container;
} revised_container;
List *containerList =NULL;//zww:用于深拷贝是修改节点的container

GLOBAL inline char *AddPostFixName(const char *name)
{
	char *newName = (char *)malloc(strlen(name) + 1); 

	sprintf(newName, "%s_", name);

	free(name);
	return newName;
}
/***********************--------------Define For SPL----------****************************/

/* use HeapNew() (defined in ast.h) to allocate whole objects */
GLOBAL inline void *HeapAllocate(int number, int size)
{ 
	return calloc(number, size);
}

GLOBAL inline void HeapFree(void *ptr)
{
	free(ptr);
}



GLOBAL inline Node *NewNode(NodeType typ)
{
	Node *create = HeapNew(Node);

	create->typ = typ;
	create->coord = UnknownCoord;
	create->parenthesized = FALSE;
	create->analysis.livevars = NULL;
	return(create);
}

/*ZWW: define for spl 获取运算的数据类型*/
GLOBAL inline opDataType getOpDataType(Node *arg)
{
	opDataType tmp;
	Node *tmpN;
	assert(arg!=NULL);
	switch(arg->typ)
	{
	case Cast: tmp = getOpDataType(arg->u.cast.type);break;
	case Prim:
		if(arg->u.prim.basic <= Slonglong)
			tmp = op_int;
		else if(arg->u.prim.basic >= Float && arg->u.prim.basic <= Longdouble)
			tmp = op_float;
		else
			tmp = op_unkonwn;
		break;
	case Binop: tmp = arg->u.binop.opType; 
		break;
	case Unary: tmp = arg->u.unary.opType;
		break;
	case Id:
		tmpN = arg->u.id.decl;
		if(tmpN == NULL)/*如果是个没有声明的id则返回unknown的opdatatype*/
			tmp = op_unkonwn;
		else{
			tmpN = tmpN->u.decl.type;
			if(tmpN == NULL)
				tmp = op_unkonwn;
			else{
				if(tmpN->typ == Prim)
					if(tmpN->u.prim.basic <= Slonglong)
						tmp = op_int;
					else if(tmpN->u.prim.basic >= Float && tmpN->u.prim.basic <= Longdouble)
						tmp = op_float;
					else
						tmp = op_unkonwn;
				else
					tmp = op_unkonwn;
			}

		}
		break;
	case Const: 
		if (arg->u.Const.type == PrimSint || arg->u.Const.type == PrimUint 
			|| arg->u.Const.type == PrimSlong || arg->u.Const.type == PrimSlonglong || arg->u.Const.type == PrimSshort
			|| arg->u.Const.type == PrimUshort)
			tmp = op_int;
		else if(arg->u.Const.type == PrimFloat || arg->u.Const.type == PrimDouble|| arg->u.Const.type ==PrimLongdouble)
			tmp = op_float;
		else
			tmp = op_unkonwn;
		break;
	case Array: 
		tmpN = arg->u.array.name;
		if(tmpN != NULL){
			if (tmpN->typ == Id)
			{
				tmpN=tmpN->u.id.decl;
				if (tmpN!=NULL)
				{	
					tmpN = tmpN->u.decl.type;
					if(tmpN == NULL)
						tmp = op_unkonwn;
					else{
						if(tmpN->typ == Prim)
							if(tmpN->u.prim.basic <= Slonglong)
								tmp = op_int;
							else if(tmpN->u.prim.basic >= Float && tmpN->u.prim.basic <= Longdouble)
								tmp = op_float;
							else
								tmp = op_unkonwn;
						else
							tmp = op_unkonwn;
					}
				}else tmp = op_unkonwn;
			}
			else tmp = op_unkonwn;
		}else
			tmp = op_unkonwn;
		break;
	case Call:	tmp = op_unkonwn;
		break;
	default:
		Warning(1, "Internal Error!: unknown op data type!\n");
		break;
	}
	return tmp;
}


/*************************************************************************/
/*                                                                       */
/*                          Expression nodes                             */
/*                                                                       */
/*************************************************************************/

GLOBAL inline Node *MakeConstSint(int value)
{ Node *node = NewNode(Const);
node->u.Const.type = PrimSint;
node->u.Const.value.i  = value;
node->u.Const.text = NULL;
return node;
}

GLOBAL inline Node *MakeConstSintTextCoord(const char *text, int value, Coord coord)
{
	Node *create = MakeConstSint(value);
	create->u.Const.text = text;
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeConstPtr(unsigned int value)
{ Node *node = NewNode(Const);
node->u.Const.type = PtrVoid;
node->u.Const.value.u  = value;
node->u.Const.text = NULL;
return node;
}

GLOBAL inline Node *MakeConstPtrTextCoord(const char *text, unsigned int value, Coord coord)
{
	Node *create = MakeConstPtr(value);
	create->u.Const.text = text;
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeConstUint(unsigned int value)
{ Node *node = NewNode(Const);
node->u.Const.type = PrimUint;
node->u.Const.value.u  = value;
node->u.Const.text = NULL;
return node;
}

GLOBAL inline Node *MakeConstUintTextCoord(const char *text, unsigned int value, Coord coord)
{
	Node *create = MakeConstUint(value);
	create->u.Const.text = text;
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeConstSlong(long value)
{ Node *node = NewNode(Const);
node->u.Const.type = PrimSlong;
node->u.Const.value.l  = value;
node->u.Const.text = NULL;
return node;
}

GLOBAL inline Node *MakeConstSlongTextCoord(const char *text, long value, Coord coord)
{
	Node *create = MakeConstSlong(value);
	create->u.Const.text = text;
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeConstUlong(unsigned long value)
{ Node *node = NewNode(Const);
node->u.Const.type = PrimUlong;
node->u.Const.value.ul  = value;
node->u.Const.text = NULL;
return node;
}

GLOBAL inline Node *MakeConstUlongTextCoord(const char *text, unsigned long value, Coord coord)
{
	Node *create = MakeConstUlong(value);
	create->u.Const.text = text;
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeConstFloat(float value)
{ Node *node = NewNode(Const);
node->u.Const.type = PrimFloat;
node->u.Const.value.f  = value;
node->u.Const.text = NULL;
return node;
}

GLOBAL inline Node *MakeConstFloatTextCoord(const char *text, float value, Coord coord)
{
	Node *create = MakeConstFloat(value);
	create->u.Const.text = text;
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeConstDouble(double value)
{ Node *node = NewNode(Const);
node->u.Const.type = PrimDouble;
node->u.Const.value.d  = value;
node->u.Const.text = NULL;
return node;
}

GLOBAL inline Node *MakeConstDoubleTextCoord(const char *text, double value, Coord coord)
{
	Node *create = MakeConstDouble(value);
	create->u.Const.text = text;
	create->coord = coord;
	return(create);
}



GLOBAL inline Node *MakeString(const char *value)
{ Node *node = NewNode(Const),
*adcl = MakeAdcl(EMPTY_TQ, PrimChar, MakeConstSint(strlen(value) + 1));

node->u.Const.type = adcl;
node->u.Const.value.s  = value; /* quotes stripped, escape sequences converted */
node->u.Const.text = NULL;
return node;
}

GLOBAL inline Node *MakeStringTextCoord(const char *text, const char *value, Coord coord)
{
	Node *create = MakeString(value);
	create->u.Const.text = text;
	create->coord = coord;
	return(create);
}


GLOBAL inline Node *MakeId(const char* text)
{
	Node *create = NewNode(Id);

	create->u.id.text = text;
	create->u.id.decl = NULL;
	return(create);
}

GLOBAL inline Node *MakeIdCoord(const char* text, Coord coord)
{
	Node *create = MakeId(text);
	create->coord = coord;
	return(create);
}



GLOBAL inline Node *MakeUnary(OpType op, Node *expr)
{ Node *create = NewNode(Unary);

if (op == '*')
	op = INDIR;
else if (op == '&')
	op = ADDRESS;
else if (op == '-')
	op = UMINUS;
else if (op == '+')
	op = UPLUS;

create->u.unary.op   = op;
create->u.unary.expr = expr;

create->u.unary.type = NULL;
create->u.unary.value = NULL;
if(expr != NULL)/*zww:计算该1元操作的数据类型*/
	create->u.unary.opType = getOpDataType(create->u.unary.expr);
else create->u.unary.opType = op_unkonwn;

return create;
}

GLOBAL inline Node *MakeUnaryCoord(OpType op, Node *expr, Coord coord)
{
	Node *create = MakeUnary(op, expr);
	create->coord = coord;

	return(create);
}


GLOBAL inline Node *MakeBinop(OpType op, Node *left, Node *right)
{ Node *create = NewNode(Binop);

create->u.binop.op    = op;
create->u.binop.left  = left;
create->u.binop.right = right;
create->u.binop.type = NULL;
create->u.binop.value = NULL;

if (left!=NULL && right != NULL){ /*zww:计算该2元操作的数据类型*/
	if(op != '='){
		opDataType l,r;
		l = getOpDataType(create->u.binop.left);
		r = getOpDataType(create->u.binop.right);
		create->u.binop.opType = l > r ? l:r;
	}else//如果是赋值表达式，直接取左边的类型作为op类型
		create->u.binop.opType = getOpDataType(create->u.binop.left);
}else  create->u.binop.opType = op_unkonwn;

return(create);
}

GLOBAL inline Node *MakeBinopCoord(OpType op, Node *left, Node *right, Coord coord)
{
	Node *create = MakeBinop(op, left, right);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeCast(Node *type, Node *expr)
{ Node *create = NewNode(Cast);

create->u.cast.type = type;
create->u.cast.expr = expr;
create->u.cast.value = NULL;
return(create);
}

GLOBAL inline Node *MakeCastCoord(Node *type, Node *expr, Coord coord)
{
	Node *create = MakeCast(type, expr);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeComma(List* exprs)
{
	Node *create = NewNode(Comma);
	create->u.comma.exprs  = exprs;
	return(create);
}

GLOBAL inline Node *MakeCommaCoord(List* exprs, Coord coord)
{
	Node *create = MakeComma(exprs);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeTernary(Node *cond, Node *true_, Node *false_)
{
	Node *create = NewNode(Ternary);
	create->u.ternary.cond  = cond;
	create->u.ternary.true_  = true_;
	create->u.ternary.false_ = false_;
	create->u.ternary.colon_coord = UnknownCoord;
	create->u.ternary.type  = NULL;
	create->u.ternary.value = NULL;
	return(create);
}

GLOBAL inline Node *MakeTernaryCoord(Node *cond, Node *true_, Node *false_, Coord qmark_coord, Coord colon_coord)
{
	Node *create = MakeTernary(cond, true_, false_);
	create->coord = qmark_coord;
	create->u.ternary.colon_coord = colon_coord;
	return(create);
}

GLOBAL inline Node *MakeArray(Node *name, List* dims)
{
	Node *create = NewNode(Array);
	create->u.array.type = NULL;
	create->u.array.name = name;
	create->u.array.dims = dims;
	return(create);
}

GLOBAL inline Node *MakeArrayCoord(Node *name, List* dims, Coord coord)
{
	Node *create = MakeArray(name, dims);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeCall(Node *name, List* args)
{
	Node *create = NewNode(Call);
	create->u.call.name = name;
	create->u.call.args = args;
	create->coord = name->coord;
	return(create);
}

GLOBAL inline Node *MakeCallCoord(Node *name, List* args, Coord coord)
{
	Node *create = MakeCall(name, args);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeInitializer(List* exprs)
{
	Node *create = NewNode(Initializer);
	create->u.comma.exprs  = exprs;
	return(create);
}

GLOBAL inline Node *MakeInitializerCoord(List* exprs, Coord coord)
{
	Node *create = MakeInitializer(exprs);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeImplicitCast(Node *type, Node *expr)
{
	Node *create = NewNode(ImplicitCast);
	//printf("%d,%d\n",type->typ,expr->typ);
	create->u.implicitcast.type = type;
	create->u.implicitcast.expr = expr;
	create->u.implicitcast.value = NULL;
	return(create);
}

GLOBAL inline Node *MakeImplicitCastCoord(Node *type, Node *expr, Coord coord)
{
	Node *create = MakeImplicitCast(type, expr);
	create->coord = coord;
	return(create);
}

/*************************************************************************/
/*                                                                       */
/*                          Statement nodes                              */
/*                                                                       */
/*************************************************************************/

GLOBAL inline Node *MakeLabel(const char* name, Node *stmt)
{
	Node *create = NewNode(Label);
	create->u.label.name       = name;
	create->u.label.stmt       = stmt;
	create->u.label.references = NULL;
	return(create);
}

GLOBAL inline Node *MakeLabelCoord(const char* name, Node *stmt, Coord coord)
{
	Node *create = MakeLabel(name, stmt);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeSwitch(Node *expr, Node *stmt, List* cases)
{
	Node *create = NewNode(Switch);
	create->u.Switch.expr = expr;
	create->u.Switch.stmt = stmt;
	create->u.Switch.cases = cases;
	create->u.Switch.has_default = FALSE;
	while (cases) {
		Node *n = FirstItem(cases);
		assert(n);
		if (n->typ == Default)
			create->u.Switch.has_default = TRUE;
		cases = Rest(cases);
	}
	return(create);
}

GLOBAL inline Node *MakeSwitchCoord(Node *expr, Node *stmt, List* cases, Coord coord)
{
	Node *create = MakeSwitch(expr, stmt, cases);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeCase(Node *expr, Node *stmt, Node *container)
{
	Node *create = NewNode(Case);
	create->u.Case.expr = expr;
	create->u.Case.stmt = stmt;
	create->u.Case.container = container;
	return(create);
}

GLOBAL inline Node *MakeCaseCoord(Node *expr, Node *stmt, Node *container, Coord coord)
{
	Node *create = MakeCase(expr, stmt, container);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeDefault(Node *stmt, Node *container)
{
	Node *create = NewNode(Default);
	create->u.Default.stmt = stmt;
	create->u.Default.container = container;
	return(create);
}

GLOBAL inline Node *MakeDefaultCoord(Node *stmt, Node *container, Coord coord)
{
	Node *create = MakeDefault(stmt, container);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeIf(Node *expr, Node *stmt)
{
	Node *create = NewNode(If);
	create->u.If.expr = expr;
	create->u.If.stmt = stmt;
	return(create);
}

GLOBAL inline Node *MakeIfCoord(Node *expr, Node *stmt, Coord coord)
{
	Node *create = MakeIf(expr, stmt);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeIfElse(Node *expr, Node *true_, Node *false_)
{
	Node *create = NewNode(IfElse);
	create->u.IfElse.expr = expr;
	create->u.IfElse.true_ = true_;
	create->u.IfElse.false_ = false_;
	create->u.IfElse.else_coord = UnknownCoord;
	return(create);
}

GLOBAL inline Node *MakeIfElseCoord(Node *expr, Node *true_, Node *false_, Coord if_coord, Coord else_coord)
{
	Node *create = MakeIfElse(expr, true_, false_);
	create->coord = if_coord;
	create->u.IfElse.else_coord = else_coord;
	return(create);
}

GLOBAL inline Node *MakeWhile(Node *expr, Node *stmt)
{
	Node *create = NewNode(While);
	create->u.While.expr = expr;
	create->u.While.stmt = stmt;
	return(create);
}

GLOBAL inline Node *MakeWhileCoord(Node *expr, Node *stmt, Coord coord)
{
	Node *create = MakeWhile(expr, stmt);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeDo(Node *stmt, Node *expr)
{
	Node *create = NewNode(Do);
	create->u.Do.stmt = stmt;
	create->u.Do.expr = expr;
	create->u.Do.while_coord = UnknownCoord;
	return(create);
}

GLOBAL inline Node *MakeDoCoord(Node *stmt, Node *expr, Coord do_coord, Coord while_coord)
{
	Node *create = MakeDo(stmt, expr);
	create->coord = do_coord;
	create->u.Do.while_coord = while_coord;
	return(create);
}

GLOBAL inline Node *MakeFor(Node *init, Node *cond, Node *next, Node *stmt)
{
	Node *create = NewNode(For);
	create->u.For.init = init;
	create->u.For.cond = cond;
	create->u.For.next = next;
	create->u.For.stmt = stmt;
	return(create);
}

GLOBAL inline Node *MakeForCoord(Node *init, Node *cond, Node *next, Node *stmt, Coord coord)
{
	Node *create = MakeFor(init, cond, next, stmt);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeGoto(Node* label)
{
	Node *create = NewNode(Goto);
	create->u.Goto.label = label;
	return(create);
}

GLOBAL inline Node *MakeGotoCoord(Node* label, Coord coord)
{
	Node *create = MakeGoto(label);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeContinue(Node *container)
{
	Node *create = NewNode(Continue);
	create->u.Continue.container = container;
	return(create);
}

GLOBAL inline Node *MakeContinueCoord(Node *container, Coord coord)
{
	Node *create = MakeContinue(container);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeBreak(Node *container)
{
	Node *create = NewNode(Break);

	create->u.Break.container = container;
	return(create);
}

GLOBAL inline Node *MakeBreakCoord(Node *container, Coord coord)
{
	Node *create = MakeBreak(container);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeReturn(Node *expr)
{
	Node *create = NewNode(Return);

	create->u.Return.expr = expr;
	create->u.Return.proc = NULL;
	return(create);
}

GLOBAL inline Node *MakeReturnCoord(Node *expr, Coord coord)
{
	Node *create = MakeReturn(expr);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeBlock(Node *type, List* decl, List* stmts)
{
	Node *create = NewNode(Block);
	create->u.Block.type  = type;
	create->u.Block.decl  = decl;
	create->u.Block.stmts = stmts;
	create->u.Block.right_coord = UnknownCoord;
	return(create);
}

GLOBAL inline Node *MakeBlockCoord(Node *type, List* decl, List* stmts, Coord left_coord, Coord right_coord)
{
	Node *create = MakeBlock(type, decl, stmts);
	create->coord = left_coord;
	create->u.Block.right_coord = right_coord;
	return(create);
}

/*************************************************************************/
/*                                                                       */
/*                            Type nodes                                 */
/*                                                                       */
/*************************************************************************/

GLOBAL inline Node *MakePrim(TypeQual tq, BasicType basic)
{
	Node *create = NewNode(Prim);

	create->u.prim.tq = tq;
	create->u.prim.basic = basic;
	return(create);
}

GLOBAL inline Node *MakePrimCoord(TypeQual tq, BasicType basic, Coord coord)
{
	Node *create = MakePrim(tq, basic);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeTdef(TypeQual tq, const char *name)
{
	Node *create = NewNode(Tdef);
	create->u.tdef.name = name;
	create->u.tdef.tq = tq;
	create->u.tdef.type = NULL;
	return(create);
}

GLOBAL inline Node *MakeTdefCoord(TypeQual tq, const char *name, Coord coord)
{
	Node *create = MakeTdef(tq, name);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakePtr(TypeQual tq, Node *type)
{
	Node *create = NewNode(Ptr);
	create->u.ptr.tq = tq;
	create->u.ptr.type = type;

	return(create);
}

GLOBAL inline Node *MakePtrCoord(TypeQual tq, Node *type, Coord coord)
{
	Node *create = MakePtr(tq, type);
	create->coord = coord;

#if ALLOW_POINTER
	SyntaxErrorCoord(coord, "Syntax error: SPL does not allow using Pointer!!"); //Define For SPL
#endif


	return(create);
}

GLOBAL inline Node *MakeAdcl(TypeQual tq, Node *type, Node *dim)
{
	Node *create = NewNode(Adcl);
	create->u.adcl.tq = tq;
	create->u.adcl.type = type;
#if 0
	/* fix: we need to constant-fold dim during the parse phase in order to
	compare the types of multiply-declared arrays, but this probably isn't
	the best place to do it. -- rcm */
	create->u.adcl.dim  = SemCheckNode(dim);
#endif
	create->u.adcl.dim = dim;
	create->u.adcl.size = 0;
	return(create);
}

GLOBAL inline Node *MakeAdclCoord(TypeQual tq, Node *type, Node *dim, Coord coord)
{
	Node *create = MakeAdcl(tq, type, dim);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeFdcl(TypeQual tq, List* args, Node *returns)
{
	Node *create = NewNode(Fdcl);
	create->u.fdcl.tq = tq;
	create->u.fdcl.args = args;
	create->u.fdcl.returns = returns;
	return(create);
}

GLOBAL inline Node *MakeFdclCoord(TypeQual tq, List* args, Node *returns, Coord coord)
{
	Node *create = MakeFdcl(tq, args, returns);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeSdcl(TypeQual tq, SUEtype* type)
{
	Node *create = NewNode(Sdcl);
	create->u.sdcl.tq   = tq;
	create->u.sdcl.type = type;
	return(create);
}

GLOBAL inline Node *MakeSdclCoord(TypeQual tq, SUEtype* type, Coord coord)
{
	Node *create = MakeSdcl(tq, type);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeUdcl(TypeQual tq, SUEtype* type)
{
	Node *create = NewNode(Udcl);
	create->u.udcl.tq   = tq;
	create->u.udcl.type = type;
	return(create);
}

GLOBAL inline Node *MakeUdclCoord(TypeQual tq, SUEtype* type, Coord coord)
{
	Node *create = MakeUdcl(tq, type);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeEdcl(TypeQual tq, SUEtype* type)
{
	Node *create = NewNode(Edcl);
	create->u.edcl.tq = tq;
	create->u.edcl.type = type;
	return(create);
}

GLOBAL inline Node *MakeEdclCoord(TypeQual tq, SUEtype* type, Coord coord)
{
	Node *create = MakeEdcl(tq, type);
	create->coord = coord;
	return(create);
}


/*************************************************************************/
/*                                                                       */
/*                      Other nodes (decls et al.)                       */
/*                                                                       */
/*************************************************************************/

GLOBAL inline Node *MakeDecl(const char *name, TypeQual tq, Node *type, Node *init, Node *bitsize)
{
	Node *create = NewNode(Decl);
	create->u.decl.name = name;
	create->u.decl.tq = tq;
	create->u.decl.type = type;
	create->u.decl.init = init;
	create->u.decl.bitsize = bitsize;
	create->u.decl.references = 0;
	create->u.decl.attribs = NULL;
	return(create);
}

GLOBAL inline Node *MakeDeclCoord(const char *name, TypeQual tq, Node *type, Node *init, Node *bitsize, Coord coord)
{
	Node *create = MakeDecl(name, tq, type, init, bitsize);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeAttrib(const char *name, Node *arg)
{
	Node *create = NewNode(Attrib);
	create->u.attrib.name = name;
	create->u.attrib.arg = arg;
	return create;
}

GLOBAL inline Node *MakeAttribCoord(const char *name, Node *arg, Coord coord)
{
	Node *create = MakeAttrib(name, arg);
	create->coord = coord;
	return create;
}

GLOBAL inline Node *MakeProc(Node *decl, Node *body)
{
	Node *create = NewNode(Proc);
	create->u.proc.decl = decl;
	create->u.proc.body = body;
	return(create);
}

GLOBAL inline Node *MakeProcCoord(Node *decl, Node *body, Coord coord)
{
	Node *create = MakeProc(decl, body);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeText(const char *text, Bool start_new_line)
{
	Node *create = NewNode(Text);
	create->u.text.text = text;
	create->u.text.start_new_line = start_new_line;
	return create;
}

GLOBAL inline Node *MakeTextCoord(const char *text, Bool start_new_line, Coord coord)
{
	Node *create = MakeText(text, start_new_line);
	create->coord = coord;
	return create;
}


/*************************************************************************/
/*                                                                       */
/*                            Extensions                                 */
/*                                                                       */
/*************************************************************************/


/*****************************************************************

Converting nodes between types

*****************************************************************/

GLOBAL Node *ConvertIdToTdef(Node *id, TypeQual tq, Node *type)
{
	assert(id->typ == Id);
	id->typ = Tdef;
	id->u.tdef.name = id->u.id.text;
	id->u.tdef.tq = tq;
	id->u.tdef.type = type;
	return(id);
}

GLOBAL Node *ConvertIdToDecl(Node *id, TypeQual tq, Node *type, Node *init, Node *bitsize)
{
	assert(id->typ == Id);
	id->typ = Decl;
	id->u.decl.name = id->u.id.text;
	id->u.decl.tq = tq;
	id->u.decl.type = type;
	id->u.decl.init = init;
	id->u.decl.bitsize = bitsize;
	id->u.decl.references = 0;
	return(id);
}

GLOBAL Node *ConvertIdToAttrib(Node *id, Node *arg)
{
	assert(id->typ == Id);
	id->typ = Attrib;
	id->u.attrib.name = id->u.id.text;
	id->u.attrib.arg = arg;
	return(id);
}


#if 0
/* dead code -- rcm */
GLOBAL Node *AdclFdclToPtr(Node *node)
{
	if (node->typ == Decl)
		if (node->u.decl.type->typ == Adcl)
			node->u.decl.type->typ = Ptr;
		else if (node->u.decl.type->typ == Fdcl)
			node->u.decl.type = MakePtrCoord(EMPTY_TQ, node->u.decl.type, node->coord);

	return node;
}
#endif




/*****************************************************************

Node-kind predicates

*****************************************************************/

GLOBAL inline Bool IsExpr(Node *node)
{ return KindsOfNode(node) & KIND_EXPR; }

GLOBAL inline Bool IsStmt(Node *node)
{ return KindsOfNode(node) & KIND_STMT; }

GLOBAL inline Bool IsType(Node *node)
{ return KindsOfNode(node) & KIND_TYPE; }

GLOBAL inline Bool IsDecl(Node *node)
{ return KindsOfNode(node) & KIND_DECL; }


/*************************************************************************/
/*                                                                       */
/*                          Expression nodes                             */
/*                                                                       */
/*************************************************************************/

PRIVATE inline Kinds KindsOfConst()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfId()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfBinop()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfUnary()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfCast()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfComma()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfTernary()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfArray()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfCall()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfInitializer()
{ return KIND_EXPR | KIND_STMT; }

PRIVATE inline Kinds KindsOfImplicitCast()
{ return KIND_EXPR | KIND_STMT; }

/*************************************************************************/
/*                                                                       */
/*                          Statement nodes                              */
/*                                                                       */
/*************************************************************************/

PRIVATE inline Kinds KindsOfLabel()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfSwitch()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfCase()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfDefault()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfIf()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfIfElse()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfWhile()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfDo()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfFor()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfGoto()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfContinue()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfBreak()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfReturn()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfBlock()
{ return ANSIOnly ? KIND_STMT : KIND_STMT | KIND_EXPR; }


/*************************************************************************/
/*                                                                       */
/*                             Type nodes                                */
/*                                                                       */
/*************************************************************************/

PRIVATE inline Kinds KindsOfPrim()
{ return KIND_TYPE; }

PRIVATE inline Kinds KindsOfTdef()
{ return KIND_TYPE; }

PRIVATE inline Kinds KindsOfPtr()
{ return KIND_TYPE; }

PRIVATE inline Kinds KindsOfAdcl()
{ return KIND_TYPE; }

PRIVATE inline Kinds KindsOfFdcl()
{ return KIND_TYPE; }

PRIVATE inline Kinds KindsOfSdcl()
{ return KIND_TYPE | KIND_DECL; }

PRIVATE inline Kinds KindsOfUdcl()
{ return KIND_TYPE | KIND_DECL; }

PRIVATE inline Kinds KindsOfEdcl()
{ return KIND_TYPE | KIND_DECL; }

/*************************************************************************/
/*                                                                       */
/*                      Other nodes (decls et al.)                       */
/*                                                                       */
/*************************************************************************/

PRIVATE inline Kinds KindsOfDecl()
{ return KIND_DECL; }

PRIVATE inline Kinds KindsOfAttrib()
{ return 0; }

PRIVATE inline Kinds KindsOfProc()
{ return KIND_DECL; }

PRIVATE inline Kinds KindsOfText()
{ return KIND_DECL | KIND_STMT; }


/*************************************************************************/
/*                                                                       */
/*                            Extensions                                 */
/*                                                                       */
/*************************************************************************/

/***********************--------------Define For SPL----------****************************/
GLOBAL inline Node *MakeComposite(Node *decl, Node *body, Bool multi)
{
	Node *create = NewNode(Composite);
	create->u.composite.decl = decl;
	create->u.composite.body = body;
	create->u.composite.multi = multi;

	return(create);
}

GLOBAL inline Node *MakeCompositeCoord(Node *decl, Node *body, Bool multi, Coord coord)
{
	Node *create = MakeComposite(decl, body, multi);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeOperator(Node *decl, Node *body)
{
	Node *create = NewNode(Operator_);
	create->u.operator_.decl = decl;
	create->u.operator_.body = body;
	create->u.operator_.ot = Common_;

	return(create);
}

GLOBAL inline Node *MakeOperatorCoord(Node *decl, Node *body, Coord coord)
{
	Node *create = MakeOperator(decl, body);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeSTRdcl(TypeQual tq, StreamType *type)
{
	Node *create = NewNode(STRdcl);
	create->u.strdcl.tq   = tq;
	create->u.strdcl.type = type;

	return(create);
}

GLOBAL inline Node *MakeSTRdclCoord(TypeQual tq, StreamType *type, Coord coord)
{
	Node *create = MakeSTRdcl(tq, type);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeComdcl(TypeQual tq, Node *inout)
{
	Node *create = NewNode(Comdcl);
	create->u.comdcl.tq = tq;
	create->u.comdcl.inout = inout;

	return(create);
}

GLOBAL inline Node *MakeComdclCoord(TypeQual tq, Node *inout, Coord coord)
{
	Node *create = MakeComdcl(tq, inout);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeComInOut(TypeQual tq, List *inputs, List *outputs)
{
	Node *create = NewNode(ComInOut);
	create->u.comInOut.tq = tq;
	create->u.comInOut.inputs = inputs;
	create->u.comInOut.outputs = outputs;
	return(create);
}

GLOBAL inline Node *MakeComInOutCoord(TypeQual tq, List *inputs, List *outputs, Coord coord)
{
	Node *create = MakeComInOut(tq, inputs, outputs);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeComBody(Node *type, Node *param,List *decl, List *comstmts)
{
	Node *create = NewNode(ComBody);
	create->u.comBody.type = type;
	create->u.comBody.param = param;
	create->u.comBody.decl = decl;
	create->u.comBody.comstmts = comstmts;
	return(create);
}

GLOBAL inline Node *MakeComBodyCoord(Node *type, Node *param,List *decl, List *comstmts, Coord left_coord, Coord right_coord)
{
	Node *create = MakeComBody(type, param, decl, comstmts);
	create->u.comBody.left_coord = left_coord;
	create->u.comBody.right_coord = right_coord;
	create->coord = left_coord;
	return(create);
}

GLOBAL inline Node *MakeOperBody(Node *type, List *state, Node *init, Node *work, List *window)
{
	Node *create = NewNode(OperBody);
	create->u.operBody.type = type;
	create->u.operBody.state = state;
	create->u.operBody.init = init;
	create->u.operBody.work = work;
	create->u.operBody.window = window;

	return(create);
}

GLOBAL inline Node *MakeOperBodyCoord(Node *type, List *state, Node *init, Node *work, List *window, Coord left_coord, Coord right_coord)
{
	Node *create = MakeOperBody(type, state,init,work, window);
	create->u.operBody.left_coord = left_coord;
	create->u.operBody.right_coord = right_coord;
	create->coord = left_coord;

	return(create);
}

GLOBAL inline Node *MakeParam(List *parameters)
{
	Node *create = NewNode(Param);
	create->u.param.parameters = parameters;

	return(create);
}

GLOBAL inline Node *MakeParamCoord(List *parameters, Coord coord)
{
	Node *create = MakeParam(parameters);
	create->coord = coord;

	return(create);
}


GLOBAL inline Node *MakeOperdcl(TypeQual tq, List *outputs, List *inputs, List *arguments)
{
	Node *create = NewNode(Operdcl);
	create->u.operdcl.tq = tq;
	create->u.operdcl.outputs = outputs;
	create->u.operdcl.inputs = inputs;
	create->u.operdcl.arguments = arguments;

	return(create);
}

GLOBAL inline Node *MakeOperdclCoord(TypeQual tq, List *outputs, List *inputs, List *arguments, Coord coord)
{
	Node *create = MakeOperdcl(tq, outputs, inputs, arguments);
	create->coord = coord;

	return(create);
}


GLOBAL inline Node *MakeWindowSliding(TypeQual tq, Node *sliding_value)
{
	Node *create = NewNode(Sliding);
	create->u.sliding.tq = tq;
	create->u.sliding.sliding_value = sliding_value;
	return(create);
}

GLOBAL inline Node *MakeWindowSlidingCoord(TypeQual tq, Node *sliding_value, Coord coord)
{
	Node *create = MakeWindowSliding(tq, sliding_value);
	create->coord = coord;

	return(create);
}



GLOBAL inline Node *MakeWindowSlidingValue( Node *count)
{
	List *exprs = NULL;
	Node *create = NewNode(Comma);
	exprs = AppendItem(exprs,count);
	exprs = AppendItem(exprs,count);
	create->u.comma.exprs = exprs;
	return(create);
}

GLOBAL inline Node *MakeWindowSlidingValueCoord(Node *count, Coord coord)
{
	Node *create = MakeWindowSlidingValue(count);
	create->coord = coord;

	return(create);
}


GLOBAL inline Node *MakeWindowTumbling(TypeQual tq, Node *tumbling_value)
{
	Node *create = NewNode(Tumbling);
	create->u.tumbling.tq = tq;
	create->u.tumbling.tumbling_value = tumbling_value;

	return(create);
}

GLOBAL inline Node *MakeWindowTumbingCoord(TypeQual tq, Node *tumbling_value, Coord coord)
{
	Node *create = MakeWindowTumbling(tq, tumbling_value);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeWindow(Node *id, Node *wtype)
{
	Node *create = NewNode(Window);
	create->u.window.id = id;
	create->u.window.wtype = wtype;

	return(create);
}

GLOBAL inline Node *MakeWindowCoord(Node *id, Node *wtype, Coord coord)
{
	Node *create = MakeWindow(id,wtype);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeCompositeCall(Node *call, Node *operdcl, Bool style)
{
	Node *create = NewNode(CompositeCall);
	create->u.comCall.call = call;
	create->u.comCall.operdcl = operdcl;
	create->u.comCall.style = style;
	create->u.comCall.actual_composite = NULL;//zww

	return(create);
}

GLOBAL inline Node *MakeCompositeCallCoord(Node *call, Node *operdcl, Bool style, Coord coord)
{
	Node *create = NULL;

	if (style == FALSE && call->u.composite.multi == TRUE)
	{
		SyntaxErrorCoord(coord, "Syntax error: can not call multi-input or multi-output Composite: '%s' in default composite call!", call->u.composite.decl->u.decl.name);
		//system("pause"); exit(1);
	}
	create = MakeCompositeCall(call, operdcl, style);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeCompositeId(const char* text)
{
	Node *create = NewNode(Id);
	create->u.id.text = text;
	create->u.id.decl = NULL;

	return(create);
}

GLOBAL inline Node *MakeCompositeIdCoord(const char* text, Coord coord)
{
	Node *create = MakeId(text);
	create->coord = coord;

	return(LookupCompositeIdsNode(create));
}

/************************--------------New For SPL----------************************/
GLOBAL inline Node *MakeSplit(Node *type)
{
	Node *create = NewNode(Split);
	create->u.split.type = type;

	return(create);
}

GLOBAL inline Node *MakeSplitCoord(Node *type, Coord coord)
{
	Node *create = MakeSplit(type);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeJoin(Node *type)
{
	Node *create = NewNode(Join);
	create->u.join.type = type;

	return(create);
}

GLOBAL inline Node *MakeJoinCoord(Node *type, Coord coord)
{
	Node *create = MakeJoin(type);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeRoundRobin(List *arguments)
{
	Node *create = NewNode(RoundRobin);
	create->u.roundrobin.arguments = arguments;

	return(create);
}

GLOBAL inline Node *MakeRoundRobinCoord(List *arguments, Coord coord)
{
	Node *create = MakeRoundRobin(arguments);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeDuplicate(Node *expr)
{
	Node *create = NewNode(Duplicate);
	create->u.duplicate.expr = expr;

	return(create);
}

GLOBAL inline Node *MakeDuplicateCoord(Node *expr, Coord coord)
{
	Node *create = MakeDuplicate(expr);
	create->coord = coord;

	return(create);
}

GLOBAL inline Node *MakeSplitJoin(Node *output, Node *input,List *decl,List *initstmts, Node *split,List *stmts, Node *join)
{
	Node *create = NewNode(SplitJoin);
	create->u.splitJoin.input = input;
	create->u.splitJoin.output = output;
	create->u.splitJoin.split = split;
	create->u.splitJoin.decl = decl;
	create->u.splitJoin.initstmts = initstmts;
	create->u.splitJoin.stmts = stmts;
	create->u.splitJoin.join = join;
	create->u.splitJoin.replace_composite = NULL; // SPL, for unfold.c
	create->u.splitJoin.splitOperator = NULL; // SPL, for unfold.c
	create->u.splitJoin.joinOperator = NULL; // SPL, for unfold.c

	return(create);
}

GLOBAL inline Node *MakeSplitJoinCoord(Node *output, Node *input, List *decl,List *initstmts,Node *split, List *stmts, Node *join, Coord coord)
{
	Node *create = MakeSplitJoin(output, input,decl,initstmts,split,stmts,join);
	create->coord=coord;

	return(create);
}

GLOBAL inline Node *MakePipeline(Node *output, Node *input, List *decl,List *stmts)
{
	Node *create = NewNode(Pipeline);
	create->u.pipeline.input = input;
	create->u.pipeline.output = output;
	create->u.pipeline.decl = decl;
	create->u.pipeline.stmts = stmts;
	create->u.pipeline.replace_composite = NULL; // SPL, for unfold.c

	return(create);
}

GLOBAL inline Node *MakePipelineCoord(Node *output, Node *input, List *decl,List *stmts, Coord coord)
{
	Node *create = MakePipeline(output, input, decl,stmts);
	create->coord=coord;

	return(create);
}

/* 新文法*/



GLOBAL inline Node *MakeAdd(Node *content)
{
	Node *create = NewNode(Add);
	create->u.add.content = content;
	return(create);
}

GLOBAL inline Node *MakeAddCoord(Node *content,Coord coord)
{
	Node *create = MakeAdd(content);
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeIterCount()
{
	Node *create = NewNode(Itco);
	create->u.itco.text = "_iterator_count";
	return(create);
}
GLOBAL inline Node *MakeIterCountCoord(Coord coord)
{
	Node *create = MakeIterCount();
	create->coord = coord;
	return(create);
}

GLOBAL inline Node *MakeOutput(Node *node,Node *output)
{
	if(node->typ == Pipeline){
		node->u.pipeline.output = output;
	}else if(node->typ == SplitJoin){
		node->u.splitJoin.output = output;
	}else if(node->typ == Operator_){
		List *outputs = NULL;
		if(output->typ == Comma)
			outputs = output->u.comma.exprs;
		else
			outputs = JoinLists(outputs,MakeNewList(output));
		node->u.operator_.decl->u.decl.type->u.operdcl.outputs = outputs;
	}else if(node->typ == CompositeCall){
		List *outputs = NULL;
		if(output->typ == Comma)
			outputs = output->u.comma.exprs;
		else
			outputs = JoinLists(outputs,MakeNewList(output));
		node->u.comCall.operdcl->u.operdcl.outputs = outputs;
	}
	return node;
}
/*----------13----SPL node----------*/
PRIVATE inline Kinds KindsOfSTRdcl()
{ return KIND_TYPE | KIND_STMT; }

PRIVATE inline Kinds KindsOfComdcl()
{ return KIND_TYPE; }

PRIVATE inline Kinds KindsOfComposite()
{ return KIND_DECL; }

PRIVATE inline Kinds KindsOfComInOut()
{ return KIND_DECL; }

PRIVATE inline Kinds KindsOfComBody()
{return KIND_STMT;}

PRIVATE inline Kinds KindsOfParam()
{return KIND_STMT;}

PRIVATE inline Kinds KindsOfOperBody()
{return KIND_STMT;}

PRIVATE inline Kinds KindsOfOperdcl()
{return KIND_TYPE;}

PRIVATE inline Kinds KindsOfOperator_()
{return KIND_DECL | KIND_STMT;}


PRIVATE inline Kinds KindsOfWindow()
{return KIND_STMT;}

PRIVATE inline Kinds KindsOfSliding()
{return KIND_STMT;}

PRIVATE inline Kinds KindsOfTumbling()
{return KIND_STMT;}

/*------------7--New For SPL----------*/
PRIVATE inline Kinds KindsOfCompositeCall()
{return KIND_STMT;}

PRIVATE inline Kinds KindsOfPipeline()
{ return KIND_DECL | KIND_STMT; }

PRIVATE inline Kinds KindsOfSplitJoin()
{ return KIND_DECL | KIND_STMT; }

PRIVATE inline Kinds KindsOfSplit()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfJoin()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfRoundRobin()
{ return KIND_STMT; }

PRIVATE inline Kinds KindsOfDuplicate()
{ return KIND_STMT; }

/*****新文法*1**/

PRIVATE inline Kinds KindsOfAdd()
{ return KIND_DECL | KIND_STMT ; }












/***********************--------------Define For SPL----------****************************/





/*************************************************************************/
/*                                                                       */
/*                           KindsOfNode                                 */
/*                                                                       */
/*************************************************************************/

GLOBAL inline Kinds KindsOfNode(Node *node)
{
#define CODE(name, node, union) return KindsOf##name()
	ASTSWITCH(node, CODE);
#undef CODE

	return 0; /* unreachable -- eliminates warning */
}  



/*************************************************************************/
/*                                                                       */
/*                           AST Operations                              */
/*                                                                       */
/*************************************************************************/

GLOBAL Node *NodeCopy(Node *from, TreeOpDepth d)  //zww
{ //zww
	List *tmplist,*templist2;/*define for spl*/
	Node *tmpnode;/*define for spl*/
	revised_container *rc=NULL;//zww
	Node *newNode = NewNode(from->typ);
	*newNode = *from;

	switch (from->typ) {
		/* for nodes with sub-lists, make newNode copy of sub-list */
	case Const: break;       
	case Id:  if (gIsTransform == TRUE && gIsTypelist == FALSE && gIsCall == FALSE)
			  {
				  tmpnode = LookupPARAMSymbol(ParameterPassTable,from->u.id.text,from->u.id.decl);
				  if(tmpnode!=NULL )
				  {/*如果是参数符号表内*/
					  if (tmpnode->typ == Id)/*输入流是个id*/ 
					  {
						  newNode = tmpnode;
					  }
					  else if (tmpnode->typ == Decl)/*输入流是个strdcl的声明*/
					  {
						  newNode->u.id.text = tmpnode->u.decl.name;
						  newNode->u.id.decl = tmpnode;
					  }
					  else
					  {/*如果是param后面的参数*/
						  newNode = tmpnode; 
						  gIsInSymbolTable = TRUE;
					  }
				  }
				  else if(from->u.id.decl != NULL)
				  {
					  newNode->u.id.decl=LookupASTSymbol(ToTransformDecl,from->u.id.text,from->u.id.decl);
				  }
			  }
			  break;          
	case Binop:break;       
	case Unary: break;      
	case Cast: break;       
	case Comma:  newNode->u.comma.exprs = ListCopy(newNode->u.comma.exprs);  break;     
	case Ternary: break;    
	case Array: newNode->u.array.dims = ListCopy(newNode->u.array.dims);  break;      
	case Call:  newNode->u.call.args = ListCopy(newNode->u.call.args);  break;      
	case Initializer: newNode->u.initializer.exprs = ListCopy(newNode->u.initializer.exprs);  break;
	case ImplicitCast:   break;
	case Label: break;      
	case Switch:
		if (gIsTransform == TRUE){
			revised_container *recontainer=HeapNew(revised_container);
			recontainer->new_container=newNode;
			recontainer->old_container=from;
			containerList=AppendItem(containerList,recontainer);
		}			
		else newNode->u.Switch.cases = ListCopy(newNode->u.Switch.cases); break;
	case Case:
		if (gIsTransform == TRUE){
			List *list=containerList;
			while(list)
			{
				rc=(revised_container *)(FirstItem(list));
				if(rc->old_container==from->u.Case.container) break;
				else rc=NULL;
				list = Rest(list);
			}
			if(rc!=NULL)newNode->u.Case.container=rc->new_container;
		}
		else break;
	case Default: 
		if (gIsTransform == TRUE){
			List *list=containerList;
			while(list)
			{
				rc=(revised_container *)(FirstItem(list));
				if(rc->old_container==from->u.Default.container) break;
				else rc=NULL;
				list = Rest(list);
			}
			if(rc!=NULL)newNode->u.Default.container=rc->new_container;
		}
		else break;
	case If: break;         
	case IfElse: break;     
	case While:
		if (gIsTransform == TRUE){
			revised_container *recontainer=HeapNew(revised_container);
			recontainer->new_container=newNode;
			recontainer->old_container=from;
			containerList=AppendItem(containerList,recontainer);
		}
		else break;      
	case Do: 
		if (gIsTransform == TRUE){
			revised_container *recontainer=HeapNew(revised_container);
			recontainer->new_container=newNode;
			recontainer->old_container=from;
			containerList=AppendItem(containerList,recontainer);
		}
		else break;
	case For: 
		if (gIsTransform == TRUE){
			revised_container *recontainer=HeapNew(revised_container);
			recontainer->new_container=newNode;
			recontainer->old_container=from;
			containerList=AppendItem(containerList,recontainer);
		}
		else break;
	case Goto: break;       
	case Continue: 
		if (gIsTransform == TRUE){
			List *list=containerList;
			while(list)
			{
				rc=(revised_container *)(FirstItem(list));
				if(rc->old_container==from->u.Continue.container) break;
				else rc=NULL;
				list = Rest(list);
			}
			if(rc!=NULL)newNode->u.Continue.container=rc->new_container;
		}
		else break;
	case Break:
		if (gIsTransform == TRUE){
			List *list=containerList;
			while(list)
			{
				rc=(revised_container *)(FirstItem(list));
				if(rc->old_container==from->u.Break.container) break;
				else rc=NULL;
				list = Rest(list);
			}
			if(rc!=NULL)newNode->u.Break.container=rc->new_container;
		}
		else break;
	case Return:break;      
	case Block: 
		{
			newNode->u.Block.decl = ListCopy(newNode->u.Block.decl); 
			newNode->u.Block.stmts = ListCopy(newNode->u.Block.stmts);  
			break;

		}
	case Prim: break;       
	case Tdef: break;       
	case Ptr:  break;       
	case Adcl: break;       
	case Fdcl: newNode->u.fdcl.args = ListCopy(newNode->u.fdcl.args);  break;        
	case Sdcl: break;       
	case Udcl: break;       
	case Edcl: break;       
	case Decl:
		if (gIsTransform == TRUE && gIsTypelist == FALSE){
			if (from->u.decl.type->typ != Comdcl && from->u.decl.type->typ != Operdcl  )
			{
				tmpnode = LookupPARAMSymbol(ParameterPassTable,from->u.decl.name,from);
				if(tmpnode != NULL){
					//if(tmpnode->typ == Decl)/*替换输出流（一般情况）*/
					newNode = tmpnode;
					gIsInSymbolTable = TRUE;
				}else
					newNode = ASTInsertSymbol(ToTransformDecl,from->u.decl.name,from->u.decl.name,from,newNode);/*将该声明拷贝信息插入到符号表ToTransformDecl*/
			}
		}
		break;
	case Attrib: break;     
	case Proc: break;       
	case Text: break;       
		/***********************--------------zww:Define For SPL----------****************************/
	case STRdcl: break;     
	case Comdcl: break;      
	case Composite:break;   
	case ComInOut:
		newNode->u.comInOut.outputs = ListCopy(newNode->u.comInOut.outputs); newNode->u.comInOut.inputs = ListCopy(newNode->u.comInOut.inputs);break;   
	case ComBody: newNode->u.comBody.decl = ListCopy(newNode->u.comBody.decl);newNode->u.comBody.comstmts = ListCopy(newNode->u.comBody.comstmts);
		break;    
	case Param:  newNode->u.param.parameters = ListCopy(newNode->u.param.parameters); break;       
	case OperBody: newNode->u.operBody.state= ListCopy(newNode->u.operBody.state);newNode->u.operBody.window= ListCopy(newNode->u.operBody.window); break;   
	case Operdcl: 
		newNode->u.operdcl.outputs = ListCopy(newNode->u.operdcl.outputs);newNode->u.operdcl.inputs = ListCopy(newNode->u.operdcl.inputs); newNode->u.operdcl.arguments = ListCopy(newNode->u.operdcl.arguments);break;   
	case Operator_: break;      
	case Window:  break;       
	case Sliding:  break;  
	case Tumbling: break;   
		/*--------------New For SPL----------*/
	case CompositeCall:	break;
	case Pipeline: 	newNode->u.pipeline.decl=ListCopy(newNode->u.pipeline.decl);newNode->u.pipeline.stmts=ListCopy(newNode->u.pipeline.stmts);break;	
	case SplitJoin:	newNode->u.splitJoin.output;newNode->u.splitJoin.decl=ListCopy(newNode->u.splitJoin.decl);newNode->u.splitJoin.initstmts=ListCopy(newNode->u.splitJoin.initstmts);newNode->u.splitJoin.stmts=ListCopy(newNode->u.splitJoin.stmts);break;
	case Split:	break;	
	case Join: break;			
	case RoundRobin: newNode->u.roundrobin.arguments=ListCopy(newNode->u.roundrobin.arguments);break;	
	case Duplicate:	break;
	default:
		break;

	}

	if (d == Subtree) 
	{
		/* recursively copy children */
		if (gIsInSymbolTable == FALSE)/*表示已经存在于符号表，已经被复制了不需要再进行复制*/
		{
			Node *tmpNode = NULL;
			if(newNode->typ == CompositeCall)
			{
				tmpNode = newNode->u.comCall.call;
				newNode->u.comCall.call = NULL; // 置 NULL
			}
#define CHILD(n)   n = NodeCopy(n, d)
			ASTWALK(newNode, CHILD);
#undef CHILD
			if(newNode->typ == CompositeCall)
			{
				assert(newNode->u.comCall.call == NULL);
				assert(tmpNode && tmpNode->typ == Composite);
				newNode->u.comCall.call = tmpNode;  // 恢复
			}

		}
		else
			gIsInSymbolTable = FALSE;
	}
	return newNode;
}
/****************************zww:myNodeCopy*********************************************
*功能：对节点进行深拷贝，并进行变量的动态匹配，以及输入输出流的替换
*****************************************************************************************/

GLOBAL Node *myNodeCopy(Node *from, TreeOpDepth d)   //zww
{//该from为一个operator节点,将operator中的call所指的composite进行深拷贝
	Node *tmp_oper, *tmp_composite, *copynode, *tmpnode, *tmp_oper_node, *tmp_comp_node;
	List *tmp_oper_list, *tmp_comp_list;

	// assert(gIsTransform == TRUE && from->typ == CompositeCall && from->u.comCall.call == NULL);
	assert(gIsTransform == TRUE && from->typ == CompositeCall && from->u.comCall.call != NULL);

	tmp_oper = from;/*取出待扩展的operator*/
	tmp_composite = from->u.comCall.call;/*取出要复制的composite*/
	//输入流
	tmp_oper_list = tmp_oper->u.comCall.operdcl->u.operdcl.inputs;   //operator的输入流
	tmp_comp_list = tmp_composite->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.inputs;    //composite的输入流
	while(tmp_oper_list!=NULL && tmp_comp_list !=NULL){	//operator和composite的输入流替换
		tmp_comp_node = FirstItem(tmp_comp_list);
		tmp_oper_node = FirstItem(tmp_oper_list);
		//流的标识符可能是个id节点，也可能是一个decl节点
		if (tmp_oper_node->typ == Id && tmp_comp_node->typ == Id)/*最一般的情况*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.id.text,tmp_oper_node->u.id.text,tmp_comp_node,tmp_oper_node);
		}else if (tmp_oper_node->typ == Decl && tmp_comp_node->typ == Id)/*只发生在被替换的operator为第一个operator且输入流非id而是一个具体的strdcl*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.id.text,tmp_oper_node->u.decl.name,tmp_comp_node,tmp_oper_node);
		}else if (tmp_oper_node->typ == Id && tmp_comp_node->typ == Decl)/*只发生在被复制的composite的输入流非id而是一个具体的strdcl*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.decl.name,tmp_oper_node->u.id.text,tmp_comp_node,tmp_oper_node);
		}else if (tmp_oper_node->typ == Decl && tmp_comp_node->typ == Decl)/*上第二和第三情况同时发生*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.decl.name,tmp_oper_node->u.decl.name,tmp_comp_node,tmp_oper_node);
		}
		tmp_comp_list = GetNextList(tmp_comp_list);
		tmp_oper_list = GetNextList(tmp_oper_list);
	}
	//输出流
	tmp_oper_list = tmp_oper->u.comCall.operdcl->u.operdcl.outputs;   //operator的输入流
	tmp_comp_list = tmp_composite->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.outputs;    //composite的输入流
	while(tmp_oper_list!=NULL && tmp_comp_list !=NULL){	//operator和composite的输出流替换
		tmp_comp_node = FirstItem(tmp_comp_list);
		tmp_oper_node = FirstItem(tmp_oper_list);
		//流的标识符可能是个id节点，也可能是一个decl节点
		if (tmp_oper_node->typ == Id && tmp_comp_node->typ == Id)/*最一般的情况*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.id.text,tmp_oper_node->u.id.text,tmp_comp_node,tmp_oper_node);
		}else if (tmp_oper_node->typ == Decl && tmp_comp_node->typ == Id)/*只发生在被替换的operator为第一个operator且输入流非id而是一个具体的strdcl*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.id.text,tmp_oper_node->u.decl.name,tmp_comp_node,tmp_oper_node);
		}else if (tmp_oper_node->typ == Id && tmp_comp_node->typ == Decl)/*只发生在被复制的composite的输入流非id而是一个具体的strdcl*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.decl.name,tmp_oper_node->u.id.text,tmp_comp_node,tmp_oper_node);
		}else if (tmp_oper_node->typ == Decl && tmp_comp_node->typ == Decl)/*上第二和第三情况同时发生*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.decl.name,tmp_oper_node->u.decl.name,tmp_comp_node,tmp_oper_node);
		}
		tmp_comp_list = GetNextList(tmp_comp_list);
		tmp_oper_list = GetNextList(tmp_oper_list);
	}
	
	copynode = NodeCopy(tmp_composite,d);
	
	ResetASTSymbolTable(ParameterPassTable);/*结点复制完成并且参数替换后将符号表清空*/
	return copynode;

}

/*********************************************************************************
*该函数是对composite进行一次深得拷贝，即将原来的composite中的所有结构重建，
*同时用输入输出composite_input，composite_output作为输入输出来代替原composite的输入输出边
*同时修改原composite中operator的work和window中对输入输出边的引用
*from：需要拷贝的composite
*composite_inputList：拷贝后的composite的输入边(里面都是decl类型)
*composite_outputList:拷贝后的composite的输出边(里面都是decl类型)
*zww:201203219
**********************************************************************************/
GLOBAL Node *compositeCopy(Node *composite,List *composite_inputList,List *composite_outputList)
{
	Node  *copynode = NULL, *tmpnode = NULL, *tmp_oper_node = NULL, *tmp_comp_node = NULL;
	List *tmp_oper_list = NULL, *tmp_comp_list =NULL;
	Node *from = NodeCopy(composite,Subtree);
	assert(gIsTransform == TRUE&&from->typ == Composite);
	//输入流
	tmp_oper_list = composite_inputList;   //新生成composite的输入流
	tmp_comp_list = from->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.inputs;    //composite的输入流
	while(tmp_oper_list!=NULL && tmp_comp_list !=NULL){	//operator和composite的输入流替换
		tmp_comp_node = (Node *)FirstItem(tmp_comp_list);
		tmp_oper_node = (Node *)FirstItem(tmp_oper_list);
		assert(tmp_oper_node->typ == Decl);

		if (tmp_oper_node->typ == Decl && tmp_comp_node->typ == Id)/*只发生在被替换的operator为第一个operator且输入流非id而是一个具体的strdcl*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.id.text,tmp_oper_node->u.decl.name,tmp_comp_node,tmp_oper_node);
		}else if (tmp_oper_node->typ == Decl && tmp_comp_node->typ == Decl)
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.decl.name,tmp_oper_node->u.decl.name,tmp_comp_node,tmp_oper_node);
		}
		tmp_comp_list = GetNextList(tmp_comp_list);
		tmp_oper_list = GetNextList(tmp_oper_list);
	}
	//输出流
	tmp_oper_list = composite_outputList;   //operator的输出流
	tmp_comp_list = from->u.composite.decl->u.decl.type->u.comdcl.inout->u.comInOut.outputs;    //composite的输处流
	while(tmp_oper_list!=NULL && tmp_comp_list !=NULL){	//operator和composite的输入流替换
		tmp_comp_node = (Node *)FirstItem(tmp_comp_list);
		tmp_oper_node = (Node *)FirstItem(tmp_oper_list);
		assert(tmp_oper_node->typ == Decl);

		if (tmp_oper_node->typ == Decl && tmp_comp_node->typ == Id)/*只发生在被替换的operator为第一个operator且输入流非id而是一个具体的strdcl*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.id.text,tmp_oper_node->u.decl.name,tmp_comp_node,tmp_oper_node);
		}else if (tmp_oper_node->typ == Decl && tmp_comp_node->typ == Decl)/*上第二和第三情况同时发生*/
		{
			ASTInsertSymbol(ParameterPassTable,tmp_comp_node->u.decl.name,tmp_oper_node->u.decl.name,tmp_comp_node,tmp_oper_node);
		}
		tmp_comp_list = GetNextList(tmp_comp_list);
		tmp_oper_list = GetNextList(tmp_oper_list);
	}
	containerList=NULL;
	copynode = NodeCopy(from,Subtree);
	ResetASTSymbolTable(ParameterPassTable);/*结点复制完成并且参数替换后将符号表清空*/
	return copynode;
}


/****************************zww:TransformOperator**************************************
*功能：对CompositeCall中call指向的composite进行展开，输入输出流的替换
*****************************************************************************************/
GLOBAL Node *TransformOperator(Node *node)
{  //zww
	//Node *tmp_composite;//目标composite
	gIsTransform = TRUE;
	containerList=NULL;
	node->u.comCall.actual_composite = myNodeCopy(node,Subtree);
	ResetASTSymbolTable(ToTransformDecl);
	gIsTransform = FALSE;
	return node;
}



PRIVATE void SetCoordsNode(Node *node, Coord *c)
{
	node->coord = *c;

	/* handle special-case coordinates as well */
	switch (node->typ) {
	case Ternary: node->u.ternary.colon_coord = *c; break;
	case IfElse:  node->u.IfElse.else_coord = *c; break;
	case Do:      node->u.Do.while_coord = *c; break;
	case Block:   node->u.Block.right_coord = *c; break;
	case Sdcl:    
		if (SUE_ELABORATED(node->u.sdcl.tq)) {
			node->u.sdcl.type->coord = *c; 
			node->u.sdcl.type->right_coord = *c;
		}
		break;
	case Udcl:
		if (SUE_ELABORATED(node->u.udcl.tq)) {
			node->u.udcl.type->coord = *c; 
			node->u.udcl.type->right_coord = *c;
		}
		break;
	case Edcl:
		if (SUE_ELABORATED(node->u.edcl.tq)) {
			node->u.edcl.type->coord = *c; 
			node->u.edcl.type->right_coord = *c;
		}
		break;
	default:
		break;
	}
}


/* SetCoords sets all coordinates on a node or subtree to c. */
GLOBAL void SetCoords(Node *node, Coord c, TreeOpDepth d)
{
	if (d == NodeOnly)
		SetCoordsNode(node, &c);
	else WalkTree(node, (WalkProc)SetCoordsNode, &c, Preorder);

	if (d == Subtree) {
#define CHILD(n)   SetCoords(n, c, d)
		ASTWALK(node, CHILD);
#undef CHILD
	}
}


GLOBAL void WalkTree(Node *node, WalkProc proc, void *ptr, WalkOrder order)
{
	if (order == Preorder)
		proc(node, ptr);

#define CHILD(n)   WalkTree(n, proc, ptr, order)
	ASTWALK(node, CHILD);
#undef CHILD

	if (order == Postorder)
		proc(node, ptr);
}

