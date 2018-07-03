//Extended by --Liu Xiaoxian, DML, HUST
#pragma ident "ast.h,v 1.22 1995/05/11 18:54:09 rcm Exp Copyright 1994 Massachusetts Institute of Technology"

#ifndef _AST_H_
#define _AST_H_


#include "basics.h"
#include "dataflow.h"



/* definition of Node data structure */

typedef enum {
  /* expression nodes */
  Const, Id, Binop, Unary, Cast, Comma, Ternary, Array, Call, Initializer,
  ImplicitCast,

  /* statement nodes */
  Label, Switch, Case, Default, If, IfElse, While, Do, For, Goto, Continue, 
  Break, Return, Block, 
  
  /* type nodes */
  Prim, Tdef, Ptr, Adcl, Fdcl, Sdcl, Udcl, Edcl, 
 
  /* declaration node */
  Decl, 

  /* GCC __attribute__ extension */
  Attrib, 

  /* procedure def node */
  Proc,


  /* random text and preprocessor command node */
  Text,

  /*----------12----SPL node----------*/
  STRdcl,		/*stream SPL*/
  Comdcl,		/*composite SPL*/
  Composite,	/* composite def node SPL*/
  ComInOut,		/*composite SPL*/
  ComBody,		/*composite.block SPL*/
  Param,		/*composite.body.param SPL*/		 
  OperBody,		/*operator SPL*/	
  Operdcl,		/*operator SPL*/
  Operator_,	/* operator def node SPL*/
  Window,		/*operator.body.window SPL*/
  Sliding,		/*sliding window SPL*/	
  Tumbling,		/*tumbling window SPL*/
  
  /*-------7-------New For SPL----------*/
  CompositeCall,/*composite.head.inout.stream.id SPL*/
  Pipeline,	
  SplitJoin,	
  Split,		
  Join,	
  RoundRobin,	
  Duplicate,
  
  /********1********新文法**********/
  Add,
  Itco			/*iterator count*/
  /*
  删除掉的节点： Var,Graph,StreamFor,StreamIf,StreamIfElse,Eviction,Trigger,logic
  修改过的节点： comBodyNode,PipelineNode,SplitJoinNode,slidingNode,tumblingNode
  新增加的节点： Add
  */
} NodeType;


/*************************************************************************/
/*                                                                       */
/*                      Type representations                             */
/*                                                                       */
/*************************************************************************/

/* TypeQual: storage classes, type qualifiers, and additional attributes */

typedef enum {

  EMPTY_TQ = 0,
/* 
   Declaration qualifiers.  A declaration can have multiple qualifiers,
   though some are mutually exclusive (like storage classes).
   During the parsing of a declaration, decl qualifiers are kept in 
   the tq field of its primitive type.  After the decl has been parsed,
   SetDeclType then moves the qualifiers to their proper place, the 
   Decl node.

   To add a new decl qualifier:
      1. Define a new symbol below, being careful that its bits do
         not conflict with existing decl and type qualifiers,
         since they can coexist in the same TypeQual variable.
      2. Insert the symbol in T_DECL_QUALS (below).
      3. If the qualifier is parsed in the source (rather than calculated),
         add a lexical token (c4.l) and a new production to storage.classes
         (ANSI-C.y).
      4. Add code to print out its name in TQtoText (type.c).
      5. Add merging logic to MergeTypeQuals (type.c), if necessary.
*/

  T_AUTO     = 0x00001,
  T_EXTERN   = 0x00002,
  T_REGISTER = 0x00003,
  T_STATIC   = 0x00004,
  T_TYPEDEF  = 0x00005,
  /* Insert new storage classes here */

  T_STORAGE_CLASSES = (T_AUTO | T_EXTERN | T_REGISTER | T_STATIC | T_TYPEDEF),
#define STORAGE_CLASS(tq) ((tq) & T_STORAGE_CLASSES)

  /* the following qualifiers are all mutually exclusive,
     so they can share bits */
  T_TOP_DECL    = 0x00010,   /* top-level decl */
  T_BLOCK_DECL  = 0x00020,   /* local decl in a block */
  T_FORMAL_DECL = 0x00030,   /* formal parameter decl */
  T_SU_DECL     = 0x00040,   /* struct/union field decl */
  T_ENUM_DECL   = 0x00050,   /* enumerated constant decl */
  /* Insert new decl locations here */
  T_STREAM_DECL  = 0x00060,   /* stream field decl SPL*/
  T_COMMAL_DECL = 0x00070,
  
  T_DECL_LOCATIONS = (T_TOP_DECL | T_BLOCK_DECL | T_FORMAL_DECL |
		      T_SU_DECL | T_ENUM_DECL | T_STREAM_DECL | T_COMMAL_DECL/*SPL*/),
#define DECL_LOCATION(tq)  ((tq) & T_DECL_LOCATIONS)


    /* Flag for redundant external declaration, which is defined as
     * an external declaration (NOT a definition, so it must have no
     * initializer or function body) of a name previously declared external 
     * in the same scope.  A trivial example:
     *    extern int x;
     *    extern int x;    <-- redundant
     * But:
     *    { extern int x; }
     *    { extern int x; }  <-- not redundant
     * because the two declarations have different scopes.
     */
  T_REDUNDANT_EXTERNAL_DECL = 0x00100,

  /* Insert new decl qualifiers here */

  T_DECL_QUALS = (T_STORAGE_CLASSES | T_DECL_LOCATIONS | T_REDUNDANT_EXTERNAL_DECL),
#define DECL_QUALS(tq)    ((tq) & T_DECL_QUALS)


/*
   Type qualifiers.  Multiple type qualifiers may apply to a type.  
   They may be associated with any primitive or complex type.  
   Some type qualifiers may be moved after parsing -- for instance,
   T_INLINE is moved to the top-level Fdcl it is describing.

   To add a new type qualifier:
      1. Define a new symbol below, being careful that its bits do
         not conflict with existing storage classes and type qualifiers,
         since they can coexist in the same TypeQual variable.
      2. Insert the symbol in T_TYPE_QUALS (below).
      3. Add a lexical token (c4.l) and a new production to either
         type.qualifiers or pointer.type.qualifiers (ANSI-C.y), depending 
         on whether the type qualifier is allowed only at the beginning
         of a type, or can appear after '*' like const and volatile.
      4. Add code to print out its name in TQtoText (type.c).
      5. Add its symbol to TQ_ALWAYS_COMPATIBLE (below) if an object with
         the type qualifier is always assignment-compatible with an object
	 without the type qualifier.
      6. Add merging logic to MergeTypeQuals (type.c), if necessary.
 */

    T_CONST    = 0x01000,   /* leave some room for new decl qualifiers */
    T_VOLATILE = 0x02000,
    T_INLINE   = 0x04000,
    T_SUE_ELABORATED  = 0x08000,   /* on an Sdcl/Udcl/Edcl, indicates 
				      whether SUE's field list appeared
				      at that point in the source */
    T_STREAM_ELABORATED  = 0x10000,   /* on an STRdcl/, indicates 
								 whether stream's field list appeared
								 at that point in the source SPL*/
#define SUE_ELABORATED(tq)   (((tq) & T_SUE_ELABORATED) != 0)
#define STREAM_ELABORATED(tq)   (((tq) & T_STREAM_ELABORATED) != 0)/*SPL*/
    /* Insert new type qualifiers here */
    
    T_TYPE_QUALS = (T_CONST | T_VOLATILE | T_INLINE | T_SUE_ELABORATED | T_STREAM_ELABORATED/*SPL*/)
#define TYPE_QUALS(tq)    ((tq) & T_TYPE_QUALS)
    

/* Type qualifiers listed in TQ_COMPATIBLE are ignored
   when checking two types for compatibility (weaker than strict equality). */
#define TQ_COMPATIBLE   (T_CONST | T_VOLATILE | T_INLINE | T_SUE_ELABORATED | T_STREAM_ELABORATED/*SPL*/)

} TypeQual;



