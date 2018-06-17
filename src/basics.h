
#pragma ident "basics.h,v 1.15 1995/05/05 19:18:23 randall Exp Copyright 1994 Massachusetts Institute of Technology"

#ifndef _BASICS_H_
#define _BASICS_H_


#ifndef __GNUC__
#define inline
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#ifdef WIN32
#include <conio.h>
#endif

#ifdef NO_PROTOTYPES
extern int      fprintf();
extern int      fputc();
extern int      fputs();
extern FILE     *fopen();
extern int      fclose();
extern int      printf();
extern int	sscanf();
extern int      _flsbuf();
extern void     bcopy();
extern int      toupper();
extern char *   memcpy();
extern int      fflush();
#endif


#define GLOBAL
#define PRIVATE static

/* NoReturn indicates that the function never returns (like "exit") */
#ifdef __GNUC__
#define NoReturn volatile void
#else
#define NoReturn void
#endif

/* for generic prototypes (use Generic *) */
typedef void Generic;
typedef void **GenericREF;  /* address of a Generic, for pass by reference */

typedef int Bool;
#define TRUE 1
#define FALSE 0


/* assertion checking */
#undef assert
#ifdef NDEBUG
#define assert(x) ((void) 0)
#else
#define assert(x)  ((x) ? (void)0 : (void)Fail(__FILE__, __LINE__, #x))
#endif

#define FAIL(str) \
	Fail(__FILE__, __LINE__, str)
#define UNREACHABLE FAIL("UNREACHABLE")



#define PLURAL(i) (((i) == 1) ? "" : "s")
#define TABSTRING "    "


#define MAX_FILES       256
#define MAX_FILENAME    200
#define MAX_SCOPE_DEPTH 100
#define MAX_OPERATORS   600
#define MAX_LIBFUNC     100


/* Basic Typedefs */
typedef struct nodeStruct Node;
typedef struct tablestruct SymbolTable;
typedef int OpType;
typedef struct paramstruct paramList;
typedef struct constArrayStruct constArrayList;
//cwb：将composite中param存放于各operator中
struct paramstruct
{
	Node *paramnode;
	struct paramstruct *next;
};
struct constArrayStruct
{
	Node *arraynode;
	struct constArrayStruct *next;
};

//zww:表示op的类型
typedef enum {
	op_int = 1,/*整型*/
	op_float,/*浮点型*/
	op_unkonwn/*未知*/
}opDataType;

/***********************--------------Define For SPL----------****************************/
//typedef struct tablestruct SymbolTable;
typedef struct ASTtablestruct ASTSymbolTable;//zww

#ifndef SPL_DEBUG
//#define SPL_DEBUG
#endif

#ifndef SPL_GRAMMAR
#define SPL_GRAMMAR
#endif
/***********************--------------Define For SPL----------****************************/

typedef struct coord {
	int line;
	short offset;
	short file;
	Bool includedp;
} Coord;

GLOBAL extern Coord UnknownCoord;
#define IsUnknownCoord(coord)  ((coord).file == UnknownCoord.file)



#define PRINT_COORD(out, c) \
{ if (PrintLineOffset) \
	fprintf(out,"%s:%d:%d", FileNames[(c).file], (int)(c).line, \
	(int)(c).offset); \
	else fprintf(out, "%s:%d", FileNames[(c).file], (int) (c).line); }


#define REFERENCE(var)  ((var)->u.decl.references++)
#define VAR_NAME(var)   ((var)->u.decl.name)



/* Prototypes/definitions from other files */

#include "heap.h"
#include "list.h"
#include "symbol.h"




/* Basic Global Variables */

GLOBAL extern const float VersionNumber;     /* main.c */
GLOBAL extern const char *const VersionDate; /* main.c */
GLOBAL extern const char * Executable;       /* program name, main.c */
GLOBAL extern List *Program;                 /* main.c */
GLOBAL extern int WarningLevel;              /* main.c */
GLOBAL extern int Line, LineOffset, Errors, Warnings;    /* warning.c */
GLOBAL extern unsigned int CurrentFile;      /* c4.l: current file number */
GLOBAL extern char *Filename;                /* c4.l */
GLOBAL extern char *FileNames[MAX_FILES];    /* c4.l: file # to name mapping*/
GLOBAL extern const char *PhaseName;         /* main.c */
GLOBAL extern Bool FileIncludedp[MAX_FILES]; /* c4.l */
GLOBAL extern Bool CurrentIncludedp;         /* c4.l */

/* ANSI defines the following name spaces (K&R A11.1, pg 227): */
GLOBAL extern SymbolTable *Identifiers, *Labels, *Tags;

/* This table is used to ensure consistency across the translation unit */
GLOBAL extern SymbolTable *Externals;
/***********************--------------Define For SPL----------****************************/
/* This table is used to ensure consistency across the translation unit */
GLOBAL extern SymbolTable *CompositeIds; /* main.c */
GLOBAL extern Node *gMainComposite; /* main.c */
GLOBAL extern Bool gIsAfterPropagate; /* main.c */
GLOBAL extern ASTSymbolTable *ToTransformDecl;//zww
GLOBAL extern ASTSymbolTable *ParameterPassTable;//zww
GLOBAL extern ASTSymbolTable *VariableRenameTable;//ly
GLOBAL List *VariableRenameProgram(List *program);//ly
/***********************--------------Define For SPL----------****************************/

