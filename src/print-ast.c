/*************************************************************************
 *
 *  C-to-C Translator
 *
 *  Adapted from Clean ANSI C Parser
 *  Eric A. Brewer, Michael D. Noakes
 *  
 *  print-ast.c,v
 * Revision 1.17  1995/05/11  18:54:25  rcm
 * Added gcc extension __attribute__.
 *
 * Revision 1.16  1995/04/21  05:44:36  rcm
 * Cleaned up data-flow analysis, and separated into two files, dataflow.c
 * and analyze.c.  Fixed void pointer arithmetic bug (void *p; p+=5).
 * Moved CVS Id after comment header of each file.
 *
 * Revision 1.15  1995/03/23  15:31:20  rcm
 * Dataflow analysis; removed IsCompatible; replaced SUN4 compile-time symbol
 * with more specific symbols; minor bug fixes.
 *
 * Revision 1.14  1995/02/13  02:00:19  rcm
 * Added ASTWALK macro; fixed some small bugs.
 *
 * Revision 1.13  1995/02/01  23:01:28  rcm
 * Added Text node and #pragma collection
 *
 * Revision 1.12  1995/02/01  21:07:22  rcm
 * New AST constructors convention: MakeFoo makes a foo with unknown coordinates,
 * whereas MakeFooCoord takes an explicit Coord argument.
 *
 * Revision 1.11  1995/01/27  01:39:03  rcm
 * Redesigned type qualifiers and storage classes;  introduced "declaration
 * qualifier."
 *
 * Revision 1.10  1995/01/25  02:15:25  rcm
 * Pointer values are once again printed
 *
 * Revision 1.9  1995/01/20  03:38:11  rcm
 * Added some GNU extensions (long long, zero-length arrays, cast to union).
 * Moved all scope manipulation out of lexer.
 *
 * Revision 1.8  1995/01/06  16:48:58  rcm
 * added copyright message
 *
 * Revision 1.7  1994/12/23  09:18:36  rcm
 * Added struct packing rules from wchsieh.  Fixed some initializer problems.
 *
 * Revision 1.6  1994/12/20  09:24:11  rcm
 * Added ASTSWITCH, made other changes to simplify extensions
 *
 * Revision 1.5  1994/11/22  01:54:40  rcm
 * No longer folds constant expressions.
 *
 * Revision 1.4  1994/11/10  03:13:26  rcm
 * Fixed line numbers on AST nodes.
 *
 * Revision 1.3  1994/11/03  07:38:53  rcm
 * Added code to output C from the parse tree.
 *
 * Revision 1.2  1994/10/28  18:52:45  rcm
 * Removed ALEWIFE-isms.
 *
 *
 *  Created: Tue Apr 27 13:17:36 EDT 1993
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
#pragma ident "print-ast.c,v 1.17 1995/05/11 18:54:25 rcm Exp Copyright 1994 Massachusetts Institute of Technology"

#include <ctype.h>
#include "ast.h"




/* main debugging entry points -- handy for calling from gdb */

GLOBAL void DPN(Node *n)
{
  PrintNode(stdout, n, 0);
  putchar('\n');
  fflush(stdout);
}

GLOBAL void DPL(List *list)
{
  PrintList(stdout, list, 0);
  putchar('\n');
  fflush(stdout);
}




/*************************************************************************/
/*                                                                       */
/*                          Expression nodes                             */
/*                                                                       */
/*************************************************************************/

PRIVATE inline void PrintConst(FILE *out, Node *node, ConstNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Const: ");
  PrintConstant(out, node, TRUE);
}

PRIVATE inline void PrintId(FILE *out, Node *node, idNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Id: %s", u->text);
  if (u->value) {
    PrintCRSpaces(out, offset + 2);
    fputs("Value: ", out);
    PrintCRSpaces(out, offset + 4);
    PrintNode(out, u->value, offset + 4);
  }
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->decl, offset + 2);
}

PRIVATE inline void PrintBinop(FILE *out, Node *node, binopNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Binop: ");
  PrintOp(out, u->op);
  if (u->type) {
    PrintCRSpaces(out, offset + 2);
    PrintNode(out, u->type, offset + 2);
  }
  if (u->value) {
    PrintCRSpaces(out, offset + 2);
    fputs("Value: ", out);
    PrintCRSpaces(out, offset + 4);
    PrintNode(out, u->value, offset + 4);
  }
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->left,  offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->right, offset + 2);
}

