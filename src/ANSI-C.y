%{

    /* Copyright (C) 1989,1990 James A. Roskind, All rights reserved.
    This grammar was developed  and  written  by  James  A.  Roskind. 
    Copying  of  this  grammar  description, as a whole, is permitted 
    providing this notice is intact and applicable  in  all  complete 
    copies.   Translations as a whole to other parser generator input 
    languages  (or  grammar  description  languages)   is   permitted 
    provided  that  this  notice is intact and applicable in all such 
    copies,  along  with  a  disclaimer  that  the  contents  are   a 
    translation.   The reproduction of derived text, such as modified 
    versions of this grammar, or the output of parser generators,  is 
    permitted,  provided  the  resulting  work includes the copyright 
    notice "Portions Copyright (c)  1989,  1990  James  A.  Roskind". ke 
    Derived products, such as compilers, translators, browsers, etc., 
    that  use  this  grammar,  must also provide the notice "Portions 
    Copyright  (c)  1989,  1990  James  A.  Roskind"  in   a   manner 
    appropriate  to  the  utility,  and in keeping with copyright law 
    (e.g.: EITHER displayed when first invoked/executed; OR displayed 
    continuously on display terminal; OR via placement in the  object 
    code  in  form  readable in a printout, with or near the title of 
    the work, or at the end of the file).  No royalties, licenses  or 
    commissions  of  any  kind are required to copy this grammar, its 
    translations, or derivative products, when the copies are made in 
    compliance with this notice. Persons or corporations that do make 
    copies in compliance with this notice may charge  whatever  price 
    is  agreeable  to  a  buyer, for such copies or derivative works. 
    THIS GRAMMAR IS PROVIDED ``AS IS'' AND  WITHOUT  ANY  EXPRESS  OR 
    IMPLIED  WARRANTIES,  INCLUDING,  WITHOUT LIMITATION, THE IMPLIED 
    WARRANTIES  OF  MERCHANTABILITY  AND  FITNESS  FOR  A  PARTICULAR 
    PURPOSE.

    James A. Roskind
    Independent Consultant
    516 Latania Palm Drive
    Indialantic FL, 32903
    (407)729-4348
    jar@ileaf.com
    or ...!uunet!leafusa!jar


    ---end of copyright notice---


This file is a companion file to a C++ grammar description file.

*/

/*************************************************************************
 *
 *  C-to-C Translator
 *
 *  Adapted from Clean ANSI C Parser
 *  Eric A. Brewer, Michael D. Noakes
 *  
 *  File: ANSI-C.y
 *  ANSI-C.y,v
 * Revision 1.18  1995/05/11  18:53:51  rcm
 * Added gcc extension __attribute__.
 *
 * Revision 1.17  1995/04/21  05:43:54  rcm
 * Cleaned up data-flow analysis, and separated into two files, dataflow.c
 * and analyze.c.  Fixed void pointer arithmetic bug (void *p; p+=5).
 * Moved CVS Id after comment header of each file.
 *
 * Revision 1.16  1995/04/09  21:30:36  rcm
 * Added Analysis phase to perform all analysis at one place in pipeline.
 * Also added checking for functions without return values and unreachable
 * code.  Added tests of live-variable analysis.
 *
 * Revision 1.15  1995/02/13  01:59:53  rcm
 * Added ASTWALK macro; fixed some small bugs.
 *
 * Revision 1.14  1995/02/10  22:09:37  rcm
 * Allow comma at end of enum
 *
 * Revision 1.13  1995/02/01  23:01:20  rcm
 * Added Text node and #pragma collection
 *
 * Revision 1.12  1995/02/01  21:07:01  rcm
 * New AST constructors convention: MakeFoo makes a foo with unknown coordinates,
 * whereas MakeFooCoord takes an explicit Coord argument.
 *
 * Revision 1.11  1995/02/01  07:36:18  rcm
 * Renamed list primitives consistently from '...Element' to '...Item'
 *
 * Revision 1.10  1995/01/27  01:38:45  rcm
 * Redesigned type qualifiers and storage classes;  introduced "declaration
 * qualifier."
 *
 * Revision 1.9  1995/01/25  21:38:10  rcm
 * Added TypeModifiers to make type modifiers extensible
 *
 * Revision 1.8  1995/01/25  02:16:10  rcm
 * Changed how Prim types are created and merged.
 *
 * Revision 1.7  1995/01/20  03:37:53  rcm
 * Added some GNU extensions (long long, zero-length arrays, cast to union).
 * Moved all scope manipulation out of lexer.
 *
 * Revision 1.6  1994/12/23  09:16:00  rcm
 * Marks global declarations
 *
 * Revision 1.5  1994/12/20  09:23:44  rcm
 * Added ASTSWITCH, made other changes to simplify extensions
 *
 * Revision 1.4  1994/11/22  01:54:20  rcm
 * No longer folds constant expressions.
 *
 * Revision 1.3  1994/11/10  03:07:26  rcm
 * Line-number behavior changed.  Now keeps coordinates of first terminal
 * in production, rather than end of production.
 *
 * Revision 1.2  1994/10/28  18:51:53  rcm
 * Removed ALEWIFE-isms.
 *
 *
 *************************************************************************/
#pragma ident "ANSI-C.y,v 1.18 1995/05/11 18:53:51 rcm Exp"

/* FILENAME: C.Y */

/*  This  is a grammar file for the dpANSI C language.  This file was 
last modified by J. Roskind on 3/7/90. Version 1.00 */




/* ACKNOWLEDGMENT:

Without the effort expended by the ANSI C standardizing committee,  I 
would  have been lost.  Although the ANSI C standard does not include 
a fully disambiguated syntax description, the committee has at  least 
provided most of the disambiguating rules in narratives.

Several  reviewers  have also recently critiqued this grammar, and/or 
assisted in discussions during it's preparation.  These reviewers are 
certainly not responsible for the errors I have committed  here,  but 
they  are responsible for allowing me to provide fewer errors.  These 
colleagues include: Bruce Blodgett, and Mark Langley. */

/* Added by Eric A. Brewer */

#define _Y_TAB_H_  /* prevents redundant inclusion of y.tab.h */
#include "ast.h"

#ifndef YYDEBUG
int yydebug=0;
#endif
#define YYERROR_VERBOSE 1

extern int yylex(void);
GLOBAL List *GrabPragmas(List *stmts_or_decls);  /* from c4.l */
PRIVATE void WarnAboutPrecedence(OpType op, Node *node);

PRIVATE void yyerror(const char *msg)
{
    SyntaxError(msg);
}

/* End of create code (EAB) */




%}

/* This refined grammar resolves several typedef ambiguities  in  the 
draft  proposed  ANSI  C  standard  syntax  down  to  1  shift/reduce 
conflict, as reported by a YACC process.  Note  that  the  one  shift 
reduce  conflicts  is the traditional if-if-else conflict that is not 
resolved by the grammar.  This ambiguity can  be  removed  using  the 
method  described in the Dragon Book (2nd edition), but this does not 
appear worth the effort.

There was quite a bit of effort made to reduce the conflicts to  this 
level,  and  an  additional effort was made to make the grammar quite 
similar to the C++ grammar being developed in  parallel.   Note  that 
this grammar resolves the following ANSI C ambiguity as follows:

ANSI  C  section  3.5.6,  "If  the [typedef name] is redeclared at an 
inner scope, the type specifiers shall not be omitted  in  the  inner 
declaration".   Supplying type specifiers prevents consideration of T 
as a typedef name in this grammar.  Failure to supply type specifiers 
forced the use of the TYPEDEFname as a type specifier.
              
ANSI C section 3.5.4.3, "In a parameter declaration, a single typedef 
name in parentheses is  taken  to  be  an  abstract  declarator  that 
specifies  a  function  with  a  single  parameter,  not as redundant 
parentheses around the identifier".  This is extended  to  cover  the 
following cases:

typedef float T;
int noo(const (T[5]));
int moo(const (T(int)));
...

Where  again the '(' immediately to the left of 'T' is interpreted as 
being the start of a parameter type list,  and  not  as  a  redundant 
paren around a redeclaration of T.  Hence an equivalent code fragment 
is:

typedef float T;
int noo(const int identifier1 (T identifier2 [5]));
int moo(const int identifier1 (T identifier2 (int identifier3)));
...

*/


%union {
    Node      *n;
    List      *L;

  /* tq: type qualifiers */
    struct {
        TypeQual   tq;
	Coord      coord;   /* coordinates where type quals began */ 
    } tq;

  /* tok: token coordinates */
    Coord tok;
}


/* Define terminal tokens */

%token <tok> '&' '*' '+' '-' '~' '!'
%token <tok> '<' '>' '|' '^' '%' '/' '(' ')' '.' '?' ';'

%token <tok> '{' '}' ',' '[' ']' ':'

/* ANSI keywords, extensions below */
%token <tok> AUTO            DOUBLE          INT             STRUCT
%token <tok> BREAK           ELSE            LONG            SWITCH
%token <tok> CASE            ENUM            REGISTER        TYPEDEF
%token <tok> CHAR            EXTERN          RETURN          UNION
%token <tok> CONST           FLOAT           SHORT           UNSIGNED
%token <tok> CONTINUE        FOR             SIGNED          VOID
%token <tok> DEFAULT         GOTO            SIZEOF          VOLATILE
%token <tok> DO              IF              STATIC          WHILE

/*-----------------Define For SPL-------2011.11.14--Liu Xiaoxian, DML, HUST---------*/
%token <tok> COMPOSITE		 INPUT			 OUTPUT			 STREAM
%token <tok> PARAM			 ADD  		
%token <tok> INIT			 WORK			 WINDOW
%token <tok> TUMBLING		 SLIDING		 
		 			 
%token <tok> SPLITJOIN        SPLIT				JOIN 
%token <tok> DUPLICATE        ROUNDROBIN		PIPELINE
%token <tok> FILEREADER       FILEWRITER		
/*-----------------Define For SPL-------2011.11.14--Liu Xiaoxian, DML, HUST---------*/

/* unary op tokens added by Eric Brewer */

%token <tok> UPLUS UMINUS INDIR ADDRESS POSTINC POSTDEC PREINC PREDEC BOGUS

/* ANSI Grammar suggestions */
%token <n> IDENTIFIER STRINGliteral
%token <n> FLOATINGconstant
%token <n> INTEGERconstant OCTALconstant HEXconstant WIDECHARconstant
%token <n> CHARACTERconstant

/* New Lexical element, whereas ANSI suggested non-terminal */

/* 
   Lexer distinguishes this from an identifier.
   An identifier that is CURRENTLY in scope as a typedef name is provided
   to the parser as a TYPEDEFname
*/
%token <n> TYPEDEFname

/* Multi-Character operators */
%token <tok>  ARROW            /*    ->                              */
%token <tok>  ICR DECR         /*    ++      --                      */
%token <tok>  LS RS            /*    <<      >>                      */
%token <tok>  LE GE EQ NE      /*    <=      >=      ==      !=      */
%token <tok>  ANDAND OROR      /*    &&      ||                      */
%token <tok>  ELLIPSIS         /*    ...                             */