/* BasicType covers all of the different fundamental types.
   New basic types should also be added to InitTypes(). */
typedef enum {
  /* Unspecified=0, */
  Uchar=1, Schar, Char,
  Ushort, Sshort,
  Uint, Sint, 
  Int_ParseOnly /* used only in parsing -- FinishPrimType converts to Sint */, 
  Ulong, Slong,
  Ulonglong, Slonglong,
  Float, Double, Longdouble, 
  Void, Ellipsis,
  MaxBasicType, /* must be last */
} BasicType;


/* 

While a BasicType is constructed during parsing, its high-order bits
(above the bits needed for the BasicType itself) are flags
representing type modifiers like signedness and length.  (Extension
languages may have additional flags for their own type modifiers.)
After the BasicType is finalized with FinishPrimType, these 
high-order bits are cleared and the BasicType is set to one of the
enumerated values above.  (Thus the final BasicType value is a small
integer, which will be faster in a switch statement than a bitfield
representation like Sshort == Short | Int.)


To add a new category of type modifier:

   1. Insert it in TypeSpecifier (below), being careful not to conflict
      with bits used by other modifiers.
   2. Change to BASIC2EXPLODED and EXPLODED2BASIC to convert it (below).
   3. Add merging and finishing logic to MergePrimTypes and FinishPrimType
      (type.c).
   4. Add type names to TypeSpecifierName (type.c).

*/

typedef enum { 
  /* Length -- 0 in these bits means no sign specified */
  Short    = 0x0100, 
  Long     = 0x0200, 
  Longlong = 0x0300,
  LengthMask = Short | Long | Longlong,


  /* Signedness -- 0 in these bits means no sign specified */
  Signed   = 0x0400,
  Unsigned = 0x0800,
  SignMask = Signed | Unsigned,


  /* INSERT EXTENSIONS HERE */ 

  /* Base type -- Char, Int, Float, Double, etc.  Uses subset of
     values defined for BasicType. */
  BasicTypeMask = 0x00FF

  } TypeSpecifier;


typedef struct {
  TypeSpecifier base;
  TypeSpecifier sign;
  TypeSpecifier length;
  /* INSERT EXTENSIONS HERE */ 
} ExplodedType;


#define BASIC2EXPLODED(bt, et)  \
   ((et).sign = (bt) & SignMask, (et).length = (bt) & LengthMask, \
  /* INSERT EXTENSIONS HERE */     \
    (et).base = (bt) & BasicTypeMask)

#define EXPLODED2BASIC(et, bt)  \
   ((bt) = (et).sign | (et).length | \
  /* INSERT EXTENSIONS HERE */     \
    (et).base)

typedef enum{S_Duplicate,S_RoundRobin}SplitStyle;//zww 20121016 添加split节点的两种类型


typedef struct {
    NodeType typ;
    
    Bool complete;  /* true_ if struct has been elaborated with field list */
    Coord coord;  /* location of definition (left brace) */
    Coord right_coord;  /* location of right brace */

    Bool visited;  /* this struct has been visited in size calculation */ 
    int size;
    int align;

    const char *name;
    List *fields;
} SUEtype;  /* struct/union/enum type */



/*************************************************************************/
/*                                                                       */
/*                         Node structure                                */
/*                                                                       */
/*************************************************************************/

/* ChildNode and ChildList are defined for documentation only: these
   represent the tree-like links of the AST.  All other links point
   across in the tree (e.g., decl for an Id node). Any procedure walking
   the AST must follow only ChildNode links, or it runs the risk of falling
   into a cycle. -- rcm */
typedef Node ChildNode;
typedef List ChildList;


/*************************************************************************/
/*                                                                       */
/*                          Expression nodes                             */
/*                                                                       */
/*************************************************************************/

typedef struct {
	const char* text;  /* text will be NULL if constant was not
			      derived from source, but computed by 
			      type-checker. */
	ChildNode *type;  /* type is required to be one of:
			     Sint, Uint, Slong, Ulong,
			     Float, Double, or Adcl(Char) */
	union {
	  TARGET_INT    i;
	  TARGET_UINT   u;
	  TARGET_LONG   l;
	  TARGET_ULONG ul;
	  float         f;
	  double        d;
	  const char *  s;
	} value;
} ConstNode;

typedef struct {
	const char* text;
	Node *decl;  
	/* type is decl->u.decl.type */
	Node *value;
} idNode;

typedef struct {
	OpType op;
	opDataType opType;/*zww:define for spl,记录该2元操作的数据类型：整型，浮点型*/
	ChildNode *left;
	ChildNode *right;
	Node *type;
	Node *value;
	ChildList *valueList;/*zww:对结构体常量传播的dot操作*/
} binopNode;

typedef struct {
	OpType op;
	opDataType opType;/*zww:define for spl,记录该2元操作的数据类型：整型，浮点型*/
	ChildNode *expr;
	Node *type;
	Node *value;
} unaryNode;

typedef struct {
	ChildNode *type;
	ChildNode *expr;
	Node *value;
} castNode;

/* Formerly Comma was overloaded to represent both comma expressions and 
   for brace-initializers.  The semantics of brace-initializers are quite
   different, however, so it seemed clearer to eliminate the overloading and
   introduce a new AST node, Initializer, instead. -- rcm */
typedef struct {
	ChildList *exprs;
} commaNode;

typedef struct {
	ChildNode *cond;
	ChildNode *true_;
	ChildNode *false_;
	Coord colon_coord;  /* source line and offset of colon */
	Node *type;
	Node *value;
} ternaryNode;

typedef struct {
	Node *type;
	ChildNode *name;
	ChildList *dims;
	Node *value;//zww：常量传播用
} arrayNode;

typedef struct {
	ChildNode *name;
	ChildList *args;
} callNode;

typedef struct {
	ChildList *exprs;
} initializerNode;


/* ImplicitCasts are used by the semantic checker to implicitly give
   an expression a type or value "from above";  for instance, char variables
   are implicitly converted to integers in expressions, not by
   changing the type on the identifier node, but by inserting
   a implicitcast above the identifier node which specifies its new
   type-in-context. -- rcm */
typedef struct {
	ChildNode *expr;
	Node *type;
	Node *value;
} implicitcastNode;


/*************************************************************************/
/*                                                                       */
/*                          Statement nodes                              */
/*                                                                       */
/*************************************************************************/

typedef struct {
	const char* name;
	ChildNode *stmt;
	List *references;
	FlowValue label_values;
} labelNode;



typedef struct {
	ChildNode *expr;
	ChildNode *stmt;
	List *cases;
	Bool has_default;    /* true_ if cases includes a Default node */
	struct SwitchCheck *check;  /* points to hash table of case
				       expression values for
				       duplicate-checking */
	FlowValue switch_values, break_values;
} SwitchNode;

typedef struct {
	ChildNode *expr;
	ChildNode *stmt;
	Node *container;
} CaseNode;

typedef struct {
	ChildNode *stmt;
	Node *container;
} DefaultNode;