PRIVATE inline void PrintUnary(FILE *out, Node *node, unaryNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Unary: ");
  PrintOp(out, u->op);
  if (u->type) {
    PrintCRSpaces(out, offset + 2);
    PrintNode(out, u->type, offset + 2);
  }
  if (u->value) {
    PrintCRSpaces(out, offset + 2);
    fputs("Value: ", out);
    PrintCRSpaces(out, offset + 4);
    PrintNode(out, u->value, offset + 4);
  }
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr, offset + 2);
}

PRIVATE inline void PrintCast(FILE *out, Node *node, castNode *u, int offset, Bool norecurse)
{
  fputs("Cast: ", out);
  if (u->value) {
    PrintCRSpaces(out, offset + 2);
    fputs("Value: ", out);
    PrintCRSpaces(out, offset + 4);
    PrintNode(out, u->value, offset + 4);
  }
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->type, offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr, offset + 2);
}

PRIVATE inline void PrintComma(FILE *out, Node *node, commaNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Comma: List: exprs");
  PrintCRSpaces(out, offset + 2);
  PrintList(out, u->exprs, offset + 2);
}

PRIVATE inline void PrintTernary(FILE *out, Node *node, ternaryNode *u, int offset, Bool norecurse)
{
  fputs("Ternary: ", out);
  if (u->value) {
    PrintCRSpaces(out, offset + 2);
    fputs("Value: ", out);
    PrintCRSpaces(out, offset + 4);
    PrintNode(out, u->value, offset + 4);
  }
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->cond,  offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->true_,  offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->false_, offset + 2);
}

PRIVATE inline void PrintArray(FILE *out, Node *node, arrayNode *u, int offset, Bool norecurse)
{
  fputs("Array: ", out);

#if 0
  if (u->value) {
	  PrintCRSpaces(out, offset + 2);
	  fputs("Value: ", out);
	  PrintCRSpaces(out, offset + 4);
	  PrintNode(out, u->value, offset + 4);
  }
#endif // SPL 11.30

  if (u->type) {
    PrintCRSpaces(out, offset + 2);
    PrintNode(out, u->type, offset + 2);
  }
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->name, offset + 2);
  PrintCRSpaces(out, offset + 2);
  fputs("List: dims", out);
  PrintCRSpaces(out, offset + 4);
  PrintList(out, u->dims, offset + 4);
}

PRIVATE inline void PrintCall(FILE *out, Node *node, callNode *u, int offset, Bool norecurse)
{
  fputs("Call: ", out);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->name, offset + 2);
  PrintCRSpaces(out, offset + 2);
  fputs("List: args", out);
  PrintCRSpaces(out, offset + 4);
  PrintList(out, u->args, offset + 4);
}

PRIVATE inline void PrintInitializer(FILE *out, Node *node, initializerNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Initializer: List: exprs");
  PrintCRSpaces(out, offset + 2);
  PrintList(out, u->exprs, offset + 2);
}

PRIVATE inline void PrintImplicitCast(FILE *out, Node *node, implicitcastNode *u, int offset, Bool norecurse)
{
  fputs("ImplicitCast: ", out);
  if (u->type) {
    PrintCRSpaces(out, offset + 2);
    fputs("Type:", out);
    PrintCRSpaces(out, offset + 4);
    PrintNode(out, u->type, offset + 2);
  }
  if (u->value) {
    PrintCRSpaces(out, offset + 2);
    fputs("Value: ", out);
    PrintCRSpaces(out, offset + 4);
    PrintNode(out, u->value, offset + 4);
  }
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr, offset + 2);
}

/*************************************************************************/
/*                                                                       */
/*                          Statement nodes                              */
/*                                                                       */
/*************************************************************************/

PRIVATE inline void PrintLabel(FILE *out, Node *node, labelNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Label: %s (0x%p)",  u->name, node);
#if 0
  fprintf(out, "Label: %s",  u->name);
#endif
  if (u->stmt) {
    PrintCRSpaces(out, offset + 2);
    PrintNode(out, u->stmt, offset + 2);
  }
}