/* modifying assignment operators */
%token <tok> '='
%token <tok> MULTassign  DIVassign    MODassign   /*   *=      /=      %=      */
%token <tok> PLUSassign  MINUSassign              /*   +=      -=              */
%token <tok> LSassign    RSassign                 /*   <<=     >>=             */
%token <tok> ANDassign   ERassign     ORassign    /*   &=      ^=      |=      */

/* GCC extensions */
%token <tok> INLINE
%token <tok> ATTRIBUTE


%type <tok> lblock rblock


%type <L> translation.unit external.definition 
%type <n> function.definition

/****************************Define For SPL************************************/
/*for composite*/
%type <n> composite.definition 
%type <n> composite.head composite.body.no.new.scope
%type <n> composite.head.inout composite.head.inout.member 
%type <L> composite.head.inout.member.list
/*for composite.body */
%type <n> composite.body.param.opt   
%type <L> composite.body.statement.list 
/*for SPL compositestmt*/
%type <n> composite.body.operator
%type <n> operator.splitjoin operator.pipeline operator.add
/*for streamit filter*/
%type <n> operator.default.call
%type <n> split.statement join.statement 
%type <n> duplicate.statement roundrobin.statement
/*for files filter*/
%type <n>  operator.file.writer
/*for operator body*/
%type <n> operator.selfdefine.body.init.opt operator.selfdefine.body.work  operator.selfdefine.body.window.list.opt
%type <L> operator.selfdefine.window.list  
%type <n> operator.selfdefine.window  window.type 
/*for splitjoin and pipeline statement*/
%type <L> splitjoinPipeline.statement.list
/****************************Define For SPL************************************/

%type <n> constant string.literal.list
%type <n> primary.expression postfix.expression unary.expression
%type <n> cast.expression multiplicative.expression additive.expression
%type <n> shift.expression relational.expression equality.expression
%type <n> AND.expression exclusive.OR.expression inclusive.OR.expression
%type <n> logical.AND.expression logical.OR.expression conditional.expression
%type <n> assignment.expression constant.expression expression.opt
%type <L> attributes.opt attributes attribute attribute.list
%type <n> attrib any.word

%type <n> initializer.opt initializer initializer.list
%type <n> bit.field.size.opt bit.field.size enumerator.value.opt

%type <n> costream.composite.statement statement labeled.statement expression.statement 
%type <n> selection.statement iteration.statement jump.statement
%type <n> compound.statement compound.statement.no.new.scope

%type <n> basic.declaration.specifier basic.type.specifier
%type <n> type.name expression type.specifier declaration.specifier
%type <n> typedef.declaration.specifier typedef.type.specifier
%type <n> abstract.declarator unary.abstract.declarator
%type <n> postfixing.abstract.declarator array.abstract.declarator
%type <n> postfix.abstract.declarator old.function.declarator
%type <n> struct.or.union.specifier struct.or.union elaborated.type.name
%type <n> sue.type.specifier sue.declaration.specifier enum.specifier

/**********Define For SPL ********/
%type <n> stream.type.specifier
%type <L> stream.declaration.list
/**********Define For SPL ********/

%type <n> parameter.declaration 
%type <n> identifier.declarator parameter.typedef.declarator
%type <n> declarator paren.typedef.declarator
%type <n> clean.typedef.declarator simple.paren.typedef.declarator
%type <n> unary.identifier.declarator paren.identifier.declarator
%type <n> postfix.identifier.declarator clean.postfix.typedef.declarator
%type <n> paren.postfix.typedef.declarator postfix.old.function.declarator
%type <n> struct.identifier.declarator struct.declarator

%type <L> declaration declaration.list declaring.list default.declaring.list
%type <L> argument.expression.list identifier.list statement.list 
%type <L> parameter.type.list parameter.list 
%type <L> struct.declaration.list struct.declaration struct.declaring.list
%type <L> struct.default.declaring.list enumerator.list
%type <L> old.function.declaration.list

%type <n> unary.operator assignment.operator
%type <n> identifier.or.typedef.name

%type <tq> type.qualifier type.qualifier.list declaration.qualifier.list
%type <tq> declaration.qualifier storage.class
%type <tq> pointer.type.qualifier pointer.type.qualifier.list
%type <L> operator.arguments
%type <n> operator.selfdefine.body
%type <n>  basic.type.name

%start prog.start

%%
prog.start: /*P*/
          translation.unit { Program = GrabPragmas($1); }
		  ;

/*************************************************************************/
/*                                                                       */
/*                      SPL Grammar Definition----begin                  */
/*                                                                       */
/*************************************************************************/

/***********************stream.definition*****************************/
stream.type.specifier:
		  STREAM '<' stream.declaration.list '>'
		{
			$$ = SetSTRdclNameFields(MakeSTRdclCoord(EMPTY_TQ, NULL, $1), NULL, $3, $2, $4);
		}
		;

stream.declaration.list:
		  type.specifier paren.identifier.declarator 
		{ 
			$$ = MakeNewList(SetDeclType($2, $1, Stream));	
		}
		|  type.specifier paren.identifier.declarator postfixing.abstract.declarator
		{ 
			$$ = MakeNewList(SetDeclType( ModifyDeclType($2,$3),$1, Stream));
			// $$ = MakeNewList(SetDeclTypeMult($2, $4, $1, Stream));
			//$$ = MakeNewList(SetDeclType($2, $1, Stream));	
		}
		| stream.declaration.list ',' type.specifier paren.identifier.declarator
		{ 
			$$ = JoinLists($1, MakeNewList(SetDeclType($4, $3, Stream)));
		}
		| stream.declaration.list ',' type.specifier paren.identifier.declarator postfixing.abstract.declarator
		{ 
			$$ = JoinLists($1, MakeNewList(SetDeclType(ModifyDeclType($4, $5), $3, Stream)));
			//$$ = JoinLists($1, MakeNewList(SetDeclTypeMult($4, $6, $3, Stream)));
			//$$ = JoinLists($1, MakeNewList(SetDeclType($4, $3, Stream)));
		}
		;


/***********************composite.definition*****************************/
composite.definition:						/*node struct*/
		  composite.head 
		{
			IsInCompositeParam = TRUE;
			$1 = DefineComposite($1);
		}
		  composite.body.no.new.scope
		{
			///*DEBUG*/printf("have found composite.definition!\n\n");
			$$ = SetCompositeBody($1, $3);
			
		}
		;

/***********************composite.head*****************************/
composite.head:								/*node struct*/
		  COMPOSITE IDENTIFIER '(' composite.head.inout ')'  
		{
			///*DEBUG*/printf("have found composite.head!\n");
			$$ = SetDeclType(ModifyDeclType(ConvertIdToDecl($2, EMPTY_TQ, NULL, NULL, NULL), MakeComdclCoord(EMPTY_TQ, $4, $3)),
								MakeDefaultPrimType(EMPTY_TQ, $1), 
								Redecl);
		}
		;

composite.head.inout:						/*node struct*/
		  /*empty*/{ $$ = NULL; }
		| INPUT composite.head.inout.member.list ',' OUTPUT composite.head.inout.member.list
		{
			$$ = MakeComInOutCoord(EMPTY_TQ, $2, $5, $1);
		}
		| INPUT composite.head.inout.member.list
		{
			$$ = MakeComInOutCoord(EMPTY_TQ, $2, NULL, $1);
		}
		| OUTPUT composite.head.inout.member.list
		{
			$$ = MakeComInOutCoord(EMPTY_TQ, NULL, $2, $1);
		}
		| OUTPUT composite.head.inout.member.list ',' INPUT composite.head.inout.member.list
		{
			$$ = MakeComInOutCoord(EMPTY_TQ, $5, $2, $1);
		}
		;

composite.head.inout.member.list:			/*list struct*/
		  composite.head.inout.member
		{
			$$ = MakeNewList($1);
		}
		| composite.head.inout.member.list ',' composite.head.inout.member
		{
			$$ = AppendItem($1,$3);
		}
		;

composite.head.inout.member:				/*node struct*/
		  stream.type.specifier IDENTIFIER
		{
			$$ = SetDeclType(ConvertIdToDecl($2, EMPTY_TQ, NULL, NULL, NULL), $1, Commal);
		}
		;

/***********************composite.body(framework)*****************************/	
composite.body.no.new.scope:								/*node struct*/
			 '{' composite.body.param.opt composite.body.statement.list '}'
		{
			/////*DEBUG*/printf("have found composite.body!\n");
			$$ = MakeComBodyCoord(PrimVoid, $2, NULL,$3, $1, $4);
		}
		|  '{' composite.body.param.opt declaration.list composite.body.statement.list '}'
		{
			/////*DEBUG*/printf("have found composite.body!\n");
			$$ = MakeComBodyCoord(PrimVoid, $2, $3,$4, $1, $5);
		}
		;

/************************------composite.body(details)------****************************/
composite.body.param.opt:					/*node struct*/
		  /*empty*/ { $$ = NULL; }
		| PARAM parameter.list ';'
		{
			///*DEBUG*/printf("have found composite.body.param!\n");
			$$ = MakeParamCoord($2, $1);
			IsInCompositeParam = FALSE;
		}
		;

composite.body.statement.list:
		 costream.composite.statement
		{
			$$ = MakeNewList($1);
		}
		| composite.body.statement.list costream.composite.statement
		{
			$$ = AppendItem($1,$2);
		}
		;

/********************-----------composite.body.operator---------********************/


composite.body.operator:    
			operator.add
		{
			$$ = $1;
		}
		| operator.file.writer
		{
			///*DEBUG*/printf("have found operator.files\n");
			$$ = $1;
		}
		;

operator.file.writer:
		  FILEWRITER '(' IDENTIFIER ')' operator.arguments ';'
		{
			$$ = MakeFileWriterOperator($3, $5, $1);
		}
		;

operator.add:
		  ADD operator.default.call	//add composite-call(real-param[i]);
		  {
			  $$ = MakeAddCoord($2,$1);
		  }
		| ADD operator.pipeline
		{
			$$ = MakeAddCoord($2,$1);
		}
		| ADD operator.splitjoin
		{
			$$ = MakeAddCoord($2,$1);
		}
		;

operator.pipeline:
		  PIPELINE lblock splitjoinPipeline.statement.list rblock      //add \B7\BDʽ
		{
			$$ = MakePipelineCoord(NULL,NULL,NULL,$3,$1);
		}
		| PIPELINE lblock  declaration.list splitjoinPipeline.statement.list rblock      //add \B7\BDʽ
		{
			$$ = MakePipelineCoord(NULL,NULL,$3,$4,$1);
		}
		;  