/* If statement with no Else */
typedef struct {
	ChildNode *expr;
	ChildNode *stmt;
} IfNode;

typedef struct {
	ChildNode *expr;
	ChildNode *true_;
	ChildNode *false_;
	Coord else_coord;  /* coordinates of ELSE keyword */
} IfElseNode;

typedef struct {
	ChildNode *expr;
	ChildNode *stmt;
	FlowValue loop_values, break_values;
} WhileNode;

typedef struct {
	ChildNode *stmt;
	ChildNode *expr;
	Coord while_coord;  /* coordinates of WHILE keyword */
	FlowValue loop_values, continue_values, break_values;
} DoNode;

typedef struct {
	ChildNode *init;
	ChildNode *cond;
	ChildNode *next;
	ChildNode *stmt;
	FlowValue loop_values, continue_values, break_values;
} ForNode;

typedef struct {
	Node *label;
} GotoNode;

typedef struct {
	Node *container;
} ContinueNode;

typedef struct {
	Node *container;
} BreakNode;

typedef struct {
	ChildNode *expr;
	Node *proc;   /* points to Proc node containing this return stmt */
} ReturnNode;

typedef struct {
	ChildList *decl;
	ChildList *stmts;
	Coord right_coord;  /* coordinates of right brace */
	Node *type;  /* the type of a {...} block is void;
			the type of a ({..}) block is the type of its last
			  statement (initially NULL, then filled in by
			  SemCheckNode) */
} BlockNode;

/*************************************************************************/
/*                                                                       */
/*                          Type nodes                                   */
/*                                                                       */
/*************************************************************************/

typedef struct {
	TypeQual tq;
	BasicType basic;
} primNode;

typedef struct {
	const char* name;
	TypeQual tq;
	Node *type;
} tdefNode;

typedef struct {
        TypeQual tq;
        ChildNode *type;
} ptrNode;

typedef struct {
        TypeQual tq;
        ChildNode *type;
        ChildNode *dim;
	int   size;
} adclNode;

typedef struct {
	TypeQual tq;
	ChildList *args;
	ChildNode *returns;
} fdclNode;

typedef struct {
	TypeQual tq;
	SUEtype* type; /* this is a child link iff SUE_ELABORATED(tq) */
} sdclNode;

typedef struct {
	TypeQual tq;
	SUEtype* type; /* this is a child link iff SUE_ELABORATED(tq) */
} udclNode;

typedef struct {
	TypeQual tq;
	SUEtype* type; /* this is a child link iff SUE_ELABORATED(tq) */
} edclNode;


/*************************************************************************/
/*                                                                       */
/*                          Other nodes                                  */
/*                                                                       */
/*************************************************************************/

typedef struct {
	const char* name;
	TypeQual tq;       /* storage class and decl qualifiers */
	ChildNode *type;
	ChildNode *init;    /* in other versions of c-parser, init is
			       overloaded to be the offset for structs -- 
			       but NOT in c-to-c */
	ChildNode *prim_init;//zww：为了保留数组定义是初始化列表 12.2.10
	ChildNode *bitsize;
	int  references;    /* number of references to declared name */
	List *attribs;      /* GCC __attribute__ declarations */
} declNode;

typedef struct {
  const char *name;         /* name of attribute, like "packed" or "const" */
  ChildNode *arg;           /* expression arguments of attribute */
} attribNode;

typedef struct {
	ChildNode *decl;
	ChildNode *body;
	FlowValue init_values;//zww:SPL
	FlowValue return_values;
} procNode;

typedef struct {
  const char *text;     /* may be NULL (treated same as "") */
  Bool start_new_line;  /* true_ ==> nothing but whitespace should appear
			   before <text> on line */
} textNode;




/*************************************************************************/
/*                                                                       */
/*                            Extensions                                 */
/*                                                                       */
/*************************************************************************/

/***********************Define Nodes For SPL----begin*********************/

/*************************************************************************/
/*                                                                       */
/*                      SPL Type Nodes                                   */
/*                                                                       */
/*************************************************************************/
typedef struct {
	NodeType typ;

	Bool complete;  /* true_ if struct has been elaborated with field list */
	Coord coord;  /* location of definition (left '<') */
	Coord right_coord;  /* location of right '>' */

	Bool visited;  /* this struct has been visited in size calculation */ 
	int size;
	int align;

	const char *name;
	List *fields;
} StreamType;  /*SPL stream type  */
typedef enum {
	Duplicate_,
	Roundrobin_,
	Join_,
	Common_,
} OperatorType; // SPL


/*************************************************************************/
/*                                                                       */
/*                      SPL Composite Nodes                              */
/*                                                                       */
/*************************************************************************/
/*******13******spl node*******/
typedef struct {/*define for stream. SPL*/
	TypeQual tq;
	StreamType *type; /* this is a child link iff S_ELABORATED(tq) */
} strdclNode;

typedef struct {
	TypeQual tq;
	ChildNode *inout;
} comDeclNode;

typedef struct {      
	ChildNode *decl;  /*type->comInoutNode,name->id.name*/
	ChildNode *body;
	Bool multi;
	FlowValue composite_values;/*for data flow among composites*/
} compositeNode;

typedef struct {
	TypeQual tq;
	ChildList *outputs;
	ChildList *inputs;
} comInOutNode;

typedef struct {
	ChildNode *param;
	ChildList *decl;
	ChildList *comstmts;	/*	替代以前的var结构和graph结构 
							SPL composite中的特殊语句 和 c语句
							包括：	普通c语句
									stream类型的定义语句
									composite中的if-else语句
									operator_ 语句
									composite调用语句
									splitjoin语句
									pipeline语句
							*/  
	
	Coord left_coord;
	Coord right_coord;  /* coordinates of right brace */
	Node *type;  /* the type of a {...} block is void;
				 the type of a ({..}) block is the type of its last
				 statement (initially NULL, then filled in by
				 SemCheckNode) */
} comBodyNode;

typedef struct {
	ChildList *parameters;
} paramNode;

typedef struct {
	ChildList *state;  /*declaration.list  改动：无state关键字，其余结构不变*/
	ChildNode *init;   /*compound.statement*/
	ChildNode *work; /*compound.statement assert(work != null),kernel*/
	ChildList *window; /*if window == null, its peek=pop=1;*/
	Coord left_coord;
	Coord right_coord;  /* coordinates of right brace */
	Node *type;  /* the type of a {...} block is void;
				 the type of a ({..}) block is the type of its last
				 statement (initially NULL, then filled in by
				 SemCheckNode) */
} operBodyNode;

typedef struct {
	TypeQual tq;
	ChildList *outputs;/*stream.type.specifier identifier.list | identifier.list*/
	ChildList *inputs;/*identifier.list*/
	ChildList *arguments;/*argument.expression.list*/
} operDeclNode;



typedef struct {      
	ChildNode *decl;  /*type->streamif.operator | splitjoin.operator | pipeline.operator | operator.invoke | operator.stream.define */
	ChildNode *body;  /*body != null, it is a self-define operator*/
	FlowValue oper_values;
	OperatorType ot;
	paramList *params; //chenwenbin 20140723 operator引用的param变量
	int paramSize;  //chenwenbin 引用的param变量个数
	paramList *dimParams;//chenwenbin 用作数组维度的param变量
	constArrayList *ArrayInit; //cwb 记录初始化的数组变量aaa
} operatorNode;

typedef struct {
	ChildNode *id;
	ChildNode *wtype;
} windowNode;

