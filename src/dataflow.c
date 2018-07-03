/*************************************************************************
 *
 *  C-to-C Translator
 *
 *  Rob Miller
 *  
 *  dataflow.c,v
 * Revision 1.4  1995/05/11  18:54:19  rcm
 * Added gcc extension __attribute__.
 *
 * Revision 1.3  1995/04/21  05:44:18  rcm
 * Cleaned up data-flow analysis, and separated into two files, dataflow.c
 * and analyze.c.  Fixed void pointer arithmetic bug (void *p; p+=5).
 * Moved CVS Id after comment header of each file.
 *
 * Revision 1.2  1995/04/09  21:30:43  rcm
 * Added Analysis phase to perform all analysis at one place in pipeline.
 * Also added checking for functions without return values and unreachable
 * code.  Added tests of live-variable analysis.
 *
 * Revision 1.1  1995/03/23  15:31:08  rcm
 * Dataflow analysis; removed IsCompatible; replaced SUN4 compile-time symbol
 * with more specific symbols; minor bug fixes.
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
#pragma ident "dataflow.c,v 1.4 1995/05/11 18:54:19 rcm Exp Copyright 1994 Massachusetts Institute of Technology"

#include "ast.h"
//zww负责数据流部分


/*************************************************************************

  Data-flow frameworks

  Specify your data-flow problem by providing the following.
  Note especially the "Very Important" sections.


    1. Datatype: a set V of values to be propagated, inserted in the
       FlowValue union in dataflow.h.  Some typical datatypes are
       already present: bit vectors, lists, and void pointers (which
       can be used for anything).  V also contains a distinguished
       Undefined element, represented below by U (and in the
       implementation by a FlowValue structure with field "undefined"
       == TRUE).

    2. Direction of propagation: Forwards or Backwards.

    3. Meet operation (^): a function mapping FlowValue x FlowValue to 
       FlowValue, which satisfies the following properties:

       >>>> VERY IMPORTANT <<<<<<
          - commutative:  x^y = y^x
	  - associative:  (x^y)^z = x^(y^z)
	  - idempotent:   (x^y)^y = x^y
	  - has identity U (the distinguished Undefined element):
                 x ^ U = x
       >>>> end very important part <<<<<<

       With these properties, (V,^) is a lattice.  In particular, we can
       define the partial ordering <= on V to be
 
                 x <= y  iff  x ^ y = x

       (Thus the distinguished Undefined element U is the top of the
       lattice, since U >= x for all elements x in V.)

       Finally, the lattice (V,^) must be finite:

          - finite:   all chains x_1 <= x_2 <= ... in V must have finite 
	              length.  (If V is finite, you don't have to worry
		      about this one.)

    4. Equality operation (=): a function mapping FlowValue x FlowValue to
       Bool, which determines whether two elements of V are the same.

    5. Transfer function (trans): a function mapping Node x Point x
       FlowValue to FlowValue, which transforms a value in V as it
       passes through the specified point (Entry or Exit) in the specified
       AST node.

       The transfer function for a particular node/point, trans(N,P), is a
       member of a set of transfer functions F, which map FlowValue to
       FlowValue.  The functions in F must satisfy the following:

          - closed under composition:  f, g in F ==> fg in F
	  - contains identity: 
                there exists i in F s.t. i(x) = x for all x in V

       >>>> VERY IMPORTANT <<<<<<
	  - monotonic:
	        f(x ^ y) <= f(x) ^ f(y)  for all f in F and x,y in V

            an equivalent statement of monotonicity which may be easier
            for you to establish:

	        x <= y  ==>  f(x) <= f(y)  for all f in F and x,y in V

            Without monotonicity, your framework will not converge in all
	    cases.
       >>>> end very important part <<<<<<

       The functions in F may optionally satisfy:

          - strict:  f(U) = U   (where U is distinguished Undefined element)
            If F is strict, unreachable nodes will remain U throughout
	    the data-flow algorithm, so they will never contribute flow
	    values to the rest of the program.  This is normally desirable.
	    Causing f(U) = U is as simple as checking the undefined bit
	    in your input FlowValue and returning the same value immediately
	    if the bit is set.


       The trans subroutine also receives a boolean flag, FinalPass,
       that indicates whether this is the final visit to the
       node/point in the data-flow algorithm.  When FinalPass is true_,
       the trans subroutine can consider its FlowValue input to be 
       a final, converged value, and can use it to mark up the tree
       with permanent analysis results.
       

    6. Initial element I: an element in V which should be passed into
       to the topmost node of the AST.



  Except for the datatype, each of these parts of the framework must be
  passed to the dataflow control routine, IterateDataFlow.


 *************************************************************************/




/*************************************************************************/
/*                                                                       */
/*                      Generic data-flow framework                      */
/*                                                                       */
/*************************************************************************/

PRIVATE Bool Forw;
PRIVATE MeetOp Meet;
PRIVATE EqualOp Equal;
PRIVATE TransOp Trans;

PRIVATE Bool Changed;
PRIVATE Bool Final;
PRIVATE Bool break_flag = FALSE;

#define Entry(n, v)  Trans(n, v, EntryPoint, Final)
#define Exit(n, v)   Trans(n, v, ExitPoint, Final)


PRIVATE FlowValue DataFlow(Node *node, FlowValue in);
PRIVATE FlowValue DataFlowSerialList(List *list, FlowValue in);
PRIVATE inline FlowValue DataFlowBranch(Node *cond, Node *true_, Node *false_, FlowValue in);
PRIVATE FlowValue FlowInto(FlowValue *dest, FlowValue src);
PRIVATE void ConstFlowInto(FlowValue *dest, FlowValue src);

// zww
PRIVATE inline Bool CompareConstValue(ConstNode n1, ConstNode n2)
{
	if(n1.type->u.prim.basic != n2.type->u.prim.basic) return TRUE;
	
	switch (n1.type->u.prim.basic)
	{
		case Sint:
			if(n1.value.i != n2.value.i) return TRUE;
			else return FALSE;
		case Uint:
			if(n1.value.u != n2.value.u) return TRUE;
			else return FALSE;

		case Slong:
			if(n1.value.l != n2.value.l) return TRUE;
			else return FALSE;
		case Ulong:
			if(n1.value.ul != n2.value.ul) return TRUE;
			else return FALSE;

		case Float:
			if(n1.value.f != n2.value.f) return TRUE;
			else return FALSE;
		case Double:
			if(n1.value.d != n2.value.d) return TRUE;
			else return FALSE;

		case Char:
		case Schar:
		case Uchar:
			if(strcmp(n1.value.s,n1.value.s) != 0) return TRUE;
			else return FALSE;

		default:
			return FALSE;
	}

}