// \BB\A8\C0\A8\BA\C5\D6е\C4\CD\EA\D5\FB\BDṹΪ\A3\BA \C9\F9\C3\F7\D3\EF\BE\E4(\BF\C9ѡ) \B3\F5ʼ\BB\AF\D3\EF\BE䣨\BF\C9ѡ\A3\A9 split\D3\EF\BE\E4 splitjoinPipeline\D3\EF\BE\E4 join\D3\EF\BE\E4
operator.splitjoin:
		  SPLITJOIN lblock split.statement  splitjoinPipeline.statement.list  join.statement rblock  //add \B7\BDʽ add splitjoin{ }
		{
			$$ = MakeSplitJoinCoord(NULL,NULL,NULL,NULL,$3,$4,$5,$1);
		}
		| SPLITJOIN lblock declaration.list split.statement splitjoinPipeline.statement.list join.statement rblock  //add \B7\BDʽ add splitjoin{ }
		{
			$$ = MakeSplitJoinCoord(NULL,NULL,$3,NULL,$4,$5,$6,$1);
		}
	    | SPLITJOIN lblock declaration.list statement.list split.statement splitjoinPipeline.statement.list join.statement rblock  //add \B7\BDʽ add splitjoin{ }
		{
			$$ = MakeSplitJoinCoord(NULL,NULL,$3,$4,$5,$6,$7,$1);
		}
		;

split.statement:
		  SPLIT duplicate.statement 
		{
			///*DEBUG*/printf("have found SPLIT duplicate.statement \n");
			$$ = MakeSplitCoord($2, $1);
		}
		| SPLIT roundrobin.statement
		{
			///*DEBUG*/printf("have found SPLIT roundrobin.statement \n");
			$$ = MakeSplitCoord($2, $1);
		}
		;

splitjoinPipeline.statement.list:
		 statement  
		{
			$$ = MakeNewList($1);
		}
		| operator.add
		{
			$$ = MakeNewList($1);
		}
		| splitjoinPipeline.statement.list statement
		{
			$$ = AppendItem($1,$2);
		}
		| splitjoinPipeline.statement.list operator.add
		{
			$$ = AppendItem($1,$2);
		}
		;

join.statement:
		  JOIN roundrobin.statement
		{
			///*DEBUG*/printf("have found JOIN roundrobin.statement \n");
			$$ = MakeJoinCoord($2, $1);
		}
		;

roundrobin.statement:
		  ROUNDROBIN '(' ')' ';'
		{
			$$ = MakeRoundRobinCoord(NULL, $1);
		}
		| ROUNDROBIN '(' argument.expression.list ')' ';'
		{
			$$ = MakeRoundRobinCoord($3, $1);
		}
		;

duplicate.statement:
		  DUPLICATE '('  ')' ';'
		{
			$$ = MakeDuplicateCoord(NULL, $1);
		}
		| DUPLICATE '(' assignment.expression ')'  ';'
		{
			$$ = MakeDuplicateCoord($3, $1);
		}
		;

/********************-----------operator.default---------********************/
operator.default.call:
		  IDENTIFIER  operator.arguments ';' /*composite call(StreamIt style)*///operator.param.list \B2\BB\C4\DCΪ\BF\D5\D2\D4\C7\F8\B7ֺ\AF\CA\FD\B5\F7\D3\C3/*composite call*/
		{
			///*DEBUG*/printf("have found operator.default.call\n");
			$$ = MakeCompositeCallCoord(MakeCompositeIdCoord($1->u.id.text, $1->coord), 
										ModifyOperatorDeclArguments(MakeOperdclCoord(EMPTY_TQ, NULL, NULL, NULL, $1->coord), $2), 
										FALSE, $1->coord);
		}
		;

operator.arguments:
		  '(' argument.expression.list ')'
		{
			///*DEBUG*/printf("have found operator.param.list \n");
			$$ = $2;
		}
		| 
		  '(' ')'
		{
			///*DEBUG*/printf("have found operator.param.list-2 \n");
			$$ = NULL;
		}
		;
		 
/**********************----------operator.selfdefine.body definition----------*********************/
operator.selfdefine.body:		  /*node struct*/
		   lblock operator.selfdefine.body.init.opt  operator.selfdefine.body.work operator.selfdefine.body.window.list.opt rblock            
		 {
			 ///*DEBUG*/printf("have found operator.selfdefine.body\n");
			$$ = MakeOperBodyCoord(PrimVoid, NULL, $2, $3, $4,$1,$5);
		 }
		 | lblock declaration.list operator.selfdefine.body.init.opt  operator.selfdefine.body.work operator.selfdefine.body.window.list.opt rblock          
		 {
			 ///*DEBUG*/printf("have found operator.selfdefine.body\n");
			$$ = MakeOperBodyCoord(PrimVoid, $2, $3, $4, $5,$1,$6);
		 }
		 ;

operator.selfdefine.body.init.opt:/*list struct*/
		  /*empty*/{ $$ = NULL; }
		| INIT compound.statement
		{
			///*DEBUG*/printf("operator.selfdefine.logic.init.list.opt is not null!!!\n");
			$$ = $2;
		}
		;

operator.selfdefine.body.work:/*list struct*/
		  WORK compound.statement
		{
			///*DEBUG*/printf("have found WORK compound.statement\n");
			$$ = $2;
		}
		;

/**********************----------window definition----------*********************/
operator.selfdefine.body.window.list.opt:					 /*list struct*/
		  /*empty*/{ $$ = NULL; }
		  | WINDOW '{' operator.selfdefine.window.list '}'
		{ 
			$$ = $3; /*MakeWindowCoord*/
		}
		;

operator.selfdefine.window.list:		 /*list struct*/
		  operator.selfdefine.window
		{ 
			$$ = MakeNewList($1);
		}
		| operator.selfdefine.window.list operator.selfdefine.window
		{ 
			$$ = AppendItem($1,$2);
		}
		;

operator.selfdefine.window:			 /*node struct*/
		  IDENTIFIER window.type ';' 
		{ 
			///*DEBUG*/printf("have found IDENTIFIER ':' window.type ';'\n");
			$$ = MakeWindowCoord(LookupStreamIdsNode($1), $2, $2->coord); 
		}
		;

window.type:				 /*node struct*/
		  SLIDING '('  ')'  
		{
			///*DEBUG*/printf("have found SLIDING ',' window.eviction\n");
			$$ = MakeWindowSlidingCoord(EMPTY_TQ, NULL, $2);
		}
		| TUMBLING '('  ')'                  
		{ 
			///*DEBUG*/printf("have found TUMBLING ',' window.trigger\n");
			$$ = MakeWindowTumbingCoord(EMPTY_TQ,NULL, $2); 
		}
		| SLIDING '(' expression ')'  
		{
			///*DEBUG*/printf("have found SLIDING ',' window.eviction\n");
			$$ = MakeWindowSlidingCoord(EMPTY_TQ, $3, $2);
		}
		| TUMBLING '(' expression ')'                  
		{ 
			///*DEBUG*/printf("have found TUMBLING ',' window.trigger\n");
			$$ = MakeWindowTumbingCoord(EMPTY_TQ, $3, $2); 
		}
		;

/*************************************************************************/
/*                                                                       */
/*                      SPL Grammar Definition----over                   */
/*                                                                       */
/*************************************************************************/


/********************************************************************************
*										*
*                                EXPRESSIONS                                    *
*										*
********************************************************************************/

primary.expression:             /* P */ /* 6.3.1 EXTENDED */  
          /* A typedef name cannot be used as a variable.  Fill in type later */
         IDENTIFIER           { $$ = $1; }
        | constant
        | string.literal.list
        | '(' expression ')'    { if ($2->typ == Comma) $2->coord = $1;
                                  $2->parenthesized = TRUE;
                                  $$ = $2; }
          /* GCC-inspired non ANSI-C extension */
        | '(' lblock statement.list rblock ')'
            { if (ANSIOnly)
	         SyntaxError("statement expressions not allowed with -ansi switch");
               $$ = MakeBlockCoord(NULL, NULL, GrabPragmas($3), $1, $4); }
        | '(' lblock declaration.list statement.list rblock ')'
            { if (ANSIOnly)
	         SyntaxError("statement expressions not allowed with -ansi switch");
              $$ = MakeBlockCoord(NULL, $3, GrabPragmas($4), $1, $5); }
        ;

postfix.expression:             /* P */ /* 6.3.2 CLARIFICATION */
          primary.expression
        | postfix.expression '[' expression ']'
            { $$ = ExtendArray($1, $3, $2); }
        | postfix.expression '(' ')'
            { $$ = MakeCallCoord($1, NULL, $2); }
        | postfix.expression '(' argument.expression.list ')'  //\BA\AF\CA\FD\B5\F7\D3\C3\D0\CEʽ\A3\A1
            { $$ = MakeCallCoord($1, $3, $2); }
//++++++++sql \D0\C2\CEķ\A8+++++++++++++++++++**********************************************************************************
		| postfix.expression '('  ')' operator.selfdefine.body //\C4\DA\D6\C3operator\A3\A1
		{
			$1 = ModifyDeclType(ConvertIdToDecl($1, EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ,NULL, NULL, NULL, $1->coord) );
			$1 = SetDeclType($1, MakeDefaultPrimType(EMPTY_TQ, $1->coord), Redecl) ;
			$1 = DefineOperator($1);
			$$ = SetOperatorBody($1, $4);
		}
		| postfix.expression '(' argument.expression.list ')' operator.selfdefine.body //\C4\DA\D6\C3operator\A3\A1
		{
			$1 = ModifyDeclType(ConvertIdToDecl($1, EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ,NULL, $3, NULL, $1->coord) );
			$1 = SetDeclType($1, MakeDefaultPrimType(EMPTY_TQ, $1->coord), Redecl) ;
			$1 = DefineOperator($1);
			$$ = SetOperatorBody($1, $5);
		}
		| postfix.expression '('  ')' operator.arguments 
		{
			$1 = ModifyDeclType(ConvertIdToDecl($1, EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ, NULL, NULL, NULL, $1->coord));

			///*DEBUG*/printf("have found composite template call\n");
			$$ = MakeCompositeCallCoord(MakeCompositeIdCoord($1->u.decl.name, $1->coord), ModifyOperatorDeclArguments($1->u.decl.type, $4), TRUE, $1->coord);
		}
		| postfix.expression '(' argument.expression.list ')' operator.arguments 
		{
			$1 = ModifyDeclType(ConvertIdToDecl($1, EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ, NULL, $3, NULL, $1->coord));

			///*DEBUG*/printf("have found composite template call\n");
			$$ = MakeCompositeCallCoord(MakeCompositeIdCoord($1->u.decl.name, $1->coord), ModifyOperatorDeclArguments($1->u.decl.type, $5), TRUE, $1->coord);
		}
		| SPLITJOIN '(' IDENTIFIER ')'  lblock split.statement  splitjoinPipeline.statement.list  join.statement rblock
		{
			$$ = MakeSplitJoinCoord(NULL, LookupStreamIdsNode($3),NULL,NULL,$6,GrabPragmas($7),$8,$1);
		}
		| SPLITJOIN '(' IDENTIFIER ')'  lblock  declaration.list split.statement  splitjoinPipeline.statement.list  join.statement rblock 
		{
			$$ = MakeSplitJoinCoord(NULL, LookupStreamIdsNode($3),$6,NULL,$7,GrabPragmas($8),$9,$1);
		}
		| SPLITJOIN '(' IDENTIFIER ')'  lblock declaration.list statement.list split.statement splitjoinPipeline.statement.list  join.statement rblock 
		{
			$$ = MakeSplitJoinCoord(NULL, LookupStreamIdsNode($3),$6,$7,$8,GrabPragmas($9),$10,$1);
		}
		|  PIPELINE '(' IDENTIFIER ')' lblock  splitjoinPipeline.statement.list rblock //pipelineΪ\B5\A5\CA\E4\C8뵥\CA\E4\B3\F6\BDṹ
		{
			$$ = MakePipelineCoord(NULL, LookupStreamIdsNode($3), NULL,$6, $1);
		}
		|  PIPELINE '(' IDENTIFIER ')' lblock declaration.list splitjoinPipeline.statement.list rblock //pipelineΪ\B5\A5\CA\E4\C8뵥\CA\E4\B3\F6\BDṹ
		{
			$$ = MakePipelineCoord(NULL, LookupStreamIdsNode($3), $6,$7, $1);
		}
		|  FILEREADER '(' ')' operator.arguments
		{
			$$ = MakeFileReaderOperator(NULL,$4,$1);
		}