typedef struct {
	TypeQual tq;
	ChildNode *sliding_value;
} slidingNode;

typedef struct {
	TypeQual tq;
	ChildNode *tumbling_value;
} tumblingNode;

/*-------7-------New For SPL----------*/
typedef struct {
	ChildNode *call;/*call->operDeclNode*/
	ChildNode *operdcl;
	Bool style;/*if style == true_, it is a SPL style call, else it is a StreamIt style call*/
	ChildNode *actual_composite;// zww：在指向深拷贝实际展开后的composite
} comCallNode;

typedef struct {
	ChildNode *output;
	ChildNode *input;
	ChildList *decl;
	ChildList *stmts;	/*splitjoin 和 pipeline中的语句
						包括：	普通c语言语句
								'add'关键字开头的调用语句
								分支判断语句 spIf和spIfElse
								循环语句spfor
						*/
	ChildNode *replace_composite; // SPL, for unfold.c
} PipelineNode;

typedef struct { 
	ChildNode *output;
	ChildNode *input;

	ChildList *decl;
	ChildList *initstmts;/* 为c语言初始化语句*/
	ChildNode *split;
	ChildList *stmts;	/*splitjoin 和 pipeline中的语句
						包括：	普通c语言语句
								'add'关键字开头的调用语句
								分支判断语句 spIf和spIfElse
								循环语句spfor
						*/
	ChildNode *join;
	ChildNode *replace_composite; // SPL, for unfold.c
	ChildNode *splitOperator;
	ChildNode *joinOperator;
} SplitJoinNode;

typedef struct {
	ChildNode *type;
} splitNode;

typedef struct {
	ChildNode *type;
} joinNode;

typedef struct {
	ChildList *arguments;
} roundrobinNode;

typedef struct {
	ChildNode *expr;
} duplicateNode;

/**********1*****新文法*******************/
typedef struct {
	ChildNode *content;/* */
} addNode;

typedef struct {
	const char* text;/*iterator count node*/
} itcoNode;


/***********************Define Nodes For SPL----over**********************/



/*************************************************************************/
/*                                                                       */
/*                            Node definition                            */
/*                                                                       */
/*************************************************************************/


struct nodeStruct {
	NodeType typ;
	Coord coord; /* location of first terminal in production */

	/* pass is incremented for each top-level call to PrintNode, so 
	   that PrintNode can print out decls anywhere they are used
	   without infinite recursion on recursive data structures. */
	short pass;

	/* parenthesized is set on expressions which were parenthesized
	   in the original source:  e.g., (x+y)*(w+z) would have 
	   parenthesized==TRUE on both PLUS nodes, and parenthesized==FALSE
	   on both MULT nodes. */
	short parenthesized;

	/* data-flow analysis information */
	Analysis analysis;

	union {
		adclNode adcl;
		arrayNode array;
		attribNode attrib;
		binopNode binop;
		BlockNode Block;
		BreakNode Break;
		callNode call;
		CaseNode Case;
		castNode cast;
		commaNode comma;
		ConstNode Const;
		ContinueNode Continue;
		declNode decl;
		DefaultNode Default;
		DoNode Do;
		edclNode edcl;
		fdclNode fdcl;
		ForNode For;		
		GotoNode Goto;
		idNode id;
		IfNode If;
		IfElseNode IfElse;
		implicitcastNode implicitcast;
		initializerNode initializer;
		labelNode label;
		primNode prim;
		procNode proc;
		ptrNode ptr;
		ReturnNode Return;
		sdclNode sdcl;
		SwitchNode Switch;
		tdefNode tdef;
		ternaryNode ternary;
		textNode text;
		udclNode udcl;
		unaryNode unary;
		WhileNode While;
		/*********13**************--------------Define For SPL----------****************************/
		strdclNode		strdcl;
		comDeclNode		comdcl;
		compositeNode	composite;
		comInOutNode	comInOut;
		comBodyNode		comBody;
		paramNode		param;
		operBodyNode	operBody;
		operDeclNode	operdcl;	
		operatorNode	operator_;
		windowNode		window;
		slidingNode		sliding;
		tumblingNode	tumbling;
		
		/*-------7-------New For SPL----------*/
		comCallNode		comCall;
		PipelineNode	pipeline;
		SplitJoinNode	splitJoin;
		splitNode		split;
		joinNode		join;
		roundrobinNode	roundrobin;				
		duplicateNode	duplicate;
		
		/**********1*****新文法*******************/
		addNode			add;
		itcoNode		itco;
		
	} u;
};




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

          ASTSWITCH, ASTWALK, LISTWALK


  These macros abstracts over a switch statement applied to 
  node->typ, to simplify operations on the syntax tree.


ASTSWITCH
  Parameters:
      node    abstract syntax tree node

      CODE    #defined macro taking three parameters:

                   name       enumerated constant corresponding to node->typ
		   node       same as node passed to ASTSWITCH
		   union      pointer to field in node->u appropriate for
		                 this type of node


  ASTSWITCH expands into a big switch statement.  The code after each
  case of the switch statement is CODE(name, node, union), followed by
  a break.  (Thus CODE does not need to include a break statement.)
  The default case of the switch statement, which is executed when the
  node type is invalid, is an assertion failure.

  Typically, CODE will concatenate its name parameter with a prefix to create
  the name of a method function, then call that function with some parameters.
  Examples may be found in SemCheckNode (sem-check.c) and PrintNode 
  (print-ast.c).



ASTWALK
  Parameters:
      node    abstract syntax tree node

      CODE    #defined macro taking one parameter, which is a child
              pointer of <node>.  The parameter may be used in an 
	      assignment to change the child pointer.


  ASTWALK expands CODE once for each non-nil child of <node>.  
  (Child pointers which are NULL will not be seen by CODE.)
  It also walks over child lists of <node>, calling CODE on each
  member.

  Typically CODE will make a recursive call on each child node.
  For examples, see NodeCopy and DiscardCoords (ast.c).



LISTWALK
  Parameters:
      l       List pointer

      CODE    #defined macro taking one parameter, which is a
              member of list l.  The parameter may be used in an
              assignment to change the member of the list.


  LISTWALK executes CODE  for each non-nil member of <l>.
  For examples, see the definition of ASTWALK below.



* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define ASTSWITCH(n, CODE)  \
switch (n->typ) { \
 case Const:         CODE(Const, n, &n->u.Const); break; \
 case Id:            CODE(Id, n, &n->u.id); break; \
 case Binop:         CODE(Binop, n, &n->u.binop); break; \
 case Unary:         CODE(Unary, n, &n->u.unary); break; \
 case Cast:          CODE(Cast, n, &n->u.cast); break; \
 case Comma:         CODE(Comma, n, &n->u.comma); break; \
 case Ternary:       CODE(Ternary, n, &n->u.ternary); break; \
 case Array:         CODE(Array, n, &n->u.array); break; \
 case Call:          CODE(Call, n, &n->u.call); break; \
 case Initializer:   CODE(Initializer, n, &n->u.initializer); break; \
 case ImplicitCast:  CODE(ImplicitCast, n, &n->u.implicitcast); break; \
 case Label:         CODE(Label, n, &n->u.label); break; \
 case Switch:        CODE(Switch, n, &n->u.Switch); break; \
 case Case:          CODE(Case, n, &n->u.Case); break; \
 case Default:       CODE(Default, n, &n->u.Default); break; \
 case If:            CODE(If, n, &n->u.If); break; \
 case IfElse:        CODE(IfElse, n, &n->u.IfElse); break; \
 case While:         CODE(While, n, &n->u.While); break; \
 case Do:            CODE(Do, n, &n->u.Do); break; \
 case For:           CODE(For, n, &n->u.For); break; \
 case Goto:          CODE(Goto, n, &n->u.Goto); break; \
 case Continue:      CODE(Continue, n, &n->u.Continue); break; \
 case Break:         CODE(Break, n, &n->u.Break); break; \
 case Return:        CODE(Return, n, &n->u.Return); break; \
 case Block:         CODE(Block, n, &n->u.Block); break; \
 case Prim:          CODE(Prim, n, &n->u.prim); break; \
 case Tdef:          CODE(Tdef, n, &n->u.tdef); break; \
 case Ptr:           CODE(Ptr, n, &n->u.ptr); break; \
 case Adcl:          CODE(Adcl, n, &n->u.adcl); break; \
 case Fdcl:          CODE(Fdcl, n, &n->u.fdcl); break; \
 case Sdcl:          CODE(Sdcl, n, &n->u.sdcl); break; \
 case Udcl:          CODE(Udcl, n, &n->u.udcl); break; \
 case Edcl:          CODE(Edcl, n, &n->u.edcl); break; \
 case Decl:          CODE(Decl, n, &n->u.decl); break; \
 case Attrib:		 CODE(Attrib, n, &n->u.attrib); break; \
 case Proc:          CODE(Proc, n, &n->u.proc); break; \
 case Text:          CODE(Text, n, &n->u.text); break; \
 /****************13******--------------Define For SPL----------****************************/ \
 case STRdcl:        CODE(STRdcl, n, &n->u.strdcl); break; \
 case Comdcl:        CODE(Comdcl, n, &n->u.comdcl); break; \
 case Composite:     CODE(Composite, n, &n->u.composite); break; \
 case ComInOut:      CODE(ComInOut, n, &n->u.comInOut); break; \
 case ComBody:       CODE(ComBody, n, &n->u.comBody); break; \
 case Param:         CODE(Param, n, &n->u.param); break; \
 case OperBody:      CODE(OperBody, n, &n->u.operBody); break; \
 case Operdcl:       CODE(Operdcl, n, &n->u.operdcl); break; \
 case Operator_:     CODE(Operator_, n, &n->u.operator_); break; \
 case Window:        CODE(Window, n, &n->u.window); break; \
 case Sliding:       CODE(Sliding, n, &n->u.sliding); break; \
 case Tumbling:      CODE(Tumbling, n, &n->u.tumbling); break; \
 /*---------7-----New For SPL----------*/\
 case CompositeCall: CODE(CompositeCall, n, &n->u.comCall); break; \
 case Pipeline:		 CODE(Pipeline, n, &n->u.pipeline); break; \
 case SplitJoin:	 CODE(SplitJoin, n, &n->u.splitJoin); break; \
 case Split:		 CODE(Split, n, &n->u.split); break; \
 case Join:			 CODE(Join, n, &n->u.join); break; \
 case RoundRobin:	 CODE(RoundRobin, n, &n->u.roundrobin); break; \
 case Duplicate:	 CODE(Duplicate, n, &n->u.duplicate); break; \
 /**************新文法******2*************/\
 case Add:	 		CODE(Add, n, &n->u.add); break; \
 /***********************--------------Define For SPL----------****************************/ \
 default:            FAIL("unexpected node type"); break; \
 }