PRIVATE inline void PrintSwitch(FILE *out, Node *node, SwitchNode *u, int offset, Bool norecurse)
{
  ListMarker marker; 
  Node *cse;
  
  fprintf(out, "Switch: (0x%p)", node);
#if 0
  fprintf(out, "Switch: ");
#endif
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr, offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->stmt, offset + 2);
  PrintCRSpaces(out, offset + 2);
  
  IterateList(&marker, u->cases);
  fprintf(out, "Cases:");
  while (NextOnList(&marker, (GenericREF) &cse)) {
    fprintf(out, " %d", cse->coord.line);
  }
}

PRIVATE inline void PrintCase(FILE *out, Node *node, CaseNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Case: (container = 0x%p)", u->container);
#if 0
  fprintf(out, "Case: ");
#endif
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr, offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->stmt, offset + 2);
}

PRIVATE inline void PrintDefault(FILE *out, Node *node, DefaultNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Default: (container = 0x%p)", u->container);
#if 0
  fprintf(out, "Default: ");
#endif
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->stmt, offset + 2);
}

PRIVATE inline void PrintIf(FILE *out, Node *node, IfNode *u, int offset, Bool norecurse)
{
  fputs("If: ", out);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr,  offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->stmt,  offset + 2);
}

PRIVATE inline void PrintIfElse(FILE *out, Node *node, IfElseNode *u, int offset, Bool norecurse)
{
  fputs("IfElse: ", out);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr,  offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->true_,  offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->false_, offset + 2);
}

PRIVATE inline void PrintWhile(FILE *out, Node *node, WhileNode *u, int offset, Bool norecurse)
{
  fprintf(out, "While: (0x%p) ", node);
#if 0
  fprintf(out, "While: ");
#endif
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr, offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->stmt, offset + 2);
}


PRIVATE inline void PrintDo(FILE *out, Node *node, DoNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Do: (0x%p) ", node);
#if 0
  fprintf(out, "Do: ");
#endif
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->stmt, offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr, offset + 2);
}

PRIVATE inline void PrintFor(FILE *out, Node *node, ForNode *u, int offset, Bool norecurse)
{
  fprintf(out, "For: (0x%p) ", node);
#if 0
  fprintf(out, "For: ");
#endif
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->init, offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->cond, offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->next, offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->stmt, offset + 2);
}

PRIVATE inline void PrintGoto(FILE *out, Node *node, GotoNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Goto: %s", 
	  (u->label ? u->label->u.label.name : "nil"));
}

PRIVATE inline void PrintContinue(FILE *out, Node *node, ContinueNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Continue: (container = 0x%p)", u->container);
#if 0
  fprintf(out, "Continue: ");
#endif
}

PRIVATE inline void PrintBreak(FILE *out, Node *node, BreakNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Break: (container = 0x%p)", u->container);
#if 0
  fprintf(out, "Break: ");
#endif
}

PRIVATE inline void PrintReturn(FILE *out, Node *node, ReturnNode *u, int offset, Bool norecurse)
{
  fputs("Return: ", out);
#if 0
  if (u->expr) {
    PrintCRSpaces(out, offset + 2);
    PrintNode(out, NodeDataType(u->expr), offset + 2);
  }
#endif
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->expr, offset + 2);
}

PRIVATE inline void PrintBlock(FILE *out, Node *node, BlockNode *u, int offset, Bool norecurse)
{
  fputs("Block:", out);
  PrintCRSpaces(out, offset + 2);
  fprintf(out, "Type: (0x%p)", u->type);
#if 0
  fputs("Type:", out);
#endif
  PrintCRSpaces(out, offset + 4);
  PrintNode(out, u->type, offset + 4);
  PrintCRSpaces(out, offset + 2);
  fputs("List: decl", out);
  PrintCRSpaces(out, offset + 4);
  PrintList(out, u->decl,  offset + 4);
  
  PrintCRSpaces(out, offset + 2);
  fputs("List: stmts", out);
  PrintCRSpaces(out, offset + 4);
  PrintList(out, u->stmts, offset + 4);
}


/*************************************************************************/
/*                                                                       */
/*                            Type nodes                                 */
/*                                                                       */
/*************************************************************************/

PRIVATE inline void PrintPrim(FILE *out, Node *node, primNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Prim: ");
  PrintPrimType(out, node);
}

PRIVATE inline void PrintTdef(FILE *out, Node *node, tdefNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Tdef: %s (0x%p) (type=0x%p)  ",
	  u->name, node, u->type);