//+++++++++++++++++++++++++++*************************************************************************
        | postfix.expression '.' IDENTIFIER
            { $$ = MakeBinopCoord('.', $1, $3, $2); }
        | postfix.expression ARROW IDENTIFIER
            { $$ = MakeBinopCoord(ARROW, $1, $3, $2); }
        | postfix.expression ICR
            { $$ = MakeUnaryCoord(POSTINC, $1, $2); }
        | postfix.expression DECR
            { $$ = MakeUnaryCoord(POSTDEC, $1, $2); }

          /* EXTENSION: TYPEDEFname can be used to name a struct/union field */
        | postfix.expression '.'   TYPEDEFname
            { $$ = MakeBinopCoord('.', $1, $3, $2); }
        | postfix.expression ARROW TYPEDEFname
            { $$ = MakeBinopCoord(ARROW, $1, $3, $2); }
        ;

argument.expression.list:       /* P */ /* 6.3.2 */
          assignment.expression
            { $$ = MakeNewList($1); }
        | argument.expression.list ',' assignment.expression
            { $$ = AppendItem($1, $3); }
        ;

unary.expression:               /* P */ /* 6.3.3 */
          postfix.expression
		  { $$ = LookupPostfixExpression($1); }
        | ICR unary.expression
            { $$ = MakeUnaryCoord(PREINC, $2, $1); }
        | DECR unary.expression
            { $$ = MakeUnaryCoord(PREDEC, $2, $1); }
        | unary.operator cast.expression
            { $1->u.unary.expr = $2;
              $$ = $1; }
        | SIZEOF unary.expression
            { $$ = MakeUnaryCoord(SIZEOF, $2, $1); }
        | SIZEOF '(' type.name ')'
            { $$ = MakeUnaryCoord(SIZEOF, $3, $1); }
        ;

unary.operator:                 /* P */ /* 6.3.3 */
          '&'     { $$ = MakeUnaryCoord('&', NULL, $1); }
        | '*'     { $$ = MakeUnaryCoord('*', NULL, $1); }
        | '+'     { $$ = MakeUnaryCoord('+', NULL, $1); }
        | '-'     { $$ = MakeUnaryCoord('-', NULL, $1); }
        | '~'     { $$ = MakeUnaryCoord('~', NULL, $1); }
        | '!'     { $$ = MakeUnaryCoord('!', NULL, $1); }
        ;

cast.expression:                /* P */ /* 6.3.4 */
          unary.expression
        | '(' type.name ')' cast.expression  { $$ = MakeCastCoord($2, $4, $1); }
        ;

multiplicative.expression:      /* P */ /* 6.3.5 */
          cast.expression
        | multiplicative.expression '*' cast.expression
            { $$ = MakeBinopCoord('*', $1, $3, $2); }
        | multiplicative.expression '/' cast.expression
            { $$ = MakeBinopCoord('/', $1, $3, $2); }
        | multiplicative.expression '%' cast.expression
            { $$ = MakeBinopCoord('%', $1, $3, $2); }
        ;

additive.expression:            /* P */ /* 6.3.6 */
          multiplicative.expression
        | additive.expression '+' multiplicative.expression
            { $$ = MakeBinopCoord('+', $1, $3, $2); }
        | additive.expression '-' multiplicative.expression
            { $$ = MakeBinopCoord('-', $1, $3, $2); }
        ;

shift.expression:               /* P */ /* 6.3.7 */
          additive.expression
        | shift.expression LS additive.expression
            { $$ = MakeBinopCoord(LS, $1, $3, $2); }
        | shift.expression RS additive.expression
            { $$ = MakeBinopCoord(RS, $1, $3, $2); }
        ;

relational.expression:          /* P */ /* 6.3.8 */
          shift.expression
        | relational.expression '<' shift.expression
            { $$ = MakeBinopCoord('<', $1, $3, $2); }
        | relational.expression '>' shift.expression
            { $$ = MakeBinopCoord('>', $1, $3, $2); }
        | relational.expression LE shift.expression
            { $$ = MakeBinopCoord(LE, $1, $3, $2); }
        | relational.expression GE shift.expression
            { $$ = MakeBinopCoord(GE, $1, $3, $2); }
        ;

equality.expression:            /* P */ /* 6.3.9 */
          relational.expression
        | equality.expression EQ relational.expression
            { $$ = MakeBinopCoord(EQ, $1, $3, $2); }
        | equality.expression NE relational.expression
            { $$ = MakeBinopCoord(NE, $1, $3, $2); }
        ;

AND.expression:                 /* P */ /* 6.3.10 */
          equality.expression
        | AND.expression '&' equality.expression
            { $$ = MakeBinopCoord('&', $1, $3, $2); }
        ;

exclusive.OR.expression:        /* P */ /* 6.3.11 */
          AND.expression
        | exclusive.OR.expression '^' AND.expression
            { 
              WarnAboutPrecedence('^', $1);
              WarnAboutPrecedence('^', $3);
	      $$ = MakeBinopCoord('^', $1, $3, $2); }
        ;

inclusive.OR.expression:        /* P */ /* 6.3.12 */
          exclusive.OR.expression
        | inclusive.OR.expression '|' exclusive.OR.expression
            { WarnAboutPrecedence('|', $1);
              WarnAboutPrecedence('|', $3);
              $$ = MakeBinopCoord('|', $1, $3, $2); }
        ;

logical.AND.expression:         /* P */ /* 6.3.13 */
          inclusive.OR.expression
        | logical.AND.expression ANDAND inclusive.OR.expression
            { $$ = MakeBinopCoord(ANDAND, $1, $3, $2); }
        ;

logical.OR.expression:          /* P */ /* 6.3.14 */
          logical.AND.expression
        | logical.OR.expression OROR logical.AND.expression
            { WarnAboutPrecedence(OROR, $1);
              WarnAboutPrecedence(OROR, $3);
              $$ = MakeBinopCoord(OROR, $1, $3, $2); }
        ;

conditional.expression:         /* P */ /* 6.3.15 */
          logical.OR.expression
        | logical.OR.expression '?' expression ':' conditional.expression
            { $$ = MakeTernaryCoord($1, $3, $5, $2, $4); }
        ;

assignment.expression:          
          conditional.expression
        | unary.expression assignment.operator assignment.expression
            { 
				if($2->u.binop.op == '='){
					if($3->typ == Operator_ || $3->typ == CompositeCall || $3->typ == Pipeline || $3->typ == SplitJoin){
						$$ = MakeOutput($3,$1);
					}else{
						$2->u.binop.left = $1;
						$2->u.binop.right = $3;
						$$ = $2;
					}
				}else{
					$2->u.binop.left = $1;
					$2->u.binop.right = $3;
					$$ = $2;
				 }
		  }
        ;

assignment.operator:           
          '='             { $$ = MakeBinopCoord('=', NULL, NULL, $1); }
        | MULTassign      { $$ = MakeBinopCoord(MULTassign, NULL, NULL, $1);  }
        | DIVassign       { $$ = MakeBinopCoord(DIVassign, NULL, NULL, $1);   }
        | MODassign       { $$ = MakeBinopCoord(MODassign, NULL, NULL, $1);   }
        | PLUSassign      { $$ = MakeBinopCoord(PLUSassign, NULL, NULL, $1);  }
        | MINUSassign     { $$ = MakeBinopCoord(MINUSassign, NULL, NULL, $1); }
        | LSassign        { $$ = MakeBinopCoord(LSassign, NULL, NULL, $1);    }
        | RSassign        { $$ = MakeBinopCoord(RSassign, NULL, NULL, $1);    }
        | ANDassign       { $$ = MakeBinopCoord(ANDassign, NULL, NULL, $1);   }
        | ERassign        { $$ = MakeBinopCoord(ERassign, NULL, NULL, $1);    }
        | ORassign        { $$ = MakeBinopCoord(ORassign, NULL, NULL, $1);    }
        ;

expression:                     /* P */ /* 6.3.17 */
          assignment.expression
        | expression ',' assignment.expression
         {  
              if ($1->typ == Comma) 
              {
				 AppendItem($1->u.comma.exprs, $3);
				 $$ = $1;
			  }
              else
             {
				  $$ = MakeCommaCoord(AppendItem(MakeNewList($1), $3), $1->coord);
			 }
	    }
        ;

constant.expression:            /* P */ /* 6.4   */
          conditional.expression { $$ = $1; }
        ;

expression.opt:                 /* P */ /* For convenience */
          /* Nothing */   { $$ = (Node *) NULL; }
        | expression      { $$ = $1; }
        ;

/********************************************************************************
*										*
*                               DECLARATIONS					*
*										*
*    The following is different from the ANSI C specified grammar.  The changes *
* were made to disambiguate typedef's presence in declaration.specifiers        *
* (vs. in the declarator for redefinition) to allow struct/union/enum tag       *
* declarations without declarators, and to better reflect the parsing of        *
* declarations (declarators must be combined with declaration.specifiers ASAP   *
* so that they are visible in scope).					        *
*										*
* Example of typedef use as either a declaration.specifier or a declarator:	*
*										*
*   typedef int T;								*
*   struct S { T T; }; / * redefinition of T as member name * /			*
*										*
* Example of legal and illegal statements detected by this grammar:		*
*										*
*   int;              / * syntax error: vacuous declaration      * /		*
*   struct S;         / * no error: tag is defined or elaborated * /		*
*										*
* Example of result of proper declaration binding:				*
*										*
*   /* Declare "a" with a type in the name space BEFORE parsing initializer * / *
*   int a = sizeof(a);								*
*										*
*   /* Declare "b" with a type before parsing "c" * /				*
*   int b, c[sizeof(b)];							*
*										*
********************************************************************************/

