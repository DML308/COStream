/***********************--------------Define For SPL----------****************************/
#pragma ident "composite.c,v 1.0 2011/11/19 16:20--Liu Xiaoxian, DML, HUST"

#include "ast.h"

PRIVATE Node *CurrentComposite = NULL;
GLOBAL  Bool TrackCompositeIds = FALSE;

GLOBAL void CompositeConflict(Node *orig, Node *create)
{ 
	Node *ofdcl, *nfdcl;

	assert(orig);
	assert(create);
	assert(orig->typ == Decl);
	assert(create->typ == Decl);
}

GLOBAL void parameter_conflict(Node *orig, Node *create)
{
	SyntaxErrorCoord(create->coord,	"Syntax error: parameter '%s' used multiple times", VAR_NAME(orig));
}

GLOBAL Node *DefineComposite(Node *decl)
{
	Node *comdcl = NULL, *comInOut = NULL, *inout = NULL;
	ListMarker marker;
	extern char *yytext;
	Bool multi = FALSE;

	assert(decl != NULL  &&  decl->typ == Decl);
	comdcl = decl->u.decl.type;
	assert(comdcl != NULL);
	if (comdcl->typ != Comdcl) 
	{
		SyntaxErrorCoord(decl->coord, "Syntax error: expecting a composite(SPL) definition");
		return(decl);
	}

	if (gIsUnfold == FALSE)
		assert(')' == *yytext); 

	assert(0 == Level);

	decl = FinishDecl(decl);
	NodeSetDeclLocation(decl, T_TOP_DECL);

	/* enter scope of composite body */
	EnterScope();

	comInOut = comdcl->u.comdcl.inout;
	if (NULL != comInOut)
	{
		/* add output streams and input streams to the scope of the upcoming block */
		IterateList(&marker, comInOut->u.comInOut.outputs);
		if(ListLength(comInOut->u.comInOut.outputs) > 1) multi = TRUE; 
		while (NextOnList(&marker, (GenericREF) &inout)) 
		{
			assert(Decl == inout->typ);
			InsertSymbol(Identifiers, inout->u.decl.name, inout, (ConflictProc) var_conflict);
		}

		IterateList(&marker, comInOut->u.comInOut.inputs);
		if(ListLength(comInOut->u.comInOut.inputs) > 1) multi = TRUE; 
		while (NextOnList(&marker, (GenericREF) &inout)) 
		{
			assert(Decl == inout->typ);
			InsertSymbol(Identifiers, inout->u.decl.name, inout, (ConflictProc) var_conflict);
		}
	}

#if 0
	PrintSymbolTable(stdout, Identifiers);
#endif

	/* return CurrentComposite with no body */
	CurrentComposite = MakeCompositeCoord(decl, NULL, multi, decl->coord);
	InsertSymbol(CompositeIds, decl->u.decl.name, CurrentComposite, (ConflictProc) var_conflict);

	return CurrentComposite;
}

GLOBAL Node *SetCompositeBody(Node *composite, Node *comBody)
{
	assert(composite != NULL  &&  composite->typ == Composite);
	assert(comBody == NULL  ||  comBody->typ == ComBody);
	assert(composite->u.composite.decl != NULL  &&  composite->u.composite.decl->typ == Decl);

	/* exit composite body scope */
	ExitScope();

	composite->u.composite.body = comBody;

	if (comBody == NULL) 
	{
		WarningCoord(4, composite->u.composite.decl->coord,	"composite '%s' has no code", composite->u.composite.decl->u.decl.name);
	} 
	else 
	{
		/* check for unreferenced/unresolved labels,
		all labels are now out of scope */
		//ResetSymbolTable(Labels);
	}

    CurrentComposite = NULL;

    return(composite);
}

GLOBAL Node *LookupCompositeIdsNode(Node *id)
{
	Node *var = NULL;
	const char *name = id->u.id.text;


	assert(id && id->typ == Id);

	if (!LookupSymbol(CompositeIds, name, (GenericREF) &var))
	{
		var = Undeclared;
		SyntaxErrorCoord(id->coord, "Syntax error: undeclared CompositeId: '%s'", name);
	} 
	else 
	{
		//if(Isvar->u.composite.multi);
		REFERENCE(var);
		if (TrackCompositeIds) 
		{
			fprintf(stderr, "=== `%s' = ", name);
			PrintNode(stderr, var, 0);
			printf("\n");
		}
	}


	return(var);
}