#if 0
  fprintf(out, "Tdef: %s ", u->name);
#endif
  PrintTQ(out, u->tq); 
#if 0
  if (!norecurse) {
    PrintCRSpaces(out, offset + 2);
    PrintNode(out, u->type, offset + 2);
  }
#endif
}

PRIVATE inline void PrintPtr(FILE *out, Node *node, ptrNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Ptr: ");
  PrintTQ(out, u->tq);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->type, offset + 2);
}

PRIVATE inline void PrintAdcl(FILE *out, Node *node, adclNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Adcl: ");
  PrintTQ(out, u->tq);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->type, offset + 2);
  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->dim,  offset + 2);
  if (u->size > 0) {
    PrintCRSpaces(out, offset + 2);
    fprintf(out, "%d", u->size);
  }
}

PRIVATE inline void PrintFdcl(FILE *out, Node *node, fdclNode *u, int offset, Bool norecurse)
{
  fputs("Fdcl: ", out);
  PrintTQ(out, u->tq);
  PrintCRSpaces(out, offset + 2);
  fputs("List: Args: ", out);
  PrintCRSpaces(out, offset + 4);
  PrintList(out, u->args,    offset + 4);
  PrintCRSpaces(out, offset + 2);
  fputs("Returns: ", out);
  PrintCRSpaces(out, offset + 4);
  PrintNode(out, u->returns, offset + 4);
}

PRIVATE inline void PrintSdcl(FILE *out, Node *node, sdclNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Sdcl: (0x%p) ", node);
#if 0
  fprintf(out, "Sdcl: ");
#endif
  if (norecurse)
    fprintf(out, "%s\n", u->type->name);
  else {
    PrintCRSpaces(out, offset + 2);
    PrintTQ(out, u->tq);
    PrintSUE(out, u->type, offset + 4, TRUE);
  }
}

PRIVATE inline void PrintUdcl(FILE *out, Node *node, udclNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Udcl: (0x%p) ", node);
#if 0
  fprintf(out, "Udcl:  ");
#endif
  
  if (norecurse)
    fprintf(out, "%s\n", u->type->name);
  else {
    PrintCRSpaces(out, offset + 2);
    PrintTQ(out, u->tq);
    PrintSUE(out, u->type, offset + 4, TRUE);
  }
}

PRIVATE inline void PrintEdcl(FILE *out, Node *node, edclNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Edcl: (0x%p) ", node);
#if 0
  fprintf(out, "Edcl: ");
#endif
  
  if (norecurse) 
    fprintf(out, "%s\n", u->type->name);
  else {
    PrintCRSpaces(out, offset + 2);
    PrintTQ(out, u->tq);
    PrintSUE(out, u->type, offset + 4, TRUE);
  }
}

/*************************************************************************/
/*                                                                       */
/*                      Other nodes (decls et al.)                       */
/*                                                                       */
/*************************************************************************/

PRIVATE inline void PrintDecl(FILE *out, Node *node, declNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Decl: %s (0x%p) ", u->name ? u->name : "", node);

  PrintTQ(out, u->tq);

#if 0
  fprintf(out, "Decl: ");
#endif
  
  if (norecurse)
    fprintf(out, "\n");
  else {
    PrintCRSpaces(out, offset + 2);
    PrintNode(out, u->type,    offset + 2);
    PrintCRSpaces(out, offset + 2);
    PrintNode(out, u->init,    offset + 2);
    PrintCRSpaces(out, offset + 2);
    PrintNode(out, u->bitsize, offset + 2);
  }
}

PRIVATE inline void PrintAttrib(FILE *out, Node *node, attribNode *u, int offset, Bool norecurse)
{
  fprintf(out, "Attrib: %s", u->name);

  PrintCRSpaces(out, offset + 2);
  PrintNode(out, u->arg, offset + 2);
}

PRIVATE inline void PrintProc(FILE *out, Node *node, procNode *u, int offset, Bool norecurse)
{
  fputs("Proc:\n  ", out);
  PrintNode(out, u->decl, 2);
  fputs("\n  ", out);
  PrintNode(out, u->body, 2);
}

PRIVATE inline void PrintText(FILE *out, Node *node, textNode *u, int offset, Bool norecurse)
{
  fputs("Text: ", out);
  if (u->start_new_line)
    fputs("(new line) ", out);
  PrintString(out, u->text);
}