/*************************************************************************/
/*                                                                       */
/*                     Data flow for each node type                      */
/*                                                                       */
/*************************************************************************/

PRIVATE inline FlowValue DataFlowConst(Node *node, ConstNode *u, FlowValue v)
{
  if (Forw)
    return Exit(node, Entry(node, v));
  else 
    return Entry(node, Exit(node, v));
}

PRIVATE inline FlowValue DataFlowId(Node *node, idNode *u, FlowValue v)
{
  if (Forw)	//zww
	  return Exit(node,
		DataFlow(u->value,
			Entry(node, v)));
  else 
    return Entry(node, Exit(node, v));
}

PRIVATE inline FlowValue DataFlowBinop(Node *node, binopNode *u, FlowValue v)
{
  if (u->op == ANDAND || u->op == OROR) {
    /* short-circuiting Boolean operators act like a branch:
       always evaluates left operand, sometimes evaluates right operand */
    if (Forw)
      return Exit(node, 
		  DataFlowBranch(u->left, u->right, NULL, 
				 Entry(node, v)));
    else
      return Entry(node, 
		   DataFlowBranch(u->left, u->right, NULL, 
				  Exit(node, v)));

  }
  else {
    /* all other operators always evaluate both operands */ 
    if (Forw)
      return Exit(node, 
		  DataFlow(u->right, 
			   DataFlow(u->left, 
				    Entry(node, v))));
    else
      return Entry(node, 
		   DataFlow(u->left, 
			    DataFlow(u->right, 
				     Exit(node, v))));
  }

}

PRIVATE inline FlowValue DataFlowUnary(Node *node, unaryNode *u, FlowValue v)
{
  if (u->op == SIZEOF) {
    /* expression in sizeof is never executed */
    if (Forw)
      return Exit(node, Entry(node, v));
    else 
      return Entry(node, Exit(node, v));
  }
  else {
    if (Forw)
      return Exit(node, DataFlow(u->expr, Entry(node, v)));
    else 
      return Entry(node, DataFlow(u->expr, Exit(node, v)));
  }
}

