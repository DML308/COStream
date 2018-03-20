/***********************--------------Define For SPL----------****************************/
#pragma ident "stream.c,v 1.0 2011/11/18 17:54--Liu Xiaoxian, DML, HUST"

#include "ast.h"

GLOBAL Bool TrackStreamIds = FALSE;

GLOBAL void stream_conflict(Node *orig, Node *create)
{
	SyntaxErrorCoord(create->coord,	"Syntax error: stream '%s' used multiple times", VAR_NAME(orig));
}

PRIVATE StreamType *MakeStream(NodeType typ, const char *name, List *fields)
{
	StreamType *create = HeapNew(StreamType);
	create->typ = typ;
	create->complete = FALSE;
	create->coord = UnknownCoord;
	create->right_coord = UnknownCoord;
	create->name   = name;
	create->visited = FALSE;
	create->size   = 0;
	create->align  = 1;
	create->fields = fields;//important

	return(create);
}

GLOBAL void PrintStream(FILE *out, StreamType *stream, int offset, Bool recursep)
{
	ListMarker marker;
	Node *decl;
	const char *name;

	if (stream == NULL) 
	{
		fprintf(out, "Null stream");
		return;
	} 

	name = (stream->name) ? stream->name : "nil";

	switch (stream->typ) 
	{
	case STRdcl:
		fprintf(out, "stream %s (%d)", name, stream->size);
		break;
	default:
		fprintf(out, "unexpected StreamType %d", stream->typ);
	}

	if (recursep) 
	{
		IterateList(&marker, stream->fields);
		while (NextOnList(&marker, (GenericREF) &decl)) 
		{
			assert(decl->typ == Decl || decl->typ == Id);
			PrintCRSpaces(out, offset);
			PrintNode(out, decl, offset);
		}
	}
}

GLOBAL Node *SetSTRdclNameFields(Node *strdcl, Node *id, List *fields, Coord left_coord, Coord right_coord)
{
	StreamType *stream;
	const char *name;

	assert(strdcl != NULL  &&  strdcl->typ == STRdcl);
	assert(strdcl->u.strdcl.type == NULL);  /* not allocated yet */

	name = NULL;

	/* create stream */
	stream = MakeStream(strdcl->typ, name, fields);
	stream->complete = TRUE;
	stream->coord = left_coord;
	stream->right_coord = right_coord;

	stream->name = InsertUniqueSymbol(Tags, stream, "___stream");

	strdcl->u.strdcl.tq |= T_STREAM_ELABORATED;
	strdcl->u.strdcl.type = stream;
	return(strdcl);
}

GLOBAL int Stream_Sizeof(StreamType *stream)/*Define For SPL*/
{ 
	assert(stream);
	assert(stream->typ == STRdcl);
	return stream->size; 
}

GLOBAL Node *Stream_FindField(StreamType *stream, Node *field_name)
{ 
	ListMarker marker;
	Node *field;
	const char *name;
	const char *xsname;
	int xs;
	assert(field_name->typ == Id);
	name = field_name->u.id.text;
	/* Find the field in the struct/union fields */
	IterateList(&marker, stream->fields);
	while (NextOnList(&marker, (GenericREF) &field))
	{
		assert(field->typ == Decl);
		xsname = field->u.decl.name;

		for(xs = 0; xs < strlen(name); ++xs)
			if(name[xs] != xsname[xs])
				break;
		if(xs == strlen(name))
			return field;
		//if (strcmp(name, field->u.decl.name) == 0)
		//	return field;
	}

	return NULL;
}

GLOBAL int Stream_Alignment(StreamType *stream)
{ 
	assert(stream);
	return stream->align; 
}

GLOBAL Bool Stream_SameTagp(StreamType *ts1, StreamType *ts2)
{ 
	if (!ts1->name  ||  !ts2->name)
		return FALSE;
	else return strcmp(ts1->name, ts2->name) == 0; 
}

GLOBAL Node *LookupStreamIdsNode(Node *id)
{
	Node *var = NULL;
	const char *name;

	assert(id && id->typ == Id);
	name = id->u.id.text;

	if (!LookupSymbol(Identifiers, name, (GenericREF) &var))
	{
		var = Undeclared;
		SyntaxErrorCoord(id->coord, "Syntax error: undeclared StreamId: '%s'", name);
	} 
	else 
	{
		REFERENCE(var);
		if (TrackStreamIds) 
		{
			fprintf(stderr, "=== `%s' = ", name);
			PrintNode(stderr, var, 0);
			printf("\n");
		}
	}

	id->u.id.decl = var;

	return(id);
}

GLOBAL List *LookupStreamIdsList(List *idList)
{
	Node *var = NULL;
	Node *id = NULL;
	const char *name = NULL;
	ListMarker marker;

	if (idList != NULL)
	{
		IterateList(&marker, idList);
		while (NextOnList(&marker, (GenericREF) &id))
		{
			assert(id && id->typ == Id);
			name = id->u.id.text;
			if (!LookupSymbol(Identifiers, name, (GenericREF) &var))
			{
				var = Undeclared;
				SyntaxErrorCoord(id->coord, "Syntax error: undeclared StreamId: '%s'", name);
			} 
			else 
			{
				REFERENCE(var);
				if (TrackStreamIds) 
				{
					fprintf(stderr, "=== `%s' = ", name);
					PrintNode(stderr, var, 0);
					printf("\n");
				}
			}

			id->u.id.decl = var;

		}
	}

	return(idList);
}