/*************************************************************************/
/*                                                                       */
/*                            Extensions                                 */
/*                                                                       */
/*************************************************************************/

/***********************--------------Define For SPL----------****************************/
 /*----------13----SPL node----------*/
PRIVATE inline void PrintSTRdcl(FILE *out, Node *node, strdclNode *u, int offset, Bool norecurse)
{
	fprintf(out, "STRdcl: (0x%p) ", node);
#if 0
	fprintf(out, "STRdcl: ");
#endif
	if (norecurse)
		fprintf(out, "%s\n", u->type->name);
	else {
		PrintCRSpaces(out, offset + 2);
		PrintTQ(out, u->tq);
		PrintStream(out, u->type, offset + 4, TRUE);
	}
}

PRIVATE inline void PrintComdcl(FILE *out, Node *node, comDeclNode *u, int offset, Bool norecurse)
{
	fputs("Comdcl: ", out);
	PrintTQ(out, u->tq);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->inout,    offset + 2);
}

PRIVATE inline void PrintComposite(FILE *out, Node *node, compositeNode *u, int offset, Bool norecurse)
{
	fputs("Composite:\n  ", out);
	PrintNode(out, u->decl, 2);
	fputs("\n  ", out);
	PrintNode(out, u->body, 2);
}

PRIVATE inline void PrintComInOut(FILE *out, Node *node, comInOutNode *u, int offset, Bool norecurse)
{
#if 0
	fputs("ComInOut: ", out);
	PrintTQ(out, u->tq);

	PrintCRSpaces(out, offset);
#endif

	fputs("List: Outputs: ", out);
	PrintCRSpaces(out, offset + 2);
	PrintList(out, u->outputs,    offset + 2);

	PrintCRSpaces(out, offset);
	fputs("List: Inputs: ", out);
	PrintCRSpaces(out, offset + 2);
	PrintList(out, u->inputs,    offset + 2);
}

PRIVATE inline void PrintComBody(FILE *out, Node *node, comBodyNode *u, int offset, Bool norecurse)
{
	fputs("ComBody: ", out);

	PrintCRSpaces(out, offset + 2);
	fprintf(out, "Type: (0x%p)", u->type);
#if 0
	fputs("Type:", out);
#endif
	PrintCRSpaces(out, offset + 4);
	PrintNode(out, u->type, offset + 4);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->param,    offset + 2);

	PrintCRSpaces(out, offset);
	fputs("List: Comdecl: ", out);
	PrintCRSpaces(out, offset + 2);
	PrintList(out, u->decl,offset + 2);

	PrintCRSpaces(out, offset);
	fputs("List: Comstmts: ", out);
	PrintCRSpaces(out, offset + 2);
	PrintList(out, u->comstmts,    offset + 2);
}

PRIVATE inline void PrintParam(FILE *out, Node *node, paramNode *u, int offset, Bool norecurse)
{
	fputs("Param: ", out);

	PrintCRSpaces(out, offset + 2);
	fputs("List: Parameters: ", out);
	PrintCRSpaces(out, offset + 4);
	PrintList(out, u->parameters,    offset + 4);
}

PRIVATE inline void PrintOperBody(FILE *out, Node *node, operBodyNode *u, int offset, Bool norecurse)
{
	fputs("OperBody: ", out);

	PrintCRSpaces(out, offset + 2);
	fprintf(out, "Type: (0x%p)", u->type);
#if 0
	fputs("Type:", out);
#endif
	PrintCRSpaces(out, offset + 4);
	PrintNode(out, u->type, offset + 4);

	PrintCRSpaces(out, offset + 2);
	fputs("List: State: ", out);
	PrintCRSpaces(out, offset + 4);
	PrintList(out, u->state,    offset + 4);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->init,    offset + 2);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->work,    offset + 2);

	PrintCRSpaces(out, offset + 2);
	fputs("List: Windows: ", out);
	PrintCRSpaces(out, offset + 4);
	PrintList(out, u->window,    offset + 4);
}