/*                        */    /* ? */ /* ?.?.? */
declaration: /*P*/
          declaring.list ';'
            { $$ = $1; }
        | default.declaring.list ';'
            { $$ = $1; }
        | sue.declaration.specifier ';'
            { $$ = MakeNewList(ForceNewSU($1, $2)); }
        | sue.type.specifier ';'
            { $$ = MakeNewList(ForceNewSU($1, $2)); }
        ;

/*                        */    /* ? */ /* ?.?.? */
declaring.list: /*P*/
          declaration.specifier declarator
            { 
	      SetDeclType($2, $1, Redecl);
	    }
          attributes.opt { SetDeclAttribs($2, $4); }
          initializer.opt
            { 
              $$ = MakeNewList(SetDeclInit($2, $6)); 
            }
		| type.specifier declarator                       /*main SPL's tuple add here too*/
            { 
              SetDeclType($2, $1, Redecl);
            }
          attributes.opt { SetDeclAttribs($2, $4); }
          initializer.opt
            { 
              $$ = MakeNewList(SetDeclInit($2, $6)); 
			}
        | declaring.list ',' declarator                 /*spl like this*/
            { 
	      $<L>$ = AppendDecl($1, $3, Redecl);
			 }
          attributes.opt { SetDeclAttribs($3, $5); }
          initializer.opt
            { 
              SetDeclInit($3, $7); 
            }


        /******** ERROR PRODUCTIONS ********/
        | /* error production: catch missing identifier */
          declaration.specifier error
            { 
              SyntaxError("declaration without a variable"); 
            }
          attributes.opt
          initializer.opt
            { 
              $$ = NULL; /* empty list */ 
            }
        | /* error production: catch missing identifier */
          type.specifier error
            { 
              SyntaxError("declaration without a variable"); 
            }
          attributes.opt
          initializer.opt
            { 
              $$ = NULL; /* empty list */ 
            }
        | declaring.list ',' error
        ;

/*                        */    /* ? */ /* ?.?.? */
/* Note that if a typedef were redeclared, then a decl-spec must be supplied */
default.declaring.list:  /*P*/ /* Can't  redeclare typedef names */
          declaration.qualifier.list identifier.declarator
            { 
              SetDeclType($2, MakeDefaultPrimType($1.tq, $1.coord), NoRedecl);
            }
          attributes.opt { SetDeclAttribs($2, $4); }
	  initializer.opt
            { 
              $$ = MakeNewList(SetDeclInit($2, $6)); 
            }
	  | stream.type.specifier identifier.declarator
		{
			SetDeclType($2, $1, Redecl);
            $$ = MakeNewList(SetDeclInit($2, NULL)); 
		}
        | type.qualifier.list identifier.declarator
            { 
              SetDeclType($2, MakeDefaultPrimType($1.tq, $1.coord), NoRedecl);
            }
          attributes.opt { SetDeclAttribs($2, $4); }
	  initializer.opt
            { 
              $$ = MakeNewList(SetDeclInit($2, $6)); 
	    }
        | default.declaring.list ',' identifier.declarator
            { $<L>$ = AppendDecl($1, $3, NoRedecl); }
          attributes.opt { SetDeclAttribs($3, $5); }
	  initializer.opt
            { SetDeclInit($3, $7); }

        /********  ERROR PRODUCTIONS ********/
        | /* error production: catch missing identifier */
          declaration.qualifier.list error
            { 
              SyntaxError("declaration without a variable"); 
	    }
          attributes.opt
          initializer.opt
            { 
              $$ = NULL; /* empty list */ 
	    }
        | /* error production: catch missing identifier */
          type.qualifier.list error
            { 
              SyntaxError("declaration without a variable"); 
	    }
          attributes.opt
          initializer.opt
            { 
              $$ = NULL; /* empty list */ 
            }
        | default.declaring.list ',' error
        ;

/*                        */    /* ? */ /* ?.?.? */
declaration.specifier: /*P*/
          basic.declaration.specifier        /* Arithmetic or void */
            { $$ = FinishPrimType($1); }
        | sue.declaration.specifier          /* struct/union/enum  */
        | typedef.declaration.specifier      /* typedef            */
        ;

/*                        */    /* ? */ /* ?.?.? */
/* StorageClass + Arithmetic or void */
basic.declaration.specifier:  /*P*/
          basic.type.specifier storage.class
            { $$ = TypeQualifyNode($1, $2.tq); } 
        | declaration.qualifier.list basic.type.name
            { $$ = TypeQualifyNode($2, $1.tq); $$->coord = $1.coord; }
        | basic.declaration.specifier declaration.qualifier
            { $$ = TypeQualifyNode($1, $2.tq); }
        | basic.declaration.specifier basic.type.name
            { $$ = MergePrimTypes($1, $2); }
        ;

/*                        */    /* ? */ /* ?.?.? */
/* StorageClass + struct/union/enum */
sue.declaration.specifier: /*P*/   
          sue.type.specifier storage.class
            { $$ = TypeQualifyNode($1, $2.tq); }
        | declaration.qualifier.list elaborated.type.name
            { $$ = TypeQualifyNode($2, $1.tq); $$->coord = $1.coord; }
        | sue.declaration.specifier declaration.qualifier
            { $$ = TypeQualifyNode($1, $2.tq); }
        ;

/*                        */    /* ? */ /* ?.?.? */
/* Storage Class + typedef types */
typedef.declaration.specifier:  /*P*/      
          typedef.type.specifier storage.class
            { $$ = TypeQualifyNode($1, $2.tq); }
        | declaration.qualifier.list TYPEDEFname
            { $$ = ConvertIdToTdef($2, $1.tq, GetTypedefType($2)); $$->coord = $1.coord; }
        | typedef.declaration.specifier declaration.qualifier
            { $$ = TypeQualifyNode($1, $2.tq); }
        ;

/*                        */    /* ? */ /* ?.?.? */
/* Type qualifier AND storage class */
declaration.qualifier.list:  /*P*/
          storage.class
        | type.qualifier.list storage.class
            { $$.tq = MergeTypeQuals($1.tq, $2.tq, $2.coord);
              $$.coord = $1.coord; }
        | declaration.qualifier.list declaration.qualifier
            { $$.tq = MergeTypeQuals($1.tq, $2.tq, $2.coord);
              $$.coord = $1.coord; }
        ;

/*                        */    /* ? */ /* ?.?.? */
declaration.qualifier: /*P*/
          type.qualifier
        | storage.class
        ;

/*                        */    /* ? */ /* ?.?.? */
type.specifier: /*P*/
          basic.type.specifier               /* Arithmetic or void */
            { $$ = FinishPrimType($1); }
        | sue.type.specifier                 /* Struct/Union/Enum  */
        | typedef.type.specifier             /* Typedef            */
        ;

/*                        */    /* ? */ /* ?.?.? */
basic.type.specifier: /*P*/
          basic.type.name            /* Arithmetic or void */
        | type.qualifier.list basic.type.name
            { $$ = TypeQualifyNode($2, $1.tq); $$->coord = $1.coord; }
        | basic.type.specifier type.qualifier
            { $$ = TypeQualifyNode($1, $2.tq); }
        | basic.type.specifier basic.type.name
            { $$ = MergePrimTypes($1, $2); }
        ;

/*                        */    /* ? */ /* ?.?.? */
sue.type.specifier: /*P*/
          elaborated.type.name              /* struct/union/enum */
        | type.qualifier.list elaborated.type.name
            { $$ = TypeQualifyNode($2, $1.tq); $$->coord = $1.coord; }
        | sue.type.specifier type.qualifier
            { $$ = TypeQualifyNode($1, $2.tq); }
        ;

/*                        */    /* ? */ /* ?.?.? */
/* typedef types */
typedef.type.specifier:  /*P*/             
          TYPEDEFname
            { $$ = ConvertIdToTdef($1, EMPTY_TQ, GetTypedefType($1)); }
        | type.qualifier.list TYPEDEFname
            { $$ = ConvertIdToTdef($2, $1.tq, GetTypedefType($2)); $$->coord = $1.coord; }
        | typedef.type.specifier type.qualifier
            { $$ = TypeQualifyNode($1, $2.tq); }
        ;

/*                        */    /* ? */ /* ?.?.? */
type.qualifier.list: /*P*/
          type.qualifier
        | type.qualifier.list type.qualifier
            { $$.tq = MergeTypeQuals($1.tq, $2.tq, $2.coord);
              $$.coord = $1.coord; }
        ;

pointer.type.qualifier.list:
          pointer.type.qualifier
        | pointer.type.qualifier.list pointer.type.qualifier
            { $$.tq = MergeTypeQuals($1.tq, $2.tq, $2.coord);
              $$.coord = $1.coord; }
        ;

/*                        */    /* ? */ /* ?.?.? */
elaborated.type.name: /*P*/
          struct.or.union.specifier
        | enum.specifier
        ;

/*                        */    /* ? */ /* ?.?.? */
declarator: /*P*/
          paren.typedef.declarator       /* would be ambiguous as parameter */
        | parameter.typedef.declarator   /* not ambiguous as param          */
        | identifier.declarator
        | old.function.declarator
            {
	      Warning(2, "function prototype parameters must have types");
              $$ = AddDefaultParameterTypes($1);
            }
        ;

/*                        */    /* ? */ /* ?.?.? */
/* Redundant '(' placed immediately to the left of the TYPEDEFname  */
paren.typedef.declarator: /*P*/
          paren.postfix.typedef.declarator
        | '*' paren.typedef.declarator
            { $$ = SetDeclType($2, MakePtrCoord(EMPTY_TQ, NULL, $1), Redecl);
               }
        | '*' '(' simple.paren.typedef.declarator ')' 
            { $$ = SetDeclType($3, MakePtrCoord(EMPTY_TQ, NULL, $1), Redecl); 
               }
        | '*' pointer.type.qualifier.list '(' simple.paren.typedef.declarator ')' 
            { $$ = SetDeclType($4, MakePtrCoord(   $2.tq,    NULL, $1), Redecl);
               }
        | '*' pointer.type.qualifier.list paren.typedef.declarator
            { $$ = SetDeclType($3, MakePtrCoord(   $2.tq,    NULL, $1), Redecl); 
               }
        ;
        
/*                        */    /* ? */ /* ?.?.? */
/* Redundant '(' to left of TYPEDEFname */
paren.postfix.typedef.declarator: /*P*/ 
          '(' paren.typedef.declarator ')'
            { $$ = $2;  
              }
        | '(' simple.paren.typedef.declarator postfixing.abstract.declarator ')'
            { $$ = ModifyDeclType($2, $3); 
               }
        | '(' paren.typedef.declarator ')' postfixing.abstract.declarator
            { $$ = ModifyDeclType($2, $4); 
               }
        ;

/*                        */    /* ? */ /* ?.?.? */
simple.paren.typedef.declarator: /*P*/
          TYPEDEFname
            { $$ = ConvertIdToDecl($1, EMPTY_TQ, NULL, NULL, NULL); }
        | '(' simple.paren.typedef.declarator ')'
            { $$ = $2;  
               }
        ;