#define ASTWALK(n, CODE)  \
switch ((n)->typ) { \
 case Const:         if ((n)->u.Const.type) {CODE((n)->u.Const.type);} break; \
 case Id:            break; \
 case Binop:         if ((n)->u.binop.left) {CODE((n)->u.binop.left);} if ((n)->u.binop.right) {CODE((n)->u.binop.right);} break; \
 case Unary:         if ((n)->u.unary.expr) {CODE((n)->u.unary.expr);} break; \
 case Cast:          if ((n)->u.cast.type) {CODE((n)->u.cast.type);} if ((n)->u.cast.expr) {CODE((n)->u.cast.expr);} break; \
 case Comma:         if ((n)->u.comma.exprs) {LISTWALK((n)->u.comma.exprs, CODE);} break; \
 case Ternary:       if ((n)->u.ternary.cond) {CODE((n)->u.ternary.cond);} if ((n)->u.ternary.true_) {CODE((n)->u.ternary.true_);} if ((n)->u.ternary.false_) {CODE((n)->u.ternary.false_);} break; \
 case Array:         if ((n)->u.array.name) {CODE((n)->u.array.name);} if ((n)->u.array.dims) {LISTWALK((n)->u.array.dims, CODE);} break; \
 case Call:          if ((n)->u.call.name) {CODE((n)->u.call.name);} if ((n)->u.call.args) {LISTWALK((n)->u.call.args, CODE);} break; \
 case Initializer:   if ((n)->u.initializer.exprs) {LISTWALK((n)->u.initializer.exprs, CODE);} break; \
 case ImplicitCast:  if ((n)->u.implicitcast.expr) {CODE((n)->u.implicitcast.expr);} break; \
 case Label:         if ((n)->u.label.stmt) {CODE((n)->u.label.stmt);} break; \
 case Switch:        if ((n)->u.Switch.expr) {CODE((n)->u.Switch.expr);} if ((n)->u.Switch.stmt) {CODE((n)->u.Switch.stmt);} break; \
 case Case:          if ((n)->u.Case.expr) {CODE((n)->u.Case.expr);} if ((n)->u.Case.stmt) {CODE((n)->u.Case.stmt);} break; \
 case Default:       if ((n)->u.Default.stmt) {CODE((n)->u.Default.stmt);} break; \
 case If:            if ((n)->u.If.expr) {CODE((n)->u.If.expr);} if ((n)->u.If.stmt) {CODE((n)->u.If.stmt);} break; \
 case IfElse:        if ((n)->u.IfElse.expr) {CODE((n)->u.IfElse.expr);} if ((n)->u.IfElse.true_) {CODE((n)->u.IfElse.true_);} if ((n)->u.IfElse.false_) {CODE((n)->u.IfElse.false_);} break; \
 case While:         if ((n)->u.While.expr) {CODE((n)->u.While.expr);} if ((n)->u.While.stmt) {CODE((n)->u.While.stmt);} break; \
 case Do:            if ((n)->u.Do.stmt) {CODE((n)->u.Do.stmt);} if ((n)->u.Do.expr) {CODE((n)->u.Do.expr);} break; \
 case For:           if ((n)->u.For.init) {CODE((n)->u.For.init);} if ((n)->u.For.cond) {CODE((n)->u.For.cond);} if ((n)->u.For.next) {CODE((n)->u.For.next);} if ((n)->u.For.stmt) {CODE((n)->u.For.stmt);} break; \
 case Goto:          break; \
 case Continue:      break; \
 case Break:         break; \
 case Return:        if ((n)->u.Return.expr) {CODE((n)->u.Return.expr);} break; \
 case Block:         if ((n)->u.Block.decl) {LISTWALK((n)->u.Block.decl, CODE);} if ((n)->u.Block.stmts) {LISTWALK((n)->u.Block.stmts, CODE);} break; \
 case Prim:          break; \
 case Tdef:          break; \
 case Ptr:           if ((n)->u.ptr.type) {CODE((n)->u.ptr.type);} break; \
 case Adcl:          if ((n)->u.adcl.type) {CODE((n)->u.adcl.type);} if ((n)->u.adcl.dim) {CODE((n)->u.adcl.dim);} break; \
 case Fdcl:          if ((n)->u.fdcl.args) {LISTWALK((n)->u.fdcl.args, CODE);} if ((n)->u.fdcl.returns) {CODE((n)->u.fdcl.returns);} break; \
 case Sdcl:          if (SUE_ELABORATED((n)->u.sdcl.tq) && (n)->u.sdcl.type->fields) {LISTWALK((n)->u.sdcl.type->fields, CODE);} break; \
 case Udcl:          if (SUE_ELABORATED((n)->u.udcl.tq) && (n)->u.udcl.type->fields) {LISTWALK((n)->u.udcl.type->fields, CODE);} break; \
 case Edcl:          if (SUE_ELABORATED((n)->u.edcl.tq) && (n)->u.edcl.type->fields) {LISTWALK((n)->u.edcl.type->fields, CODE);} break; \
 case Decl:          if ((n)->u.decl.type) {CODE((n)->u.decl.type);} if ((n)->u.decl.init) {CODE((n)->u.decl.init);} if ((n)->u.decl.bitsize) {CODE((n)->u.decl.bitsize);} break; \
 case Attrib:     if (n->u.attrib.arg) {CODE(n->u.attrib.arg);} break; \
 case Proc:          if ((n)->u.proc.decl) {CODE((n)->u.proc.decl);} if ((n)->u.proc.body) {CODE((n)->u.proc.body);} break; \
 case Text:          break; \
 /************13***********--------------zww:Define For SPL----------****************************/ \
 case STRdcl:        if (STREAM_ELABORATED((n)->u.strdcl.tq) && (n)->u.strdcl.type->fields) {LISTWALK((n)->u.strdcl.type->fields, CODE);} break;\
 case Comdcl:        if ((n)->u.comdcl.inout){CODE((n)->u.comdcl.inout);}break; \
 case Composite:     if ((n)->u.composite.decl){CODE((n)->u.composite.decl);}if ((n)->u.composite.body){CODE((n)->u.composite.body);} break;\
 case ComInOut:      if ((n)->u.comInOut.inputs){LISTWALK((n)->u.comInOut.inputs,CODE);}if ((n)->u.comInOut.outputs){LISTWALK((n)->u.comInOut.outputs,CODE);} break; \
 case ComBody:       if ((n)->u.comBody.param){CODE((n)->u.comBody.param);}  if ((n)->u.comBody.decl){LISTWALK((n)->u.comBody.decl,CODE);} if((n)->u.comBody.comstmts){LISTWALK((n)->u.comBody.comstmts ,CODE);} break;\
 case Param:         if ((n)->u.param.parameters){LISTWALK((n)->u.param.parameters ,CODE);} break;\
 case OperBody:      if ((n)->u.operBody.state){LISTWALK((n)->u.operBody.state ,CODE);}if ((n)->u.operBody.init) {CODE((n)->u.operBody.init);} if ((n)->u.operBody.work) {CODE((n)->u.operBody.work);} if ((n)->u.operBody.window){LISTWALK((n)->u.operBody.window ,CODE);} break;\
 case Operdcl:       if ((n)->u.operdcl.inputs){LISTWALK((n)->u.operdcl.inputs ,CODE);}if ((n)->u.operdcl.outputs){LISTWALK((n)->u.operdcl.outputs,CODE);}if ((n)->u.operdcl.arguments){LISTWALK((n)->u.operdcl.arguments ,CODE);}break; \
 case Operator_:     if ((n)->u.operator_.decl) {CODE((n)->u.operator_.decl);} if ((n)->u.operator_.body) {CODE((n)->u.operator_.body);}break; \
 case Window:        if ((n)->u.window.id) {CODE((n)->u.window.id);} if ((n)->u.window.wtype){CODE((n)->u.window.wtype);} break; \
 case Sliding:       if ((n)->u.sliding.sliding_value) {CODE((n)->u.sliding.sliding_value);} break; \
 case Tumbling:      if ((n)->u.tumbling.tumbling_value) {CODE((n)->u.tumbling.tumbling_value);} break; \
 /*-------7-------New For SPL----------*/\
 case CompositeCall: if ((n)->u.comCall.call) {CODE((n)->u.comCall.call);} if ((n)->u.comCall.operdcl) {CODE((n)->u.comCall.operdcl);}break;\
 case Pipeline:		 if ((n)->u.pipeline.output) {CODE((n)->u.pipeline.output);} if ((n)->u.pipeline.input) {CODE((n)->u.pipeline.input);} if ((n)->u.pipeline.decl){LISTWALK((n)->u.pipeline.decl ,CODE);} if ((n)->u.pipeline.stmts){LISTWALK((n)->u.pipeline.stmts ,CODE);}break;\
 case SplitJoin:	 if ((n)->u.splitJoin.output) {CODE((n)->u.splitJoin.output);}if ((n)->u.splitJoin.input) {CODE((n)->u.splitJoin.input);}if ((n)->u.splitJoin.decl){LISTWALK((n)->u.splitJoin.decl ,CODE);}if ((n)->u.splitJoin.initstmts){LISTWALK((n)->u.splitJoin.initstmts ,CODE);}if ((n)->u.splitJoin.split) {CODE((n)->u.splitJoin.split);}if ((n)->u.splitJoin.stmts){LISTWALK((n)->u.splitJoin.stmts ,CODE);}if ((n)->u.splitJoin.join) {CODE((n)->u.splitJoin.join);} break; \
 case Split:		 if ((n)->u.split.type) {CODE((n)->u.split.type);} break; \
 case Join:			 if ((n)->u.join.type) {CODE((n)->u.join.type);} break; \
 case RoundRobin:	 if ((n)->u.roundrobin.arguments){LISTWALK((n)->u.roundrobin.arguments ,CODE);}break; \
 case Duplicate:	 if ((n)->u.duplicate.expr) {CODE((n)->u.duplicate.expr);} break; \
 /***********1***新文法***************/\
 case Add:	 		if ((n)->u.add.content){CODE((n)->u.add.content);} break;\
 /***********************--------------zww:Define For SPL----------****************************/ \
 default:            FAIL("Unrecognized node type"); break; \
 }