PRIVATE inline void PrintOperdcl(FILE *out, Node *node, operDeclNode *u, int offset, Bool norecurse)
{
	fputs("Operdcl: ", out);
	PrintTQ(out, u->tq);

	PrintCRSpaces(out, offset + 2);
	fputs("List: Outputs: ", out);
	PrintCRSpaces(out, offset + 4);
	PrintList(out, u->outputs,    offset + 4);

	PrintCRSpaces(out, offset + 2);
	fputs("List: Inputs: ", out);
	PrintCRSpaces(out, offset + 4);
	PrintList(out, u->inputs,    offset + 4);

	PrintCRSpaces(out, offset + 2);
	fputs("List: Arguments: ", out);
	PrintCRSpaces(out, offset + 4);
	PrintList(out, u->arguments,    offset + 4);
}

PRIVATE inline void PrintOperator_(FILE *out, Node *node, operatorNode *u, int offset, Bool norecurse)
{
	fputs("Operator_: ", out);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->decl,    offset + 2);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->body,    offset + 2);
}

PRIVATE inline void PrintWindow(FILE *out, Node *node, windowNode *u, int offset, Bool norecurse)
{
	if(u != NULL)
	{
		PrintNode(out,u->id,offset);
		PrintCRSpaces(out, offset);
		if (u->wtype->typ == Sliding)
			fputs("WindowType: slidingWindow ", out);
		else
			fputs("WindowType: tumblingWindow ", out);
		PrintCRSpaces(out, offset + 2);
		PrintNode(out, u->wtype, offset+2);
	}
}


PRIVATE inline void PrintSliding(FILE *out, Node *node, slidingNode *u, int offset, Bool norecurse)
{
	PrintNode(out,u->sliding_value,offset);
	PrintCRSpaces(out, offset);
}


PRIVATE inline void PrintTumbling(FILE *out, Node *node, tumblingNode *u, int offset, Bool norecurse)
{
	PrintNode(out,u->tumbling_value,offset);
	PrintCRSpaces(out, offset);
}


  /*-------7-------New For SPL----------*/

PRIVATE inline void PrintCompositeCall(FILE *out, Node *node, comCallNode *u, int offset, Bool norecurse)
{
	fputs("CompositeCall:  ", out);
	
	PrintCRSpaces(out, offset + 2);
	assert(&(u->call->u.composite) != NULL);

	PrintCRSpaces(out, offset + 2); // 先打印operdcl信息比较好！
	PrintNode(out, u->operdcl,    offset + 2);

	//PrintNode(out, u->call->u.composite.decl, offset + 2);
	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->actual_composite, offset + 2);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->call, offset + 2);
}

PRIVATE inline void PrintPipeline(FILE *out, Node *node, PipelineNode *u, int offset, Bool norecurse)
{
	fputs("Pipeline: ", out);

	if (gIsAfterPropagate == FALSE)
	{
		PrintCRSpaces(out, offset + 2);
		PrintNode(out, u->output,    offset + 2);

		PrintCRSpaces(out, offset + 2);
		PrintNode(out, u->input,    offset + 2);

		PrintCRSpaces(out, offset + 2);
		fputs("List: decl: ", out);
		PrintCRSpaces(out, offset + 4);
		PrintList(out, u->decl,    offset + 4);

		PrintCRSpaces(out, offset + 2);
		fputs("List: stmts: ", out);
		PrintCRSpaces(out, offset + 4);
		PrintList(out, u->stmts,    offset + 4);
	}
	else
	{
		PrintCRSpaces(out, offset + 2);
		PrintNode(out, u->replace_composite,    offset + 2);
	}	
}

PRIVATE inline void PrintSplitJoin(FILE *out, Node *node, SplitJoinNode *u, int offset, Bool norecurse)
{
	fputs("SplitJoin: ", out);

	if (gIsAfterPropagate == FALSE)
	{
		PrintCRSpaces(out, offset + 2);
		PrintNode(out, u->output,    offset + 2);

		PrintCRSpaces(out, offset + 2);
		PrintNode(out, u->input,    offset + 2);

		PrintCRSpaces(out, offset + 2);
		fputs("List: decl: ", out);
		PrintCRSpaces(out, offset + 4);
		PrintList(out, u->decl,    offset + 4);

		PrintCRSpaces(out, offset + 2);
		fputs("List: initstmts: ", out);
		PrintCRSpaces(out, offset + 4);
		PrintList(out, u->initstmts, offset + 4);

		PrintCRSpaces(out, offset + 2);
		PrintNode(out, u->split,    offset + 2);

		PrintCRSpaces(out, offset + 2);
		fputs("List: stmts: ", out);
		PrintCRSpaces(out, offset + 4);
		PrintList(out, u->stmts,    offset + 4);

		PrintCRSpaces(out, offset + 2);
		PrintNode(out, u->join,    offset + 2);
	}
	else
	{
		PrintCRSpaces(out, offset + 2);
		PrintNode(out, u->replace_composite,    offset + 2);
	}
}


