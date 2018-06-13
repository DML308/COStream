/***********************--------------Define For SPL----------****************************/
#pragma ident "operator.c,v 1.2 2011/11/20 20:24--Liu Xiaoxian, DML, HUST"

#include "ast.h"

PRIVATE Node *CurrentOperator = NULL;

GLOBAL void OperatorConflict(Node *orig, Node *create)
{ 
	Node *ofdcl, *nfdcl;

	assert(orig);
	assert(create);
	assert(orig->typ == Decl);
	assert(create->typ == Decl);
}

GLOBAL Node *DefineOperator(Node *decl)
{
	Node *operdcl = NULL;
	ListMarker marker;
	extern char *yytext;
	Bool multi = FALSE;

	assert(decl != NULL  &&  decl->typ == Decl);
	operdcl = decl->u.decl.type;
	assert(operdcl != NULL);
	if (operdcl->typ != Operdcl) 
	{
		SyntaxErrorCoord(decl->coord, "Syntax error: expecting a self-define Operator(SPL) definition");
		return(decl);
	}

	/*if (gIsUnfold == FALSE && gIsFileOperator == FALSE)
		assert('{' == *yytext); 
	else if (gIsFileOperator == TRUE)
		assert(';' == *yytext); */

//	assert(1 == Level);

	decl = FinishDecl(decl);
	NodeSetDeclLocation(decl, T_BLOCK_DECL);

	/* enter scope of operator_ body */
	EnterScope();

#if 0
	PrintSymbolTable(stdout, OperatorIds);
#endif

	/* return CurrentOperator with no body */
	CurrentOperator = MakeOperatorCoord(decl, NULL, decl->coord);
	return CurrentOperator;
}

GLOBAL Node *SetOperatorBody(Node *operator_, Node *operBody)
{
	assert(operator_ != NULL  &&  operator_->typ == Operator_);
	assert(operBody == NULL  ||  operBody->typ == OperBody);
	assert(operator_->u.operator_.decl != NULL  &&  operator_->u.operator_.decl->typ == Decl);

	/* exit operator_ body scope */
	ExitScope();

	operator_->u.operator_.body = operBody;

	if (operBody == NULL) 
	{
		WarningCoord(4, operator_->u.operator_.decl->coord,	"operator_ `%s' has no code", operator_->u.operator_.decl->u.decl.name);
	} 
	else 
	{
		/* check for unreferenced/unresolved labels,
		all labels are now out of scope */
		//ResetSymbolTable(Labels);
	}

    CurrentOperator = NULL;

    return(operator_);
}

GLOBAL Node *ModifyOperatorDeclArguments(Node *operdcl, List *arguments)
{
	assert(operdcl != NULL && operdcl->typ == Operdcl);
	assert(NULL == operdcl->u.operdcl.arguments);
	
	operdcl->u.operdcl.arguments = arguments;

	return(operdcl);
}