/*                        */    /* ? */ /* ?.?.? */
parameter.typedef.declarator: /*P*/
          TYPEDEFname 
            { $$ = ConvertIdToDecl($1, EMPTY_TQ, NULL, NULL, NULL); }
        | TYPEDEFname postfixing.abstract.declarator
            { $$ = ConvertIdToDecl($1, EMPTY_TQ, $2, NULL, NULL);   }
        | clean.typedef.declarator
        ;

/*
   The  following have at least one '*'. There is no (redundant) 
   '(' between the '*' and the TYPEDEFname. 
*/
/*                        */    /* ? */ /* ?.?.? */
clean.typedef.declarator: /*P*/
          clean.postfix.typedef.declarator
        | '*' parameter.typedef.declarator
            { $$ = SetDeclType($2, MakePtrCoord(EMPTY_TQ, NULL, $1), Redecl); 
               }
        | '*' pointer.type.qualifier.list parameter.typedef.declarator  
            { $$ = SetDeclType($3, MakePtrCoord($2.tq, NULL, $1), Redecl); 
               }
        ;

/*                        */    /* ? */ /* ?.?.? */
clean.postfix.typedef.declarator: /*P*/
          '(' clean.typedef.declarator ')'
            { $$ = $2; 
               }
        | '(' clean.typedef.declarator ')' postfixing.abstract.declarator
            { $$ = ModifyDeclType($2, $4); 
               }
        ;

/*                        */    /* ? */ /* ?.?.? */
abstract.declarator: /*P*/
          unary.abstract.declarator
        | postfix.abstract.declarator
        | postfixing.abstract.declarator
        ;

/*                        */    /* ? */ /* ?.?.? */
unary.abstract.declarator: /*P*/
          '*' 
            { $$ = MakePtrCoord(EMPTY_TQ, NULL, $1); }
        | '*' pointer.type.qualifier.list 
            { $$ = MakePtrCoord($2.tq, NULL, $1); }
        | '*' abstract.declarator
            { $$ = SetBaseType($2, MakePtrCoord(EMPTY_TQ, NULL, $1)); 
               }
        | '*' pointer.type.qualifier.list abstract.declarator
            { $$ = SetBaseType($3, MakePtrCoord($2.tq, NULL, $1)); 
               }
        ;

/*                        */    /* ? */ /* ?.?.? */
postfix.abstract.declarator: /*P*/
          '(' unary.abstract.declarator ')'
            { $$ = $2; 
               }
        | '(' postfix.abstract.declarator ')'
            { $$ = $2; 
               }
        | '(' postfixing.abstract.declarator ')'
            { $$ = $2; 
               }
        | '(' unary.abstract.declarator ')' postfixing.abstract.declarator
            { $$ = SetBaseType($2, $4); 
               }
        ;

/*                        */    /* ? */ /* ?.?.? */
postfixing.abstract.declarator: /*P*/
          array.abstract.declarator     { $$ = $1;                   }
        | '(' ')'                       { $$ = MakeFdclCoord(EMPTY_TQ, NULL, NULL, $1); }
        | '(' parameter.type.list ')'   { $$ = MakeFdclCoord(EMPTY_TQ, $2, NULL, $1); }
        ;

/*                        */    /* ? */ /* ?.?.? */
identifier.declarator: /*P*/
          unary.identifier.declarator
        | paren.identifier.declarator
        ;

/*                        */    /* ? */ /* ?.?.? */
unary.identifier.declarator: /*P293*/
          postfix.identifier.declarator
        | '*' identifier.declarator
            { $$ = ModifyDeclType($2, MakePtrCoord(EMPTY_TQ, NULL, $1)); 
               }
        | '*' pointer.type.qualifier.list identifier.declarator
            { $$ = ModifyDeclType($3, MakePtrCoord(   $2.tq,    NULL, $1)); 
               }
        ;
        
/*                        */    /* ? */ /* ?.?.? */
postfix.identifier.declarator: /*P296*/
          paren.identifier.declarator postfixing.abstract.declarator/*\B6\E0ά\CA\FD\D7\E9\B9\E6Լ\B5\BD\D5\E2\C0\EF*/
            { $$ = ModifyDeclType($1, $2); }
        | '(' unary.identifier.declarator ')'
            { $$ = $2; 
               }
        | '(' unary.identifier.declarator ')' postfixing.abstract.declarator
            { $$ = ModifyDeclType($2, $4); 
               }
        ;

/*                        */    /* ? */ /* ?.?.? */
paren.identifier.declarator: /*P299*/
          IDENTIFIER
            { $$ = ConvertIdToDecl($1, EMPTY_TQ, NULL, NULL, NULL); }
        | '(' paren.identifier.declarator ')'
            { $$ = $2; 
               }
        ;

/*                        */    /* ? */ /* ?.?.? */
old.function.declarator: /*P301*/
          postfix.old.function.declarator
            { 
/*              OldStyleFunctionDefinition = TRUE; */
              $$ = $1; 
            }
        | '*' old.function.declarator
            { $$ = SetDeclType($2, MakePtrCoord(EMPTY_TQ, NULL, $1), SU); 
               }
        | '*' pointer.type.qualifier.list old.function.declarator
            { $$ = SetDeclType($3, MakePtrCoord($2.tq, NULL, $1), SU); 
               }
        ;

/*                        */    /* ? */ /* ?.?.? */
postfix.old.function.declarator: /*P*/
          paren.identifier.declarator '(' identifier.list ')'  
            { $$ = ModifyDeclType($1, MakeFdclCoord(EMPTY_TQ, $3, NULL, $2)); }
        | '(' old.function.declarator ')'
            { $$ = $2; 
               }
        | '(' old.function.declarator ')' postfixing.abstract.declarator
            { $$ = ModifyDeclType($2, $4); 
               }
        ;

/* 
    ANSI C section 3.7.1 states  

      "An identifier declared as a typedef name shall not be redeclared 
       as a parameter".  

    Hence the following is based only on IDENTIFIERs 
*/
/*                        */    /* ? */ /* ?.?.? */
identifier.list: /*P*/ /* only used by postfix.old.function.declarator */
          IDENTIFIER
            { $$ = MakeNewList($1); }
        | identifier.list ',' IDENTIFIER
            { $$ = AppendItem($1, $3); }
        ;

/*                        */    /* ? */ /* ?.?.? */
identifier.or.typedef.name: /*P*/
          IDENTIFIER
        | TYPEDEFname
        ;

/*                        */    /* ? */ /* ?.?.? */
type.name: /*P*/
          type.specifier
            { $$ = FinishType($1); }
        | type.specifier abstract.declarator
            { $$ = FinishType(SetBaseType($2, $1)); }
        | type.qualifier.list /* DEFAULT_INT */
            { $$ = MakeDefaultPrimType($1.tq, $1.coord); }
        | type.qualifier.list /* DEFAULT_INT */ abstract.declarator
	    { $$ = SetBaseType($2, MakeDefaultPrimType($1.tq, $1.coord)); }
        ;


/* Productions for __attribute__ adapted from the original in gcc 2.6.3. */

attributes.opt:
      /* empty */
  		{ $$ = NULL; }
	| attributes
		{ $$ = $1; }
	;

attributes:
      attribute
		{ $$ = $1; }
	| attributes attribute
		{ $$ = JoinLists ($1, $2); }
	;

attribute:
      ATTRIBUTE '(' '(' attribute.list ')' ')'
		{ if (ANSIOnly)
	            SyntaxError("__attribute__ not allowed with -ansi switch");
                  $$ = $4; }
	;

attribute.list:
      attrib
		{ $$ = MakeNewList($1); }
	| attribute.list ',' attrib
		{ $$ = AppendItem($1, $3); }
	;
 
attrib:
    /* empty */
		{ $$ = NULL; }
	| any.word
		{ $$ = ConvertIdToAttrib($1, NULL); }
	| any.word '(' expression ')'
		{ $$ = ConvertIdToAttrib($1, $3); }
	;


any.word:
	  IDENTIFIER
	| TYPEDEFname
	| CONST { $$ = MakeIdCoord(UniqueString("const"), $1); }
	;

/*                        */    /* ? */ /* ?.?.? */
initializer.opt: /*P*/
          /* nothing */                  { $$ = NULL; }
        | '=' initializer                { $$ = $2; }
        ;

/*                        */    /* ? */ /* ?.?.? */
initializer: /*P*/
          '{' initializer.list '}'       { $$ = $2; $$->coord = $1; }
        | '{' initializer.list ',' '}'   { $$ = $2; $$->coord = $1; }
        | assignment.expression          { $$ = $1;}		
        ;

/*                        */    /* ? */ /* ?.?.? */
initializer.list: /*P*/
          initializer
            { $$ = MakeInitializerCoord(MakeNewList($1), $1->coord);}
        | initializer.list ',' initializer
            { 
              assert($1->typ == Initializer);
			  AppendItem($1->u.initializer.exprs, $3);
              $$ = $1; 
            }
        ;

/*                        */    /* ? */ /* ?.?.? */
parameter.type.list: /*P*/
          parameter.list
        | parameter.list ',' ELLIPSIS   { $$ = AppendItem($1, EllipsisNode); }

        /******** ERROR PRODUCTIONS (EAB) ********/
        | ELLIPSIS
            { Node *n = MakePrimCoord(EMPTY_TQ, Void, $1);
	      SyntaxErrorCoord(n->coord, "First argument cannot be `...'");
              $$ = MakeNewList(n);
            }
        ;

/*                        */    /* ? */ /* ?.?.? */
parameter.list: /*P*/
          parameter.declaration
            { $$ = MakeNewList($1); }
        | parameter.list ',' parameter.declaration
            { $$ = AppendItem($1, $3); }

        /******** ERROR PRODUCTIONS (EAB) ********/
        | parameter.declaration '=' initializer
            { 
	      SyntaxErrorCoord($1->coord, "formals cannot have initializers");
              $$ = MakeNewList($1); 
            }
        | parameter.list ',' error
            { $$ = $1; }
        ;