PRIVATE inline void PrintSplit(FILE *out, Node *node, splitNode *u, int offset, Bool norecurse)
{
	fputs("Split: ", out);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->type,    offset + 2);
}

PRIVATE inline void PrintJoin(FILE *out, Node *node, joinNode *u, int offset, Bool norecurse)
{
	fputs("Join: ", out);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->type,    offset + 2);
}

PRIVATE inline void PrintRoundRobin(FILE *out, Node *node, roundrobinNode *u, int offset, Bool norecurse)
{
	fputs("RoundRobin: ", out);

	PrintCRSpaces(out, offset + 2);
	fputs("List: Arguments: ", out);
	PrintCRSpaces(out, offset + 4);
	PrintList(out, u->arguments,    offset + 4);
}

PRIVATE inline void PrintDuplicate(FILE *out, Node *node, duplicateNode *u, int offset, Bool norecurse)
{
	fputs("Duplicate: ", out);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->expr,    offset + 2);
}


/********3********新文法**********/

PRIVATE inline void PrintAdd(FILE *out, Node *node, addNode *u, int offset, Bool norecurse)
{
	fputs("Add: ", out);

	PrintCRSpaces(out, offset + 2);
	PrintNode(out, u->content,    offset + 2);
}

PRIVATE inline void PrintItco(FILE *out, Node *node, itcoNode *u, int offset, Bool norecurse)
{
	fprintf(out, "ItetatorCount: %s", u->text);
}

/***********************--------------Define For SPL----------****************************/

/*************************************************************************/
/*                                                                       */
/*                      PrintNode and PrintList                          */
/*                                                                       */
/*************************************************************************/


GLOBAL short PassCounter = 0;
GLOBAL int PrintInvocations = 0;  /* number of pending PrintNodes on call
				     stack */


GLOBAL void PrintNode(FILE *out, Node *node, int offset)
{
  Bool norecurse;
  
  if (node == NULL) {
    fprintf(out, "nil");
    return;
  }
  
  if (PrintInvocations++ == 0) {
    /* then we're the first invocation for this pass over the tree */
    ++PassCounter;
  }
  norecurse = (node->pass == PassCounter);
  node->pass = PassCounter;
  
#define CODE(name, node, union) Print##name(out,node,union,offset,norecurse)
  ASTSWITCH(node, CODE)
#undef CODE
#if 0

  if (node->analysis.livevars) {
    PrintCRSpaces(out, offset+2);
    PrintAnalysis(out, node);
  }

#endif 
  --PrintInvocations;
}


GLOBAL void PrintList(FILE *out, List *list, int offset)
{
  ListMarker marker;
  Node *item;
  Bool firstp = TRUE;
  
  if (PrintInvocations++ == 0) {
    /* then we're the first invocation for this pass over the tree */
    ++PassCounter;
  }
  
  IterateList(&marker, list);
  while (NextOnList(&marker, (GenericREF) &item)) {
    if (firstp == TRUE)
      firstp = FALSE;
    else if (offset < 0)
      fputs("\n\n", out);
    else
      PrintCRSpaces(out, offset);
    
    PrintNode(out, item, offset);
  }
  
  if (firstp == TRUE) fputs("nil", out);
  
  --PrintInvocations;
}




/*************************************************************************/
/*                                                                       */
/*                      Low-level output routines                        */
/*                                                                       */
/*************************************************************************/

int print_float(FILE *fd, float val)
{
  int   i;
  char  fmt[8];
  char  buf[64];
  float tmp;
  
  i = 7;
  while (1)
    {
      sprintf(fmt, "%%.%dg", i);
      sprintf(buf, fmt, val);
      sscanf(buf, "%f", &tmp);
      if (tmp == val) break;
      i += 1;
      assert(i < 20);
    }
  
  return fprintf(fd, "%s", buf);
}