/* Global Flags */
GLOBAL extern Bool IsInCompositeParam;       /* main.c--Define For SPL */
GLOBAL extern Bool DebugLex;                 /* main.c */
GLOBAL extern Bool PrintLineOffset;          /* main.c */
GLOBAL extern Bool IgnoreLineDirectives;     /* main.c */
GLOBAL extern Bool ANSIOnly;                 /* main.c */
GLOBAL extern Bool FormatReadably;           /* main.c */
GLOBAL extern Bool PrintLiveVars;            /* main.c */

/* Basic Global Procedures */

/* pretty-printing */
GLOBAL void DPN(Node *n);
GLOBAL void DPL(List *list);
GLOBAL void PrintNode(FILE *out, Node *node, int tab_depth);
GLOBAL int PrintConstant(FILE *out, Node *c, Bool with_name);
GLOBAL void PrintCRSpaces(FILE *out, int spaces);
GLOBAL void PrintSpaces(FILE *out, int spaces);
GLOBAL void PrintList(FILE *out, List *list, int tab_depth);
GLOBAL int PrintOp(FILE *out, OpType op);  /* operators.c */
GLOBAL void CharToText(char *array, unsigned char value);
GLOBAL inline int PrintChar(FILE *out, int c);    /* print.c */
GLOBAL int PrintString(FILE *out, const char *string); /* print.c */

/* warning.c */
GLOBAL NoReturn Fail(const char *file, int line, const char *msg);
GLOBAL void SyntaxError(const char *fmt, ...);
GLOBAL void Warning(int level, const char *fmt, ...);
GLOBAL void SyntaxErrorCoord(Coord c, const char *fmt, ...);
GLOBAL void WarningCoord(int level, Coord c, const char *fmt, ...);

/* parsing phase */
GLOBAL int yyparse(void), yylex(void);
GLOBAL char *UniqueString(const char *string);   /* strings.c */
GLOBAL void SetFile(const char *filename, int line);       /* c4.l */

/* verification */
GLOBAL void VerifyParse(List *program);          /* verify-parse.c */

/* semantic check -- sem-check.c */
GLOBAL List *SemanticCheckProgram(List *program);
GLOBAL Node *SemCheckNode(Node *node);
GLOBAL List *SemCheckList(List *list);

/* transform phase -- transform.c */
GLOBAL List *TransformProgram(List *program);

/* unfold phase -- unfold.c--Define For SPL */
GLOBAL Node *UnfoldPipeline(Node *node); /* unfold.c */
GLOBAL Node *UnfoldSplitJoin(Node *node); /* unfold.c */
GLOBAL Node *CreateCompositeInMultiSP(Node *node);//MultiSP
/***********************--------------Define For SPL----------****************************/
GLOBAL Node *TransformOperator(Node *node);//zww
GLOBAL Node *GetValue(Node *node); /* propagator.c */

/*zww: transform phase -- transforms.c define for spl 全局标志 */
GLOBAL extern Bool gIsTransform;		/* transform.c */
GLOBAL extern Bool gIsTypelist;			/* transform.c */
GLOBAL extern Bool gIsInSymbolTable;	/* transform.c */
GLOBAL extern Bool gIsCall;				/* transform.c */
GLOBAL extern List *gDeclList;			/* transform.c */
GLOBAL extern List *gProcList;			/* transform.c */

GLOBAL extern Node *gCurrentInputStreamNode;	/* unfold.c */
GLOBAL extern Node *gCurrentOutputStreamNode;	/* unfold.c */
GLOBAL extern List *gCurrentCompositeCallList;  /* unfold.c */
GLOBAL extern List *gCurrentParamList[64];			/* unfold.c */
GLOBAL extern List *gCurrentDeclList[64];			/* unfold.c */
GLOBAL extern List *gCurrentSplitList;			/* unfold.c */
GLOBAL extern List *gCurrentJoinList;			/* unfold.c */
GLOBAL extern int gMultiSPFlag;					/* unfold.c */
GLOBAL extern int gLevelPipeline;				/* unfold.c */
GLOBAL extern int gLevelSplitjoin;				/* unfold.c */
GLOBAL extern Bool gIsInSplitJoin;				/* unfold.c */
GLOBAL extern Bool gIsInPipeline;				/* unfold.c */
GLOBAL extern Bool gIsRoundrobin;				/* unfold.c */
GLOBAL extern Bool gIsDuplicate;				/* unfold.c */
GLOBAL extern Bool gIsUnfold;				/* unfold.c */
GLOBAL extern Bool gIsFileOperator;				/* file_rw.c */
GLOBAL extern List *gfrtaCallList;  /* unfold.c */
/***********************--------------Define For SPL----------****************************/

/*构造新的splitjoin结构*/
GLOBAL Node *MakeSplitOperator(Node *input, List *arguments, int style); /* unfold.c */
GLOBAL Node *MakeJoinOperator(Node *output, List *inputs, List *arguments)	; /* unfold.c */
GLOBAL Node *MakeNewStreamId(const char *name, Node *decl);
GLOBAL Node *MakeNewStream(const char *name, Node *copyStream);
GLOBAL Node *MakeMyWindow(Node *id, Node *count, int style);
GLOBAL Node *MakeOperatorHead(const char *name, List *outputs, List *inputs);
GLOBAL Node *MakeDuplicateWork(List *outputs, Node *input, Node *argument);
GLOBAL Node *MakeRoundrobinWork(List *outputs, Node *input, List *arguments);
GLOBAL Node *MakeOperatorBody(Node *work, List *windows);
GLOBAL Node *MakeJoinWork(Node *output, List *inputs, List *arguments);
/* output phase -- output.c */
GLOBAL void OutputProgram(FILE *out, List *program);


#endif  /* ifndef _BASICS_H_ */