/*                        */    /* ? */ /* ?.?.? */
parameter.declaration: /*P*/
          declaration.specifier
            { $$ = $1; }
        | declaration.specifier abstract.declarator
            { $$ = SetBaseType($2, $1); 
            }
        | declaration.specifier identifier.declarator
            { $$ = SetDeclType($2, $1, Formal); 
            }
        | declaration.specifier parameter.typedef.declarator
            { $$ = SetDeclType($2, $1, Formal); 
            }
        | declaration.qualifier.list /* DEFAULT_INT */ 
            { $$ = MakeDefaultPrimType($1.tq, $1.coord); }
        | declaration.qualifier.list /* DEFAULT_INT */ abstract.declarator
            { $$ = SetBaseType($2, MakeDefaultPrimType($1.tq, $1.coord)); }
        | declaration.qualifier.list /* DEFAULT_INT */ identifier.declarator
            { $$ = SetDeclType($2, MakeDefaultPrimType($1.tq, $1.coord), Formal); }
        | type.specifier
            { $$ = $1; }
        | type.specifier abstract.declarator
            { $$ = SetBaseType($2, $1); 
            }
        | type.specifier identifier.declarator
            { $$ = SetDeclType($2, $1, Formal); 
            }
        | type.specifier parameter.typedef.declarator
            { $$ = SetDeclType($2, $1, Formal); 
            }
        | type.qualifier.list /* DEFAULT_INT */ 
            { $$ = MakeDefaultPrimType($1.tq, $1.coord); }
        | type.qualifier.list /* DEFAULT_INT */ abstract.declarator
            { $$ = SetBaseType($2, MakeDefaultPrimType($1.tq, $1.coord)); }
        | type.qualifier.list /* DEFAULT_INT */ identifier.declarator
            { $$ = SetDeclType($2, MakeDefaultPrimType($1.tq, $1.coord), Formal); }
        ;

/*                        */    /* ? */ /* ?.?.? */
array.abstract.declarator: /*P*/
          '[' ']'
            { $$ = MakeAdclCoord(EMPTY_TQ, NULL, NULL, $1); }
        | '[' constant.expression ']'
            { $$ = MakeAdclCoord(EMPTY_TQ, NULL, $2, $1); }
        | array.abstract.declarator '[' constant.expression ']'
            { $$ = SetBaseType($1, MakeAdclCoord(EMPTY_TQ, NULL, $3, $2)); }

        /******** ERROR PRODUCTIONS ********/
        | /* error production: catch empty dimension that isn't first */
          array.abstract.declarator '[' ']'
            { 
              SyntaxError("array declaration with illegal empty dimension");
              $$ = SetBaseType($1, MakeAdclCoord(EMPTY_TQ, NULL, SintOne, $2)); 
            }
        ;


/********************************************************************************
*										*
*                      STRUCTURES, UNION, and ENUMERATORS			*
*										*
********************************************************************************/

/*                        */    /* ? */ /* ?.?.? */
struct.or.union.specifier: /*P*/
          struct.or.union '{' struct.declaration.list '}'
            { 
              $$ = SetSUdclNameFields($1, NULL, $3, $2, $4);
            }
        | struct.or.union identifier.or.typedef.name
          '{' struct.declaration.list '}'
            { 
              $$ = SetSUdclNameFields($1, $2, $4, $3, $5);
	    }
        | struct.or.union identifier.or.typedef.name
            { 
              $$ = SetSUdclName($1, $2, $1->coord);
	    }
        /* EAB: create rules for empty structure declarations */
        | struct.or.union '{' '}'
            { 
              if (ANSIOnly)
                 Warning(1, "empty structure declaration");
              $$ = SetSUdclNameFields($1, NULL, NULL, $2, $3); 
	    }
        | struct.or.union identifier.or.typedef.name '{' '}'
            { 
              if (ANSIOnly)
                 Warning(1, "empty structure declaration");
              $$ = SetSUdclNameFields($1, $2, NULL, $3, $4); 
	    }
        ;

/*                        */    /* ? */ /* ?.?.? */
struct.or.union: /*P*/ 
          STRUCT   { $$ = MakeSdclCoord(EMPTY_TQ, NULL, $1); }
        | UNION    { $$ = MakeUdclCoord(EMPTY_TQ, NULL, $1); }
        ;

/*                        */    /* ? */ /* ?.?.? */
struct.declaration.list: /*P*/
          struct.declaration
        | struct.declaration.list struct.declaration
            { $$ = JoinLists($1, $2); }
        ;

/*                        */    /* ? */ /* ?.?.? */
struct.declaration: /*P*/
          struct.declaring.list ';'
        | struct.default.declaring.list ';'
        ;

/* doesn't redeclare typedef */
/*                        */    /* ? */ /* ?.?.? */
struct.default.declaring.list: /*P*/        
          type.qualifier.list struct.identifier.declarator
            { 
	      $$ = MakeNewList(SetDeclType($2,
					    MakeDefaultPrimType($1.tq, $1.coord),
					    SU)); 
	    }
        | struct.default.declaring.list ',' struct.identifier.declarator
            { $$ = AppendDecl($1, $3, SU); }
        ;

/*                        */    /* ? */ /* ?.?.? */
struct.declaring.list:  /*P*/       
          type.specifier struct.declarator
            { $$ = MakeNewList(SetDeclType($2, $1, SU)); }
        | struct.declaring.list ',' struct.declarator
            { $$ = AppendDecl($1, $3, SU); }
        ;


/*                        */    /* ? */ /* ?.?.? */
struct.declarator: /*P*/
          declarator bit.field.size.opt attributes.opt
            { SetDeclAttribs($1, $3);
              $$ = SetDeclBitSize($1, $2); }
        | bit.field.size attributes.opt
            { $$ = MakeDeclCoord(NULL, EMPTY_TQ, NULL, NULL, $1, $1->coord);
              SetDeclAttribs($$, $2); }
        ;

/*                        */    /* ? */ /* ?.?.? */
struct.identifier.declarator: /*P*/
          identifier.declarator bit.field.size.opt attributes.opt
            { $$ = SetDeclBitSize($1, $2);
              SetDeclAttribs($1, $3); }
        | bit.field.size attributes.opt
            { $$ = MakeDeclCoord(NULL, EMPTY_TQ, NULL, NULL, $1, $1->coord);
              SetDeclAttribs($$, $2); }
        ;

/*                        */    /* ? */ /* ?.?.? */
bit.field.size.opt: /*P*/
          /* nothing */   { $$ = NULL; }
        | bit.field.size
        ;

/*                        */    /* ? */ /* ?.?.? */
bit.field.size: /*P*/
          ':' constant.expression { $$ = $2; }
        ;

/*                        */    /* ? */ /* ?.?.? */
enum.specifier: /*P*/
          ENUM '{' enumerator.list comma.opt '}'
            { $$ = BuildEnum(NULL, $3, $1, $2, $5); }
        | ENUM identifier.or.typedef.name '{' enumerator.list comma.opt '}'
            { $$ = BuildEnum($2, $4, $1, $3, $6);   }
        | ENUM identifier.or.typedef.name
            { $$ = BuildEnum($2, NULL, $1, $2->coord, $2->coord); }
        ;

/*                        */    /* ? */ /* ?.?.? */
enumerator.list: /*P*/
          identifier.or.typedef.name enumerator.value.opt
            { $$ = MakeNewList(BuildEnumConst($1, $2)); }
        | enumerator.list ',' identifier.or.typedef.name enumerator.value.opt
            { $$ = AppendItem($1, BuildEnumConst($3, $4)); }
        ;

/*                        */    /* ? */ /* ?.?.? */
enumerator.value.opt: /*P*/
          /* Nothing */               { $$ = NULL; }
        | '=' constant.expression     { $$ = $2;   }
        ;

comma.opt: /* not strictly ANSI */
          /* Nothing */    { }
        | ','              { }
        ;

/********************************************************************************
*										*
*                                  STATEMENTS					*
*										*
********************************************************************************/
costream.composite.statement: 
          composite.body.operator
	    | statement
		;
statement:                      /* P */ /* 6.6   */
          labeled.statement
        | compound.statement
        | expression.statement
        | selection.statement
        | iteration.statement
        | jump.statement
          /******** ERROR PRODUCTIONS ********/
        | error ';'
           {  $$ = NULL; }
        ;

labeled.statement:              /* P */ /* 6.6.1 */
          IDENTIFIER ':'             
           { $<L>$ = BuildLabel($1, NULL); }
          statement
           { $$->u.label.stmt = $4; }

        | CASE constant.expression ':' statement
            { $$ = AddContainee(MakeCaseCoord($2, $4, NULL, $1)); }
        | DEFAULT ':' statement
            { $$ = AddContainee(MakeDefaultCoord($3, NULL, $1)); }

          /* Required extension to 6.6.1 */
        | TYPEDEFname ':' statement
            { $$ = BuildLabel($1, $3); }
        ;

compound.statement:             /* P */ /* 6.6.2 */
          lblock rblock
            { $$ = MakeBlockCoord(PrimVoid, NULL, NULL, $1, $2); }
        | lblock declaration.list rblock
            { $$ = MakeBlockCoord(PrimVoid, GrabPragmas($2), NULL, $1, $3); }
        | lblock composite.body.statement.list rblock
            { $$ = MakeBlockCoord(PrimVoid, NULL, GrabPragmas($2), $1, $3); }
        | lblock declaration.list composite.body.statement.list rblock
            { $$ = MakeBlockCoord(PrimVoid, $2, GrabPragmas($3), $1, $4); }
        ;

lblock: '{'  { EnterScope(); }
rblock: '}'  { ExitScope(); }

/* compound.statement.no.new.scope is used by function.definition,
   since the new scope is begun with formal argument declarations,
   not with the opening '{' */
compound.statement.no.new.scope:             /* P */ /* 6.6.2 */
          '{' '}'
            { $$ = MakeBlockCoord(PrimVoid, NULL, NULL, $1, $2); }
        | '{' declaration.list '}'
            { $$ = MakeBlockCoord(PrimVoid, GrabPragmas($2), NULL, $1, $3); }
        | '{' statement.list '}'
            { $$ = MakeBlockCoord(PrimVoid, NULL, GrabPragmas($2), $1, $3); }
        | '{' declaration.list statement.list '}'
            { $$ = MakeBlockCoord(PrimVoid, $2, GrabPragmas($3), $1, $4); }
        ;



declaration.list:               /* P */ /* 6.6.2 */
          declaration                   { $$ = GrabPragmas($1); }
        | declaration.list declaration  { $$ = JoinLists(GrabPragmas($1),
                                                         $2); }
        ;

statement.list:                 /* P */ /* 6.6.2 */
          statement                   { $$ = GrabPragmas(MakeNewList($1)); }
        | statement.list statement    { $$ = AppendItem(GrabPragmas($1), 
                                                        $2); }
        ;

expression.statement:           /* P */ /* 6.6.3 */
          expression.opt ';'
        ;

selection.statement:            /* P */ /* 6.6.4 */
          IF '(' expression ')' costream.composite.statement
            { $$ = MakeIfCoord($3, $5, $1); }
        | IF '(' expression ')' costream.composite.statement ELSE costream.composite.statement
            { $$ = MakeIfElseCoord($3, $5, $7, $1, $6); }
        | SWITCH { PushContainer(Switch); } '(' expression ')' statement
            { $$ = PopContainer(MakeSwitchCoord($4, $6, NULL, $1)); }
        ;