#define LISTWALK(l, CODE)  \
{ ListMarker _listwalk_marker; Node *_listwalk_ref; \
  IterateList(&_listwalk_marker, l); \
  while (NextOnList(&_listwalk_marker, (GenericREF)&_listwalk_ref)) { \
     if (_listwalk_ref) {CODE(_listwalk_ref);}                     \
     SetCurrentOnList(&_listwalk_marker, (Generic *)_listwalk_ref); \
  }\
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
                            AST constructors
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
GLOBAL inline Node *NewNode(NodeType typ);

GLOBAL inline Node *MakeConstSint(int value);
GLOBAL inline Node *MakeConstSintTextCoord(const char *text, int value, Coord coord);
GLOBAL inline Node *MakeConstPtr(unsigned int value);
GLOBAL inline Node *MakeConstPtrTextCoord(const char *text, unsigned int value, Coord coord);
GLOBAL inline Node *MakeConstUint(unsigned int value);
GLOBAL inline Node *MakeConstUintTextCoord(const char *text, unsigned int value, Coord coord);
GLOBAL inline Node *MakeConstSlong(long value);
GLOBAL inline Node *MakeConstSlongTextCoord(const char *text, long value, Coord coord);
GLOBAL inline Node *MakeConstUlong(unsigned long value);
GLOBAL inline Node *MakeConstUlongTextCoord(const char *text, unsigned long value, Coord coord);
GLOBAL inline Node *MakeConstFloat(float value);
GLOBAL inline Node *MakeConstFloatTextCoord(const char *text, float value, Coord coord);
GLOBAL inline Node *MakeConstDouble(double value);
GLOBAL inline Node *MakeConstDoubleTextCoord(const char *text, double value, Coord coord);
GLOBAL inline Node *MakeString(const char *value);
GLOBAL inline Node *MakeStringTextCoord(const char *text, const char *value, Coord coord);
GLOBAL inline Node *MakeId(const char* text);
GLOBAL inline Node *MakeIdCoord(const char* text, Coord coord);
GLOBAL inline Node *MakeUnary(OpType op, Node *expr);
GLOBAL inline Node *MakeUnaryCoord(OpType op, Node *expr, Coord coord);
GLOBAL inline Node *MakeBinop(OpType op, Node *left, Node *right);
GLOBAL inline Node *MakeBinopCoord(OpType op, Node *left, Node *right, Coord coord);
GLOBAL inline Node *MakeCast(Node *type, Node *expr);
GLOBAL inline Node *MakeCastCoord(Node *type, Node *expr, Coord coord);
GLOBAL inline Node *MakeComma(List* exprs);
GLOBAL inline Node *MakeCommaCoord(List* exprs, Coord coord);
GLOBAL inline Node *MakeTernary(Node *cond, Node *true_, Node *false_);
GLOBAL inline Node *MakeTernaryCoord(Node *cond, Node *true_, Node *false_, Coord qmark_coord, Coord colon_coord);
GLOBAL inline Node *MakeArray(Node *name, List* dims);
GLOBAL inline Node *MakeArrayCoord(Node *name, List* dims, Coord coord);
GLOBAL inline Node *MakeCall(Node *name, List* args);
GLOBAL inline Node *MakeCallCoord(Node *name, List* args, Coord coord);
GLOBAL inline Node *MakeInitializer(List* exprs);
GLOBAL inline Node *MakeInitializerCoord(List* exprs, Coord coord);
GLOBAL inline Node *MakeImplicitCast(Node *type, Node *expr);
GLOBAL inline Node *MakeImplicitCastCoord(Node *type, Node *expr, Coord coord);
GLOBAL inline Node *MakeLabel(const char* name, Node *stmt);
GLOBAL inline Node *MakeLabelCoord(const char* name, Node *stmt, Coord coord);
GLOBAL inline Node *MakeSwitch(Node *expr, Node *stmt, List* cases);
GLOBAL inline Node *MakeSwitchCoord(Node *expr, Node *stmt, List* cases, Coord coord);
GLOBAL inline Node *MakeCase(Node *expr, Node *stmt, Node *container);
GLOBAL inline Node *MakeCaseCoord(Node *expr, Node *stmt, Node *container, Coord coord);
GLOBAL inline Node *MakeDefault(Node *stmt, Node *container);
GLOBAL inline Node *MakeDefaultCoord(Node *stmt, Node *container, Coord coord);
GLOBAL inline Node *MakeIf(Node *expr, Node *stmt);
GLOBAL inline Node *MakeIfCoord(Node *expr, Node *stmt, Coord coord);
GLOBAL inline Node *MakeIfElse(Node *expr, Node *true_, Node *false_);
GLOBAL inline Node *MakeIfElseCoord(Node *expr, Node *true_, Node *false_, Coord if_coord, Coord else_coord);
GLOBAL inline Node *MakeWhile(Node *expr, Node *stmt);
GLOBAL inline Node *MakeWhileCoord(Node *expr, Node *stmt, Coord coord);
GLOBAL inline Node *MakeDo(Node *stmt, Node *expr);
GLOBAL inline Node *MakeDoCoord(Node *stmt, Node *expr, Coord do_coord, Coord while_coord);
GLOBAL inline Node *MakeFor(Node *init, Node *cond, Node *next, Node *stmt);
GLOBAL inline Node *MakeForCoord(Node *init, Node *cond, Node *next, Node *stmt, Coord coord);
GLOBAL inline Node *MakeGoto(Node* label);
GLOBAL inline Node *MakeGotoCoord(Node* label, Coord coord);
GLOBAL inline Node *MakeContinue(Node *container);
GLOBAL inline Node *MakeContinueCoord(Node *container, Coord coord);
GLOBAL inline Node *MakeBreak(Node *container);
GLOBAL inline Node *MakeBreakCoord(Node *container, Coord coord);
GLOBAL inline Node *MakeReturn(Node *expr);
GLOBAL inline Node *MakeReturnCoord(Node *expr, Coord coord);
GLOBAL inline Node *MakeBlock(Node *type, List* decl, List* stmts);
GLOBAL inline Node *MakeBlockCoord(Node *type, List* decl, List* stmts, Coord left_coord, Coord right_coord);
GLOBAL inline Node *MakePrim(TypeQual tq, BasicType basic);
GLOBAL inline Node *MakePrimCoord(TypeQual tq, BasicType basic, Coord coord);
GLOBAL inline Node *MakeTdef(TypeQual tq, const char *name);
GLOBAL inline Node *MakeTdefCoord(TypeQual tq, const char *name, Coord coord);
GLOBAL inline Node *MakePtr(TypeQual tq, Node *type);
GLOBAL inline Node *MakePtrCoord(TypeQual tq, Node *type, Coord coord);
GLOBAL inline Node *MakeAdcl(TypeQual tq, Node *type, Node *dim);
GLOBAL inline Node *MakeAdclCoord(TypeQual tq, Node *type, Node *dim, Coord coord);
GLOBAL inline Node *MakeFdcl(TypeQual tq, List* args, Node *returns);
GLOBAL inline Node *MakeFdclCoord(TypeQual tq, List* args, Node *returns, Coord coord);
GLOBAL inline Node *MakeSdcl(TypeQual tq, SUEtype* type);
GLOBAL inline Node *MakeSdclCoord(TypeQual tq, SUEtype* type, Coord coord);
GLOBAL inline Node *MakeUdcl(TypeQual tq, SUEtype* type);
GLOBAL inline Node *MakeUdclCoord(TypeQual tq, SUEtype* type, Coord coord);
GLOBAL inline Node *MakeEdcl(TypeQual tq, SUEtype* type);
GLOBAL inline Node *MakeEdclCoord(TypeQual tq, SUEtype* type, Coord coord);
GLOBAL inline Node *MakeDecl(const char *name, TypeQual tq, Node *type, Node *init, Node *bitsize);
GLOBAL inline Node *MakeDeclCoord(const char *name, TypeQual tq, Node *type, Node *init, Node *bitsize, Coord coord);
GLOBAL inline Node *MakeAttrib(const char *name, Node *arg);
GLOBAL inline Node *MakeAttribCoord(const char *name, Node *arg, Coord coord);
GLOBAL inline Node *MakeProc(Node *decl, Node *body);
GLOBAL inline Node *MakeProcCoord(Node *decl, Node *body, Coord coord);
GLOBAL inline Node *MakeText(const char *text, Bool start_new_line);
GLOBAL inline Node *MakeTextCoord(const char *text, Bool start_new_line, Coord coord);

/* Insert your new constructors here */
/***********************--------------Define For SPL----------****************************/
/*
修改了comBodyNode PipelineNode SplitJoinNode
*/
GLOBAL void SetIsInComposite(Bool inSteamit);
GLOBAL Node *InsertStreamIds(Node *decl);

GLOBAL inline Node *MakeComdcl(TypeQual tq, Node *inout);
GLOBAL inline Node *MakeComdclCoord(TypeQual tq, Node *inout, Coord coord);

GLOBAL inline Node *MakeComposite(Node *decl, Node *body, Bool multi);
GLOBAL inline Node *MakeCompositeCoord(Node *decl, Node *body, Bool multi, Coord coord);

GLOBAL inline Node *MakeComInOut(TypeQual tq, List *inputs, List *outputs);
GLOBAL inline Node *MakeComInOutCoord(TypeQual tq, List *inputs, List *outputs, Coord coord);

GLOBAL inline Node *MakeComBody(Node *type, Node *param,List *decl, List *comstmts);
GLOBAL inline Node *MakeComBodyCoord(Node *type, Node *param,List *decl, List *comstmts, Coord left_coord, Coord right_coord);

GLOBAL inline Node *MakeParam(List *parameters);
GLOBAL inline Node *MakeParamCoord(List *parameters, Coord coord);

GLOBAL inline Node *MakeOperBody(Node *type, List *state, Node *init, Node *work, List *window);
GLOBAL inline Node *MakeOperBodyCoord(Node *type, List *state, Node *init, Node *work, List *window, Coord left_coord, Coord right_coord);

GLOBAL inline Node *MakeOperdcl(TypeQual tq, List *outputs, List *inputs, List *arguments);
GLOBAL inline Node *MakeOperdclCoord(TypeQual tq, List *outputs, List *inputs, List *arguments, Coord coord);

GLOBAL inline Node *MakeOperator(Node *decl, Node *body);
GLOBAL inline Node *MakeOperatorCoord(Node *decl, Node *body, Coord coord);

GLOBAL inline Node *MakeWindow(Node *id, Node *wtype);
GLOBAL inline Node *MakeWindowCoord(Node *id, Node *wtype, Coord coord);

GLOBAL inline Node *MakeWindowSliding(TypeQual tq, Node *sliding_value);
GLOBAL inline Node *MakeWindowSlidingCoord(TypeQual tq, Node *sliding_value, Coord coord);

GLOBAL inline Node *MakeWindowTumbling(TypeQual tq, Node *tumbling_value);
GLOBAL inline Node *MakeWindowTumbingCoord(TypeQual tq, Node *tumbling_value, Coord coord);

/*--------------New For SPL----------*/
GLOBAL inline Node *MakeCompositeCall(Node *call, Node *operdcl, Bool style);
GLOBAL inline Node *MakeCompositeCallCoord(Node *call, Node *operdcl, Bool style, Coord coord);

GLOBAL inline Node *MakePipeline(Node *output, Node *input, List *decl,List *stmts);
GLOBAL inline Node *MakePipelineCoord(Node *output, Node *input,List *decl,List *stmts, Coord coord);

GLOBAL inline Node *MakeSplitJoin(Node *output, Node *input, List *decl,List *initstmts,Node *split,List *stmts, Node *join);
GLOBAL inline Node *MakeSplitJoinCoord(Node *output, Node *input,List *decl,List *initstmts,Node *split,List *stmts, Node *join, Coord coord);

GLOBAL inline Node *MakeSplit(Node *type);
GLOBAL inline Node *MakeSplitCoord(Node *type, Coord coord);

GLOBAL inline Node *MakeJoin(Node *type);
GLOBAL inline Node *MakeJoinCoord(Node *type, Coord coord);

GLOBAL inline Node *MakeRoundRobin(List *arguments);
GLOBAL inline Node *MakeRoundRobinCoord(List *arguments, Coord coord);

GLOBAL inline Node *MakeDuplicate(Node *expr);
GLOBAL inline Node *MakeDuplicateCoord(Node *expr, Coord coord);

GLOBAL inline Node *MakeSTRdcl(TypeQual tq, StreamType *type);
GLOBAL inline Node *MakeSTRdclCoord(TypeQual tq, StreamType *type, Coord coord);
/***************新文法*************/
GLOBAL inline Node *MakeAdd(Node *content);
GLOBAL inline Node *MakeAddCoord(Node *content,Coord coord);

GLOBAL inline Node *MakeOutput(Node *node,Node *output);
GLOBAL inline Node *MakeOutputList(Node *node,List *outputs);

/**************新文法结束***************/

/**************文法优化***************/
GLOBAL inline Node *MakeIterCount();
GLOBAL inline Node *MakeIterCountCoord(Coord coord);

GLOBAL Node *LookupCompositeIdsNode(Node *id);
GLOBAL Node *LookupStreamIdsNode(Node *id);
GLOBAL List *LookupStreamIdsList(List *idList);
GLOBAL Node *AddDeclNodeConstTq(Node *node);/* SPL */

GLOBAL Node *DefineComposite(Node *decl);
GLOBAL Node *SetCompositeBody(Node *composite, Node *comBody);
GLOBAL Node *DefineOperator(Node *decl);
GLOBAL Node *SetOperatorBody(Node *operator_, Node *operBody);
GLOBAL inline char *AddPostFixName(const char *name);

GLOBAL Node *compositeCopy(Node *from,List *composite_inputList,List *composite_outputList);

/*workEstimate.c*/
GLOBAL int workEstimate(Node *from,int w);/*define for spl*/
GLOBAL int workEstimate_init(Node *from,int w);/*define for spl*/
GLOBAL int workEstimateUseDataFlow(Node *from,int w);/*use dataflow method to estimate work */
/* 修改C2C原有函数的访问权限 */
GLOBAL BasicType BasicTypeOfConstantValue(Node *type);
/***********************--------------Define For SPL----------****************************/



/* End of new constructors */


GLOBAL Node *ConvertIdToTdef(Node *id, TypeQual tq, Node *type);
GLOBAL Node *ConvertIdToDecl(Node *id, TypeQual tq, Node *type, Node *init, Node *bitsize);
GLOBAL Node *ConvertIdToAttrib(Node *id, Node *arg);



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
                           AST operations
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


typedef enum {
  NodeOnly,
  Subtree
} TreeOpDepth;

typedef enum {
  Preorder,
  Postorder
} WalkOrder;

typedef void (*WalkProc) (Node *, void *);

GLOBAL Node *NodeCopy(Node *from, TreeOpDepth d);
GLOBAL void SetCoords(Node *tree, Coord c, TreeOpDepth d);
GLOBAL void WalkTree(Node *tree, void (*proc) (), void *ptr, WalkOrder order);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
                           AST predicates
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

GLOBAL inline Bool IsExpr(Node *node);
GLOBAL inline Bool IsStmt(Node *node);
GLOBAL inline Bool IsType(Node *node);
GLOBAL inline Bool IsDecl(Node *node);

typedef int Kinds;
#define KIND_EXPR  1
#define KIND_STMT  2
#define KIND_TYPE  4
#define KIND_DECL  8//1,2,4,8对应的二进制位均不重复

GLOBAL inline Kinds KindsOfNode(Node *node);


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
               Other AST declarations and prototypes
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* operators.c */
#include "operators.h"
#ifndef _Y_TAB_H_
#define _Y_TAB_H_
#include "y.tab.h"  /* for definitions of operator symbols */
#endif

/* container.c */
GLOBAL void PushContainer(NodeType typ);
GLOBAL Node *PopContainer(Node *n);
GLOBAL Node *AddContainee(Node *c);

/* from various files */
#include "type.h"

#endif /* ifndef _AST_H_ */