int print_double(FILE *fd, double val)
{
  int    i;
  char   fmt[8];
  char   buf[64];
  double tmp;
  
  i = 16;
  while (1)
    {
      sprintf(fmt, "%%.%dlg", i);
      sprintf(buf, fmt, val);
      sscanf(buf, "%lf", &tmp);
      if (tmp == val) break;
      i += 1;
      assert(i < 20);
    }
  
  return fprintf(fd, "%s", buf);
}

GLOBAL int PrintConstant(FILE *out, Node *c, Bool with_name)
{ int len = 0;
  
  if (with_name)
    switch (c->u.Const.type->typ) {
    case Prim:
      len = PrintPrimType(out, c->u.Const.type) + 1;
      fputc(' ', out);
      break;
      /* Used for strings */
    case Adcl:
      assert(c->u.Const.type->u.adcl.type->typ == Prim);
      fprintf(out, "array of ");
      len = PrintPrimType(out, c->u.Const.type->u.adcl.type) + 10;
      fputc(' ', out);
      break;
    default:
      len = fprintf(out, "??? ");
    }
  
  switch (c->u.Const.type->typ) {
  case Prim:
    switch (c->u.Const.type->u.prim.basic) {
    case Sint:
      return len + fprintf(out, "%d", c->u.Const.value.i);
      
      /*Manish 2/3 hack to print pointer constants */
    case Uint:
      return len + fprintf(out, "%uU", c->u.Const.value.u);
    case Slong:
      return len + fprintf(out, "%ldL", c->u.Const.value.l);
    case Ulong:
      return len + fprintf(out, "%luUL", c->u.Const.value.ul);
    case Float:
      return len + print_float(out, c->u.Const.value.f);
    case Double:
      return len + print_double(out, c->u.Const.value.d);
    case Char:
    case Schar:
    case Uchar:
      return len + PrintChar(out, c->u.Const.value.i);
      
    default:
      Fail(__FILE__, __LINE__, "");
      return 0;
    }
    
    /* Manish 2/3  Print Constant Pointers */
  case Ptr:
    return len + fprintf(out, "%u", c->u.Const.value.u);
    /* Used for strings */
  case Adcl:
    return len + PrintString(out, c->u.Const.value.s);
    
  default:
    assert(("Unrecognized constant type", TRUE));
    return 0;
  }
}

void PrintCRSpaces(FILE *out, int spaces)
{ fputc('\n', out); while (spaces--) fputc(' ', out); }

void PrintSpaces(FILE *out, int spaces)
{ while (spaces--) fputc(' ', out); }


GLOBAL void CharToText(char *array, unsigned char value)
{
  if (value < ' ') {
    static const char *names[32] = {
      "nul","soh","stx","etx","eot","enq","ack","bel",
      "\\b", "\\t", "\\n", "\\v", "ff", "cr", "so", "si",
      "dle","dc1","dc2","dc3","dc4","nak","syn","etb",
      "can","em", "sub","esc","fs", "gs", "rs", "us" };
    sprintf(array, "0x%02x (%s)", value, names[value]);
  } else if (value < 0x7f) {
    sprintf(array, "'%c'", value);
  } else if (value == 0x7f) {
    strcpy(array, "0x7f (del)");
  } else { /* value >= 0x80 */
    sprintf(array, "0x%x", value);
  }
}


GLOBAL inline int PrintChar(FILE *out, int value)
{
  switch(value) {
  case '\n': return fprintf(out, "\\n");
  case '\t': return fprintf(out, "\\t");
  case '\v': return fprintf(out, "\\v");
  case '\b': return fprintf(out, "\\b");
  case '\r': return fprintf(out, "\\r");
  case '\f': return fprintf(out, "\\f");
  case '\a': return fprintf(out, "\\a");
  case '\\': return fprintf(out, "\\\\");
  case '\?': return fprintf(out, "\\\?");
  case '\"': return fprintf(out, "\\\"");
  case '\'': return fprintf(out, "\\\'");
  default:
    if (isprint(value)) {
      fputc(value, out);
      return 1;
    } else {
      return fprintf(out, "\\%o", value);
    }
  }
}


GLOBAL int PrintString(FILE *out, const char *s)
{
  int len = 0;
  
  fputc('\"', out);
  while (*s != 0) {
    len += PrintChar(out, *s++);
  }
  fputc('\"', out);
  
  return len + 2;
}