iteration.statement:            /* P */ /* 6.6.5 */
          WHILE 
            { PushContainer(While);} 
          '(' expression ')' statement
            { $$ = PopContainer(MakeWhileCoord($4, $6, $1)); }
        | DO 
            { PushContainer(Do);} 
          statement WHILE '(' expression ')' ';'
            { $$ = PopContainer(MakeDoCoord($3, $6, $1, $4)); }
        | FOR '(' expression.opt ';' expression.opt ';' expression.opt ')'  
            { PushContainer(For);} 
          costream.composite.statement
            { $$ = PopContainer(MakeForCoord($3, $5, $7, $10, $1)); }

        /******** ERROR PRODUCTIONS (EAB) ********/
        | FOR '(' error ';' expression.opt ';' expression.opt ')'  
            { PushContainer(For);} 
          costream.composite.statement
            { $$ = PopContainer(MakeForCoord(NULL, $5, $7, $10, $1)); }
        | FOR '(' expression.opt ';' expression.opt ';' error ')'  
            { PushContainer(For);} 
          costream.composite.statement
            { $$ = PopContainer(MakeForCoord($3, $5, NULL, $10, $1)); }
        | FOR '(' expression.opt ';' error ';' expression.opt ')'  
            { PushContainer(For);} 
         costream.composite.statement
            { $$ = PopContainer(MakeForCoord($3, NULL, $7, $10, $1)); }
        | FOR '(' error ')' { PushContainer(For);} costream.composite.statement
            { $$ = PopContainer(MakeForCoord(NULL, SintZero, NULL, $6, $1)); }
        ;

jump.statement:                 /* P */ /* 6.6.6 */
          GOTO IDENTIFIER ';'
            { $$ = ResolveGoto($2, $1); }
        | CONTINUE ';'
            { $$ = AddContainee(MakeContinueCoord(NULL, $1)); }
        | BREAK ';'
            { $$ = AddContainee(MakeBreakCoord(NULL, $1)); }
        | RETURN expression.opt ';'
            { $$ = AddReturn(MakeReturnCoord($2, $1)); }

        /* Required extension/clarification to 6.6.6 */
        | GOTO TYPEDEFname ';'
            { $$ = ResolveGoto($2, $1); }
        ;

/********************************************************************************
*										*
*                            EXTERNAL DEFINITIONS                               *
*										*
********************************************************************************/

translation.unit:               /* P */ /* 6.7   */
          external.definition
        | translation.unit external.definition   
                  { $$ = JoinLists(GrabPragmas($1), $2); }

        ;

external.definition:            /* P */ /* 6.7   */
          declaration
            {
              if (yydebug)
                {
                  ///*DEBUG*/printf("external.definition # declaration\n");
                  PrintNode(stdout, FirstItem($1), 0); 
                  ///*DEBUG*/printf("\n\n\n");
				}
              $$ = $1;
            }
        | function.definition  
            { 
              if (yydebug)
                {
                  ///*DEBUG*/printf("external.definition # function.definition\n");
                  PrintNode(stdout, $1, 0); 
                  ///*DEBUG*/printf("\n\n\n");
                }
              $$ = MakeNewList($1); 
            }
		| composite.definition
		{
			if (yydebug)
                {

                 // printf("external.definition # composite.definition\n");
                  PrintNode(stdout, $1, 0); 
                 // printf("\n\n\n");
                }
              $$ = MakeNewList($1); 
		}
		;

function.definition:            /* P */ /* BASED ON 6.7.1 */
          identifier.declarator
            { 
              $1 = DefineProc(FALSE, 
                              SetDeclType($1,
					  MakeDefaultPrimType(EMPTY_TQ, $1->coord),
					  Redecl));
            }
          compound.statement.no.new.scope
            { $$ = SetProcBody($1, $3); }
        | identifier.declarator BOGUS
            /* this rule is never used, it exists solely to force the parser to
	       read the '{' token for the previous rule, thus starting a create
	       scope in the correct place */
        | declaration.specifier      identifier.declarator
            { $2 = DefineProc(FALSE, SetDeclType($2, $1, Redecl)); }
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $4); }
        | type.specifier             identifier.declarator
            { $2 = DefineProc(FALSE, SetDeclType($2, $1, Redecl)); }  /*SPL like this*/
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $4); }
        | declaration.qualifier.list identifier.declarator
            { 
              $2 = DefineProc(FALSE, 
	                      SetDeclType($2,
				 	  MakeDefaultPrimType($1.tq, $1.coord),
				          Redecl));
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $4); }
        | type.qualifier.list        identifier.declarator
            { 
              $2 = DefineProc(FALSE, 
                              SetDeclType($2,
					  MakeDefaultPrimType($1.tq, $1.coord),
					  Redecl));
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $4); }
        | old.function.declarator
            { 
              $1 = DefineProc(TRUE, 
                              SetDeclType($1,
					  MakeDefaultPrimType(EMPTY_TQ, $1->coord),
					  Redecl));
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($1, $3); }
        | declaration.specifier old.function.declarator 
            {  Node *decl = SetDeclType($2, $1, Redecl);  

               AddParameterTypes(decl, NULL);
               $2 = DefineProc(TRUE, decl);
            }
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $4); }
        | type.specifier old.function.declarator
            { Node *decl = SetDeclType($2, $1, Redecl);

              AddParameterTypes(decl, NULL);
              $2 = DefineProc(TRUE, decl);
            }
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $4); }
        | declaration.qualifier.list old.function.declarator
            { Node *type = MakeDefaultPrimType($1.tq, $1.coord),
                   *decl = SetDeclType($2, type, Redecl);

              AddParameterTypes(decl, NULL);
              $2 = DefineProc(TRUE, decl);
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $4); }
        | type.qualifier.list        old.function.declarator
            { Node *type = MakeDefaultPrimType($1.tq, $1.coord),
                   *decl = SetDeclType($2, type, Redecl);

              AddParameterTypes(decl, NULL);
              $2 = DefineProc(TRUE, decl);
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $4); }
        | old.function.declarator old.function.declaration.list
            { Node *type = MakeDefaultPrimType(EMPTY_TQ, $1->coord),
                   *decl = SetDeclType($1, type, Redecl);

              AddParameterTypes(decl, $2);
              $1 = DefineProc(TRUE, decl);
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($1, $4); }
        | declaration.specifier old.function.declarator old.function.declaration.list
            { Node *decl = SetDeclType($2, $1, Redecl);

              AddParameterTypes(decl, $3);
              $2 = DefineProc(TRUE, decl);
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $5); }
        | type.specifier old.function.declarator old.function.declaration.list
            { Node *decl = SetDeclType($2, $1, Redecl);

              AddParameterTypes(decl, $3);
              $2 = DefineProc(TRUE, decl);
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $5); }
        | declaration.qualifier.list old.function.declarator old.function.declaration.list
            { Node *type = MakeDefaultPrimType($1.tq, $1.coord),
                   *decl = SetDeclType($2, type, Redecl);

              AddParameterTypes(decl, $3);
              $2 = DefineProc(TRUE, decl);
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $5); }
        | type.qualifier.list old.function.declarator old.function.declaration.list
            { Node *type = MakeDefaultPrimType($1.tq, $1.coord), 
                   *decl = SetDeclType($2, type, Redecl);
				       

              AddParameterTypes(decl, $3);
              $2 = DefineProc(TRUE, decl);
            } 
          compound.statement.no.new.scope
            { $$ = SetProcBody($2, $5); }
        ;


old.function.declaration.list:
             { OldStyleFunctionDefinition = TRUE; }
             declaration.list
             { OldStyleFunctionDefinition = FALSE; 
               $$ = $2; }
        ;

/********************************************************************************
*										*
*                          CONSTANTS and LITERALS                               *
*										*
********************************************************************************/

/* 
  CONSTANTS.  Note ENUMERATIONconstant is treated like a variable with a type
  of "enumeration constant" (elsewhere)
*/
constant: /*P*/
          FLOATINGconstant      { $$ = $1; }
        | INTEGERconstant       { $$ = $1; }
        | OCTALconstant         { $$ = $1; }
        | HEXconstant           { $$ = $1; }
        | CHARACTERconstant     { $$ = $1; }
        ;

/* STRING LITERALS */
string.literal.list: /*P*/
          STRINGliteral         { $$ = $1; }
        | string.literal.list STRINGliteral
            { const char *first_text  = $1->u.Const.text;
              const char *second_text = $2->u.Const.text;
              int   length = strlen(first_text) + strlen(second_text) + 1;
              char *buffer = HeapNewArray(char, length);
              char *new_text, *new_val;
	
              /* since text (which includes quotes and escape codes)
		 is always longer than value, it's safe to use buffer
		 to concat both */
              strcpy(buffer, NodeConstantStringValue($1));
	      strcat(buffer, NodeConstantStringValue($2));
              new_val = UniqueString(buffer);

              strcpy(buffer, first_text);
	      strcat(buffer, second_text);
              new_text = buffer;
              $$ = MakeStringTextCoord(new_text, new_val, $1->coord);
	     }
        ;

type.qualifier: /*P*/
          CONST     { $$.tq = T_CONST;    $$.coord = $1; } 
        | VOLATILE  { $$.tq = T_VOLATILE; $$.coord = $1; }
        | INLINE    { $$.tq = T_INLINE;   $$.coord = $1; }
        ;

pointer.type.qualifier: /*P*/
          CONST     { $$.tq = T_CONST;    $$.coord = $1; } 
        | VOLATILE  { $$.tq = T_VOLATILE; $$.coord = $1; }
        ;

storage.class: /*P*/
          TYPEDEF   { $$.tq = T_TYPEDEF;  $$.coord = $1; } 
        | EXTERN    { $$.tq = T_EXTERN;   $$.coord = $1; } 
        | STATIC    { $$.tq = T_STATIC;   $$.coord = $1; } 
        | AUTO      { $$.tq = T_AUTO;     $$.coord = $1; } 
        | REGISTER  { $$.tq = T_REGISTER; $$.coord = $1; } 
        ;

basic.type.name: /*P*/
          VOID      { $$ = StartPrimType(Void, $1);    } 
        | CHAR      { $$ = StartPrimType(Char, $1);     } 
        | INT       { $$ = StartPrimType(Int_ParseOnly, $1);     } 
        | FLOAT     { $$ = StartPrimType(Float, $1);   } 
        | DOUBLE    { $$ = StartPrimType(Double, $1);  } 

        | SIGNED    { $$ = StartPrimType(Signed, $1);  } 
        | UNSIGNED  { $$ = StartPrimType(Unsigned, $1);} 

        | SHORT     { $$ = StartPrimType(Short, $1);   } 
        | LONG      { $$ = StartPrimType(Long, $1);    }
        ;

%%
/* ----end of grammar----*/


PRIVATE void WarnAboutPrecedence(OpType op, Node *node)
{
  if (node->typ == Binop && !node->parenthesized) {
    OpType subop = node->u.binop.op;

    if (op == OROR && subop == ANDAND)
      WarningCoord(4, node->coord, "suggest parentheses around && in operand of ||");
    else if ((op == '|' || op == '^') && 
	     (subop == '+' || subop == '-' || subop == '&' || subop == '^') &&
	     op != subop)
      WarningCoord(4, node->coord, "suggest parentheses around arithmetic in operand of %c", op);
  }
}