PRIVATE inline FlowValue DataFlowCast(Node *node, castNode *u, FlowValue v)
{
  if (Forw)
    return Exit(node, DataFlow(u->expr, Entry(node, v)));
  else 
    return Entry(node, DataFlow(u->expr, Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowComma(Node *node, commaNode *u, FlowValue v)
{
  if (Forw)
    return Exit(node, DataFlowSerialList(u->exprs, Entry(node, v)));
  else 
    return Entry(node, DataFlowSerialList(u->exprs, Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowTernary(Node *node, ternaryNode *u, FlowValue v)
{
  if (Forw)
  {// LXX: modify, 12.05
	  FlowValue src = DataFlow(u->cond, Entry(node, v));

	  if(NodeConstantBooleanValue(GetValue(u->cond)) == TRUE)
	  {
		  src = DataFlow(u->true_, src);
		  NodeSetConstantValue(node, GetValue(u->true_));
	  }
	  else
	  {
		  src = DataFlow(u->false_, src);
		  NodeSetConstantValue(node, GetValue(u->false_));
	  }

	  return Exit(node, src);
  }
  else 
    return Entry(node, 
		 DataFlowBranch(u->cond, u->true_, u->false_, 
				Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowArray(Node *node, arrayNode *u, FlowValue v)
{
  if (Forw)
    return Exit(node, 
		DataFlowSerialList(u->dims, 
				   DataFlow(u->name, 
					    Entry(node, v))));
  else
    return Entry(node, 
		 DataFlow(u->name, 
			  DataFlowSerialList(u->dims, 
					     Exit(node, v))));
}

PRIVATE inline FlowValue DataFlowCall(Node *node, callNode *u, FlowValue v)
{
  if (Forw)
    return Exit(node, 
		DataFlowSerialList(u->args, 
				   DataFlow(u->name, 
					    Entry(node, v))));
  else
    return Entry(node, 
		 DataFlow(u->name, 
			  DataFlowSerialList(u->args, 
					     Exit(node, v))));
}

PRIVATE inline FlowValue DataFlowInitializer(Node *node, initializerNode *u, FlowValue v)
{
  if (Forw)
    return Exit(node, DataFlowSerialList(u->exprs, Entry(node, v)));
  else
    return Entry(node, DataFlowSerialList(u->exprs, Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowImplicitCast(Node *node, implicitcastNode *u, FlowValue v)
{
  if (Forw)
    return Exit(node, DataFlow(u->expr, Entry(node, v)));
  else
    return Entry(node, DataFlow(u->expr, Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowLabel(Node *node, labelNode *u, FlowValue v)
{
  if (Forw)
    return Exit(node, 
		DataFlow(u->stmt, 
			 Entry(node, 
			       FlowInto(&u->label_values, v))));
  else
    return FlowInto(&u->label_values, 
		    Entry(node, 
			  DataFlow(u->stmt, 
				   Exit(node, v))));
}
  

PRIVATE inline FlowValue DataFlowSwitch(Node *node, SwitchNode *u, FlowValue v)
{
  if (Forw) {
    u->break_values.undefined=TRUE;
	u->switch_values.undefined=TRUE;
    FlowInto(&u->switch_values, DataFlow(u->expr, Entry(node, v)));
	u->switch_values.undefined=FALSE;
    /* if expression doesn't match any case label, control may short-circuit
       directly from expression to the end of the switch.  This can only
       happen when switch has no default case: */
    //if (!u->has_default)
     // FlowInto(&u->break_values, u->switch_values);

    /* expression does not flow directly to statement -- it's a jump to
     a case label */
    v.undefined = TRUE;  /* pass in "Undefined" to statement */
	//u->break_values.undefined=FALSE;
	DataFlow(u->stmt,v);
	if (!u->has_default&&u->break_values.undefined==TRUE&&u->switch_values.undefined==FALSE)
	{
		FlowInto(&u->break_values, u->switch_values);
	}
	u->break_values.undefined=FALSE;
   // FlowInto(&u->break_values, DataFlow(u->stmt, v));
    return Exit(node, u->break_values);
  }
  else {
    DataFlow(u->stmt, FlowInto(&u->break_values, Exit(node, v)));
    if (!u->has_default)
      FlowInto(&u->switch_values, u->break_values);
    return Entry(node, DataFlow(u->expr, u->switch_values));
  }
}

PRIVATE inline FlowValue DataFlowCase(Node *node, CaseNode *u, FlowValue v)
{
  FlowValue e,v1,v2;//zhangweiwei
  Node *value=NULL,*casevalue=NULL;
  Bool flag=FALSE;
  assert(u->container->typ == Switch);
  //u->container->u.Switch.break_values.undefined=TRUE;
  if (Forw)
  { 
	  value=GetValue(u->container->u.Switch.expr);
	  casevalue=GetValue(node->u.Case.expr);
	  if(value!=NULL&&(!CompareConstValue(value->u.Const,casevalue->u.Const))&& v.undefined==TRUE)
	  {//值相等
		  //v1=Meet(v,u->container->u.Switch.switch_values);
		  u->container->u.Switch.break_values.undefined=TRUE;
		  e.undefined=TRUE;
		  v.undefined=FALSE;
		  FlowInto(&e,Entry(node,v));
		  v.undefined=TRUE;
		  e.undefined=FALSE;
		  v2=DataFlow(u->stmt,e);
		  if(u->container->u.Switch.break_values.undefined==FALSE) 
		  {	  break_flag=FALSE;
			  u->container->u.Switch.switch_values.undefined=TRUE;
			  FlowInto(&u->container->u.Switch.switch_values,u->container->u.Switch.break_values);
			  u->container->u.Switch.switch_values.undefined=FALSE;
			  return Exit(node,u->container->u.Switch.break_values);
		  }
		  else 
		  {	  break_flag=TRUE;
			  u->container->u.Switch.switch_values.undefined=TRUE;
			  FlowInto(&u->container->u.Switch.switch_values,v2);
			  u->container->u.Switch.switch_values.undefined=FALSE;
			  return Exit(node,u->container->u.Switch.switch_values);

			  //return v2;
		  }

	  }
	  else if(value!=NULL&&(!CompareConstValue(value->u.Const,casevalue->u.Const))&& v.undefined==FALSE )
	  {//值相等
		  //v1=Meet(v,u->container->u.Switch.switch_values);
		  u->container->u.Switch.break_values.undefined=TRUE;
		  e.undefined=TRUE;
		  FlowInto(&e,Entry(node,u->container->u.Switch.switch_values));
		  e.undefined=FALSE;
		  v2=DataFlow(u->stmt,e);
		  if(u->container->u.Switch.break_values.undefined==FALSE) 
		  {	  break_flag=FALSE;
			  u->container->u.Switch.switch_values.undefined=TRUE;
			  FlowInto(&u->container->u.Switch.switch_values,u->container->u.Switch.break_values);
			  u->container->u.Switch.switch_values.undefined=FALSE;
			  return Exit(node,u->container->u.Switch.break_values);
		  }
		  else 
		  {	  break_flag=TRUE;
			  u->container->u.Switch.switch_values.undefined=TRUE;
			  FlowInto(&u->container->u.Switch.switch_values,v2);
			  u->container->u.Switch.switch_values.undefined=FALSE;
			  return Exit(node,u->container->u.Switch.switch_values);

			  //return v2;
		  }

	  }
	  else if(value!=NULL&&(CompareConstValue(value->u.Const,casevalue->u.Const))&& v.undefined==FALSE && break_flag==TRUE)
	  {//值不相等，上一个case后没有break
		  e.undefined=TRUE;
		  FlowInto(&e,Entry(node,u->container->u.Switch.switch_values));
		  e.undefined=FALSE;
		  v2=DataFlow(u->stmt,e);
		  if(u->container->u.Switch.break_values.undefined==FALSE) 
		  {	  break_flag=FALSE;
			  u->container->u.Switch.switch_values.undefined=TRUE;
			  FlowInto(&u->container->u.Switch.switch_values,u->container->u.Switch.break_values);
			  u->container->u.Switch.switch_values.undefined=FALSE;
			  return Exit(node,u->container->u.Switch.break_values);
		  }
		  else 
		  {	  break_flag=TRUE;
			  u->container->u.Switch.switch_values.undefined=TRUE;
			  FlowInto(&u->container->u.Switch.switch_values,v2);
			  u->container->u.Switch.switch_values.undefined=FALSE;
			  return Exit(node,u->container->u.Switch.switch_values);

			  //return v2;
		  }
	  }
	  else if(value!=NULL&&(CompareConstValue(value->u.Const,casevalue->u.Const))) 
	  {	//值不相等,但上一个case后有break
		  break_flag=FALSE;
		  e.undefined=TRUE;
		  v.undefined=FALSE;
		  FlowInto(&e,Entry(node,v));
		  v.undefined=TRUE;
		  e.undefined=FALSE;
		  //v2=DataFlow(u->stmt,e);
		  v.undefined=FALSE;
		  return Exit(node,v);
	  }
	  else {
		  u->container->u.Switch.break_values.undefined=TRUE;
		  return v;
	  }
  }
  else {
    v = Entry(node, 
	      DataFlow(u->stmt, 
		       Exit(node, v)));
    FlowInto(&u->container->u.Switch.switch_values, v);
    return v;
  }
}

PRIVATE inline FlowValue DataFlowDefault(Node *node, DefaultNode *u, FlowValue v)
{
  assert(u->container->typ == Switch);
  if (Forw)
  {	 //zww
	  if(v.undefined==TRUE||u->container->u.Switch.break_values.undefined==TRUE)
	  {
		  DataFlow(u->stmt,  Entry(node,u->container->u.Switch.switch_values));
		  FlowInto(&u->container->u.Switch.break_values,u->container->u.Switch.switch_values);
		  u->container->u.Switch.break_values.undefined=FALSE;
		  return Exit(node,u->container->u.Switch.break_values);
		  
	  }
	  else 
	  {	  
		  DataFlow(u->stmt,Entry(node, v));
		  if(u->container->u.Switch.break_values.undefined==FALSE)return Exit(node,u->container->u.Switch.break_values);
		  else
		  {
			  v.undefined=FALSE;
			  FlowInto(&u->container->u.Switch.break_values,v);
			  u->container->u.Switch.break_values.undefined=FALSE;
			  return Exit(node,u->container->u.Switch.break_values);

		  }

	  }

  }
   
  else {
    v = Entry(node, 
	      DataFlow(u->stmt, 
		       Exit(node, v)));
    FlowInto(&u->container->u.Switch.switch_values, v);
    return v;
  }
}

PRIVATE inline FlowValue DataFlowIf(Node *node, IfNode *u, FlowValue v)
{

	Node *value;
	if (Forw)
	{
		FlowValue tmp = DataFlow(u->expr, Entry(node, v));

		if(NodeConstantBooleanValue(GetValue(u->expr))) 
			return DataFlow(u->stmt, tmp);
		else
			return tmp;
	}
  else
    return Entry(node, 
		 DataFlowBranch(u->expr, u->stmt, NULL, 
				Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowIfElse(Node *node, IfElseNode *u, FlowValue v)
{
	if (Forw)
	{
		FlowValue src = DataFlow(u->expr, Entry(node, v));
		Node *value = GetValue(u->expr);

		assert(value);

		if(NodeConstantBooleanValue(value) == TRUE) 
			return DataFlow(u->true_, src);
		else
			return DataFlow(u->false_, src);
	}
  else
    return Entry(node, 
		 DataFlowBranch(u->expr, u->true_, u->false_, 
				Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowWhile(Node *node, WhileNode *u, FlowValue v)
{
	FlowValue e, s,v1;//zww
	Node *value;

	if (Forw) {
		//FlowInto(&u->loop_values, Entry(node, v));
		u->break_values.undefined=TRUE;
		u->loop_values.undefined=TRUE; 
		e.undefined=FALSE;
		e=Entry(node, v);
		e = DataFlow(u->expr, e);		
		//if(node->u.While.expr->u.binop.value!=NULL)printf("%d\n",node->u.While.expr->u.binop.value->u.Const.value.i);
		FlowInto(&u->break_values, e);
		node->u.While.break_values.undefined=FALSE;
		value=GetValue(node->u.While.expr);
		while (value!=NULL&&NodeConstantBooleanValue(value)==TRUE)
		{
			node->u.While.break_values.undefined=TRUE;
			node->u.While.loop_values.undefined=TRUE;
			DataFlow(u->stmt, e);
			if(node->u.While.break_values.undefined==FALSE) break;
			if(node->u.While.loop_values.undefined==FALSE && e.undefined==TRUE) 
			{
				//e.undefined=TRUE;
				FlowInto(&e,u->loop_values);
				e.undefined=FALSE;
				node->u.While.loop_values.undefined=TRUE;
			}
			e=DataFlow(u->expr,e);
			value=GetValue(node->u.While.expr);
			//printf("%s=%d\n",u->expr->u.binop.left->u.id.text,u->expr->u.binop.left->u.id.value->u.Const.value.i);
			node->u.While.break_values.undefined=TRUE;
			FlowInto(&u->break_values, e);
			node->u.While.break_values.undefined=FALSE;
		}
		node->u.While.break_values.undefined=FALSE;
		return Exit(node, u->break_values);
	}
  else {
    FlowInto(&u->break_values, Exit(node, v));
    s = DataFlow(u->stmt, u->loop_values);
    FlowInto(&u->loop_values, DataFlow(u->expr, Meet(s, u->break_values)));
    return Entry(node, u->loop_values);
  }
}

PRIVATE inline FlowValue DataFlowDo(Node *node, DoNode *u, FlowValue v)
{
	FlowValue e,v1,v2;   //zww
	Node *value=NULL;

	if (Forw) {
		u->break_values.undefined=TRUE;
		u->continue_values.undefined=TRUE;
		u->loop_values.undefined=TRUE;
		e=Entry(node,v);
		FlowInto(&u->loop_values,e);
		FlowInto(&u->break_values,e);
		u->break_values.undefined=FALSE;
		do 
		{
			node->u.Do.break_values.undefined=TRUE;
			node->u.Do.continue_values.undefined=TRUE;
			node->u.Do.loop_values.undefined=TRUE;
			FlowInto(&node->u.Do.loop_values,DataFlow(u->stmt, e));
			node->u.Do.loop_values.undefined=FALSE;
			if(node->u.Do.break_values.undefined==FALSE) break;
			if(node->u.Do.continue_values.undefined==FALSE && e.undefined==TRUE) 
			{
				node->u.Do.loop_values.undefined=TRUE;
				FlowInto(&node->u.Do.loop_values,u->continue_values);
				node->u.Do.loop_values.undefined=FALSE;
				node->u.Do.continue_values.undefined=TRUE;
			}
			e=DataFlow(u->expr,u->loop_values);
			node->u.Do.break_values.undefined=TRUE;
			FlowInto(&u->break_values, e);
			node->u.Do.break_values.undefined=FALSE;
			
			value=GetValue(node->u.Do.expr);
		} while (value!=NULL&&NodeConstantBooleanValue(value)==TRUE);
		u->break_values.undefined=FALSE;
		return Exit(node, u->break_values);
	}
  else {
    FlowInto(&u->break_values, Exit(node, v));
    FlowInto(&u->continue_values,
	     DataFlow(u->expr, Meet(u->break_values, u->loop_values)));
    FlowInto(&u->loop_values,
	     DataFlow(u->stmt, u->continue_values));
    return Entry(node, u->loop_values);
  }
}

PRIVATE inline FlowValue DataFlowFor(Node *node, ForNode *u, FlowValue v)
{
	FlowValue e;
	if (Forw)
	{
		FlowValue src, tmp;//zww

		//目前只针对循环条件表达式不为空的情况
		assert (u->cond != NULL);

		src = DataFlow(u->init, Entry(node, v));
		ConstFlowInto(&u->loop_values, src);

		src = DataFlow(u->cond, src);
		ConstFlowInto(&u->break_values, src);
		ConstFlowInto(&u->continue_values, src);

		while (NodeConstantBooleanValue(GetValue(u->cond)) == TRUE)
		{
			u->break_values.undefined = TRUE;
			u->continue_values.undefined = TRUE;
			//for的语句部分在此模拟执行了
			ConstFlowInto(&u->loop_values, DataFlow(u->stmt, src));
			//for里面的break语句命中
			if(u->break_values.undefined == FALSE) break;
			//for里面的continue语句命中
 			if(u->continue_values.undefined == FALSE && src.undefined == FALSE) 
 			{
 				ConstFlowInto(&u->loop_values, u->continue_values);
 				u->continue_values.undefined = TRUE;
 			}

			ConstFlowInto(&u->break_values, u->loop_values);
			//for的next表达式部分在此模拟执行了
			tmp = DataFlow(u->next, u->break_values);
			src = DataFlow(u->cond, tmp);
		}

		u->break_values.undefined = FALSE;
		return  Exit(node, u->break_values);
	}
	else 
	{
		FlowInto(&u->break_values, Exit(node, v));
		FlowInto(&u->continue_values, DataFlow(u->next, u->loop_values));
		e = DataFlow(u->stmt, u->continue_values);

		if (u->cond)
			FlowInto(&u->loop_values, DataFlow(u->cond, Meet(u->break_values, e)));
		else
			FlowInto(&u->loop_values, e);

		return Entry(node, DataFlow(u->init, u->loop_values));
	}
}

PRIVATE inline FlowValue DataFlowGoto(Node *node, GotoNode *u, FlowValue v)
{
  if (Forw) {
    FlowInto(&u->label->u.label.label_values, Exit(node, Entry(node, v)));

    /* return "undefined" value, since control never continues past a goto */
    v.undefined = TRUE;
    return v;
  }
  else 
    return Entry(node, Exit(node, u->label->u.label.label_values));
}

PRIVATE inline FlowValue DataFlowContinue(Node *node, ContinueNode *u, FlowValue v)
{
  FlowValue *pv;

  switch (u->container->typ) {
  case Do:    pv = &u->container->u.Do.continue_values; break;
  case While: pv = &u->container->u.While.loop_values; break;
  case For:   pv = &u->container->u.For.continue_values; break;
  default:    UNREACHABLE; pv = NULL; break;
  }

  if (Forw) {
    FlowInto(pv, Exit(node, Entry(node, v)));
    /* return "undefined" value */
    v.undefined = TRUE;
    return v;
  }
  else 
    return Entry(node, Exit(node, *pv));
}

PRIVATE inline FlowValue DataFlowBreak(Node *node, BreakNode *u, FlowValue v)
{
  FlowValue *pv;

  switch (u->container->typ) {
  case Do:    pv = &u->container->u.Do.break_values; break;
  case While: pv = &u->container->u.While.break_values; break;
  case For:   pv = &u->container->u.For.break_values; break;
  case Switch: pv = &u->container->u.Switch.break_values; break;
  default:    UNREACHABLE; pv = NULL; break;
  }

  if (Forw) {
    FlowInto(pv, Exit(node, Entry(node, v)));
    /* return "undefined" value */
	v.undefined=TRUE;
    return v;
  }
  else 
    return Entry(node, Exit(node, *pv));
}

PRIVATE inline FlowValue DataFlowReturn(Node *node, ReturnNode *u, FlowValue v)
{
	if (Forw) {	//zww

		FlowInto(&u->proc->u.proc.return_values, 
			Exit(node, DataFlow(u->expr, Entry(node, v))));
		u->proc->u.proc.decl->u.decl.type->u.fdcl.returns->u.Return.expr=GetValue(node->u.Return.expr); //zhangweiwei
		/* return "undefined" value */
		v.undefined = TRUE;
		return v;
	}
  else 
    return Entry(node, 
		 DataFlow(u->expr, 
			  Exit(node, u->proc->u.proc.return_values)));
}

PRIVATE inline FlowValue DataFlowBlock(Node *node, BlockNode *u, FlowValue v)
{
  if (Forw)
    return Exit(node, 
		DataFlowSerialList(u->stmts, 
				   DataFlowSerialList(u->decl, 
						      Entry(node, v))));
  else
    return Entry(node, 
		 DataFlowSerialList(u->decl, 
				    DataFlowSerialList(u->stmts, 
						       Exit(node, v))));
}

PRIVATE inline FlowValue DataFlowPrim(Node *node, primNode *u, FlowValue v)
{
  return v;
}

PRIVATE inline FlowValue DataFlowTdef(Node *node, tdefNode *u, FlowValue v)
{
  return v;
}

PRIVATE inline FlowValue DataFlowPtr(Node *node, ptrNode *u, FlowValue v)
{
  return v;
}

PRIVATE inline FlowValue DataFlowAdcl(Node *node, adclNode *u, FlowValue v)
{
	if (Forw)	//zww	 12.2.17
		return Exit(node,
			DataFlow(u->type,
				DataFlow(u->dim,
					Entry(node, v))));
	else 
		return v;
}

PRIVATE inline FlowValue DataFlowFdcl(Node *node, fdclNode *u, FlowValue v)
{
  return v;
}

PRIVATE inline FlowValue DataFlowSdcl(Node *node, sdclNode *u, FlowValue v)
{
  return v;
}

PRIVATE inline FlowValue DataFlowUdcl(Node *node, udclNode *u, FlowValue v)
{
  return v;
}

PRIVATE inline FlowValue DataFlowEdcl(Node *node, edclNode *u, FlowValue v)
{
  return v;
}

PRIVATE inline FlowValue DataFlowDecl(Node *node, declNode *u, FlowValue v)
{
	if (Forw){
		if(node->u.decl.type->typ != Comdcl&&node->u.decl.type->typ != STRdcl){
			Node *item,*decl,*ptr;
			if(node->u.decl.type->typ == Adcl){
				ptr = MakePtrCoord(EMPTY_TQ,node->u.decl.type->u.adcl.type,UnknownCoord);
				decl = MakeDeclCoord(node->u.decl.name,node->u.decl.tq,ptr,NULL,node->u.decl.bitsize,UnknownCoord);
			}
			else{
				decl = MakeDeclCoord(node->u.decl.name,node->u.decl.tq,node->u.decl.type,NULL,node->u.decl.bitsize,UnknownCoord);
			}
			item = MakeIdCoord(node->u.decl.name,UnknownCoord);
			item->u.id.decl = node;
			gCurrentParamList[gMultiSPFlag] = AppendItem(gCurrentParamList[gMultiSPFlag],item);		
			gCurrentDeclList[gMultiSPFlag] = AppendItem(gCurrentDeclList[gMultiSPFlag],decl);
		}
		return Exit(node, 
			DataFlow(u->type,
				DataFlow(u->init, Entry(node, v))));
	}
	else
		return Entry(node, DataFlow(u->init, Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowAttrib(Node *node, attribNode *u, FlowValue v)
{
  return v;
}

PRIVATE inline FlowValue DataFlowProc(Node *node, procNode *u, FlowValue v)
{
	if (Forw)//zww
	{	
		//return Exit(node, 
		//FlowInto(&u->return_values, 
		//DataFlow(u->body, 
		//   Entry(node, v))));
		u->init_values.undefined=TRUE;
		u->return_values.undefined=TRUE;
		FlowInto(&u->init_values,Entry(node,v));
		u->init_values.undefined=TRUE;
		FlowInto(&u->return_values,u->init_values);
		u->return_values.undefined=TRUE;
		DataFlow(u->body,
			DataFlowSerialList(node->u.proc.decl->u.decl.type->u.fdcl.args,u->return_values));
		return Exit(node,v);
	}
  else
    return Entry(node, 
		 DataFlow(u->body, 
			  FlowInto(&u->return_values, 
				   Exit(node, v))));
}

PRIVATE inline FlowValue DataFlowText(Node *node, textNode *u, FlowValue v)
{
  return v;
}


/*************************************************************************/
/*                                                                       */
/*                            Extensions                                 */
/*                                                                       */
/*************************************************************************/
/***********************--------------Define For SPL----------****************************/
PRIVATE inline FlowValue DataFlowSTRdcl(Node *node, strdclNode *u, FlowValue v)
{ 
	return v;
}

PRIVATE inline FlowValue DataFlowComdcl(Node *node, comDeclNode *u, FlowValue v)
{
	return v;
}

PRIVATE inline FlowValue DataFlowComposite(Node *node, compositeNode *u, FlowValue v)
{
	if(Forw)
	{
		gMultiSPFlag++;
#ifdef SPL_DEBUG
		printf("DataFlowComposite: %s ENTER!\n", u->decl->u.decl.name);
#endif
		u->composite_values.undefined = TRUE;
		// 数据流进入新节点
		FlowInto(&u->composite_values, v); 
		u->composite_values.undefined = FALSE;
		DataFlow(u->body, u->composite_values);

#ifdef SPL_DEBUG
		printf("DataFlowComposite: %s PASS!\n", u->decl->u.decl.name);
		if (strcmp(u->decl->u.decl.name, "Main") == 0)
			PrintConstFlowValue(u->composite_values);
#endif
		gCurrentParamList[gMultiSPFlag] = NULL;
		gCurrentDeclList[gMultiSPFlag] = NULL;
		gMultiSPFlag--;
		return Exit(node,v);
	}
	else
		return Entry(node, 
			FlowInto(&u->composite_values,
				DataFlow(u->body,  
					Exit(node, v))));
}

PRIVATE inline FlowValue DataFlowComInOut(Node *node, comInOutNode *u, FlowValue v)
{
	return v;
}

PRIVATE inline FlowValue DataFlowComBody(Node *node, comBodyNode *u, FlowValue v)
{

	if(Forw)
		return Exit(node,
			DataFlowSerialList(u->comstmts,
				DataFlowSerialList(u->decl,
					DataFlow(u->param,
						Entry(node,v)))));
	else
		return Exit(node,
			DataFlow(u->param,
				DataFlowSerialList(u->decl,
					DataFlowSerialList(u->comstmts,
						Entry(node,v)))));
}

PRIVATE inline FlowValue DataFlowParam(Node *node, paramNode *u, FlowValue v)
{
	if(Forw)
	{
		FlowValue src;

		//PrintList(stdout, u->parameters, -1);

		src = DataFlowSerialList(u->parameters, v); // 该语句的主要作用就是将形参加入到常量表中, 不能删，ModifyParam可能要修改, 12.06

		src = Exit(node, src);

		//PrintConstFlowValue(src);
		return src;
	}
	else
		return Entry(node,
			DataFlowSerialList(u->parameters,
				Exit(node,v)));
}

//cwb数据流进入自定义的Operator
operatorNode *tempoperatornode;
PRIVATE inline FlowValue DataFlowOperator_(Node *node, operatorNode *u, FlowValue v)
{
	tempoperatornode = u;
	tempoperatornode->params = NULL;
	if (Forw)
	{
		u->oper_values.undefined = TRUE;
		FlowInto(&u->oper_values, Entry(node,v));
		u->oper_values.undefined = FALSE;
		DataFlow(u->body, u->oper_values);
		return Exit(node,v);
	}
	else
		return Entry(node,
			FlowInto(&u->oper_values, 
				DataFlow(u->body, 
					Exit(node, v))));
}

PRIVATE inline FlowValue DataFlowOperdcl(Node *node, operDeclNode *u, FlowValue v)
{ 
	if(Forw)
		return Exit(node, DataFlowSerialList(u->arguments, v));
	else
		return Entry(node,
			DataFlowSerialList(u->arguments,Exit(node,v)));
}

PRIVATE inline FlowValue DataFlowOperBody(Node *node, operBodyNode *u, FlowValue v)
{
	if(Forw){
		FlowValue tmp,tmp1;
		tmp= Entry(node,v);
		if(u->state) RWV_listwalk(u->state,tmp);
		if(u->init)ReplaceWorkVar(u->init,tmp);
		ReplaceWorkVar(u->work,tmp);//将work中所有的变量的值取出
		return Exit(node,
				DataFlowSerialList(u->window,tmp));
	}
	else{
		return Entry(node,
			DataFlowSerialList(u->state,
				DataFlow(u->init,
					DataFlow(u->work,
						DataFlowSerialList(u->window,
							Exit(node,v))))));

	}
}


PRIVATE inline FlowValue DataFlowWindow(Node *node, windowNode *u, FlowValue v)
{
	if(Forw)
		return Exit(node,
			DataFlow(u->wtype,
				DataFlow(u->id,
					Entry(node,v))));
	else
		return Exit(node,
			DataFlow(u->id,
				DataFlow(u->wtype,
					Entry(node,v))));
}
//在slidingNode和tumblingNode中加个替换函数是原count后面的count值全部为常数
PRIVATE inline FlowValue DataFlowSliding(Node *node, slidingNode *u, FlowValue v)
{
	if(Forw){
		ListMarker marker;
		Node *item = NULL;
		List *newList = NULL;
		FlowValue e =  Exit(node,
				DataFlow(u->sliding_value,
					Entry(node,v)));
		IterateList(&marker,u->sliding_value->u.comma.exprs);
		while (NextOnList(&marker, (GenericREF) &item)){
			newList = AppendItem(newList,GetValue(item));
		}
		u->sliding_value->u.comma.exprs = newList;
		return e;
	}
	else{
		ListMarker marker;
		Node *item = NULL;
		FlowValue e = Exit(node,
			DataFlow(u->sliding_value,
					Entry(node,v)));
		IterateList(&marker,u->sliding_value->u.comma.exprs);
		while (NextOnList(&marker, (GenericREF) &item)){
			item = GetValue(item);
		}
		return e;
	}
}

PRIVATE inline FlowValue DataFlowTumbling(Node *node, tumblingNode *u, FlowValue v)
{
	if(Forw){

		FlowValue e = Exit(node,
			DataFlow(u->tumbling_value,
				Entry(node,v)));
		u->tumbling_value = GetValue(u->tumbling_value);
		return e;
	}
	else {
		FlowValue e = Exit(node,
			DataFlow(u->tumbling_value,
				Entry(node,v)));
		u->tumbling_value = GetValue(u->tumbling_value);
		return e;
	}
}

/*--------------New For SPL----------*/
PRIVATE inline FlowValue DataFlowCompositeCall(Node *node, comCallNode *u, FlowValue v)
{
	
	
	
	
	if(Forw) //要展开		
	{	FlowValue src;
		/* 处于展开节点状态, 不能对actual_composite进行常量传播，应直接返回, 再对replace_composite做常量传播,
		注意：没有必要对u->call进行拷贝等操作，因为在对replace_composite进行常量传播时会自动拷贝 */
		if (gIsInPipeline || gIsInSplitJoin)
		{
			Node *newNode = NodeCopy(node, Subtree);
			//PrintList(stdout, u->operdcl->u.operdcl.arguments, 0);
			// 对实参进行常量传播
			src = DataFlow(newNode->u.comCall.operdcl, v);
			gCurrentCompositeCallList = AppendItem(gCurrentCompositeCallList, newNode);
			return src;
		}
		// 对实参进行常量传播
		if (u->style == TRUE) // SPL方式的call，说明不在pipeline,splitjoin体内，因此可以进行常量传播
			src = DataFlow(u->operdcl, Entry(node, v));
		else
			src = v; // 不用进行常量传播了，在展开时已经进行过了
		// 对u->call指向的结点深拷贝

		//PrintNode(stdout, node, 0);
	
		node = TransformOperator(node);   

		//PrintNode(stdout, node, 0);
		// 修改拷贝后的composite的param
		node = ModifyCompositeParam(node, &node->u.comCall, src);
		//node = ModifyCompositeParam(node, src);
		DataFlow(u->actual_composite, src);

		return Exit(node, src);
	}
	else
		return Entry(node,
			DataFlow(u->operdcl,
					Exit(node,v)));
}

PRIVATE inline FlowValue DataFlowPipeline(Node *node, PipelineNode *u, FlowValue v)
{
	if(Forw)
	{
		Node *comHead = NULL, *comBody = NULL,*newNode = NULL;
		FlowValue tmp;
		assert(gLevelPipeline >=0);
		if(gIsInSplitJoin||gIsInPipeline){
			Node *item = NULL;
			item = CreateCompositeInMultiSP(node);
			item->u.comCall.style = 0;
			DataFlow(item,v);		
			return v;
		}
		
		gLevelPipeline++;
		if(gLevelPipeline == 1)
			gIsInPipeline = TRUE;
		gCurrentInputStreamNode = u->input;
		gCurrentOutputStreamNode = u->output;
		tmp = DataFlowSerialList(u->stmts,
				DataFlowSerialList(u->decl,
					Entry(node, v)));

		// 信息收集完毕，可以展开节点
		u->replace_composite = UnfoldPipeline(node);

		assert(gIsInPipeline == TRUE);
		assert(gLevelPipeline > 0);
		gLevelPipeline--;
		if(gLevelPipeline == 0)
			gIsInPipeline = FALSE;
		tmp = DataFlow(u->replace_composite, tmp);

		return Exit(node, tmp);
	}	
	else
		return Entry(node,
			DataFlow(u->input,
				DataFlow(u->output,
					DataFlowSerialList(u->decl,
						DataFlowSerialList(u->stmts,
							Exit(node,v))))));
		
}

PRIVATE inline FlowValue DataFlowSplitJoin(Node *node, SplitJoinNode *u, FlowValue v)
{ 
	if (Forw)
	{
		Node *newNode = NULL;
		FlowValue tmp;
		assert(gLevelSplitjoin >=  0);
		if(gIsInSplitJoin||gIsInPipeline){
			Node *item = NULL;
			item = CreateCompositeInMultiSP(node);
			item->u.comCall.style = 0;
			DataFlow(item,v);
			return v;
		}
		gLevelSplitjoin++;
		if(gLevelSplitjoin == 1)
			gIsInSplitJoin = TRUE;
		gCurrentInputStreamNode = u->input;
		gCurrentOutputStreamNode = u->output;

		tmp = DataFlow(u->join,
					DataFlowSerialList(u->stmts,
						DataFlow(u->split,
							DataFlowSerialList(u->initstmts,
								DataFlowSerialList(u->decl, 
									Entry(node, v))))));

		// 信息收集完毕，可以展开节点
		u->replace_composite = UnfoldSplitJoin(node);
		assert(gLevelSplitjoin > 0);
		gLevelSplitjoin--;
		if(gLevelSplitjoin == 0)
			gIsInSplitJoin = FALSE;
		tmp = DataFlow(u->replace_composite, tmp);

		return Exit(node, tmp);
	}
	else
		return Exit(node, 
			DataFlowSerialList(u->input, 
				DataFlowSerialList(u->output,
					DataFlow(u->split,
						DataFlow(u->stmts,
							DataFlow(u->join,
								Entry(node, v)))))));
}

PRIVATE inline FlowValue DataFlowSplit(Node *node, splitNode *u, FlowValue v)
{
	if (Forw)
	{
		FlowValue tmp;
		if (u->type->typ == RoundRobin)
		{
			tmp = DataFlow(u->type, Entry(node, v));//zww:20120801 修改对参数是变量的支持
			gIsRoundrobin = TRUE;
			gCurrentSplitList = u->type->u.roundrobin.arguments;
		}
		else
		{
			tmp = DataFlow(u->type, Entry(node, v));//zww:20120801 修改对参数是变量的支持
			gIsDuplicate = TRUE;
			if (u->type->u.duplicate.expr != NULL)
				gCurrentSplitList = MakeNewList(u->type->u.duplicate.expr);
			else
				gCurrentSplitList = NULL;
		}
		
		return Exit(node, tmp);
	}
	else
		return Entry(node, DataFlow(u->type, Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowJoin(Node *node, joinNode *u, FlowValue v)
{ 
	if (Forw)
	{
		FlowValue tmp = DataFlow(u->type, Entry(node, v));//zww:20120801 修改对参数是变量的支持
		gCurrentJoinList = u->type->u.roundrobin.arguments;
		return Exit(node, tmp);
	}
		
	else
		return Entry(node, DataFlow(u->type, Exit(node, v)));
}

PRIVATE inline FlowValue DataFlowRoundRobin(Node *node, roundrobinNode *u, FlowValue v)
{
	if(Forw)
		return Exit(node,
			DataFlowSerialList(u->arguments,
				Entry(node,v)));
	else
		return Entry(node,
			DataFlowSerialList(u->arguments,
				Exit(node,v)));
}

PRIVATE inline FlowValue DataFlowDuplicate(Node *node, duplicateNode *u, FlowValue v)
{
	if(Forw)
		return Exit(node,
			DataFlow(u->expr,
				Entry(node,v)));
	else
		return Entry(node,
			DataFlow(u->expr,
				Exit(node,v)));
}

PRIVATE inline FlowValue DataFlowAdd(Node *node, addNode *u, FlowValue v)
{
	if(Forw)
		return Exit(node,
			DataFlow(u->content,
				Entry(node,v)));
	else
		return Entry(node,
			DataFlow(u->content,
				Exit(node,v)));
}

PRIVATE inline FlowValue DataFlowItco(Node *node, itcoNode *u, FlowValue v)
{
	return v;
}
/***********************--------------Define For SPL----------****************************/




/*************************************************************************/
/*                                                                       */
/*                  Computing flow through nodes and lists               */
/*                                                                       */
/*************************************************************************/


PRIVATE FlowValue DataFlow(Node *node, FlowValue v)
{
if (node == NULL|| v.undefined == TRUE) 
    return v;
	
#define CODE(name, node, union) v = DataFlow##name(node, union, v);
  ASTSWITCH(node, CODE)
#undef CODE
	 //PrintConstFlowValue(v);
  return v;
}


PRIVATE FlowValue DataFlowSerialList(List *list, FlowValue v)
{
if (list == NULL|| v.undefined == TRUE) 
    return v;

  if (Forw)
    return DataFlowSerialList(Rest(list), DataFlow(FirstItem(list), v));
  else
    return DataFlow(FirstItem(list), DataFlowSerialList(Rest(list), v));
}


PRIVATE inline FlowValue DataFlowBranch(Node *cond, Node *true_, Node *false_, FlowValue v)
{
	FlowValue c, t, f;
	if (Forw) {//常量传播一般用不到, LXX: 12.05
		c = DataFlow(cond, v);
		t = DataFlow(true_, c);
		f = DataFlow(false_, c);
		return Meet(t, f);
	}
  else {
    t = DataFlow(true_, v);
    f = DataFlow(false_, v);
    return DataFlow(cond, Meet(t, f));
  }
}

/*************************************************************************/
/*                                                                       */
/*                       Generic dataflow framework                      */
/*                                                                       */
/*************************************************************************/

PRIVATE Bool Changed;

/*
 *   InitConfluencePoints 
 */
PRIVATE void InitConfluencePoints(Node *node)
{
  switch (node->typ) {
  case Label:
    node->u.label.label_values.undefined = TRUE;
    Changed = TRUE;
    break;
  case Switch:
    node->u.Switch.switch_values.undefined = TRUE;
    node->u.Switch.break_values.undefined = TRUE;
    Changed = TRUE;
    break;
  case While:
    node->u.While.loop_values.undefined = TRUE;
    node->u.While.break_values.undefined = TRUE;
    Changed = TRUE;
    break;
  case Do:
    node->u.Do.loop_values.undefined = TRUE;
    node->u.Do.continue_values.undefined = TRUE;
    node->u.Do.break_values.undefined = TRUE;
    Changed = TRUE;
    break;
  case For:
    node->u.For.loop_values.undefined = TRUE;
    node->u.For.continue_values.undefined = TRUE;
    node->u.For.break_values.undefined = TRUE;
    Changed = TRUE;
    break;
  case Proc:
    node->u.proc.return_values.undefined = TRUE;
	node->u.proc.init_values.undefined = TRUE;
    Changed = TRUE;
    break;
  case Composite:
	  node->u.composite.composite_values.undefined=TRUE;
	   Changed = TRUE;
	  break;
  case Operator_:
	  node->u.operator_.oper_values.undefined=TRUE;
	   Changed = TRUE;
	  break;

  default:
    break;
  }
}

PRIVATE void ConstFlowInto(FlowValue *dest, FlowValue src)
{
	dest->undefined = TRUE;
	FlowInto(dest, src);
	dest->undefined = FALSE;
}

/*
 *   FlowInto meets v with a confluence point, dest.  
 *   returns new value of dest.
 */
PRIVATE FlowValue FlowInto(FlowValue *dest, FlowValue src)
{
	FlowValue new_ = Meet(*dest, src);

#if 0
	PrintConstFlowValue(new_);
#endif

	if (!Equal(*dest, new_)) 
	{
		*dest = new_;
		Changed = TRUE;
	}
	return new_;
}


/*
 * IterateDataFlow 
 */
GLOBAL void IterateDataFlow(
			     Node *root,       /* root node */
			     FlowValue init,  /* input value for root node */
			     Direction dir,    /* direction */
			     MeetOp meet,      /* meet operation */
			     EqualOp equal,    /* equality operation */
			     TransOp trans     /* transfer function */
			     )
{
  Forw = (dir == Forwards);
  Meet = meet;
  Equal = equal;
  Trans = trans;


  /* Initialize all confluence points */
  Final = FALSE;
  Changed = FALSE;
  WalkTree(root, InitConfluencePoints, NULL, Preorder);
  
  /* Iterate until confluence points are stable */
  while (Changed) {
    Changed = FALSE;
    DataFlow(root, init);
  }

  /* Make final pass to capture data-flow information in permanent form */
  Final = TRUE;
  DataFlow(root, init);
}

GLOBAL void IteratePropagatorDataFlow(Node *mainNode, FlowValue init, Direction dir, MeetOp meet, EqualOp equal, TransOp trans)
{
	assert(dir == Forwards);

	Forw = (dir == Forwards);
	Meet = meet;
	Equal = equal;
	Trans = trans;


#if 0
	/* Initialize all confluence points */
	Final = FALSE;
	Changed = FALSE;
	WalkTree(root, InitConfluencePoints, NULL, Preorder);
#endif

	/* Iterate until confluence points are stable */
	/* Make final pass to capture data-flow information in permanent form */
	Final = TRUE;
	DataFlow(mainNode, init);

}
