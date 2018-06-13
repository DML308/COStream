
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "ANSI-C.y"


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






/* Line 189 of yacc.c  */
#line 241 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     AUTO = 258,
     DOUBLE = 259,
     INT = 260,
     STRUCT = 261,
     BREAK = 262,
     ELSE = 263,
     LONG = 264,
     SWITCH = 265,
     CASE = 266,
     ENUM = 267,
     REGISTER = 268,
     TYPEDEF = 269,
     CHAR = 270,
     EXTERN = 271,
     RETURN = 272,
     UNION = 273,
     CONST = 274,
     FLOAT = 275,
     SHORT = 276,
     UNSIGNED = 277,
     CONTINUE = 278,
     FOR = 279,
     SIGNED = 280,
     VOID = 281,
     DEFAULT = 282,
     GOTO = 283,
     SIZEOF = 284,
     VOLATILE = 285,
     DO = 286,
     IF = 287,
     STATIC = 288,
     WHILE = 289,
     COMPOSITE = 290,
     INPUT = 291,
     OUTPUT = 292,
     STREAM = 293,
     PARAM = 294,
     ADD = 295,
     INIT = 296,
     WORK = 297,
     WINDOW = 298,
     TUMBLING = 299,
     SLIDING = 300,
     SPLITJOIN = 301,
     SPLIT = 302,
     JOIN = 303,
     DUPLICATE = 304,
     ROUNDROBIN = 305,
     PIPELINE = 306,
     FILEREADER = 307,
     FILEWRITER = 308,
     UPLUS = 309,
     UMINUS = 310,
     INDIR = 311,
     ADDRESS = 312,
     POSTINC = 313,
     POSTDEC = 314,
     PREINC = 315,
     PREDEC = 316,
     BOGUS = 317,
     IDENTIFIER = 318,
     STRINGliteral = 319,
     FLOATINGconstant = 320,
     INTEGERconstant = 321,
     OCTALconstant = 322,
     HEXconstant = 323,
     WIDECHARconstant = 324,
     CHARACTERconstant = 325,
     TYPEDEFname = 326,
     ARROW = 327,
     ICR = 328,
     DECR = 329,
     LS = 330,
     RS = 331,
     LE = 332,
     GE = 333,
     EQ = 334,
     NE = 335,
     ANDAND = 336,
     OROR = 337,
     ELLIPSIS = 338,
     MULTassign = 339,
     DIVassign = 340,
     MODassign = 341,
     PLUSassign = 342,
     MINUSassign = 343,
     LSassign = 344,
     RSassign = 345,
     ANDassign = 346,
     ERassign = 347,
     ORassign = 348,
     INLINE = 349,
     ATTRIBUTE = 350
   };
#endif
/* Tokens.  */
#define AUTO 258
#define DOUBLE 259
#define INT 260
#define STRUCT 261
#define BREAK 262
#define ELSE 263
#define LONG 264
#define SWITCH 265
#define CASE 266
#define ENUM 267
#define REGISTER 268
#define TYPEDEF 269
#define CHAR 270
#define EXTERN 271
#define RETURN 272
#define UNION 273
#define CONST 274
#define FLOAT 275
#define SHORT 276
#define UNSIGNED 277
#define CONTINUE 278
#define FOR 279
#define SIGNED 280
#define VOID 281
#define DEFAULT 282
#define GOTO 283
#define SIZEOF 284
#define VOLATILE 285
#define DO 286
#define IF 287
#define STATIC 288
#define WHILE 289
#define COMPOSITE 290
#define INPUT 291
#define OUTPUT 292
#define STREAM 293
#define PARAM 294
#define ADD 295
#define INIT 296
#define WORK 297
#define WINDOW 298
#define TUMBLING 299
#define SLIDING 300
#define SPLITJOIN 301
#define SPLIT 302
#define JOIN 303
#define DUPLICATE 304
#define ROUNDROBIN 305
#define PIPELINE 306
#define FILEREADER 307
#define FILEWRITER 308
#define UPLUS 309
#define UMINUS 310
#define INDIR 311
#define ADDRESS 312
#define POSTINC 313
#define POSTDEC 314
#define PREINC 315
#define PREDEC 316
#define BOGUS 317
#define IDENTIFIER 318
#define STRINGliteral 319
#define FLOATINGconstant 320
#define INTEGERconstant 321
#define OCTALconstant 322
#define HEXconstant 323
#define WIDECHARconstant 324
#define CHARACTERconstant 325
#define TYPEDEFname 326
#define ARROW 327
#define ICR 328
#define DECR 329
#define LS 330
#define RS 331
#define LE 332
#define GE 333
#define EQ 334
#define NE 335
#define ANDAND 336
#define OROR 337
#define ELLIPSIS 338
#define MULTassign 339
#define DIVassign 340
#define MODassign 341
#define PLUSassign 342
#define MINUSassign 343
#define LSassign 344
#define RSassign 345
#define ANDassign 346
#define ERassign 347
#define ORassign 348
#define INLINE 349
#define ATTRIBUTE 350




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 211 "ANSI-C.y"

    Node      *n;
    List      *L;

  /* tq: type qualifiers */
    struct {
        TypeQual   tq;
	Coord      coord;   /* coordinates where type quals began */ 
    } tq;

  /* tok: token coordinates */
    Coord tok;



/* Line 214 of yacc.c  */
#line 483 "y.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 495 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  75
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4955

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  120
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  181
/* YYNRULES -- Number of rules.  */
#define YYNRULES  475
/* YYNRULES -- Number of states.  */
#define YYNSTATES  837

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   350

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     8,     2,     2,     2,    13,     3,     2,
      15,    16,     4,     5,    22,     6,    17,    14,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    25,    19,
       9,   107,    10,    18,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    23,     2,    24,    12,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    20,    11,    21,     7,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,    10,    13,    17,    22,    28,    29,
      33,    39,    40,    46,    49,    52,    58,    60,    64,    67,
      72,    78,    79,    83,    85,    88,    90,    92,    99,   102,
     105,   108,   113,   119,   126,   134,   143,   146,   149,   151,
     153,   156,   159,   162,   167,   173,   178,   184,   188,   192,
     195,   201,   208,   209,   212,   215,   216,   221,   223,   226,
     230,   234,   238,   243,   248,   250,   252,   254,   258,   264,
     271,   273,   278,   282,   287,   292,   298,   303,   309,   319,
     330,   342,   350,   359,   364,   368,   372,   375,   378,   382,
     386,   388,   392,   394,   397,   400,   403,   406,   411,   413,
     415,   417,   419,   421,   423,   425,   430,   432,   436,   440,
     444,   446,   450,   454,   456,   460,   464,   466,   470,   474,
     478,   482,   484,   488,   492,   494,   498,   500,   504,   506,
     510,   512,   516,   518,   522,   524,   530,   532,   536,   538,
     540,   542,   544,   546,   548,   550,   552,   554,   556,   558,
     560,   564,   566,   567,   569,   572,   575,   578,   581,   582,
     583,   590,   591,   592,   599,   600,   601,   609,   610,   616,
     617,   623,   627,   628,   629,   636,   639,   640,   641,   648,
     649,   650,   658,   659,   665,   666,   672,   676,   678,   680,
     682,   685,   688,   691,   694,   697,   700,   703,   706,   709,
     712,   714,   717,   720,   722,   724,   726,   728,   730,   732,
     735,   738,   741,   743,   746,   749,   751,   754,   757,   759,
     762,   764,   767,   769,   771,   773,   775,   777,   779,   781,
     784,   789,   795,   799,   803,   808,   813,   815,   819,   821,
     824,   826,   828,   831,   835,   839,   844,   846,   848,   850,
     852,   855,   858,   862,   866,   870,   874,   879,   881,   884,
     888,   890,   892,   894,   897,   901,   904,   908,   913,   915,
     919,   921,   924,   928,   933,   937,   942,   944,   948,   950,
     952,   954,   957,   959,   962,   963,   965,   967,   970,   977,
     979,   983,   984,   986,   991,   993,   995,   997,   998,  1001,
    1005,  1010,  1012,  1014,  1018,  1020,  1024,  1026,  1028,  1032,
    1036,  1040,  1042,  1045,  1048,  1051,  1053,  1056,  1059,  1061,
    1064,  1067,  1070,  1072,  1075,  1078,  1081,  1085,  1090,  1094,
    1099,  1105,  1108,  1112,  1117,  1119,  1121,  1123,  1126,  1129,
    1132,  1135,  1139,  1142,  1146,  1150,  1153,  1157,  1160,  1161,
    1163,  1166,  1172,  1179,  1182,  1185,  1190,  1191,  1194,  1195,
    1197,  1199,  1201,  1203,  1205,  1207,  1209,  1211,  1213,  1216,
    1217,  1222,  1227,  1231,  1235,  1238,  1242,  1246,  1251,  1253,
    1255,  1258,  1262,  1266,  1271,  1273,  1276,  1278,  1281,  1284,
    1290,  1298,  1299,  1306,  1307,  1314,  1315,  1324,  1325,  1336,
    1337,  1348,  1349,  1360,  1361,  1372,  1373,  1380,  1384,  1387,
    1390,  1394,  1398,  1400,  1403,  1405,  1407,  1409,  1410,  1414,
    1417,  1418,  1423,  1424,  1429,  1430,  1435,  1436,  1441,  1442,
    1446,  1447,  1452,  1453,  1458,  1459,  1464,  1465,  1470,  1471,
    1476,  1477,  1483,  1484,  1490,  1491,  1497,  1498,  1504,  1505,
    1508,  1510,  1512,  1514,  1516,  1518,  1520,  1523,  1525,  1527,
    1529,  1531,  1533,  1535,  1537,  1539,  1541,  1543,  1545,  1547,
    1549,  1551,  1553,  1555,  1557,  1559
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     121,     0,    -1,   275,    -1,    61,     9,   123,    10,    -1,
     199,   220,    -1,   199,   220,   216,    -1,   123,    22,   199,
     220,    -1,   123,    22,   199,   220,   216,    -1,    -1,   126,
     125,   130,    -1,    58,    86,    15,   127,    16,    -1,    -1,
      59,   128,    22,    60,   128,    -1,    59,   128,    -1,    60,
     128,    -1,    60,   128,    22,    59,   128,    -1,   129,    -1,
     128,    22,   129,    -1,   122,    86,    -1,    20,   131,   132,
      21,    -1,    20,   131,   261,   132,    21,    -1,    -1,    62,
     236,    19,    -1,   253,    -1,   132,   253,    -1,   135,    -1,
     134,    -1,    76,    15,    86,    16,   144,    19,    -1,    63,
     143,    -1,    63,   136,    -1,    63,   137,    -1,    74,   258,
     139,   259,    -1,    74,   258,   261,   139,   259,    -1,    69,
     258,   138,   139,   140,   259,    -1,    69,   258,   261,   138,
     139,   140,   259,    -1,    69,   258,   261,   262,   138,   139,
     140,   259,    -1,    70,   142,    -1,    70,   141,    -1,   254,
      -1,   135,    -1,   139,   254,    -1,   139,   135,    -1,    71,
     141,    -1,    73,    15,    16,    19,    -1,    73,    15,   154,
      16,    19,    -1,    72,    15,    16,    19,    -1,    72,    15,
     169,    16,    19,    -1,    86,   144,    19,    -1,    15,   154,
      16,    -1,    15,    16,    -1,   258,   146,   147,   148,   259,
      -1,   258,   261,   146,   147,   148,   259,    -1,    -1,    64,
     257,    -1,    65,   257,    -1,    -1,    66,    20,   149,    21,
      -1,   150,    -1,   149,   150,    -1,    86,   151,    19,    -1,
      68,    15,    16,    -1,    67,    15,    16,    -1,    68,    15,
     171,    16,    -1,    67,    15,   171,    16,    -1,    86,    -1,
     295,    -1,   296,    -1,    15,   171,    16,    -1,    15,   258,
     262,   259,    16,    -1,    15,   258,   261,   262,   259,    16,
      -1,   152,    -1,   153,    23,   171,    24,    -1,   153,    15,
      16,    -1,   153,    15,   154,    16,    -1,   153,    15,    16,
     145,    -1,   153,    15,   154,    16,   145,    -1,   153,    15,
      16,   144,    -1,   153,    15,   154,    16,   144,    -1,    69,
      15,    86,    16,   258,   138,   139,   140,   259,    -1,    69,
      15,    86,    16,   258,   261,   138,   139,   140,   259,    -1,
      69,    15,    86,    16,   258,   261,   262,   138,   139,   140,
     259,    -1,    74,    15,    86,    16,   258,   139,   259,    -1,
      74,    15,    86,    16,   258,   261,   139,   259,    -1,    75,
      15,    16,   144,    -1,   153,    17,    86,    -1,   153,    95,
      86,    -1,   153,    96,    -1,   153,    97,    -1,   153,    17,
      94,    -1,   153,    95,    94,    -1,   169,    -1,   154,    22,
     169,    -1,   153,    -1,    96,   155,    -1,    97,   155,    -1,
     156,   157,    -1,    52,   155,    -1,    52,    15,   225,    16,
      -1,     3,    -1,     4,    -1,     5,    -1,     6,    -1,     7,
      -1,     8,    -1,   155,    -1,    15,   225,    16,   157,    -1,
     157,    -1,   158,     4,   157,    -1,   158,    14,   157,    -1,
     158,    13,   157,    -1,   158,    -1,   159,     5,   158,    -1,
     159,     6,   158,    -1,   159,    -1,   160,    98,   159,    -1,
     160,    99,   159,    -1,   160,    -1,   161,     9,   160,    -1,
     161,    10,   160,    -1,   161,   100,   160,    -1,   161,   101,
     160,    -1,   161,    -1,   162,   102,   161,    -1,   162,   103,
     161,    -1,   162,    -1,   163,     3,   162,    -1,   163,    -1,
     164,    12,   163,    -1,   164,    -1,   165,    11,   164,    -1,
     165,    -1,   166,   104,   165,    -1,   166,    -1,   167,   105,
     166,    -1,   167,    -1,   167,    18,   171,    25,   168,    -1,
     168,    -1,   155,   170,   169,    -1,   107,    -1,   108,    -1,
     109,    -1,   110,    -1,   111,    -1,   112,    -1,   113,    -1,
     114,    -1,   115,    -1,   116,    -1,   117,    -1,   169,    -1,
     171,    22,   169,    -1,   168,    -1,    -1,   171,    -1,   175,
      19,    -1,   184,    19,    -1,   195,    19,    -1,   201,    19,
      -1,    -1,    -1,   193,   206,   176,   226,   177,   232,    -1,
      -1,    -1,   199,   206,   178,   226,   179,   232,    -1,    -1,
      -1,   175,    22,   206,   180,   226,   181,   232,    -1,    -1,
     193,     1,   182,   226,   232,    -1,    -1,   199,     1,   183,
     226,   232,    -1,   175,    22,     1,    -1,    -1,    -1,   197,
     217,   185,   226,   186,   232,    -1,   122,   217,    -1,    -1,
      -1,   203,   217,   187,   226,   188,   232,    -1,    -1,    -1,
     184,    22,   217,   189,   226,   190,   232,    -1,    -1,   197,
       1,   191,   226,   232,    -1,    -1,   203,     1,   192,   226,
     232,    -1,   184,    22,     1,    -1,   194,    -1,   195,    -1,
     196,    -1,   200,   299,    -1,   197,   300,    -1,   194,   198,
      -1,   194,   300,    -1,   201,   299,    -1,   197,   205,    -1,
     195,   198,    -1,   202,   299,    -1,   197,    94,    -1,   196,
     198,    -1,   299,    -1,   203,   299,    -1,   197,   198,    -1,
     297,    -1,   299,    -1,   200,    -1,   201,    -1,   202,    -1,
     300,    -1,   203,   300,    -1,   200,   297,    -1,   200,   300,
      -1,   205,    -1,   203,   205,    -1,   201,   297,    -1,    94,
      -1,   203,    94,    -1,   202,   297,    -1,   297,    -1,   203,
     297,    -1,   298,    -1,   204,   298,    -1,   239,    -1,   249,
      -1,   207,    -1,   210,    -1,   217,    -1,   221,    -1,   208,
      -1,     4,   207,    -1,     4,    15,   209,    16,    -1,     4,
     204,    15,   209,    16,    -1,     4,   204,   207,    -1,    15,
     207,    16,    -1,    15,   209,   216,    16,    -1,    15,   207,
      16,   216,    -1,    94,    -1,    15,   209,    16,    -1,    94,
      -1,    94,   216,    -1,   211,    -1,   212,    -1,     4,   210,
      -1,     4,   204,   210,    -1,    15,   211,    16,    -1,    15,
     211,    16,   216,    -1,   214,    -1,   215,    -1,   216,    -1,
       4,    -1,     4,   204,    -1,     4,   213,    -1,     4,   204,
     213,    -1,    15,   214,    16,    -1,    15,   215,    16,    -1,
      15,   216,    16,    -1,    15,   214,    16,   216,    -1,   238,
      -1,    15,    16,    -1,    15,   235,    16,    -1,   218,    -1,
     220,    -1,   219,    -1,     4,   217,    -1,     4,   204,   217,
      -1,   220,   216,    -1,    15,   218,    16,    -1,    15,   218,
      16,   216,    -1,    86,    -1,    15,   220,    16,    -1,   222,
      -1,     4,   221,    -1,     4,   204,   221,    -1,   220,    15,
     223,    16,    -1,    15,   221,    16,    -1,    15,   221,    16,
     216,    -1,    86,    -1,   223,    22,    86,    -1,    86,    -1,
      94,    -1,   199,    -1,   199,   213,    -1,   203,    -1,   203,
     213,    -1,    -1,   227,    -1,   228,    -1,   227,   228,    -1,
     119,    15,    15,   229,    16,    16,    -1,   230,    -1,   229,
      22,   230,    -1,    -1,   231,    -1,   231,    15,   171,    16,
      -1,    86,    -1,    94,    -1,    42,    -1,    -1,   107,   233,
      -1,    20,   234,    21,    -1,    20,   234,    22,    21,    -1,
     169,    -1,   233,    -1,   234,    22,   233,    -1,   236,    -1,
     236,    22,   106,    -1,   106,    -1,   237,    -1,   236,    22,
     237,    -1,   237,   107,   233,    -1,   236,    22,     1,    -1,
     193,    -1,   193,   213,    -1,   193,   217,    -1,   193,   210,
      -1,   197,    -1,   197,   213,    -1,   197,   217,    -1,   199,
      -1,   199,   213,    -1,   199,   217,    -1,   199,   210,    -1,
     203,    -1,   203,   213,    -1,   203,   217,    -1,    23,    24,
      -1,    23,   172,    24,    -1,   238,    23,   172,    24,    -1,
     238,    23,    24,    -1,   240,    20,   241,    21,    -1,   240,
     224,    20,   241,    21,    -1,   240,   224,    -1,   240,    20,
      21,    -1,   240,   224,    20,    21,    -1,    29,    -1,    41,
      -1,   242,    -1,   241,   242,    -1,   244,    19,    -1,   243,
      19,    -1,   203,   246,    -1,   243,    22,   246,    -1,   199,
     245,    -1,   244,    22,   245,    -1,   206,   247,   226,    -1,
     248,   226,    -1,   217,   247,   226,    -1,   248,   226,    -1,
      -1,   248,    -1,    25,   172,    -1,    35,    20,   250,   252,
      21,    -1,    35,   224,    20,   250,   252,    21,    -1,    35,
     224,    -1,   224,   251,    -1,   250,    22,   224,   251,    -1,
      -1,   107,   172,    -1,    -1,    22,    -1,   133,    -1,   254,
      -1,   255,    -1,   257,    -1,   263,    -1,   264,    -1,   266,
      -1,   274,    -1,     1,    19,    -1,    -1,    86,    25,   256,
     254,    -1,    34,   172,    25,   254,    -1,    50,    25,   254,
      -1,    94,    25,   254,    -1,   258,   259,    -1,   258,   261,
     259,    -1,   258,   132,   259,    -1,   258,   261,   132,   259,
      -1,    20,    -1,    21,    -1,    20,    21,    -1,    20,   261,
      21,    -1,    20,   262,    21,    -1,    20,   261,   262,    21,
      -1,   174,    -1,   261,   174,    -1,   254,    -1,   262,   254,
      -1,   173,    19,    -1,    55,    15,   171,    16,   253,    -1,
      55,    15,   171,    16,   253,    31,   253,    -1,    -1,    33,
     265,    15,   171,    16,   254,    -1,    -1,    57,   267,    15,
     171,    16,   254,    -1,    -1,    54,   268,   254,    57,    15,
     171,    16,    19,    -1,    -1,    47,    15,   173,    19,   173,
      19,   173,    16,   269,   253,    -1,    -1,    47,    15,     1,
      19,   173,    19,   173,    16,   270,   253,    -1,    -1,    47,
      15,   173,    19,   173,    19,     1,    16,   271,   253,    -1,
      -1,    47,    15,   173,    19,     1,    19,   173,    16,   272,
     253,    -1,    -1,    47,    15,     1,    16,   273,   253,    -1,
      51,    86,    19,    -1,    46,    19,    -1,    30,    19,    -1,
      40,   173,    19,    -1,    51,    94,    19,    -1,   276,    -1,
     275,   276,    -1,   174,    -1,   277,    -1,   124,    -1,    -1,
     217,   278,   260,    -1,   217,    85,    -1,    -1,   193,   217,
     279,   260,    -1,    -1,   199,   217,   280,   260,    -1,    -1,
     197,   217,   281,   260,    -1,    -1,   203,   217,   282,   260,
      -1,    -1,   221,   283,   260,    -1,    -1,   193,   221,   284,
     260,    -1,    -1,   199,   221,   285,   260,    -1,    -1,   197,
     221,   286,   260,    -1,    -1,   203,   221,   287,   260,    -1,
      -1,   221,   293,   288,   260,    -1,    -1,   193,   221,   293,
     289,   260,    -1,    -1,   199,   221,   293,   290,   260,    -1,
      -1,   197,   221,   293,   291,   260,    -1,    -1,   203,   221,
     293,   292,   260,    -1,    -1,   294,   261,    -1,    88,    -1,
      89,    -1,    90,    -1,    91,    -1,    93,    -1,    87,    -1,
     296,    87,    -1,    42,    -1,    53,    -1,   118,    -1,    42,
      -1,    53,    -1,    37,    -1,    39,    -1,    56,    -1,    26,
      -1,    36,    -1,    49,    -1,    38,    -1,    28,    -1,    43,
      -1,    27,    -1,    48,    -1,    45,    -1,    44,    -1,    32,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   386,   386,   397,   404,   408,   414,   418,   430,   429,
     444,   454,   455,   459,   463,   467,   474,   478,   485,   493,
     498,   507,   508,   517,   521,   531,   535,   543,   550,   554,
     558,   565,   569,   577,   581,   585,   592,   597,   605,   609,
     613,   617,   624,   632,   636,   643,   647,   655,   665,   671,
     680,   685,   693,   694,   702,   711,   712,   719,   723,   730,
     738,   743,   748,   753,   775,   776,   777,   778,   782,   786,
     793,   794,   796,   798,   801,   808,   815,   822,   829,   833,
     837,   841,   845,   849,   855,   857,   859,   861,   865,   867,
     872,   874,   879,   881,   883,   885,   888,   890,   895,   896,
     897,   898,   899,   900,   904,   905,   909,   910,   912,   914,
     919,   920,   922,   927,   928,   930,   935,   936,   938,   940,
     942,   947,   948,   950,   955,   956,   961,   962,   970,   971,
     978,   979,   984,   985,   992,   993,   998,   999,  1018,  1019,
    1020,  1021,  1022,  1023,  1024,  1025,  1026,  1027,  1028,  1032,
    1033,  1048,  1052,  1053,  1089,  1091,  1093,  1095,  1102,  1105,
    1101,  1111,  1114,  1110,  1120,  1123,  1119,  1133,  1132,  1143,
    1142,  1151,  1158,  1161,  1157,  1166,  1172,  1175,  1171,  1181,
    1182,  1180,  1189,  1188,  1199,  1198,  1207,  1212,  1214,  1215,
    1221,  1223,  1225,  1227,  1234,  1236,  1238,  1245,  1247,  1249,
    1256,  1257,  1260,  1267,  1268,  1273,  1275,  1276,  1281,  1282,
    1284,  1286,  1292,  1293,  1295,  1302,  1304,  1306,  1312,  1313,
    1319,  1320,  1327,  1328,  1333,  1334,  1335,  1336,  1346,  1347,
    1350,  1353,  1356,  1364,  1367,  1370,  1377,  1379,  1386,  1388,
    1390,  1399,  1400,  1403,  1410,  1413,  1420,  1421,  1422,  1427,
    1429,  1431,  1434,  1441,  1444,  1447,  1450,  1457,  1458,  1459,
    1464,  1465,  1470,  1471,  1474,  1481,  1483,  1486,  1493,  1495,
    1502,  1507,  1510,  1517,  1519,  1522,  1537,  1539,  1545,  1546,
    1551,  1553,  1555,  1557,  1566,  1567,  1572,  1574,  1579,  1586,
    1588,  1594,  1595,  1597,  1603,  1604,  1605,  1610,  1611,  1616,
    1617,  1618,  1623,  1625,  1635,  1636,  1639,  1648,  1650,  1654,
    1659,  1665,  1667,  1670,  1673,  1676,  1678,  1680,  1682,  1684,
    1687,  1690,  1693,  1695,  1697,  1703,  1705,  1707,  1712,  1728,
    1732,  1737,  1742,  1748,  1758,  1759,  1764,  1765,  1771,  1772,
    1778,  1784,  1790,  1792,  1799,  1802,  1809,  1812,  1819,  1820,
    1825,  1830,  1832,  1834,  1840,  1842,  1848,  1849,  1853,  1854,
    1863,  1864,  1867,  1868,  1869,  1870,  1871,  1872,  1874,  1880,
    1879,  1884,  1886,  1890,  1895,  1897,  1899,  1901,  1905,  1906,
    1912,  1914,  1916,  1918,  1925,  1926,  1931,  1932,  1937,  1941,
    1943,  1945,  1945,  1951,  1950,  1955,  1954,  1959,  1958,  1965,
    1964,  1969,  1968,  1973,  1972,  1976,  1976,  1981,  1983,  1985,
    1987,  1991,  2002,  2003,  2009,  2019,  2029,  2044,  2043,  2052,
    2057,  2056,  2061,  2060,  2065,  2064,  2074,  2073,  2083,  2082,
    2092,  2091,  2100,  2099,  2108,  2107,  2117,  2116,  2126,  2125,
    2135,  2134,  2143,  2142,  2151,  2150,  2160,  2159,  2173,  2173,
    2190,  2191,  2192,  2193,  2194,  2199,  2200,  2222,  2223,  2224,
    2228,  2229,  2233,  2234,  2235,  2236,  2237,  2241,  2242,  2243,
    2244,  2245,  2247,  2248,  2250,  2251
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "'&'", "'*'", "'+'", "'-'", "'~'", "'!'",
  "'<'", "'>'", "'|'", "'^'", "'%'", "'/'", "'('", "')'", "'.'", "'?'",
  "';'", "'{'", "'}'", "','", "'['", "']'", "':'", "AUTO", "DOUBLE", "INT",
  "STRUCT", "BREAK", "ELSE", "LONG", "SWITCH", "CASE", "ENUM", "REGISTER",
  "TYPEDEF", "CHAR", "EXTERN", "RETURN", "UNION", "CONST", "FLOAT",
  "SHORT", "UNSIGNED", "CONTINUE", "FOR", "SIGNED", "VOID", "DEFAULT",
  "GOTO", "SIZEOF", "VOLATILE", "DO", "IF", "STATIC", "WHILE", "COMPOSITE",
  "INPUT", "OUTPUT", "STREAM", "PARAM", "ADD", "INIT", "WORK", "WINDOW",
  "TUMBLING", "SLIDING", "SPLITJOIN", "SPLIT", "JOIN", "DUPLICATE",
  "ROUNDROBIN", "PIPELINE", "FILEREADER", "FILEWRITER", "UPLUS", "UMINUS",
  "INDIR", "ADDRESS", "POSTINC", "POSTDEC", "PREINC", "PREDEC", "BOGUS",
  "IDENTIFIER", "STRINGliteral", "FLOATINGconstant", "INTEGERconstant",
  "OCTALconstant", "HEXconstant", "WIDECHARconstant", "CHARACTERconstant",
  "TYPEDEFname", "ARROW", "ICR", "DECR", "LS", "RS", "LE", "GE", "EQ",
  "NE", "ANDAND", "OROR", "ELLIPSIS", "'='", "MULTassign", "DIVassign",
  "MODassign", "PLUSassign", "MINUSassign", "LSassign", "RSassign",
  "ANDassign", "ERassign", "ORassign", "INLINE", "ATTRIBUTE", "$accept",
  "prog.start", "stream.type.specifier", "stream.declaration.list",
  "composite.definition", "$@1", "composite.head", "composite.head.inout",
  "composite.head.inout.member.list", "composite.head.inout.member",
  "composite.body.no.new.scope", "composite.body.param.opt",
  "composite.body.statement.list", "composite.body.operator",
  "operator.file.writer", "operator.add", "operator.pipeline",
  "operator.splitjoin", "split.statement",
  "splitjoinPipeline.statement.list", "join.statement",
  "roundrobin.statement", "duplicate.statement", "operator.default.call",
  "operator.arguments", "operator.selfdefine.body",
  "operator.selfdefine.body.init.opt", "operator.selfdefine.body.work",
  "operator.selfdefine.body.window.list.opt",
  "operator.selfdefine.window.list", "operator.selfdefine.window",
  "window.type", "primary.expression", "postfix.expression",
  "argument.expression.list", "unary.expression", "unary.operator",
  "cast.expression", "multiplicative.expression", "additive.expression",
  "shift.expression", "relational.expression", "equality.expression",
  "AND.expression", "exclusive.OR.expression", "inclusive.OR.expression",
  "logical.AND.expression", "logical.OR.expression",
  "conditional.expression", "assignment.expression", "assignment.operator",
  "expression", "constant.expression", "expression.opt", "declaration",
  "declaring.list", "$@2", "$@3", "$@4", "$@5", "@6", "$@7", "$@8", "$@9",
  "default.declaring.list", "$@10", "$@11", "$@12", "$@13", "@14", "$@15",
  "$@16", "$@17", "declaration.specifier", "basic.declaration.specifier",
  "sue.declaration.specifier", "typedef.declaration.specifier",
  "declaration.qualifier.list", "declaration.qualifier", "type.specifier",
  "basic.type.specifier", "sue.type.specifier", "typedef.type.specifier",
  "type.qualifier.list", "pointer.type.qualifier.list",
  "elaborated.type.name", "declarator", "paren.typedef.declarator",
  "paren.postfix.typedef.declarator", "simple.paren.typedef.declarator",
  "parameter.typedef.declarator", "clean.typedef.declarator",
  "clean.postfix.typedef.declarator", "abstract.declarator",
  "unary.abstract.declarator", "postfix.abstract.declarator",
  "postfixing.abstract.declarator", "identifier.declarator",
  "unary.identifier.declarator", "postfix.identifier.declarator",
  "paren.identifier.declarator", "old.function.declarator",
  "postfix.old.function.declarator", "identifier.list",
  "identifier.or.typedef.name", "type.name", "attributes.opt",
  "attributes", "attribute", "attribute.list", "attrib", "any.word",
  "initializer.opt", "initializer", "initializer.list",
  "parameter.type.list", "parameter.list", "parameter.declaration",
  "array.abstract.declarator", "struct.or.union.specifier",
  "struct.or.union", "struct.declaration.list", "struct.declaration",
  "struct.default.declaring.list", "struct.declaring.list",
  "struct.declarator", "struct.identifier.declarator",
  "bit.field.size.opt", "bit.field.size", "enum.specifier",
  "enumerator.list", "enumerator.value.opt", "comma.opt",
  "costream.composite.statement", "statement", "labeled.statement", "@18",
  "compound.statement", "lblock", "rblock",
  "compound.statement.no.new.scope", "declaration.list", "statement.list",
  "expression.statement", "selection.statement", "$@19",
  "iteration.statement", "$@20", "$@21", "$@22", "$@23", "$@24", "$@25",
  "$@26", "jump.statement", "translation.unit", "external.definition",
  "function.definition", "$@27", "$@28", "$@29", "$@30", "$@31", "$@32",
  "$@33", "$@34", "$@35", "$@36", "$@37", "$@38", "$@39", "$@40", "$@41",
  "old.function.declaration.list", "$@42", "constant",
  "string.literal.list", "type.qualifier", "pointer.type.qualifier",
  "storage.class", "basic.type.name", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,    38,    42,    43,    45,   126,    33,    60,
      62,   124,    94,    37,    47,    40,    41,    46,    63,    59,
     123,   125,    44,    91,    93,    58,   258,   259,   260,   261,
     262,   263,   264,   265,   266,   267,   268,   269,   270,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   286,   287,   288,   289,   290,   291,
     292,   293,   294,   295,   296,   297,   298,   299,   300,   301,
     302,   303,   304,   305,   306,   307,   308,   309,   310,   311,
     312,   313,   314,   315,   316,   317,   318,   319,   320,   321,
     322,   323,   324,   325,   326,   327,   328,   329,   330,   331,
     332,   333,   334,   335,   336,   337,   338,    61,   339,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   120,   121,   122,   123,   123,   123,   123,   125,   124,
     126,   127,   127,   127,   127,   127,   128,   128,   129,   130,
     130,   131,   131,   132,   132,   133,   133,   134,   135,   135,
     135,   136,   136,   137,   137,   137,   138,   138,   139,   139,
     139,   139,   140,   141,   141,   142,   142,   143,   144,   144,
     145,   145,   146,   146,   147,   148,   148,   149,   149,   150,
     151,   151,   151,   151,   152,   152,   152,   152,   152,   152,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     153,   153,   153,   153,   153,   153,   153,   153,   153,   153,
     154,   154,   155,   155,   155,   155,   155,   155,   156,   156,
     156,   156,   156,   156,   157,   157,   158,   158,   158,   158,
     159,   159,   159,   160,   160,   160,   161,   161,   161,   161,
     161,   162,   162,   162,   163,   163,   164,   164,   165,   165,
     166,   166,   167,   167,   168,   168,   169,   169,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   171,
     171,   172,   173,   173,   174,   174,   174,   174,   176,   177,
     175,   178,   179,   175,   180,   181,   175,   182,   175,   183,
     175,   175,   185,   186,   184,   184,   187,   188,   184,   189,
     190,   184,   191,   184,   192,   184,   184,   193,   193,   193,
     194,   194,   194,   194,   195,   195,   195,   196,   196,   196,
     197,   197,   197,   198,   198,   199,   199,   199,   200,   200,
     200,   200,   201,   201,   201,   202,   202,   202,   203,   203,
     204,   204,   205,   205,   206,   206,   206,   206,   207,   207,
     207,   207,   207,   208,   208,   208,   209,   209,   210,   210,
     210,   211,   211,   211,   212,   212,   213,   213,   213,   214,
     214,   214,   214,   215,   215,   215,   215,   216,   216,   216,
     217,   217,   218,   218,   218,   219,   219,   219,   220,   220,
     221,   221,   221,   222,   222,   222,   223,   223,   224,   224,
     225,   225,   225,   225,   226,   226,   227,   227,   228,   229,
     229,   230,   230,   230,   231,   231,   231,   232,   232,   233,
     233,   233,   234,   234,   235,   235,   235,   236,   236,   236,
     236,   237,   237,   237,   237,   237,   237,   237,   237,   237,
     237,   237,   237,   237,   237,   238,   238,   238,   238,   239,
     239,   239,   239,   239,   240,   240,   241,   241,   242,   242,
     243,   243,   244,   244,   245,   245,   246,   246,   247,   247,
     248,   249,   249,   249,   250,   250,   251,   251,   252,   252,
     253,   253,   254,   254,   254,   254,   254,   254,   254,   256,
     255,   255,   255,   255,   257,   257,   257,   257,   258,   259,
     260,   260,   260,   260,   261,   261,   262,   262,   263,   264,
     264,   265,   264,   267,   266,   268,   266,   269,   266,   270,
     266,   271,   266,   272,   266,   273,   266,   274,   274,   274,
     274,   274,   275,   275,   276,   276,   276,   278,   277,   277,
     279,   277,   280,   277,   281,   277,   282,   277,   283,   277,
     284,   277,   285,   277,   286,   277,   287,   277,   288,   277,
     289,   277,   290,   277,   291,   277,   292,   277,   294,   293,
     295,   295,   295,   295,   295,   296,   296,   297,   297,   297,
     298,   298,   299,   299,   299,   299,   299,   300,   300,   300,
     300,   300,   300,   300,   300,   300
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     4,     2,     3,     4,     5,     0,     3,
       5,     0,     5,     2,     2,     5,     1,     3,     2,     4,
       5,     0,     3,     1,     2,     1,     1,     6,     2,     2,
       2,     4,     5,     6,     7,     8,     2,     2,     1,     1,
       2,     2,     2,     4,     5,     4,     5,     3,     3,     2,
       5,     6,     0,     2,     2,     0,     4,     1,     2,     3,
       3,     3,     4,     4,     1,     1,     1,     3,     5,     6,
       1,     4,     3,     4,     4,     5,     4,     5,     9,    10,
      11,     7,     8,     4,     3,     3,     2,     2,     3,     3,
       1,     3,     1,     2,     2,     2,     2,     4,     1,     1,
       1,     1,     1,     1,     1,     4,     1,     3,     3,     3,
       1,     3,     3,     1,     3,     3,     1,     3,     3,     3,
       3,     1,     3,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     5,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     1,     0,     1,     2,     2,     2,     2,     0,     0,
       6,     0,     0,     6,     0,     0,     7,     0,     5,     0,
       5,     3,     0,     0,     6,     2,     0,     0,     6,     0,
       0,     7,     0,     5,     0,     5,     3,     1,     1,     1,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       1,     2,     2,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     1,     2,     2,     1,     2,     2,     1,     2,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     2,
       4,     5,     3,     3,     4,     4,     1,     3,     1,     2,
       1,     1,     2,     3,     3,     4,     1,     1,     1,     1,
       2,     2,     3,     3,     3,     3,     4,     1,     2,     3,
       1,     1,     1,     2,     3,     2,     3,     4,     1,     3,
       1,     2,     3,     4,     3,     4,     1,     3,     1,     1,
       1,     2,     1,     2,     0,     1,     1,     2,     6,     1,
       3,     0,     1,     4,     1,     1,     1,     0,     2,     3,
       4,     1,     1,     3,     1,     3,     1,     1,     3,     3,
       3,     1,     2,     2,     2,     1,     2,     2,     1,     2,
       2,     2,     1,     2,     2,     2,     3,     4,     3,     4,
       5,     2,     3,     4,     1,     1,     1,     2,     2,     2,
       2,     3,     2,     3,     3,     2,     3,     2,     0,     1,
       2,     5,     6,     2,     2,     4,     0,     2,     0,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     0,
       4,     4,     3,     3,     2,     3,     3,     4,     1,     1,
       2,     3,     3,     4,     1,     2,     1,     2,     2,     5,
       7,     0,     6,     0,     6,     0,     8,     0,    10,     0,
      10,     0,    10,     0,    10,     0,     6,     3,     2,     2,
       3,     3,     1,     2,     1,     1,     1,     0,     3,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     3,
       0,     4,     0,     4,     0,     4,     0,     4,     0,     4,
       0,     5,     0,     5,     0,     5,     0,     5,     0,     2,
       1,     1,     1,     1,     1,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,   465,   471,   469,   334,   475,     0,   466,
     462,   468,   463,   335,   457,   470,   474,   473,   472,   467,
     458,   464,     0,     0,   268,   215,   459,     0,     0,   416,
       8,   414,     0,     0,     0,   187,   188,   189,     0,     0,
     205,   206,   207,     0,   212,   417,   260,   262,   261,   448,
     270,   222,     0,   223,     2,   412,   415,   218,   200,   208,
     460,   461,     0,   263,   271,   220,     0,     0,     0,     0,
     278,   279,   353,     0,     0,     1,     0,     0,   175,   261,
       0,   154,     0,   155,     0,   167,     0,     0,   238,   158,
     224,   228,   225,   240,   241,   226,   448,   192,   203,   204,
     193,   156,   196,   199,   182,   198,   202,   195,   172,   448,
     191,   169,   161,   226,   448,   210,   190,   211,   157,   214,
     194,   217,   197,   184,   216,   213,   176,   448,   219,   201,
     209,   419,     0,     0,     0,   265,   257,     0,   438,     0,
       0,   331,   413,   264,   272,   221,   266,   269,   274,   356,
     358,     0,    11,     0,     0,   205,   206,   207,     0,     0,
       0,     0,    21,     9,   171,   164,   226,   227,   186,   179,
     284,     0,     0,   229,   242,     0,   236,     0,     0,     0,
     239,   284,     0,     0,   440,   284,   284,     0,     0,   444,
     284,   284,     0,     0,   442,   284,   284,     0,     0,   446,
       0,   418,   258,   276,   306,   311,   188,   315,   318,   206,
     322,     0,     0,   304,   307,    98,    99,   100,   101,   102,
     103,     0,   325,     0,     0,     0,     0,    64,   455,   450,
     451,   452,   453,   454,     0,     0,    70,    92,   104,     0,
     106,   110,   113,   116,   121,   124,   126,   128,   130,   132,
     134,   151,     0,    65,    66,     0,   429,     0,   384,     0,
       0,     0,     0,   449,   332,     0,     0,     0,   336,     0,
       0,     0,   267,   275,     0,   354,   359,     0,   358,     0,
       0,     0,     3,     0,     0,     4,     0,     0,   284,   284,
       0,   297,   285,   286,     0,     0,   232,   243,     0,   233,
       0,   244,   159,   421,   431,     0,   297,   173,   425,   435,
       0,   297,   162,   423,   433,     0,   297,   177,   427,   437,
       0,     0,   378,   380,     0,   391,     0,   152,     0,     0,
       0,     0,   395,     0,   393,    64,   215,   104,   136,   149,
     153,     0,   386,   362,   363,     0,     0,     0,   364,   365,
     366,   367,   249,     0,   314,   312,   246,   247,   248,   313,
     249,     0,   316,   317,   321,   319,   320,   323,   324,   273,
       0,   259,     0,     0,     0,   280,   282,     0,     0,     0,
      96,     0,     0,     0,     0,    93,    94,     0,     0,     0,
       0,    86,    87,    95,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   326,   456,   328,     0,   439,   172,   176,
     385,     0,   348,   342,   284,   348,   340,   284,   329,   337,
     339,     0,   338,     0,   333,     0,   357,   356,   351,     0,
       0,    13,    16,    14,    10,     0,     0,     5,     0,     0,
       0,     0,   360,    26,    25,    23,   361,     0,   165,   180,
       0,     0,   168,   287,   230,     0,   237,   235,   234,   245,
     297,   441,   183,   297,   445,   170,   297,   443,   185,   297,
     447,   368,   409,     0,     0,     0,   408,     0,     0,     0,
       0,     0,     0,     0,   369,     0,   138,   139,   140,   141,
     142,   143,   144,   145,   146,   147,   148,     0,     0,   388,
     379,     0,   374,     0,   381,     0,   382,     0,   387,   250,
     251,     0,     0,     0,   250,   277,   310,   305,   308,     0,
     301,   309,    67,   249,     0,   281,   283,     0,     0,     0,
       0,     0,     0,     0,    72,     0,    90,    84,    88,     0,
      85,    89,   107,   109,   108,   111,   112,   114,   115,   117,
     118,   119,   120,   122,   123,   125,   127,   129,   131,     0,
     133,   327,   350,   284,   349,   345,   284,   347,   341,   343,
     330,   355,   352,    18,     0,     0,     6,    22,     0,     0,
       0,     0,    29,    30,    28,     0,    19,    24,     0,   297,
     297,   291,   298,   231,   160,   174,   163,   178,     0,     0,
     410,     0,     0,   372,   407,   411,     0,     0,     0,     0,
     373,   137,   150,   376,     0,   375,   383,   252,   253,   254,
     255,   302,     0,   250,   105,     0,     0,    97,     0,     0,
       0,    83,    76,    74,    52,    73,     0,    71,     0,   344,
     346,     0,    17,     0,     7,     0,     0,     0,     0,    20,
     166,   181,   296,   294,   295,     0,   289,   292,     0,   371,
     405,   152,     0,     0,     0,     0,   370,   377,   256,   299,
       0,     0,    68,     0,     0,    49,     0,     0,     0,    52,
      77,    75,    91,   135,    12,    15,     0,     0,     0,    39,
       0,    38,     0,    47,     0,     0,   291,     0,     0,     0,
       0,     0,     0,     0,   389,     0,   300,   303,    69,     0,
       0,     0,     0,    48,    53,     0,    55,     0,     0,     0,
       0,    37,    36,     0,     0,     0,    41,    40,    31,     0,
       0,   288,   290,     0,   392,   406,   152,   152,     0,     0,
       0,   394,     0,     0,     0,    81,     0,    54,     0,     0,
      55,     0,     0,     0,     0,     0,     0,    32,    27,   293,
       0,     0,     0,     0,     0,   390,     0,     0,     0,    82,
       0,    50,     0,     0,     0,     0,     0,    42,    33,     0,
       0,   399,   403,   401,   397,   396,    78,     0,     0,     0,
       0,    57,    51,    45,     0,    43,     0,    34,     0,     0,
       0,     0,     0,    79,     0,     0,     0,     0,    56,    58,
      46,    44,    35,   400,   404,   402,   398,    80,     0,     0,
      59,    61,     0,    60,     0,    63,    62
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    27,    28,   153,    29,    80,    30,   281,   441,   442,
     163,   287,   451,   452,   453,   454,   592,   593,   697,   700,
     764,   731,   732,   594,   641,   643,   688,   726,   759,   800,
     801,   817,   236,   237,   545,   337,   239,   240,   241,   242,
     243,   244,   245,   246,   247,   248,   249,   250,   338,   339,
     507,   340,   252,   341,   258,    32,   181,   470,   191,   476,
     288,   599,   170,   190,    33,   186,   473,   196,   479,   289,
     600,   185,   195,   259,    35,    36,    37,   260,   106,   261,
      40,    41,    42,   262,    62,    44,    89,    90,    91,   178,
      92,    93,    94,   520,   356,   357,   358,    63,    46,    47,
      79,    68,    50,   211,   149,   377,   291,   292,   293,   665,
     666,   667,   462,   531,   632,   212,   213,   214,   136,    51,
      52,   267,   268,   269,   270,   423,   426,   573,   424,    53,
     150,   275,   277,   455,   456,   343,   619,   344,   345,   512,
     201,   263,   347,   348,   349,   483,   350,   493,   491,   812,
     809,   811,   810,   709,   351,    54,    55,    56,   132,   182,
     192,   187,   197,   137,   183,   193,   188,   198,   257,   305,
     315,   310,   320,   138,   139,   253,   254,    57,    65,    58,
      59
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -635
static const yytype_int16 yypact[] =
{
    2291,    74,   148,  -635,  -635,  -635,  -635,  -635,    56,  -635,
    -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,
    -635,  -635,   -13,    94,  -635,  -635,  -635,   121,   315,  -635,
    -635,  -635,   303,   374,   311,  2123,   700,   787,  1781,   477,
    2123,   780,   787,  1842,  -635,    62,  -635,  -635,   197,   131,
    -635,  -635,   110,  -635,  2291,  -635,  -635,  -635,  -635,  -635,
    -635,  -635,    74,  -635,  -635,  -635,   159,   364,   166,   165,
    -635,  -635,   230,   239,  4806,  -635,   172,   315,  -635,   324,
     256,  -635,   479,  -635,   267,  -635,   560,    79,   324,  -635,
    -635,  -635,  -635,  -635,  -635,   288,    55,  -635,  -635,  -635,
    -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,   301,   331,
    -635,  -635,  -635,   354,    65,  -635,  -635,  -635,  -635,  -635,
    -635,  -635,  -635,  -635,  -635,  -635,   375,   386,  -635,  -635,
    -635,  -635,   399,  2498,  4038,  -635,   406,   399,  -635,  4752,
    2528,   434,  -635,  -635,  -635,  -635,   324,  -635,   324,   179,
     310,   165,     7,    95,    26,  2255,   103,   103,  4837,   172,
     400,  2570,   397,  -635,  -635,  -635,  -635,  -635,  -635,  -635,
     356,    79,   652,  -635,  -635,    79,  -635,   469,   324,   491,
    -635,   356,   399,   399,  -635,   356,   356,   399,   399,  -635,
     356,   356,   399,   399,  -635,   356,   356,   399,   399,  -635,
    1296,  -635,  -635,  -635,  -635,   237,   787,  2363,   237,   787,
    2399,    85,   545,   537,   463,  -635,  -635,  -635,  -635,  -635,
    -635,  2095,  -635,  4514,   570,   573,   582,  -635,  -635,  -635,
    -635,  -635,  -635,  -635,  4554,  4554,  -635,   755,  -635,  4609,
    -635,   115,   212,   134,    53,   431,   596,   589,   592,   503,
      19,  -635,   588,  -635,   533,  4078,  -635,   399,  -635,   311,
    1903,   477,  1964,  4752,  -635,   263,  2435,  2600,  -635,   398,
     414,  2630,  -635,  -635,  4609,  -635,   165,   620,   310,   594,
     594,   637,  -635,  4806,    26,   324,  4783,  1196,   356,   356,
     644,   554,   356,  -635,   416,    79,  -635,  -635,   430,   324,
     655,   324,  -635,  -635,  -635,   399,   554,  -635,  -635,  -635,
     399,   554,  -635,  -635,  -635,   399,   554,  -635,  -635,  -635,
     399,   646,  -635,  -635,   654,  -635,  4609,  4609,   659,   653,
     657,   316,  -635,   664,  -635,   660,   668,   916,  -635,  -635,
     658,   665,  -635,  -635,  -635,  1099,  1396,  3304,  -635,  -635,
    -635,  -635,   341,  2183,  -635,  -635,  -635,  -635,  -635,  -635,
      91,  2219,  -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,
     609,  -635,  2003,  4134,   104,   207,  2467,   680,  1687,  2095,
    -635,   614,   615,   690,  4174,  -635,  -635,  4229,   336,  4609,
     383,  -635,  -635,  -635,  4609,  4609,  4609,  4609,  4609,  4609,
    4609,  4609,  4609,  4609,  4609,  4609,  4609,  4609,  4609,  4609,
    4609,  4609,  4609,  -635,  -635,  -635,   683,  -635,  -635,  -635,
    -635,  4609,   684,  -635,   356,   684,  -635,   356,  -635,  -635,
    -635,   340,  -635,   263,  -635,  2663,  -635,   179,  -635,   692,
     628,   694,  -635,   696,  -635,    26,   707,  -635,   528,    12,
     710,  2722,  -635,  -635,  -635,  -635,  -635,  1196,  -635,  -635,
     712,  4134,  -635,  -635,  -635,   432,  -635,  -635,  -635,  -635,
     554,  -635,  -635,   554,  -635,  -635,   554,  -635,  -635,   554,
    -635,  -635,  -635,   713,   708,   721,  -635,  3846,  3789,   726,
     733,  3789,  4609,   717,  -635,  3789,  -635,  -635,  -635,  -635,
    -635,  -635,  -635,  -635,  -635,  -635,  -635,  4609,  4609,  -635,
    -635,  2819,  -635,  1099,  -635,  3401,  -635,   668,  -635,   341,
    -635,   738,   739,   753,    91,  -635,  -635,  -635,  -635,  4134,
    -635,  -635,  -635,   706,  2327,  -635,  -635,  4609,  1687,  3498,
     757,   758,   761,   772,   183,   285,  -635,  -635,  -635,   492,
    -635,  -635,  -635,  -635,  -635,   115,   115,   212,   212,   134,
     134,   134,   134,    53,    53,   431,   596,   589,   592,   312,
     503,  -635,  -635,   356,  -635,  -635,   356,  -635,  -635,  -635,
    -635,  -635,  -635,  -635,   309,   226,   324,  -635,  2035,   768,
     768,   772,  -635,  -635,  -635,   718,  -635,  -635,  2916,   554,
     554,   241,  -635,  -635,  -635,  -635,  -635,  -635,  4609,  3789,
    -635,   567,   774,  -635,  -635,  -635,   737,   338,  4609,  3789,
    -635,  -635,  -635,  -635,  2819,  -635,  -635,  -635,   324,  -635,
    -635,  -635,   568,   706,  -635,  3498,   791,  -635,   768,   768,
    4269,  -635,  -635,  -635,  4681,   183,  4609,  -635,  4609,  -635,
    -635,   594,  -635,   594,  -635,  4712,  1493,   789,   793,  -635,
    -635,  -635,  -635,  -635,  -635,   418,  -635,   795,   427,  -635,
    -635,  4609,  3886,   796,  3013,   448,  -635,  -635,  -635,  -635,
    3983,   798,  -635,  4712,  1493,  -635,   487,   768,   747,  4681,
    -635,  -635,  -635,  -635,   803,   803,   523,  3595,  1590,  -635,
    3110,  -635,  1493,  -635,   772,   811,   241,  4609,  3789,  3013,
     812,   813,   815,  4609,   799,  3789,  -635,  -635,  -635,  3595,
    1590,  3110,  1493,  -635,  -635,   768,   769,   747,   594,   822,
     823,  -635,  -635,  3207,  3595,  3692,  -635,  -635,  -635,  3110,
     826,  -635,  -635,   490,  -635,  -635,  4609,  4609,  3943,   526,
    3013,  -635,  3207,  3595,  3692,  -635,  3110,  -635,   827,   821,
     769,  4324,  4364,   773,   821,  3207,  3595,  -635,  -635,  -635,
     832,   833,   837,   838,   836,  -635,   821,  3207,  3595,  -635,
     771,  -635,   821,   839,   843,   841,   530,  -635,  -635,   821,
    3207,  -635,  -635,  -635,  -635,  -635,  -635,   821,  3207,   543,
      22,  -635,  -635,  -635,   842,  -635,   845,  -635,   821,  3013,
    3013,  3013,  3013,  -635,   821,   847,   851,   848,  -635,  -635,
    -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,  4419,  4459,
    -635,  -635,   544,  -635,   562,  -635,  -635
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -635,  -635,  -209,  -635,  -635,  -635,  -635,  -635,  -267,  -535,
    -635,  -635,  -325,  -635,  -635,   319,  -635,  -635,    51,   205,
    -247,   105,  -635,  -635,  -523,   229,   188,   151,   119,  -635,
      86,  -635,  -635,  -635,  -604,   227,  -635,  -195,   247,   252,
     380,   270,   478,   476,   481,   482,   475,  -635,  -119,   111,
    -635,  -220,  -216,  -258,     0,  -635,  -635,  -635,  -635,  -635,
    -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,
    -635,  -635,  -635,     5,  -635,  -116,  -635,    10,    20,     6,
     141,   -43,   441,    28,   -54,   -24,   -30,   -79,  -635,  -110,
     -68,   -40,  -635,  -181,  -328,  -315,   -37,   266,    38,  -635,
     604,   649,  -635,  -635,    27,   514,    84,  -635,   603,  -635,
     191,  -635,   271,  -457,  -635,  -635,   617,  -349,  -635,  -635,
    -635,   630,  -233,  -635,  -635,   473,   480,   484,  -178,  -635,
     762,   483,   641,   106,  -198,  -635,  -635,  -634,    25,   204,
     754,  -188,  -330,  -635,  -635,  -635,  -635,  -635,  -635,  -635,
    -635,  -635,  -635,  -635,  -635,  -635,   858,  -635,  -635,  -635,
    -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,  -635,
    -635,  -635,  -635,    83,  -635,  -635,  -635,   725,   -59,   760,
     634
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -437
static const yytype_int16 yytable[] =
{
      31,   374,   342,   145,   602,    34,    39,   173,   177,   112,
      38,   135,   346,   443,   107,   251,   515,   206,   174,   125,
     511,   642,   159,   528,   355,   521,   362,   365,    43,   367,
     135,   156,   172,   521,   429,    72,   686,   411,   522,   416,
      66,   284,   135,   818,   393,   206,   522,   179,   539,   652,
     652,   180,   165,   724,    31,    97,   102,   103,   436,    34,
      39,   294,   401,   402,    38,   298,   279,   280,   657,   485,
     440,   440,   631,    73,  -227,  -430,    69,  -227,     1,   141,
     154,   589,    43,    86,  -227,  -432,   590,  -227,   427,     2,
     209,   757,   177,   296,   175,   360,   177,   156,   591,   457,
     145,   369,   158,    74,   297,   282,   361,   370,   799,   272,
     484,   273,    24,   145,   134,    66,    60,   283,   209,   394,
     532,    75,   690,   135,   412,    66,   508,    61,   395,   396,
     140,   179,   598,    60,   125,   179,   251,   354,   205,   208,
     364,   300,    70,   207,    61,    14,   265,   131,   342,   518,
      71,  -428,     1,   403,   404,   251,    20,   513,   786,   374,
      24,   210,  -227,     2,   374,    24,   205,   208,   266,   549,
     206,   207,  -227,   176,  -227,   146,    76,    24,   156,   184,
     342,   740,   148,   107,  -227,   465,   125,    77,   624,   210,
     538,   569,   189,   652,   535,   536,    70,   194,   640,   552,
     553,   554,   429,   322,    71,   572,   521,   251,   635,    66,
     199,   533,   133,    66,    60,   155,   177,   397,   398,   522,
     134,    26,   534,   717,   156,    61,   102,   375,   156,   612,
     134,   112,   399,   400,    24,   422,   107,   206,   125,   528,
     156,   352,   125,   209,   574,   206,   378,   574,   447,   376,
     151,    70,   353,   427,   152,   179,   206,   300,    24,    71,
     134,   300,   467,   420,   469,   302,    45,    86,   168,   306,
     307,    76,   617,   265,   311,   312,   162,   265,    87,   316,
     317,   155,    77,   662,   174,   653,   274,    23,   421,   445,
     613,   205,   208,   616,    78,   266,   207,   620,   519,   266,
      95,   645,   251,   437,   108,   113,   524,   646,  -420,   126,
     209,   158,    85,   179,   210,    86,   523,   518,   209,    76,
      45,  -424,    81,    24,   523,    82,    87,   663,   143,   209,
      77,    88,   276,    66,   508,   664,   156,   648,   627,   161,
     342,   518,   634,   627,    76,   352,   420,   134,   166,    24,
     169,  -434,   125,    24,   674,    77,   353,    88,   205,   208,
     508,   238,   155,   207,   134,   421,   205,   208,   735,   651,
      23,   207,   458,   459,  -422,   440,   440,   205,   208,   133,
     147,   210,   207,    60,   694,   375,   695,   134,   668,   210,
     754,    66,   156,    83,    61,  -426,    84,    24,   675,    66,
     210,    24,   489,   422,   378,    88,  -436,   376,   155,   378,
     490,   669,   155,   710,   712,   161,   147,   430,   206,   200,
     431,   676,   547,   134,   155,   143,    24,    24,   300,   255,
     548,   161,   464,   432,   705,    88,   433,   518,   143,   134,
     706,   265,   440,   708,   440,   161,   466,   161,   603,   508,
     380,   297,   627,   134,   271,   134,   689,   420,   701,   286,
     145,   385,   386,   266,   715,   145,   238,   698,   702,   550,
     508,   359,   206,   363,   366,   290,   368,   551,   111,   633,
     164,    86,   238,    86,   530,   299,   701,   743,   770,   771,
     773,   209,    87,   749,    87,   720,   722,   523,   546,   701,
     342,   238,   737,   723,   701,   776,   769,   301,   575,   646,
     744,   577,   508,   420,   508,   157,   647,   751,   789,   440,
     155,   701,   342,   737,   701,   166,   418,   166,   419,   693,
     797,   166,   425,   405,   406,   737,   701,   518,   420,   205,
     208,   737,   774,   808,   207,   209,   806,   587,   508,   654,
     588,   814,   646,   238,   737,   701,   518,   597,   737,   372,
     835,   371,   210,    24,    86,    24,   508,   737,   701,   644,
     373,    88,   530,    88,   145,   171,   155,   472,   836,   737,
     701,   157,   475,   670,   508,   381,   671,   478,   382,   679,
     680,   678,   737,   205,   208,   729,   730,   383,   207,   407,
     737,   408,    60,   409,    48,    48,    67,   410,   832,   834,
     815,   816,   413,    61,   655,   656,   210,   597,   621,   622,
     414,   238,   238,   238,   238,   238,   238,   238,   238,   238,
     238,   238,   238,   238,   238,   238,   238,   238,    48,   238,
     530,   438,    48,    48,   555,   556,    24,    48,   238,    49,
      64,   557,   558,   444,    88,    23,    86,   649,    48,   460,
     650,   461,   157,   683,   684,   481,    48,   295,   487,   100,
     644,   468,   110,   482,   117,   563,   564,   130,   486,   492,
     508,   160,   488,    96,   509,   494,    48,   109,   114,   420,
      48,    67,   127,   495,    60,   525,   537,   425,   420,   166,
     541,   542,   420,    49,   597,    61,   543,   571,   157,   421,
     533,   144,   157,   582,   583,   623,   584,   625,   585,   101,
     420,   534,   420,   147,   157,   595,     3,   601,   608,   134,
     597,   167,   618,   609,   719,    64,     9,    10,    24,    12,
     610,   604,    14,   636,   605,   614,    88,   606,    60,   734,
     607,   546,   615,    20,   628,   629,    21,   692,   285,    61,
      98,    98,    98,    98,   238,   115,   119,   121,   128,   630,
     387,   753,   388,   637,   638,    67,    48,   639,   389,    67,
     714,   559,   560,   561,   562,   143,   766,   640,   322,   117,
     143,   530,   130,   672,   673,    99,    99,    99,    99,   118,
     116,   120,   122,   129,   658,   778,     3,   682,   703,   704,
     707,   713,   725,     3,   718,   745,     9,    10,    26,    12,
     157,   144,    14,     9,    10,   728,    12,   741,   677,    14,
     750,   746,   747,    20,   748,   758,    21,   761,   762,   681,
      20,   110,   510,    21,   130,   768,   730,   780,   791,   792,
     390,   391,   392,   793,   794,   795,   775,   799,   803,   804,
     805,   820,   828,    48,   821,    48,   829,   830,   787,    48,
     660,   661,   784,   546,   691,   238,   157,   727,   760,   782,
     115,   119,   121,   128,   566,   565,   819,   570,   446,   721,
     567,   256,   568,   540,   110,   463,   130,   742,    26,    67,
     130,   435,   733,   448,   738,    26,   579,   739,   167,   576,
     167,   578,   142,   278,   167,   823,   824,   825,   826,   439,
     581,     0,     0,     0,   752,   755,     0,   756,     0,     0,
       0,    98,    98,     0,   119,   128,   303,   304,     0,   765,
       0,   308,   309,   767,     0,     0,   313,   314,     0,     0,
       0,   318,   319,     0,     0,     0,     0,   160,   777,     0,
     779,     0,     0,   781,     0,   160,    99,    99,   788,   120,
     129,   790,     0,     0,     0,   699,     0,     0,     0,     0,
     796,     0,     0,   798,     0,    98,   802,   128,     0,     0,
       0,   128,     0,   807,     0,     0,     0,     0,     0,     0,
       0,   813,     0,   699,     0,     0,     0,     0,     0,     0,
     130,   417,   822,     0,     0,     0,   699,     0,   827,   736,
      99,   699,   129,   496,   497,   498,   499,   500,   501,   502,
     503,   504,   505,   506,     0,     0,     0,    48,   699,     0,
     736,   699,     0,     0,     0,     0,     0,     0,     0,   586,
       0,     0,   736,   699,     0,     0,     0,     0,   736,   471,
       0,     0,     0,     0,   474,     0,     0,     0,     0,   477,
       0,   736,   699,     0,   480,   736,     0,     0,     0,     0,
       0,     0,   167,     0,   736,   699,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   736,   699,     0,     0,
     321,   128,   215,   216,   217,   218,   219,   220,     0,   736,
       0,     0,     0,     0,   221,     0,     0,   736,  -152,   322,
     510,     0,     0,     0,     0,     3,     4,     5,     6,   324,
       0,     7,   325,   326,     8,     9,    10,    11,    12,   327,
      13,    14,    15,    16,    17,   328,   329,    18,    19,   330,
     331,   223,    20,   332,   333,    21,   334,     0,     0,     0,
      23,     0,   449,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,   450,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   335,   228,   229,   230,   231,
     232,     0,   233,   336,     0,   234,   235,   321,     0,   215,
     216,   217,   218,   219,   220,     0,     0,     0,     0,     0,
       0,   221,     0,     0,     0,  -152,   322,    26,     0,     0,
       0,     0,     3,     4,     5,     6,   324,     0,     7,   325,
     326,     8,     9,    10,    11,    12,   327,    13,    14,    15,
      16,    17,   328,   329,    18,    19,   330,   331,   223,    20,
     332,   333,    21,   334,     0,     0,     0,    23,     0,   449,
       0,     0,     0,     0,     0,   224,     0,     0,     0,     0,
     225,   226,   450,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   335,   228,   229,   230,   231,   232,     0,   233,
     336,     0,   234,   235,     0,     0,     0,   321,     0,   215,
     216,   217,   218,   219,   220,     0,     0,     0,     0,     0,
       0,   221,     0,     0,    26,  -152,   322,   323,     0,     0,
       0,     0,     3,     4,     5,     6,   324,     0,     7,   325,
     326,     8,     9,    10,    11,    12,   327,    13,    14,    15,
      16,    17,   328,   329,    18,    19,   330,   331,   223,    20,
     332,   333,    21,   334,     0,     0,     0,    23,     0,     0,
       0,     0,     0,     0,     0,   224,     0,     0,     0,     0,
     225,   226,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   335,   228,   229,   230,   231,   232,     0,   233,
     336,     0,   234,   235,     0,     0,     0,   321,     0,   215,
     216,   217,   218,   219,   220,     0,     0,     0,     0,     0,
       0,   221,     0,     0,    26,  -152,   322,   514,     0,     0,
       0,     0,     3,     4,     5,     6,   324,     0,     7,   325,
     326,     8,     9,    10,    11,    12,   327,    13,    14,    15,
      16,    17,   328,   329,    18,    19,   330,   331,   223,    20,
     332,   333,    21,   334,     0,     0,     0,    23,     0,     0,
       0,     0,     0,     0,     0,   224,     0,     0,     0,     0,
     225,   226,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   335,   228,   229,   230,   231,   232,     0,   233,
     336,     0,   234,   235,   321,     0,   215,   216,   217,   218,
     219,   220,     0,     0,     0,     0,     0,     0,   221,     0,
       0,     0,  -152,   322,    26,     0,     0,     0,     0,     3,
       4,     5,     6,   324,     0,     7,   325,   326,     8,     9,
      10,    11,    12,   327,    13,    14,    15,    16,    17,   328,
     329,    18,    19,   330,   331,   223,    20,   332,   333,    21,
     334,     0,     0,     0,    23,     0,   449,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   225,   226,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   335,
     228,   229,   230,   231,   232,     0,   233,   336,     0,   234,
     235,   321,     0,   215,   216,   217,   218,   219,   220,     0,
       0,     0,     0,     0,     0,   221,     0,     0,     0,  -152,
     322,    26,     0,     0,     0,     0,     3,     4,     5,     6,
     324,     0,     7,   325,   326,     8,     9,    10,    11,    12,
     327,    13,    14,    15,    16,    17,   328,   329,    18,    19,
     330,   331,   223,    20,   332,   333,    21,   334,     0,     0,
       0,    23,     0,     0,     0,     0,     0,     0,     0,   224,
     696,     0,     0,     0,   225,   226,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   335,   228,   229,   230,
     231,   232,     0,   233,   336,     0,   234,   235,   321,     0,
     215,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,   221,     0,     0,     0,  -152,   322,    26,     0,
       0,     0,     0,     3,     4,     5,     6,   324,     0,     7,
     325,   326,     8,     9,    10,    11,    12,   327,    13,    14,
      15,    16,    17,   328,   329,    18,    19,   330,   331,   223,
      20,   332,   333,    21,   334,     0,     0,     0,    23,     0,
       0,     0,     0,     0,     0,     0,   224,     0,     0,     0,
       0,   225,   226,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   335,   228,   229,   230,   231,   232,     0,
     233,   336,   104,   234,   235,     1,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     2,     0,     0,     0,
       0,     0,     0,     0,     0,    26,     0,     3,     4,     5,
       6,     0,     0,     7,     0,     0,     8,     9,    10,    11,
      12,     0,    13,    14,    15,    16,    17,     0,     0,    18,
      19,     0,     0,     0,    20,     0,     0,    21,     0,     0,
       0,     0,     0,   123,     0,     0,     1,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     2,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    24,     3,     4,
       5,     6,     0,     0,     7,   105,     0,     8,     9,    10,
      11,    12,     0,    13,    14,    15,    16,    17,     0,     0,
      18,    19,     0,     0,     0,    20,     0,     0,    21,    26,
       0,     0,     0,     0,   104,     0,     0,    76,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    77,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    24,     3,
       4,     5,     6,     0,     0,     7,   124,     0,     8,     9,
      10,    11,    12,     0,    13,    14,    15,    16,    17,     0,
       0,    18,    19,     0,     0,     0,    20,     0,     0,    21,
      26,     0,     0,     0,     0,   123,     0,     0,    76,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
       3,     4,     5,     6,     0,     0,     7,   105,     0,     8,
       9,    10,    11,    12,   526,    13,    14,    15,    16,    17,
       0,     0,    18,    19,     0,     0,     0,    20,     0,     0,
      21,    26,     0,     0,     0,     0,     0,     0,     0,     3,
       4,     5,     6,     0,     0,     7,   526,     0,     8,     9,
      10,    11,    12,     0,    13,    14,    15,    16,    17,     0,
      24,    18,    19,     0,     0,     0,    20,     0,   124,    21,
       0,     3,     4,     5,     6,     0,     0,     7,     0,     0,
       8,     9,    10,    11,    12,     0,    13,    14,    15,    16,
      17,     0,    26,    18,    19,     0,     0,     0,    20,     0,
       0,    21,     0,     0,     0,     0,     0,    25,   215,   216,
     217,   218,   219,   220,     0,     0,     0,     0,     0,   527,
     221,     0,     0,     0,     0,   322,     0,     0,     0,     0,
       0,    26,     4,     5,     6,     0,     0,     7,     0,    25,
       8,     0,     0,    11,     0,     0,    13,    14,    15,    16,
      17,     0,     0,    18,    19,     0,     0,   223,    20,     3,
       4,     5,     0,    26,     0,     7,     0,     0,     0,     9,
      10,    11,    12,     0,   224,    14,    15,    16,    17,   225,
     226,    18,    19,     0,     0,     0,    20,     0,     0,    21,
       0,   227,   228,   229,   230,   231,   232,   352,   233,    25,
       0,   234,   235,     0,     0,     0,     0,     0,   353,   202,
       0,     0,     0,     0,     0,     0,   134,     0,     0,     3,
       4,     5,     6,    26,     0,     7,     0,     0,     8,     9,
      10,    11,    12,   360,    13,    14,    15,    16,    17,     0,
       0,    18,    19,     0,   361,   202,    20,     0,     0,    21,
       0,    26,   134,     0,     0,     3,     4,     5,     6,     0,
       0,     7,     0,     0,     8,     9,    10,    11,    12,     0,
      13,    14,    15,    16,    17,     0,     0,    18,    19,    24,
       0,     0,    20,     0,     0,    21,     0,    25,     0,     0,
       0,     0,     4,     5,     0,     0,     0,     7,     0,   204,
       0,     0,     0,    11,     0,     1,     0,    14,    15,    16,
      17,    26,     0,    18,    19,    24,     2,     0,    20,     0,
       0,     0,     0,    25,     0,     0,     0,     3,     4,     5,
       6,     0,     0,     7,     0,   204,     8,     9,    10,    11,
      12,   533,    13,    14,    15,    16,    17,    26,     0,    18,
      19,     0,   534,   202,    20,     0,     0,    21,     0,    22,
     134,     0,    23,     3,     4,     5,     6,     0,     0,     7,
       0,     0,     8,     9,    10,    11,    12,   360,    13,    14,
      15,    16,    17,    26,     0,    18,    19,    24,   361,     0,
      20,     0,     0,    21,     0,    25,   134,     0,     0,     3,
       4,     5,     6,     0,     0,     7,     0,     0,     8,     9,
      10,    11,    12,   360,    13,    14,    15,    16,    17,    26,
       0,    18,    19,     0,   361,     0,    20,     0,     0,    21,
       0,    25,   134,     0,     0,     3,     4,     5,     6,     0,
       0,     7,     0,   204,     8,     9,    10,    11,    12,    76,
      13,    14,    15,    16,    17,    26,     0,    18,    19,    24,
      77,     0,    20,     0,     0,    21,     0,   105,     0,     0,
     421,     0,     4,     5,     6,     0,     0,     7,     0,     0,
       8,   533,     0,    11,     0,     0,    13,    14,    15,    16,
      17,    26,   534,    18,    19,    24,     0,     0,    20,     0,
     134,     0,     0,   124,     4,     5,     6,     0,     0,     7,
       0,     0,     8,     0,     0,    11,     0,     0,    13,    14,
      15,    16,    17,     0,   202,    18,    19,    26,     0,     0,
      20,    24,     0,     0,     3,     4,     5,     6,     0,   124,
       7,     0,     0,     8,     9,    10,    11,    12,     0,    13,
      14,    15,    16,    17,     0,     0,    18,    19,     0,   264,
       0,    20,     0,    26,    21,     4,     5,     6,     0,     0,
       7,   124,     0,     8,     0,     0,    11,     0,     0,    13,
      14,    15,    16,    17,     0,     0,    18,    19,     0,     0,
       0,    20,     0,     0,   203,    26,   202,     0,     0,     0,
       0,     0,    25,     0,     0,     0,     3,     4,     5,     6,
       0,     0,     7,     0,   204,     8,     9,    10,    11,    12,
       0,    13,    14,    15,    16,    17,    26,     0,    18,    19,
       0,   428,    25,    20,     0,     0,    21,     4,     5,     6,
       0,     0,     7,     0,     0,     8,     0,     0,    11,     0,
       0,    13,    14,    15,    16,    17,    26,     0,    18,    19,
       0,   434,     0,    20,     0,     0,     0,     4,     5,     6,
       0,     0,     7,     0,    25,     8,     0,     0,    11,     0,
       0,    13,    14,    15,    16,    17,   204,     0,    18,    19,
       0,     0,     0,    20,   580,     0,     0,     0,    26,     0,
       4,     5,     6,     0,    25,     7,     0,     0,     8,     0,
       0,    11,     0,     0,    13,    14,    15,    16,    17,     0,
       0,    18,    19,     0,     0,     0,    20,     0,    26,     0,
       0,     0,     0,   321,    25,   215,   216,   217,   218,   219,
     220,     0,     0,     0,     0,     0,     0,   221,     0,     0,
       0,  -152,   322,   596,     0,     0,     0,     0,    26,     0,
       0,     0,   324,     0,     0,   325,   326,    25,     0,     0,
       0,     0,   327,     0,     0,     0,     0,     0,   328,   329,
       0,     0,   330,   331,   223,     0,   332,   333,     0,   334,
       0,    26,     0,     0,     0,   449,     0,     0,     0,     0,
       0,   224,     0,     0,     0,     0,   225,   226,   450,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   335,   228,
     229,   230,   231,   232,     0,   233,   517,     0,   234,   235,
     321,     0,   215,   216,   217,   218,   219,   220,     0,     0,
       0,     0,     0,     0,   221,     0,     0,     0,  -152,   322,
     510,     0,     0,     0,     0,     0,     0,     0,     0,   324,
       0,     0,   325,   326,     0,     0,     0,     0,     0,   327,
       0,     0,     0,     0,     0,   328,   329,     0,     0,   330,
     331,   223,     0,   332,   333,     0,   334,     0,     0,     0,
       0,     0,   449,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,   450,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   335,   228,   229,   230,   231,
     232,     0,   233,   517,     0,   234,   235,   321,     0,   215,
     216,   217,   218,   219,   220,     0,     0,     0,     0,     0,
       0,   221,     0,     0,     0,  -152,   322,   659,     0,     0,
       0,     0,     0,     0,     0,     0,   324,     0,     0,   325,
     326,     0,     0,     0,     0,     0,   327,     0,     0,     0,
       0,     0,   328,   329,     0,     0,   330,   331,   223,     0,
     332,   333,     0,   334,     0,     0,     0,     0,     0,   449,
       0,     0,     0,     0,     0,   224,     0,     0,     0,     0,
     225,   226,   450,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   335,   228,   229,   230,   231,   232,     0,   233,
     517,     0,   234,   235,   321,     0,   215,   216,   217,   218,
     219,   220,     0,     0,     0,     0,     0,     0,   221,     0,
       0,     0,  -152,   322,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   324,     0,     0,   325,   326,     0,     0,
       0,     0,     0,   327,     0,     0,     0,     0,     0,   328,
     329,     0,     0,   330,   331,   223,     0,   332,   333,     0,
     334,     0,     0,     0,     0,     0,   449,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   225,   226,   450,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   335,
     228,   229,   230,   231,   232,     0,   233,   517,     0,   234,
     235,   321,     0,   215,   216,   217,   218,   219,   220,     0,
       0,     0,     0,     0,     0,   221,     0,     0,     0,  -152,
     322,   510,     0,     0,     0,     0,     0,     0,     0,     0,
     324,     0,     0,   325,   326,     0,     0,     0,     0,     0,
     327,     0,     0,     0,     0,     0,   328,   329,     0,     0,
     330,   331,   223,     0,   332,   333,     0,   334,     0,     0,
       0,     0,     0,   449,     0,     0,     0,     0,     0,   224,
       0,     0,     0,     0,   225,   226,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   335,   228,   229,   230,
     231,   232,     0,   233,   517,     0,   234,   235,   321,     0,
     215,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,   221,     0,     0,     0,  -152,   322,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   324,     0,     0,
     325,   326,     0,     0,     0,     0,     0,   327,     0,     0,
       0,     0,     0,   328,   329,     0,     0,   330,   331,   223,
       0,   332,   333,     0,   334,     0,     0,     0,     0,     0,
     449,     0,     0,     0,     0,     0,   224,     0,   763,     0,
       0,   225,   226,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   335,   228,   229,   230,   231,   232,     0,
     233,   517,     0,   234,   235,   321,     0,   215,   216,   217,
     218,   219,   220,     0,     0,     0,     0,     0,     0,   221,
       0,     0,     0,  -152,   322,   516,     0,     0,     0,     0,
       0,     0,     0,     0,   324,     0,     0,   325,   326,     0,
       0,     0,     0,     0,   327,     0,     0,     0,     0,     0,
     328,   329,     0,     0,   330,   331,   223,     0,   332,   333,
       0,   334,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   224,     0,     0,     0,     0,   225,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     335,   228,   229,   230,   231,   232,     0,   233,   517,     0,
     234,   235,   321,     0,   215,   216,   217,   218,   219,   220,
       0,     0,     0,     0,     0,     0,   221,     0,     0,     0,
    -152,   322,   626,     0,     0,     0,     0,     0,     0,     0,
       0,   324,     0,     0,   325,   326,     0,     0,     0,     0,
       0,   327,     0,     0,     0,     0,     0,   328,   329,     0,
       0,   330,   331,   223,     0,   332,   333,     0,   334,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,     0,     0,     0,     0,   225,   226,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   335,   228,   229,
     230,   231,   232,     0,   233,   517,     0,   234,   235,   321,
       0,   215,   216,   217,   218,   219,   220,     0,     0,     0,
       0,     0,     0,   221,     0,     0,     0,  -152,   322,   510,
       0,     0,     0,     0,     0,     0,     0,     0,   324,     0,
       0,   325,   326,     0,     0,     0,     0,     0,   327,     0,
       0,     0,     0,     0,   328,   329,     0,     0,   330,   331,
     223,     0,   332,   333,     0,   334,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   224,     0,     0,
       0,     0,   225,   226,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   335,   228,   229,   230,   231,   232,
       0,   233,   517,     0,   234,   235,   321,     0,   215,   216,
     217,   218,   219,   220,     0,     0,     0,     0,     0,     0,
     221,     0,     0,     0,  -152,   322,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   324,     0,     0,   325,   326,
       0,     0,     0,     0,     0,   327,     0,     0,     0,     0,
       0,   328,   329,     0,     0,   330,   331,   223,     0,   332,
     333,     0,   334,     0,     0,     0,     0,     0,   449,     0,
       0,     0,     0,     0,   224,     0,     0,     0,     0,   225,
     226,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   335,   228,   229,   230,   231,   232,     0,   233,   517,
       0,   234,   235,   321,     0,   215,   216,   217,   218,   219,
     220,     0,     0,     0,     0,     0,     0,   221,     0,     0,
       0,  -152,   322,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   324,     0,     0,   325,   326,     0,     0,     0,
       0,     0,   327,     0,     0,     0,     0,     0,   328,   329,
       0,     0,   330,   331,   223,     0,   332,   333,     0,   334,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   224,   696,     0,     0,     0,   225,   226,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   335,   228,
     229,   230,   231,   232,     0,   233,   517,     0,   234,   235,
     321,     0,   215,   216,   217,   218,   219,   220,     0,     0,
       0,     0,     0,     0,   221,     0,     0,     0,  -152,   322,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   324,
       0,     0,   325,   326,     0,     0,     0,     0,     0,   327,
       0,     0,     0,     0,     0,   328,   329,     0,     0,   330,
     331,   223,     0,   332,   333,     0,   334,   611,     0,   215,
     216,   217,   218,   219,   220,     0,     0,     0,   224,     0,
       0,   221,     0,   225,   226,  -152,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   335,   228,   229,   230,   231,
     232,     0,   233,   517,     0,   234,   235,   711,     0,   215,
     216,   217,   218,   219,   220,     0,     0,     0,   223,     0,
       0,   221,     0,     0,     0,  -152,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   224,     0,     0,     0,     0,
     225,   226,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   227,   228,   229,   230,   231,   232,   223,   233,
       0,     0,   234,   235,   772,     0,   215,   216,   217,   218,
     219,   220,     0,     0,     0,   224,     0,     0,   221,  -152,
     225,   226,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   227,   228,   229,   230,   231,   232,     0,   233,
       0,     0,   234,   235,     0,     0,   215,   216,   217,   218,
     219,   220,     0,     0,     0,   223,     0,     0,   221,     0,
       0,     0,     0,   529,   716,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   225,   226,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   227,
     228,   229,   230,   231,   232,   223,   233,     0,     0,   234,
     235,   215,   216,   217,   218,   219,   220,     0,     0,     0,
       0,     0,   224,   221,     0,     0,     0,   225,   226,     0,
       0,     0,   222,     0,     0,     0,     0,     0,     0,   227,
     228,   229,   230,   231,   232,     0,   233,     0,     0,   234,
     235,   215,   216,   217,   218,   219,   220,     0,     0,     0,
     223,     0,     0,   221,     0,     0,     0,     0,     0,     0,
       0,     0,   415,     0,     0,     0,     0,   224,     0,     0,
       0,     0,   225,   226,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   227,   228,   229,   230,   231,   232,
     223,   233,     0,     0,   234,   235,     0,   215,   216,   217,
     218,   219,   220,     0,     0,     0,     0,   224,     0,   221,
       0,     0,   225,   226,   529,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   227,   228,   229,   230,   231,   232,
       0,   233,     0,     0,   234,   235,     0,   215,   216,   217,
     218,   219,   220,     0,     0,     0,   223,     0,     0,   221,
       0,     0,     0,     0,   322,     0,     0,     0,     0,     0,
       0,     0,     0,   224,     0,     0,     0,     0,   225,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,   228,   229,   230,   231,   232,   223,   233,     0,     0,
     234,   235,   215,   216,   217,   218,   219,   220,     0,     0,
       0,     0,     0,   224,   221,   544,     0,     0,   225,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,   228,   229,   230,   231,   232,     0,   233,     0,     0,
     234,   235,   215,   216,   217,   218,   219,   220,     0,     0,
       0,   223,     0,     0,   221,   685,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   227,   228,   229,   230,   231,
     232,   223,   233,     0,     0,   234,   235,   215,   216,   217,
     218,   219,   220,     0,     0,     0,     0,     0,   224,   221,
     783,     0,     0,   225,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   227,   228,   229,   230,   231,
     232,     0,   233,     0,     0,   234,   235,   215,   216,   217,
     218,   219,   220,     0,     0,     0,   223,     0,     0,   221,
     785,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   224,     0,     0,     0,     0,   225,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,   228,   229,   230,   231,   232,   223,   233,     0,     0,
     234,   235,   215,   216,   217,   218,   219,   220,     0,     0,
       0,     0,     0,   224,   221,   831,     0,     0,   225,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,   228,   229,   230,   231,   232,     0,   233,     0,     0,
     234,   235,   215,   216,   217,   218,   219,   220,     0,     0,
       0,   223,     0,     0,   221,   833,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   227,   228,   229,   230,   231,
     232,   223,   233,     0,     0,   234,   235,   215,   216,   217,
     218,   219,   220,     0,     0,     0,     0,     0,   224,   379,
       0,     0,     0,   225,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   227,   228,   229,   230,   231,
     232,     0,   233,     0,     0,   234,   235,   215,   216,   217,
     218,   219,   220,     0,     0,     0,   223,     0,     0,   384,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   224,     0,     0,     0,     0,   225,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,   228,   229,   230,   231,   232,   223,   233,     0,     0,
     234,   235,   215,   216,   217,   218,   219,   220,     0,     0,
       0,     0,     0,   224,   221,     0,     0,     0,   225,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,   228,   229,   230,   231,   232,     0,   233,     0,     0,
     234,   235,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   223,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   227,   228,   229,   230,   231,
     232,     0,   233,     0,     0,   234,   235,     3,     4,     5,
       6,     0,     0,     7,     0,     0,     8,     9,    10,    11,
      12,     0,    13,    14,    15,    16,    17,     0,     0,    18,
      19,     0,     0,     0,    20,     0,     0,    21,     3,     4,
       5,     6,    23,     0,     7,   687,     0,     8,     9,    10,
      11,    12,     0,    13,    14,    15,    16,    17,     0,     0,
      18,    19,     0,     0,     0,    20,     0,     0,    21,     0,
       0,     0,     0,    23,     0,    25,     0,     0,     3,     4,
       5,     6,   696,     0,     7,     0,     0,     8,     9,    10,
      11,    12,     0,    13,    14,    15,    16,    17,     0,    26,
      18,    19,     0,     0,     0,    20,    25,     0,    21,     3,
       4,     5,     6,    23,     0,     7,     0,     0,     8,     9,
      10,    11,    12,     0,    13,    14,    15,    16,    17,     0,
      26,    18,    19,     4,     5,     6,    20,     0,     7,    21,
       0,     8,     0,     0,    11,     0,    25,    13,    14,    15,
      16,    17,     0,     0,    18,    19,     0,     0,     0,    20,
       0,     0,     0,     0,     4,     5,     6,     0,     0,     7,
      26,     0,     8,     0,     0,    11,     0,    25,    13,    14,
      15,    16,    17,     0,     0,    18,    19,     0,     0,     0,
      20,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      25,    26,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    26,     0,     0,     0,     0,     0,
       0,   124,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    26
};

static const yytype_int16 yycheck[] =
{
       0,   221,   200,    62,   461,     0,     0,    86,    87,    39,
       0,    48,   200,   280,    38,   134,   346,   133,    86,    43,
     345,   544,    76,   372,   205,   353,   207,   208,     0,   210,
      67,    74,    86,   361,   267,     8,   640,    18,   353,   255,
       2,    15,    79,    21,   239,   161,   361,    87,   378,   584,
     585,    88,    82,   687,    54,    35,    36,    37,   274,    54,
      54,   171,     9,    10,    54,   175,    59,    60,   591,   327,
     279,   280,   529,    86,    19,    20,    20,    22,     4,    52,
      74,    69,    54,     4,    19,    20,    74,    22,   266,    15,
     133,   725,   171,   172,    15,     4,   175,   140,    86,   287,
     159,    16,    74,     9,   172,    10,    15,    22,    86,   146,
     326,   148,    86,   172,    23,    77,    42,    22,   161,     4,
      16,     0,   645,   160,   105,    87,    22,    53,    13,    14,
      20,   171,   457,    42,   158,   175,   255,   205,   133,   133,
     208,   178,    86,   133,    53,    42,   140,    85,   346,   347,
      94,    20,     4,   100,   101,   274,    53,   345,   762,   379,
      86,   133,   107,    15,   384,    86,   161,   161,   140,   389,
     286,   161,   107,    94,   119,    16,     4,    86,   221,    96,
     378,   704,    16,   207,   119,   295,   210,    15,   513,   161,
     378,   411,   109,   728,   375,   376,    86,   114,    15,   394,
     395,   396,   435,    20,    94,   421,   534,   326,   538,   171,
     127,     4,    15,   175,    42,    74,   295,     5,     6,   534,
      23,   118,    15,   680,   267,    53,   206,   221,   271,   487,
      23,   261,    98,    99,    86,   265,   260,   353,   262,   588,
     283,     4,   266,   286,   422,   361,   221,   425,   285,   221,
      20,    86,    15,   431,    15,   295,   372,   294,    86,    94,
      23,   298,   299,   263,   301,   181,     0,     4,     1,   185,
     186,     4,   492,   267,   190,   191,    20,   271,    15,   195,
     196,   140,    15,    42,   352,    59,   107,    61,    25,   283,
     488,   286,   286,   491,    28,   267,   286,   495,   352,   271,
      34,    16,   421,   276,    38,    39,   360,    22,    20,    43,
     353,   283,     1,   353,   286,     4,   353,   515,   361,     4,
      54,    20,    19,    86,   361,    22,    15,    86,    62,   372,
      15,    94,    22,   295,    22,    94,   379,    25,   519,    15,
     538,   539,   537,   524,     4,     4,   346,    23,    82,    86,
      84,    20,   376,    86,    16,    15,    15,    94,   353,   353,
      22,   134,   221,   353,    23,    25,   361,   361,   698,    60,
      61,   361,   288,   289,    20,   584,   585,   372,   372,    15,
      16,   353,   372,    42,   651,   379,   653,    23,   608,   361,
     720,   353,   435,    19,    53,    20,    22,    86,   618,   361,
     372,    86,    86,   433,   379,    94,    20,   379,   267,   384,
      94,   609,   271,   671,   672,    15,    16,    19,   534,    20,
      22,   619,    86,    23,   283,   159,    86,    86,   465,    23,
      94,    15,    16,    19,    16,    94,    22,   635,   172,    23,
      22,   435,   651,    16,   653,    15,    16,    15,    16,    22,
     223,   519,   633,    23,    20,    23,   644,   457,   656,    62,
     519,   234,   235,   435,    16,   524,   239,   655,   656,    86,
      22,   205,   588,   207,   208,   119,   210,    94,     1,   533,
       1,     4,   255,     4,   373,    16,   684,   707,   746,   747,
     748,   534,    15,   713,    15,   683,   684,   534,   387,   697,
     698,   274,   700,    16,   702,   752,    16,    16,   424,    22,
     708,   427,    22,   513,    22,    74,    24,   715,   765,   728,
     379,   719,   720,   721,   722,   259,   260,   261,   262,   648,
     777,   265,   266,   102,   103,   733,   734,   735,   538,   534,
     534,   739,    16,   790,   534,   588,    16,    19,    22,   586,
      22,   798,    22,   326,   752,   753,   754,   451,   756,    22,
      16,    16,   534,    86,     4,    86,    22,   765,   766,   544,
     107,    94,   461,    94,   633,    15,   435,   306,    16,   777,
     778,   140,   311,    16,    22,    15,    19,   316,    15,    21,
      22,   628,   790,   588,   588,    72,    73,    15,   588,     3,
     798,    12,    42,    11,     0,     1,     2,   104,   828,   829,
      67,    68,    24,    53,   589,   590,   588,   511,   507,   508,
      87,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,    34,   412,
     529,    21,    38,    39,   397,   398,    86,    43,   421,     0,
       1,   399,   400,    16,    94,    61,     4,   573,    54,    15,
     576,   107,   221,   638,   639,    19,    62,    15,    15,    35,
     645,    16,    38,    19,    40,   405,   406,    43,    19,    15,
      22,    77,    25,    34,    19,    25,    82,    38,    39,   689,
      86,    87,    43,    25,    42,    86,    16,   431,   698,   433,
      86,    86,   702,    54,   598,    53,    16,    24,   267,    25,
       4,    62,   271,    21,    86,   511,    22,   513,    22,    19,
     720,    15,   722,    16,   283,    15,    26,    15,    15,    23,
     624,    82,    15,    25,   683,    86,    36,    37,    86,    39,
      19,   470,    42,   539,   473,    19,    94,   476,    42,   698,
     479,   640,    19,    53,    16,    16,    56,   646,   154,    53,
      35,    36,    37,    38,   537,    40,    41,    42,    43,    16,
      15,   720,    17,    16,    16,   171,   172,    16,    23,   175,
     674,   401,   402,   403,   404,   519,   735,    15,    20,   155,
     524,   680,   158,    19,    57,    35,    36,    37,    38,    19,
      40,    41,    42,    43,    86,   754,    26,    16,    19,    16,
      15,    15,    65,    26,    16,   709,    36,    37,   118,    39,
     379,   172,    42,    36,    37,    22,    39,    16,   624,    42,
      31,    19,    19,    53,    19,    66,    56,    15,    15,   635,
      53,   207,    21,    56,   210,    19,    73,    20,    16,    16,
      95,    96,    97,    16,    16,    19,   750,    86,    19,    16,
      19,    19,    15,   259,    19,   261,    15,    19,   763,   265,
     599,   600,   761,   762,   645,   648,   435,   689,   727,   760,
     155,   156,   157,   158,   408,   407,   800,   412,   284,   684,
     409,   137,   410,   379,   260,   292,   262,   706,   118,   295,
     266,   271,   697,   286,   700,   118,   433,   702,   259,   425,
     261,   431,    54,   151,   265,   809,   810,   811,   812,   278,
     437,    -1,    -1,    -1,   719,   721,    -1,   722,    -1,    -1,
      -1,   206,   207,    -1,   209,   210,   182,   183,    -1,   734,
      -1,   187,   188,   739,    -1,    -1,   192,   193,    -1,    -1,
      -1,   197,   198,    -1,    -1,    -1,    -1,   353,   753,    -1,
     756,    -1,    -1,   759,    -1,   361,   206,   207,   764,   209,
     210,   766,    -1,    -1,    -1,   656,    -1,    -1,    -1,    -1,
     776,    -1,    -1,   778,    -1,   260,   782,   262,    -1,    -1,
      -1,   266,    -1,   789,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   797,    -1,   684,    -1,    -1,    -1,    -1,    -1,    -1,
     376,   257,   808,    -1,    -1,    -1,   697,    -1,   814,   700,
     260,   702,   262,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,    -1,    -1,    -1,   433,   719,    -1,
     721,   722,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   445,
      -1,    -1,   733,   734,    -1,    -1,    -1,    -1,   739,   305,
      -1,    -1,    -1,    -1,   310,    -1,    -1,    -1,    -1,   315,
      -1,   752,   753,    -1,   320,   756,    -1,    -1,    -1,    -1,
      -1,    -1,   433,    -1,   765,   766,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   777,   778,    -1,    -1,
       1,   376,     3,     4,     5,     6,     7,     8,    -1,   790,
      -1,    -1,    -1,    -1,    15,    -1,    -1,   798,    19,    20,
      21,    -1,    -1,    -1,    -1,    26,    27,    28,    29,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    -1,    -1,    -1,
      61,    -1,    63,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    -1,    93,    94,    -1,    96,    97,     1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    -1,    -1,    -1,    19,    20,   118,    -1,    -1,
      -1,    -1,    26,    27,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    61,    -1,    63,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    -1,    93,
      94,    -1,    96,    97,    -1,    -1,    -1,     1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    -1,    -1,   118,    19,    20,    21,    -1,    -1,
      -1,    -1,    26,    27,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    -1,    93,
      94,    -1,    96,    97,    -1,    -1,    -1,     1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    -1,    -1,   118,    19,    20,    21,    -1,    -1,
      -1,    -1,    26,    27,    28,    29,    30,    -1,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    -1,    -1,    -1,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    -1,    93,
      94,    -1,    96,    97,     1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,
      -1,    -1,    19,    20,   118,    -1,    -1,    -1,    -1,    26,
      27,    28,    29,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    -1,    61,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    91,    -1,    93,    94,    -1,    96,
      97,     1,    -1,     3,     4,     5,     6,     7,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,    19,
      20,   118,    -1,    -1,    -1,    -1,    26,    27,    28,    29,
      30,    -1,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    -1,    -1,
      -1,    61,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      70,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,
      90,    91,    -1,    93,    94,    -1,    96,    97,     1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    -1,    -1,    -1,    19,    20,   118,    -1,
      -1,    -1,    -1,    26,    27,    28,    29,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    -1,    -1,    -1,    61,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    91,    -1,
      93,    94,     1,    96,    97,     4,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   118,    -1,    26,    27,    28,
      29,    -1,    -1,    32,    -1,    -1,    35,    36,    37,    38,
      39,    -1,    41,    42,    43,    44,    45,    -1,    -1,    48,
      49,    -1,    -1,    -1,    53,    -1,    -1,    56,    -1,    -1,
      -1,    -1,    -1,     1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    26,    27,
      28,    29,    -1,    -1,    32,    94,    -1,    35,    36,    37,
      38,    39,    -1,    41,    42,    43,    44,    45,    -1,    -1,
      48,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,   118,
      -1,    -1,    -1,    -1,     1,    -1,    -1,     4,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    26,
      27,    28,    29,    -1,    -1,    32,    94,    -1,    35,    36,
      37,    38,    39,    -1,    41,    42,    43,    44,    45,    -1,
      -1,    48,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,
     118,    -1,    -1,    -1,    -1,     1,    -1,    -1,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      26,    27,    28,    29,    -1,    -1,    32,    94,    -1,    35,
      36,    37,    38,    39,     1,    41,    42,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    -1,    -1,    53,    -1,    -1,
      56,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,
      27,    28,    29,    -1,    -1,    32,     1,    -1,    35,    36,
      37,    38,    39,    -1,    41,    42,    43,    44,    45,    -1,
      86,    48,    49,    -1,    -1,    -1,    53,    -1,    94,    56,
      -1,    26,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
      35,    36,    37,    38,    39,    -1,    41,    42,    43,    44,
      45,    -1,   118,    48,    49,    -1,    -1,    -1,    53,    -1,
      -1,    56,    -1,    -1,    -1,    -1,    -1,    94,     3,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,   106,
      15,    -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,
      -1,   118,    27,    28,    29,    -1,    -1,    32,    -1,    94,
      35,    -1,    -1,    38,    -1,    -1,    41,    42,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    -1,    52,    53,    26,
      27,    28,    -1,   118,    -1,    32,    -1,    -1,    -1,    36,
      37,    38,    39,    -1,    69,    42,    43,    44,    45,    74,
      75,    48,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,
      -1,    86,    87,    88,    89,    90,    91,     4,    93,    94,
      -1,    96,    97,    -1,    -1,    -1,    -1,    -1,    15,    16,
      -1,    -1,    -1,    -1,    -1,    -1,    23,    -1,    -1,    26,
      27,    28,    29,   118,    -1,    32,    -1,    -1,    35,    36,
      37,    38,    39,     4,    41,    42,    43,    44,    45,    -1,
      -1,    48,    49,    -1,    15,    16,    53,    -1,    -1,    56,
      -1,   118,    23,    -1,    -1,    26,    27,    28,    29,    -1,
      -1,    32,    -1,    -1,    35,    36,    37,    38,    39,    -1,
      41,    42,    43,    44,    45,    -1,    -1,    48,    49,    86,
      -1,    -1,    53,    -1,    -1,    56,    -1,    94,    -1,    -1,
      -1,    -1,    27,    28,    -1,    -1,    -1,    32,    -1,   106,
      -1,    -1,    -1,    38,    -1,     4,    -1,    42,    43,    44,
      45,   118,    -1,    48,    49,    86,    15,    -1,    53,    -1,
      -1,    -1,    -1,    94,    -1,    -1,    -1,    26,    27,    28,
      29,    -1,    -1,    32,    -1,   106,    35,    36,    37,    38,
      39,     4,    41,    42,    43,    44,    45,   118,    -1,    48,
      49,    -1,    15,    16,    53,    -1,    -1,    56,    -1,    58,
      23,    -1,    61,    26,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    35,    36,    37,    38,    39,     4,    41,    42,
      43,    44,    45,   118,    -1,    48,    49,    86,    15,    -1,
      53,    -1,    -1,    56,    -1,    94,    23,    -1,    -1,    26,
      27,    28,    29,    -1,    -1,    32,    -1,    -1,    35,    36,
      37,    38,    39,     4,    41,    42,    43,    44,    45,   118,
      -1,    48,    49,    -1,    15,    -1,    53,    -1,    -1,    56,
      -1,    94,    23,    -1,    -1,    26,    27,    28,    29,    -1,
      -1,    32,    -1,   106,    35,    36,    37,    38,    39,     4,
      41,    42,    43,    44,    45,   118,    -1,    48,    49,    86,
      15,    -1,    53,    -1,    -1,    56,    -1,    94,    -1,    -1,
      25,    -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
      35,     4,    -1,    38,    -1,    -1,    41,    42,    43,    44,
      45,   118,    15,    48,    49,    86,    -1,    -1,    53,    -1,
      23,    -1,    -1,    94,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    35,    -1,    -1,    38,    -1,    -1,    41,    42,
      43,    44,    45,    -1,    16,    48,    49,   118,    -1,    -1,
      53,    86,    -1,    -1,    26,    27,    28,    29,    -1,    94,
      32,    -1,    -1,    35,    36,    37,    38,    39,    -1,    41,
      42,    43,    44,    45,    -1,    -1,    48,    49,    -1,    21,
      -1,    53,    -1,   118,    56,    27,    28,    29,    -1,    -1,
      32,    94,    -1,    35,    -1,    -1,    38,    -1,    -1,    41,
      42,    43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,
      -1,    53,    -1,    -1,    86,   118,    16,    -1,    -1,    -1,
      -1,    -1,    94,    -1,    -1,    -1,    26,    27,    28,    29,
      -1,    -1,    32,    -1,   106,    35,    36,    37,    38,    39,
      -1,    41,    42,    43,    44,    45,   118,    -1,    48,    49,
      -1,    21,    94,    53,    -1,    -1,    56,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    35,    -1,    -1,    38,    -1,
      -1,    41,    42,    43,    44,    45,   118,    -1,    48,    49,
      -1,    21,    -1,    53,    -1,    -1,    -1,    27,    28,    29,
      -1,    -1,    32,    -1,    94,    35,    -1,    -1,    38,    -1,
      -1,    41,    42,    43,    44,    45,   106,    -1,    48,    49,
      -1,    -1,    -1,    53,    21,    -1,    -1,    -1,   118,    -1,
      27,    28,    29,    -1,    94,    32,    -1,    -1,    35,    -1,
      -1,    38,    -1,    -1,    41,    42,    43,    44,    45,    -1,
      -1,    48,    49,    -1,    -1,    -1,    53,    -1,   118,    -1,
      -1,    -1,    -1,     1,    94,     3,     4,     5,     6,     7,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,
      -1,    19,    20,    21,    -1,    -1,    -1,    -1,   118,    -1,
      -1,    -1,    30,    -1,    -1,    33,    34,    94,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    50,    51,    52,    -1,    54,    55,    -1,    57,
      -1,   118,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    91,    -1,    93,    94,    -1,    96,    97,
       1,    -1,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,    19,    20,
      21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,
      51,    52,    -1,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,    76,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    -1,    93,    94,    -1,    96,    97,     1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    -1,    -1,    -1,    19,    20,    21,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,
      34,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,
      -1,    -1,    46,    47,    -1,    -1,    50,    51,    52,    -1,
      54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    63,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      74,    75,    76,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    -1,    93,
      94,    -1,    96,    97,     1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,
      -1,    -1,    19,    20,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    50,    51,    52,    -1,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    91,    -1,    93,    94,    -1,    96,
      97,     1,    -1,     3,     4,     5,     6,     7,     8,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,    19,
      20,    21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    -1,    33,    34,    -1,    -1,    -1,    -1,    -1,
      40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,
      50,    51,    52,    -1,    54,    55,    -1,    57,    -1,    -1,
      -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,
      90,    91,    -1,    93,    94,    -1,    96,    97,     1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    -1,    -1,    -1,    19,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,    52,
      -1,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      63,    -1,    -1,    -1,    -1,    -1,    69,    -1,    71,    -1,
      -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    91,    -1,
      93,    94,    -1,    96,    97,     1,    -1,     3,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,
      -1,    -1,    -1,    19,    20,    21,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,
      -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,
      46,    47,    -1,    -1,    50,    51,    52,    -1,    54,    55,
      -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    -1,    93,    94,    -1,
      96,    97,     1,    -1,     3,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,
      19,    20,    21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    33,    34,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    50,    51,    52,    -1,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,
      89,    90,    91,    -1,    93,    94,    -1,    96,    97,     1,
      -1,     3,     4,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    -1,    -1,    -1,    19,    20,    21,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,
      -1,    33,    34,    -1,    -1,    -1,    -1,    -1,    40,    -1,
      -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,
      52,    -1,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    91,
      -1,    93,    94,    -1,    96,    97,     1,    -1,     3,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    -1,    -1,    -1,    19,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,
      -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    50,    51,    52,    -1,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    74,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    87,    88,    89,    90,    91,    -1,    93,    94,
      -1,    96,    97,     1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,
      -1,    19,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    -1,    -1,    33,    34,    -1,    -1,    -1,
      -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,
      -1,    -1,    50,    51,    52,    -1,    54,    55,    -1,    57,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    70,    -1,    -1,    -1,    74,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    91,    -1,    93,    94,    -1,    96,    97,
       1,    -1,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,    19,    20,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,
      51,    52,    -1,    54,    55,    -1,    57,     1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    69,    -1,
      -1,    15,    -1,    74,    75,    19,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    -1,    93,    94,    -1,    96,    97,     1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    52,    -1,
      -1,    15,    -1,    -1,    -1,    19,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    52,    93,
      -1,    -1,    96,    97,     1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    69,    -1,    -1,    15,    16,
      74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    -1,    93,
      -1,    -1,    96,    97,    -1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    52,    -1,    -1,    15,    -1,
      -1,    -1,    -1,    20,    21,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    91,    52,    93,    -1,    -1,    96,
      97,     3,     4,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,    69,    15,    -1,    -1,    -1,    74,    75,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    91,    -1,    93,    -1,    -1,    96,
      97,     3,     4,     5,     6,     7,     8,    -1,    -1,    -1,
      52,    -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    24,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    91,
      52,    93,    -1,    -1,    96,    97,    -1,     3,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    -1,    69,    -1,    15,
      -1,    -1,    74,    75,    20,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    91,
      -1,    93,    -1,    -1,    96,    97,    -1,     3,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    52,    -1,    -1,    15,
      -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    52,    93,    -1,    -1,
      96,    97,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    69,    15,    16,    -1,    -1,    74,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    -1,    93,    -1,    -1,
      96,    97,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    52,    -1,    -1,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    52,    93,    -1,    -1,    96,    97,     3,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    -1,    -1,    69,    15,
      16,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    -1,    93,    -1,    -1,    96,    97,     3,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    52,    -1,    -1,    15,
      16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    52,    93,    -1,    -1,
      96,    97,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    69,    15,    16,    -1,    -1,    74,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    -1,    93,    -1,    -1,
      96,    97,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    52,    -1,    -1,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    52,    93,    -1,    -1,    96,    97,     3,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    -1,    -1,    69,    15,
      -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    -1,    93,    -1,    -1,    96,    97,     3,     4,     5,
       6,     7,     8,    -1,    -1,    -1,    52,    -1,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    52,    93,    -1,    -1,
      96,    97,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    69,    15,    -1,    -1,    -1,    74,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    91,    -1,    93,    -1,    -1,
      96,    97,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    -1,    93,    -1,    -1,    96,    97,    26,    27,    28,
      29,    -1,    -1,    32,    -1,    -1,    35,    36,    37,    38,
      39,    -1,    41,    42,    43,    44,    45,    -1,    -1,    48,
      49,    -1,    -1,    -1,    53,    -1,    -1,    56,    26,    27,
      28,    29,    61,    -1,    32,    64,    -1,    35,    36,    37,
      38,    39,    -1,    41,    42,    43,    44,    45,    -1,    -1,
      48,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,    -1,
      -1,    -1,    -1,    61,    -1,    94,    -1,    -1,    26,    27,
      28,    29,    70,    -1,    32,    -1,    -1,    35,    36,    37,
      38,    39,    -1,    41,    42,    43,    44,    45,    -1,   118,
      48,    49,    -1,    -1,    -1,    53,    94,    -1,    56,    26,
      27,    28,    29,    61,    -1,    32,    -1,    -1,    35,    36,
      37,    38,    39,    -1,    41,    42,    43,    44,    45,    -1,
     118,    48,    49,    27,    28,    29,    53,    -1,    32,    56,
      -1,    35,    -1,    -1,    38,    -1,    94,    41,    42,    43,
      44,    45,    -1,    -1,    48,    49,    -1,    -1,    -1,    53,
      -1,    -1,    -1,    -1,    27,    28,    29,    -1,    -1,    32,
     118,    -1,    35,    -1,    -1,    38,    -1,    94,    41,    42,
      43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,    -1,
      53,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      94,   118,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   118
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,    15,    26,    27,    28,    29,    32,    35,    36,
      37,    38,    39,    41,    42,    43,    44,    45,    48,    49,
      53,    56,    58,    61,    86,    94,   118,   121,   122,   124,
     126,   174,   175,   184,   193,   194,   195,   196,   197,   199,
     200,   201,   202,   203,   205,   217,   218,   219,   220,   221,
     222,   239,   240,   249,   275,   276,   277,   297,   299,   300,
      42,    53,   204,   217,   221,   298,   218,   220,   221,    20,
      86,    94,   224,    86,     9,     0,     4,    15,   217,   220,
     125,    19,    22,    19,    22,     1,     4,    15,    94,   206,
     207,   208,   210,   211,   212,   217,   221,   198,   297,   299,
     300,    19,   198,   198,     1,    94,   198,   205,   217,   221,
     300,     1,   206,   217,   221,   297,   299,   300,    19,   297,
     299,   297,   299,     1,    94,   205,   217,   221,   297,   299,
     300,    85,   278,    15,    23,   216,   238,   283,   293,   294,
      20,   224,   276,   217,   221,   298,    16,    16,    16,   224,
     250,    20,    15,   123,   199,   200,   201,   202,   203,   204,
     220,    15,    20,   130,     1,   206,   217,   221,     1,   217,
     182,    15,   204,   207,   210,    15,    94,   207,   209,   211,
     216,   176,   279,   284,   293,   191,   185,   281,   286,   293,
     183,   178,   280,   285,   293,   192,   187,   282,   287,   293,
      20,   260,    16,    86,   106,   193,   195,   197,   199,   201,
     203,   223,   235,   236,   237,     3,     4,     5,     6,     7,
       8,    15,    24,    52,    69,    74,    75,    86,    87,    88,
      89,    90,    91,    93,    96,    97,   152,   153,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   172,   295,   296,    23,   260,   288,   174,   193,
     197,   199,   203,   261,    21,   199,   203,   241,   242,   243,
     244,    20,   216,   216,   107,   251,    22,   252,   250,    59,
      60,   127,    10,    22,    15,   220,    62,   131,   180,   189,
     119,   226,   227,   228,   209,    15,   207,   210,   209,    16,
     216,    16,   226,   260,   260,   289,   226,   226,   260,   260,
     291,   226,   226,   260,   260,   290,   226,   226,   260,   260,
     292,     1,    20,    21,    30,    33,    34,    40,    46,    47,
      50,    51,    54,    55,    57,    86,    94,   155,   168,   169,
     171,   173,   254,   255,   257,   258,   261,   262,   263,   264,
     266,   274,     4,    15,   210,   213,   214,   215,   216,   217,
       4,    15,   213,   217,   210,   213,   217,   213,   217,    16,
      22,    16,    22,   107,   171,   199,   203,   225,   258,    15,
     155,    15,    15,    15,    15,   155,   155,    15,    17,    23,
      95,    96,    97,   157,     4,    13,    14,     5,     6,    98,
      99,     9,    10,   100,   101,   102,   103,     3,    12,    11,
     104,    18,   105,    24,    87,    24,   172,   260,   217,   217,
     174,    25,   206,   245,   248,   217,   246,   248,    21,   242,
      19,    22,    19,    22,    21,   241,   172,   224,    21,   252,
     122,   128,   129,   128,    16,   199,   220,   216,   236,    63,
      76,   132,   133,   134,   135,   253,   254,   261,   226,   226,
      15,   107,   232,   228,    16,   209,    16,   216,    16,   216,
     177,   260,   232,   186,   260,   232,   179,   260,   232,   188,
     260,    19,    19,   265,   172,   173,    19,    15,    25,    86,
      94,   268,    15,   267,    25,    25,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   170,    22,    19,
      21,   132,   259,   261,    21,   262,    21,    94,   254,   204,
     213,   214,   215,   216,   204,    86,     1,   106,   237,    20,
     169,   233,    16,     4,    15,   213,   213,    16,   261,   262,
     225,    86,    86,    16,    16,   154,   169,    86,    94,   171,
      86,    94,   157,   157,   157,   158,   158,   159,   159,   160,
     160,   160,   160,   161,   161,   162,   163,   164,   165,   171,
     166,    24,   172,   247,   248,   226,   247,   226,   246,   245,
      21,   251,    21,    86,    22,    22,   220,    19,    22,    69,
      74,    86,   136,   137,   143,    15,    21,   253,   132,   181,
     190,    15,   233,    16,   232,   232,   232,   232,    15,    25,
      19,     1,   173,   254,    19,    19,   254,   171,    15,   256,
     254,   169,   169,   259,   132,   259,    21,   213,    16,    16,
      16,   233,   234,   204,   157,   262,   259,    16,    16,    16,
      15,   144,   144,   145,   258,    16,    22,    24,    25,   226,
     226,    60,   129,    59,   216,   258,   258,   144,    86,    21,
     232,   232,    42,    86,    94,   229,   230,   231,   171,   254,
      16,    19,    19,    57,    16,   171,   254,   259,   216,    21,
      22,   259,    16,   258,   258,    16,   154,    64,   146,   261,
     144,   145,   169,   168,   128,   128,    70,   138,   261,   135,
     139,   254,   261,    19,    16,    16,    22,    15,    16,   273,
     173,     1,   173,    15,   253,    16,    21,   233,    16,   138,
     261,   139,   261,    16,   257,    65,   147,   146,    22,    72,
      73,   141,   142,   139,   138,   262,   135,   254,   259,   139,
     144,    16,   230,   171,   254,   253,    19,    19,    19,   171,
      31,   254,   139,   138,   262,   259,   139,   257,    66,   148,
     147,    15,    15,    71,   140,   139,   138,   259,    19,    16,
     173,   173,     1,   173,    16,   253,   140,   139,   138,   259,
      20,   259,   148,    16,   169,    16,   154,   141,   259,   140,
     139,    16,    16,    16,    16,    19,   259,   140,   139,    86,
     149,   150,   259,    19,    16,    19,    16,   259,   140,   270,
     272,   271,   269,   259,   140,    67,    68,   151,    21,   150,
      19,    19,   259,   253,   253,   253,   253,   259,    15,    15,
      19,    16,   171,    16,   171,    16,    16
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 386 "ANSI-C.y"
    { Program = GrabPragmas((yyvsp[(1) - (1)].L)); }
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 398 "ANSI-C.y"
    {
			(yyval.n) = SetSTRdclNameFields(MakeSTRdclCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (4)].tok)), NULL, (yyvsp[(3) - (4)].L), (yyvsp[(2) - (4)].tok), (yyvsp[(4) - (4)].tok));
		}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 405 "ANSI-C.y"
    { 
			(yyval.L) = MakeNewList(SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Stream));	
		}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 409 "ANSI-C.y"
    { 
			(yyval.L) = MakeNewList(SetDeclType( ModifyDeclType((yyvsp[(2) - (3)].n),(yyvsp[(3) - (3)].n)),(yyvsp[(1) - (3)].n), Stream));
			// $$ = MakeNewList(SetDeclTypeMult($2, $4, $1, Stream));
			//$$ = MakeNewList(SetDeclType($2, $1, Stream));	
		}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 415 "ANSI-C.y"
    { 
			(yyval.L) = JoinLists((yyvsp[(1) - (4)].L), MakeNewList(SetDeclType((yyvsp[(4) - (4)].n), (yyvsp[(3) - (4)].n), Stream)));
		}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 419 "ANSI-C.y"
    { 
			(yyval.L) = JoinLists((yyvsp[(1) - (5)].L), MakeNewList(SetDeclType(ModifyDeclType((yyvsp[(4) - (5)].n), (yyvsp[(5) - (5)].n)), (yyvsp[(3) - (5)].n), Stream)));
			//$$ = JoinLists($1, MakeNewList(SetDeclTypeMult($4, $6, $3, Stream)));
			//$$ = JoinLists($1, MakeNewList(SetDeclType($4, $3, Stream)));
		}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 430 "ANSI-C.y"
    {
			IsInCompositeParam = TRUE;
			(yyvsp[(1) - (1)].n) = DefineComposite((yyvsp[(1) - (1)].n));
		}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 435 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found composite.definition!\n\n");
			(yyval.n) = SetCompositeBody((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n));
			
		}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 445 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found composite.head!\n");
			(yyval.n) = SetDeclType(ModifyDeclType(ConvertIdToDecl((yyvsp[(2) - (5)].n), EMPTY_TQ, NULL, NULL, NULL), MakeComdclCoord(EMPTY_TQ, (yyvsp[(4) - (5)].n), (yyvsp[(3) - (5)].tok))),
								MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (5)].tok)), 
								Redecl);
		}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 454 "ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 456 "ANSI-C.y"
    {
			(yyval.n) = MakeComInOutCoord(EMPTY_TQ, (yyvsp[(2) - (5)].L), (yyvsp[(5) - (5)].L), (yyvsp[(1) - (5)].tok));
		}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 460 "ANSI-C.y"
    {
			(yyval.n) = MakeComInOutCoord(EMPTY_TQ, (yyvsp[(2) - (2)].L), NULL, (yyvsp[(1) - (2)].tok));
		}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 464 "ANSI-C.y"
    {
			(yyval.n) = MakeComInOutCoord(EMPTY_TQ, NULL, (yyvsp[(2) - (2)].L), (yyvsp[(1) - (2)].tok));
		}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 468 "ANSI-C.y"
    {
			(yyval.n) = MakeComInOutCoord(EMPTY_TQ, (yyvsp[(5) - (5)].L), (yyvsp[(2) - (5)].L), (yyvsp[(1) - (5)].tok));
		}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 475 "ANSI-C.y"
    {
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 479 "ANSI-C.y"
    {
			(yyval.L) = AppendItem((yyvsp[(1) - (3)].L),(yyvsp[(3) - (3)].n));
		}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 486 "ANSI-C.y"
    {
			(yyval.n) = SetDeclType(ConvertIdToDecl((yyvsp[(2) - (2)].n), EMPTY_TQ, NULL, NULL, NULL), (yyvsp[(1) - (2)].n), Commal);
		}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 494 "ANSI-C.y"
    {
			/////*DEBUG*/printf("have found composite.body!\n");
			(yyval.n) = MakeComBodyCoord(PrimVoid, (yyvsp[(2) - (4)].n), NULL,(yyvsp[(3) - (4)].L), (yyvsp[(1) - (4)].tok), (yyvsp[(4) - (4)].tok));
		}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 499 "ANSI-C.y"
    {
			/////*DEBUG*/printf("have found composite.body!\n");
			(yyval.n) = MakeComBodyCoord(PrimVoid, (yyvsp[(2) - (5)].n), (yyvsp[(3) - (5)].L),(yyvsp[(4) - (5)].L), (yyvsp[(1) - (5)].tok), (yyvsp[(5) - (5)].tok));
		}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 507 "ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 509 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found composite.body.param!\n");
			(yyval.n) = MakeParamCoord((yyvsp[(2) - (3)].L), (yyvsp[(1) - (3)].tok));
			IsInCompositeParam = FALSE;
		}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 518 "ANSI-C.y"
    {
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 522 "ANSI-C.y"
    {
			(yyval.L) = AppendItem((yyvsp[(1) - (2)].L),(yyvsp[(2) - (2)].n));
		}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 532 "ANSI-C.y"
    {
			(yyval.n) = (yyvsp[(1) - (1)].n);
		}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 536 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found operator.files\n");
			(yyval.n) = (yyvsp[(1) - (1)].n);
		}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 544 "ANSI-C.y"
    {
			(yyval.n) = MakeFileWriterOperator((yyvsp[(3) - (6)].n), (yyvsp[(5) - (6)].L), (yyvsp[(1) - (6)].tok));
		}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 551 "ANSI-C.y"
    {
			  (yyval.n) = MakeAddCoord((yyvsp[(2) - (2)].n),(yyvsp[(1) - (2)].tok));
		  }
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 555 "ANSI-C.y"
    {
			(yyval.n) = MakeAddCoord((yyvsp[(2) - (2)].n),(yyvsp[(1) - (2)].tok));
		}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 559 "ANSI-C.y"
    {
			(yyval.n) = MakeAddCoord((yyvsp[(2) - (2)].n),(yyvsp[(1) - (2)].tok));
		}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 566 "ANSI-C.y"
    {
			(yyval.n) = MakePipelineCoord(NULL,NULL,NULL,(yyvsp[(3) - (4)].L),(yyvsp[(1) - (4)].tok));
		}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 570 "ANSI-C.y"
    {
			(yyval.n) = MakePipelineCoord(NULL,NULL,(yyvsp[(3) - (5)].L),(yyvsp[(4) - (5)].L),(yyvsp[(1) - (5)].tok));
		}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 578 "ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL,NULL,NULL,NULL,(yyvsp[(3) - (6)].n),(yyvsp[(4) - (6)].L),(yyvsp[(5) - (6)].n),(yyvsp[(1) - (6)].tok));
		}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 582 "ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL,NULL,(yyvsp[(3) - (7)].L),NULL,(yyvsp[(4) - (7)].n),(yyvsp[(5) - (7)].L),(yyvsp[(6) - (7)].n),(yyvsp[(1) - (7)].tok));
		}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 586 "ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL,NULL,(yyvsp[(3) - (8)].L),(yyvsp[(4) - (8)].L),(yyvsp[(5) - (8)].n),(yyvsp[(6) - (8)].L),(yyvsp[(7) - (8)].n),(yyvsp[(1) - (8)].tok));
		}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 593 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found SPLIT duplicate.statement \n");
			(yyval.n) = MakeSplitCoord((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok));
		}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 598 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found SPLIT roundrobin.statement \n");
			(yyval.n) = MakeSplitCoord((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok));
		}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 606 "ANSI-C.y"
    {
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 610 "ANSI-C.y"
    {
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 614 "ANSI-C.y"
    {
			(yyval.L) = AppendItem((yyvsp[(1) - (2)].L),(yyvsp[(2) - (2)].n));
		}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 618 "ANSI-C.y"
    {
			(yyval.L) = AppendItem((yyvsp[(1) - (2)].L),(yyvsp[(2) - (2)].n));
		}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 625 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found JOIN roundrobin.statement \n");
			(yyval.n) = MakeJoinCoord((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok));
		}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 633 "ANSI-C.y"
    {
			(yyval.n) = MakeRoundRobinCoord(NULL, (yyvsp[(1) - (4)].tok));
		}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 637 "ANSI-C.y"
    {
			(yyval.n) = MakeRoundRobinCoord((yyvsp[(3) - (5)].L), (yyvsp[(1) - (5)].tok));
		}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 644 "ANSI-C.y"
    {
			(yyval.n) = MakeDuplicateCoord(NULL, (yyvsp[(1) - (4)].tok));
		}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 648 "ANSI-C.y"
    {
			(yyval.n) = MakeDuplicateCoord((yyvsp[(3) - (5)].n), (yyvsp[(1) - (5)].tok));
		}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 656 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found operator.default.call\n");
			(yyval.n) = MakeCompositeCallCoord(MakeCompositeIdCoord((yyvsp[(1) - (3)].n)->u.id.text, (yyvsp[(1) - (3)].n)->coord), 
										ModifyOperatorDeclArguments(MakeOperdclCoord(EMPTY_TQ, NULL, NULL, NULL, (yyvsp[(1) - (3)].n)->coord), (yyvsp[(2) - (3)].L)), 
										FALSE, (yyvsp[(1) - (3)].n)->coord);
		}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 666 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found operator.param.list \n");
			(yyval.L) = (yyvsp[(2) - (3)].L);
		}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 672 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found operator.param.list-2 \n");
			(yyval.L) = NULL;
		}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 681 "ANSI-C.y"
    {
			 ///*DEBUG*/printf("have found operator.selfdefine.body\n");
			(yyval.n) = MakeOperBodyCoord(PrimVoid, NULL, (yyvsp[(2) - (5)].n), (yyvsp[(3) - (5)].n), (yyvsp[(4) - (5)].n),(yyvsp[(1) - (5)].tok),(yyvsp[(5) - (5)].tok));
		 }
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 686 "ANSI-C.y"
    {
			 ///*DEBUG*/printf("have found operator.selfdefine.body\n");
			(yyval.n) = MakeOperBodyCoord(PrimVoid, (yyvsp[(2) - (6)].L), (yyvsp[(3) - (6)].n), (yyvsp[(4) - (6)].n), (yyvsp[(5) - (6)].n),(yyvsp[(1) - (6)].tok),(yyvsp[(6) - (6)].tok));
		 }
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 693 "ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 695 "ANSI-C.y"
    {
			///*DEBUG*/printf("operator.selfdefine.logic.init.list.opt is not null!!!\n");
			(yyval.n) = (yyvsp[(2) - (2)].n);
		}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 703 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found WORK compound.statement\n");
			(yyval.n) = (yyvsp[(2) - (2)].n);
		}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 711 "ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 713 "ANSI-C.y"
    { 
			(yyval.n) = (yyvsp[(3) - (4)].L); /*MakeWindowCoord*/
		}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 720 "ANSI-C.y"
    { 
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 724 "ANSI-C.y"
    { 
			(yyval.L) = AppendItem((yyvsp[(1) - (2)].L),(yyvsp[(2) - (2)].n));
		}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 731 "ANSI-C.y"
    { 
			///*DEBUG*/printf("have found IDENTIFIER ':' window.type ';'\n");
			(yyval.n) = MakeWindowCoord(LookupStreamIdsNode((yyvsp[(1) - (3)].n)), (yyvsp[(2) - (3)].n), (yyvsp[(2) - (3)].n)->coord); 
		}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 739 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found SLIDING ',' window.eviction\n");
			(yyval.n) = MakeWindowSlidingCoord(EMPTY_TQ, NULL, (yyvsp[(2) - (3)].tok));
		}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 744 "ANSI-C.y"
    { 
			///*DEBUG*/printf("have found TUMBLING ',' window.trigger\n");
			(yyval.n) = MakeWindowTumbingCoord(EMPTY_TQ,NULL, (yyvsp[(2) - (3)].tok)); 
		}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 749 "ANSI-C.y"
    {
			///*DEBUG*/printf("have found SLIDING ',' window.eviction\n");
			(yyval.n) = MakeWindowSlidingCoord(EMPTY_TQ, (yyvsp[(3) - (4)].n), (yyvsp[(2) - (4)].tok));
		}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 754 "ANSI-C.y"
    { 
			///*DEBUG*/printf("have found TUMBLING ',' window.trigger\n");
			(yyval.n) = MakeWindowTumbingCoord(EMPTY_TQ, (yyvsp[(3) - (4)].n), (yyvsp[(2) - (4)].tok)); 
		}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 775 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 778 "ANSI-C.y"
    { if ((yyvsp[(2) - (3)].n)->typ == Comma) (yyvsp[(2) - (3)].n)->coord = (yyvsp[(1) - (3)].tok);
                                  (yyvsp[(2) - (3)].n)->parenthesized = TRUE;
                                  (yyval.n) = (yyvsp[(2) - (3)].n); }
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 783 "ANSI-C.y"
    { if (ANSIOnly)
	         SyntaxError("statement expressions not allowed with -ansi switch");
               (yyval.n) = MakeBlockCoord(NULL, NULL, GrabPragmas((yyvsp[(3) - (5)].L)), (yyvsp[(1) - (5)].tok), (yyvsp[(4) - (5)].tok)); }
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 787 "ANSI-C.y"
    { if (ANSIOnly)
	         SyntaxError("statement expressions not allowed with -ansi switch");
              (yyval.n) = MakeBlockCoord(NULL, (yyvsp[(3) - (6)].L), GrabPragmas((yyvsp[(4) - (6)].L)), (yyvsp[(1) - (6)].tok), (yyvsp[(5) - (6)].tok)); }
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 795 "ANSI-C.y"
    { (yyval.n) = ExtendArray((yyvsp[(1) - (4)].n), (yyvsp[(3) - (4)].n), (yyvsp[(2) - (4)].tok)); }
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 797 "ANSI-C.y"
    { (yyval.n) = MakeCallCoord((yyvsp[(1) - (3)].n), NULL, (yyvsp[(2) - (3)].tok)); }
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 799 "ANSI-C.y"
    { (yyval.n) = MakeCallCoord((yyvsp[(1) - (4)].n), (yyvsp[(3) - (4)].L), (yyvsp[(2) - (4)].tok)); }
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 802 "ANSI-C.y"
    {
			(yyvsp[(1) - (4)].n) = ModifyDeclType(ConvertIdToDecl((yyvsp[(1) - (4)].n), EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ,NULL, NULL, NULL, (yyvsp[(1) - (4)].n)->coord) );
			(yyvsp[(1) - (4)].n) = SetDeclType((yyvsp[(1) - (4)].n), MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (4)].n)->coord), Redecl) ;
			(yyvsp[(1) - (4)].n) = DefineOperator((yyvsp[(1) - (4)].n));
			(yyval.n) = SetOperatorBody((yyvsp[(1) - (4)].n), (yyvsp[(4) - (4)].n));
		}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 809 "ANSI-C.y"
    {
			(yyvsp[(1) - (5)].n) = ModifyDeclType(ConvertIdToDecl((yyvsp[(1) - (5)].n), EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ,NULL, (yyvsp[(3) - (5)].L), NULL, (yyvsp[(1) - (5)].n)->coord) );
			(yyvsp[(1) - (5)].n) = SetDeclType((yyvsp[(1) - (5)].n), MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (5)].n)->coord), Redecl) ;
			(yyvsp[(1) - (5)].n) = DefineOperator((yyvsp[(1) - (5)].n));
			(yyval.n) = SetOperatorBody((yyvsp[(1) - (5)].n), (yyvsp[(5) - (5)].n));
		}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 816 "ANSI-C.y"
    {
			(yyvsp[(1) - (4)].n) = ModifyDeclType(ConvertIdToDecl((yyvsp[(1) - (4)].n), EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ, NULL, NULL, NULL, (yyvsp[(1) - (4)].n)->coord));

			///*DEBUG*/printf("have found composite template call\n");
			(yyval.n) = MakeCompositeCallCoord(MakeCompositeIdCoord((yyvsp[(1) - (4)].n)->u.decl.name, (yyvsp[(1) - (4)].n)->coord), ModifyOperatorDeclArguments((yyvsp[(1) - (4)].n)->u.decl.type, (yyvsp[(4) - (4)].L)), TRUE, (yyvsp[(1) - (4)].n)->coord);
		}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 823 "ANSI-C.y"
    {
			(yyvsp[(1) - (5)].n) = ModifyDeclType(ConvertIdToDecl((yyvsp[(1) - (5)].n), EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ, NULL, (yyvsp[(3) - (5)].L), NULL, (yyvsp[(1) - (5)].n)->coord));

			///*DEBUG*/printf("have found composite template call\n");
			(yyval.n) = MakeCompositeCallCoord(MakeCompositeIdCoord((yyvsp[(1) - (5)].n)->u.decl.name, (yyvsp[(1) - (5)].n)->coord), ModifyOperatorDeclArguments((yyvsp[(1) - (5)].n)->u.decl.type, (yyvsp[(5) - (5)].L)), TRUE, (yyvsp[(1) - (5)].n)->coord);
		}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 830 "ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (9)].n)),NULL,NULL,(yyvsp[(6) - (9)].n),GrabPragmas((yyvsp[(7) - (9)].L)),(yyvsp[(8) - (9)].n),(yyvsp[(1) - (9)].tok));
		}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 834 "ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (10)].n)),(yyvsp[(6) - (10)].L),NULL,(yyvsp[(7) - (10)].n),GrabPragmas((yyvsp[(8) - (10)].L)),(yyvsp[(9) - (10)].n),(yyvsp[(1) - (10)].tok));
		}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 838 "ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (11)].n)),(yyvsp[(6) - (11)].L),(yyvsp[(7) - (11)].L),(yyvsp[(8) - (11)].n),GrabPragmas((yyvsp[(9) - (11)].L)),(yyvsp[(10) - (11)].n),(yyvsp[(1) - (11)].tok));
		}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 842 "ANSI-C.y"
    {
			(yyval.n) = MakePipelineCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (7)].n)), NULL,(yyvsp[(6) - (7)].L), (yyvsp[(1) - (7)].tok));
		}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 846 "ANSI-C.y"
    {
			(yyval.n) = MakePipelineCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (8)].n)), (yyvsp[(6) - (8)].L),(yyvsp[(7) - (8)].L), (yyvsp[(1) - (8)].tok));
		}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 850 "ANSI-C.y"
    {
			(yyval.n) = MakeFileReaderOperator(NULL,(yyvsp[(4) - (4)].L),(yyvsp[(1) - (4)].tok));
		}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 856 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('.', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 858 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ARROW, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 860 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(POSTINC, (yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tok)); }
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 862 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(POSTDEC, (yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tok)); }
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 866 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('.', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 868 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ARROW, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 873 "ANSI-C.y"
    { (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); }
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 875 "ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n)); }
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 880 "ANSI-C.y"
    { (yyval.n) = LookupPostfixExpression((yyvsp[(1) - (1)].n)); }
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 882 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(PREINC, (yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok)); }
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 884 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(PREDEC, (yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok)); }
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 886 "ANSI-C.y"
    { (yyvsp[(1) - (2)].n)->u.unary.expr = (yyvsp[(2) - (2)].n);
              (yyval.n) = (yyvsp[(1) - (2)].n); }
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 889 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(SIZEOF, (yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok)); }
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 891 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(SIZEOF, (yyvsp[(3) - (4)].n), (yyvsp[(1) - (4)].tok)); }
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 895 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('&', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 896 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('*', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 897 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('+', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 898 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('-', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 899 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('~', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 900 "ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('!', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 905 "ANSI-C.y"
    { (yyval.n) = MakeCastCoord((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n), (yyvsp[(1) - (4)].tok)); }
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 911 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('*', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 913 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('/', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 915 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('%', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 921 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('+', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 923 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('-', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 929 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(LS, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 931 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(RS, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 937 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('<', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 939 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('>', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 941 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(LE, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 943 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(GE, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 949 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(EQ, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 951 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(NE, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 957 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('&', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 963 "ANSI-C.y"
    { 
              WarnAboutPrecedence('^', (yyvsp[(1) - (3)].n));
              WarnAboutPrecedence('^', (yyvsp[(3) - (3)].n));
	      (yyval.n) = MakeBinopCoord('^', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 972 "ANSI-C.y"
    { WarnAboutPrecedence('|', (yyvsp[(1) - (3)].n));
              WarnAboutPrecedence('|', (yyvsp[(3) - (3)].n));
              (yyval.n) = MakeBinopCoord('|', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 980 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ANDAND, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 986 "ANSI-C.y"
    { WarnAboutPrecedence(OROR, (yyvsp[(1) - (3)].n));
              WarnAboutPrecedence(OROR, (yyvsp[(3) - (3)].n));
              (yyval.n) = MakeBinopCoord(OROR, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 994 "ANSI-C.y"
    { (yyval.n) = MakeTernaryCoord((yyvsp[(1) - (5)].n), (yyvsp[(3) - (5)].n), (yyvsp[(5) - (5)].n), (yyvsp[(2) - (5)].tok), (yyvsp[(4) - (5)].tok)); }
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1000 "ANSI-C.y"
    { 
				if((yyvsp[(2) - (3)].n)->u.binop.op == '='){
					if((yyvsp[(3) - (3)].n)->typ == Operator_ || (yyvsp[(3) - (3)].n)->typ == CompositeCall || (yyvsp[(3) - (3)].n)->typ == Pipeline || (yyvsp[(3) - (3)].n)->typ == SplitJoin){
						(yyval.n) = MakeOutput((yyvsp[(3) - (3)].n),(yyvsp[(1) - (3)].n));
					}else{
						(yyvsp[(2) - (3)].n)->u.binop.left = (yyvsp[(1) - (3)].n);
						(yyvsp[(2) - (3)].n)->u.binop.right = (yyvsp[(3) - (3)].n);
						(yyval.n) = (yyvsp[(2) - (3)].n);
					}
				}else{
					(yyvsp[(2) - (3)].n)->u.binop.left = (yyvsp[(1) - (3)].n);
					(yyvsp[(2) - (3)].n)->u.binop.right = (yyvsp[(3) - (3)].n);
					(yyval.n) = (yyvsp[(2) - (3)].n);
				 }
		  }
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1018 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('=', NULL, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1019 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(MULTassign, NULL, NULL, (yyvsp[(1) - (1)].tok));  }
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1020 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(DIVassign, NULL, NULL, (yyvsp[(1) - (1)].tok));   }
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1021 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(MODassign, NULL, NULL, (yyvsp[(1) - (1)].tok));   }
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1022 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(PLUSassign, NULL, NULL, (yyvsp[(1) - (1)].tok));  }
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1023 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(MINUSassign, NULL, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1024 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(LSassign, NULL, NULL, (yyvsp[(1) - (1)].tok));    }
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1025 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(RSassign, NULL, NULL, (yyvsp[(1) - (1)].tok));    }
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1026 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ANDassign, NULL, NULL, (yyvsp[(1) - (1)].tok));   }
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1027 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ERassign, NULL, NULL, (yyvsp[(1) - (1)].tok));    }
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1028 "ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ORassign, NULL, NULL, (yyvsp[(1) - (1)].tok));    }
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1034 "ANSI-C.y"
    {  
              if ((yyvsp[(1) - (3)].n)->typ == Comma) 
              {
				 AppendItem((yyvsp[(1) - (3)].n)->u.comma.exprs, (yyvsp[(3) - (3)].n));
				 (yyval.n) = (yyvsp[(1) - (3)].n);
			  }
              else
             {
				  (yyval.n) = MakeCommaCoord(AppendItem(MakeNewList((yyvsp[(1) - (3)].n)), (yyvsp[(3) - (3)].n)), (yyvsp[(1) - (3)].n)->coord);
			 }
	    }
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1048 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1052 "ANSI-C.y"
    { (yyval.n) = (Node *) NULL; }
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1053 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1090 "ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (2)].L); }
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1092 "ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (2)].L); }
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1094 "ANSI-C.y"
    { (yyval.L) = MakeNewList(ForceNewSU((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tok))); }
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1096 "ANSI-C.y"
    { (yyval.L) = MakeNewList(ForceNewSU((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tok))); }
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1102 "ANSI-C.y"
    { 
	      SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);
	    }
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1105 "ANSI-C.y"
    { SetDeclAttribs((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].L)); }
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1107 "ANSI-C.y"
    { 
              (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (6)].n), (yyvsp[(6) - (6)].n))); 
            }
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1111 "ANSI-C.y"
    { 
              SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);
            }
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1114 "ANSI-C.y"
    { SetDeclAttribs((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].L)); }
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1116 "ANSI-C.y"
    { 
              (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (6)].n), (yyvsp[(6) - (6)].n))); 
			}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1120 "ANSI-C.y"
    { 
	      (yyval.L) = AppendDecl((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n), Redecl);
			 }
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1123 "ANSI-C.y"
    { SetDeclAttribs((yyvsp[(3) - (5)].n), (yyvsp[(5) - (5)].L)); }
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1125 "ANSI-C.y"
    { 
              SetDeclInit((yyvsp[(3) - (7)].n), (yyvsp[(7) - (7)].n)); 
            }
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1133 "ANSI-C.y"
    { 
              SyntaxError("declaration without a variable"); 
            }
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1138 "ANSI-C.y"
    { 
              (yyval.L) = NULL; /* empty list */ 
            }
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1143 "ANSI-C.y"
    { 
              SyntaxError("declaration without a variable"); 
            }
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1148 "ANSI-C.y"
    { 
              (yyval.L) = NULL; /* empty list */ 
            }
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1158 "ANSI-C.y"
    { 
              SetDeclType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord), NoRedecl);
            }
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1161 "ANSI-C.y"
    { SetDeclAttribs((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].L)); }
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1163 "ANSI-C.y"
    { 
              (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (6)].n), (yyvsp[(6) - (6)].n))); 
            }
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1167 "ANSI-C.y"
    {
			SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);
            (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (2)].n), NULL)); 
		}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1172 "ANSI-C.y"
    { 
              SetDeclType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord), NoRedecl);
            }
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1175 "ANSI-C.y"
    { SetDeclAttribs((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].L)); }
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1177 "ANSI-C.y"
    { 
              (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (6)].n), (yyvsp[(6) - (6)].n))); 
	    }
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1181 "ANSI-C.y"
    { (yyval.L) = AppendDecl((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n), NoRedecl); }
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1182 "ANSI-C.y"
    { SetDeclAttribs((yyvsp[(3) - (5)].n), (yyvsp[(5) - (5)].L)); }
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1184 "ANSI-C.y"
    { SetDeclInit((yyvsp[(3) - (7)].n), (yyvsp[(7) - (7)].n)); }
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1189 "ANSI-C.y"
    { 
              SyntaxError("declaration without a variable"); 
	    }
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1194 "ANSI-C.y"
    { 
              (yyval.L) = NULL; /* empty list */ 
	    }
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1199 "ANSI-C.y"
    { 
              SyntaxError("declaration without a variable"); 
	    }
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1204 "ANSI-C.y"
    { 
              (yyval.L) = NULL; /* empty list */ 
            }
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1213 "ANSI-C.y"
    { (yyval.n) = FinishPrimType((yyvsp[(1) - (1)].n)); }
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1222 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1224 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1226 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1228 "ANSI-C.y"
    { (yyval.n) = MergePrimTypes((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n)); }
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1235 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1237 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1239 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1246 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1248 "ANSI-C.y"
    { (yyval.n) = ConvertIdToTdef((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq, GetTypedefType((yyvsp[(2) - (2)].n))); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1250 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1258 "ANSI-C.y"
    { (yyval.tq).tq = MergeTypeQuals((yyvsp[(1) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).coord);
              (yyval.tq).coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1261 "ANSI-C.y"
    { (yyval.tq).tq = MergeTypeQuals((yyvsp[(1) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).coord);
              (yyval.tq).coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1274 "ANSI-C.y"
    { (yyval.n) = FinishPrimType((yyvsp[(1) - (1)].n)); }
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1283 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1285 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1287 "ANSI-C.y"
    { (yyval.n) = MergePrimTypes((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n)); }
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1294 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1296 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1303 "ANSI-C.y"
    { (yyval.n) = ConvertIdToTdef((yyvsp[(1) - (1)].n), EMPTY_TQ, GetTypedefType((yyvsp[(1) - (1)].n))); }
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1305 "ANSI-C.y"
    { (yyval.n) = ConvertIdToTdef((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq, GetTypedefType((yyvsp[(2) - (2)].n))); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1307 "ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1314 "ANSI-C.y"
    { (yyval.tq).tq = MergeTypeQuals((yyvsp[(1) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).coord);
              (yyval.tq).coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1321 "ANSI-C.y"
    { (yyval.tq).tq = MergeTypeQuals((yyvsp[(1) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).coord);
              (yyval.tq).coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1337 "ANSI-C.y"
    {
	      Warning(2, "function prototype parameters must have types");
              (yyval.n) = AddDefaultParameterTypes((yyvsp[(1) - (1)].n));
            }
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1348 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok)), Redecl);
               }
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1351 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(3) - (4)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (4)].tok)), Redecl); 
               }
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1354 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(4) - (5)].n), MakePtrCoord(   (yyvsp[(2) - (5)].tq).tq,    NULL, (yyvsp[(1) - (5)].tok)), Redecl);
               }
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1357 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(3) - (3)].n), MakePtrCoord(   (yyvsp[(2) - (3)].tq).tq,    NULL, (yyvsp[(1) - (3)].tok)), Redecl); 
               }
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1365 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n);  
              }
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1368 "ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(3) - (4)].n)); 
               }
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1371 "ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1378 "ANSI-C.y"
    { (yyval.n) = ConvertIdToDecl((yyvsp[(1) - (1)].n), EMPTY_TQ, NULL, NULL, NULL); }
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1380 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n);  
               }
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1387 "ANSI-C.y"
    { (yyval.n) = ConvertIdToDecl((yyvsp[(1) - (1)].n), EMPTY_TQ, NULL, NULL, NULL); }
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1389 "ANSI-C.y"
    { (yyval.n) = ConvertIdToDecl((yyvsp[(1) - (2)].n), EMPTY_TQ, (yyvsp[(2) - (2)].n), NULL, NULL);   }
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1401 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok)), Redecl); 
               }
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1404 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(3) - (3)].n), MakePtrCoord((yyvsp[(2) - (3)].tq).tq, NULL, (yyvsp[(1) - (3)].tok)), Redecl); 
               }
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1411 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1414 "ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1428 "ANSI-C.y"
    { (yyval.n) = MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1430 "ANSI-C.y"
    { (yyval.n) = MakePtrCoord((yyvsp[(2) - (2)].tq).tq, NULL, (yyvsp[(1) - (2)].tok)); }
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1432 "ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok))); 
               }
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1435 "ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(3) - (3)].n), MakePtrCoord((yyvsp[(2) - (3)].tq).tq, NULL, (yyvsp[(1) - (3)].tok))); 
               }
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1442 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1445 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1448 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1451 "ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1457 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n);                   }
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1458 "ANSI-C.y"
    { (yyval.n) = MakeFdclCoord(EMPTY_TQ, NULL, NULL, (yyvsp[(1) - (2)].tok)); }
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1459 "ANSI-C.y"
    { (yyval.n) = MakeFdclCoord(EMPTY_TQ, (yyvsp[(2) - (3)].L), NULL, (yyvsp[(1) - (3)].tok)); }
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1472 "ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok))); 
               }
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1475 "ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(3) - (3)].n), MakePtrCoord(   (yyvsp[(2) - (3)].tq).tq,    NULL, (yyvsp[(1) - (3)].tok))); 
               }
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1482 "ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n)); }
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1484 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1487 "ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1494 "ANSI-C.y"
    { (yyval.n) = ConvertIdToDecl((yyvsp[(1) - (1)].n), EMPTY_TQ, NULL, NULL, NULL); }
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1496 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1503 "ANSI-C.y"
    { 
/*              OldStyleFunctionDefinition = TRUE; */
              (yyval.n) = (yyvsp[(1) - (1)].n); 
            }
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1508 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok)), SU); 
               }
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1511 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(3) - (3)].n), MakePtrCoord((yyvsp[(2) - (3)].tq).tq, NULL, (yyvsp[(1) - (3)].tok)), SU); 
               }
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1518 "ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(1) - (4)].n), MakeFdclCoord(EMPTY_TQ, (yyvsp[(3) - (4)].L), NULL, (yyvsp[(2) - (4)].tok))); }
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1520 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1523 "ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1538 "ANSI-C.y"
    { (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); }
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1540 "ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n)); }
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1552 "ANSI-C.y"
    { (yyval.n) = FinishType((yyvsp[(1) - (1)].n)); }
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1554 "ANSI-C.y"
    { (yyval.n) = FinishType(SetBaseType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n))); }
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1556 "ANSI-C.y"
    { (yyval.n) = MakeDefaultPrimType((yyvsp[(1) - (1)].tq).tq, (yyvsp[(1) - (1)].tq).coord); }
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1558 "ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord)); }
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1566 "ANSI-C.y"
    { (yyval.L) = NULL; }
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1568 "ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (1)].L); }
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1573 "ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (1)].L); }
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1575 "ANSI-C.y"
    { (yyval.L) = JoinLists ((yyvsp[(1) - (2)].L), (yyvsp[(2) - (2)].L)); }
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1580 "ANSI-C.y"
    { if (ANSIOnly)
	            SyntaxError("__attribute__ not allowed with -ansi switch");
                  (yyval.L) = (yyvsp[(4) - (6)].L); }
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1587 "ANSI-C.y"
    { (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); }
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1589 "ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n)); }
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1594 "ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1596 "ANSI-C.y"
    { (yyval.n) = ConvertIdToAttrib((yyvsp[(1) - (1)].n), NULL); }
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1598 "ANSI-C.y"
    { (yyval.n) = ConvertIdToAttrib((yyvsp[(1) - (4)].n), (yyvsp[(3) - (4)].n)); }
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 1605 "ANSI-C.y"
    { (yyval.n) = MakeIdCoord(UniqueString("const"), (yyvsp[(1) - (1)].tok)); }
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1610 "ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1611 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (2)].n); }
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1616 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); (yyval.n)->coord = (yyvsp[(1) - (3)].tok); }
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1617 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (4)].n); (yyval.n)->coord = (yyvsp[(1) - (4)].tok); }
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1618 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n);}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1624 "ANSI-C.y"
    { (yyval.n) = MakeInitializerCoord(MakeNewList((yyvsp[(1) - (1)].n)), (yyvsp[(1) - (1)].n)->coord);}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1626 "ANSI-C.y"
    { 
              assert((yyvsp[(1) - (3)].n)->typ == Initializer);
			  AppendItem((yyvsp[(1) - (3)].n)->u.initializer.exprs, (yyvsp[(3) - (3)].n));
              (yyval.n) = (yyvsp[(1) - (3)].n); 
            }
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1636 "ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), EllipsisNode); }
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1640 "ANSI-C.y"
    { Node *n = MakePrimCoord(EMPTY_TQ, Void, (yyvsp[(1) - (1)].tok));
	      SyntaxErrorCoord(n->coord, "First argument cannot be `...'");
              (yyval.L) = MakeNewList(n);
            }
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1649 "ANSI-C.y"
    { (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); }
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1651 "ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n)); }
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1655 "ANSI-C.y"
    { 
	      SyntaxErrorCoord((yyvsp[(1) - (3)].n)->coord, "formals cannot have initializers");
              (yyval.L) = MakeNewList((yyvsp[(1) - (3)].n)); 
            }
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1660 "ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (3)].L); }
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1666 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1668 "ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n)); 
            }
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1671 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Formal); 
            }
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1674 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Formal); 
            }
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1677 "ANSI-C.y"
    { (yyval.n) = MakeDefaultPrimType((yyvsp[(1) - (1)].tq).tq, (yyvsp[(1) - (1)].tq).coord); }
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1679 "ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord)); }
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1681 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord), Formal); }
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1683 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1685 "ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n)); 
            }
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1688 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Formal); 
            }
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1691 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Formal); 
            }
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1694 "ANSI-C.y"
    { (yyval.n) = MakeDefaultPrimType((yyvsp[(1) - (1)].tq).tq, (yyvsp[(1) - (1)].tq).coord); }
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1696 "ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord)); }
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1698 "ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord), Formal); }
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1704 "ANSI-C.y"
    { (yyval.n) = MakeAdclCoord(EMPTY_TQ, NULL, NULL, (yyvsp[(1) - (2)].tok)); }
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1706 "ANSI-C.y"
    { (yyval.n) = MakeAdclCoord(EMPTY_TQ, NULL, (yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].tok)); }
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1708 "ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(1) - (4)].n), MakeAdclCoord(EMPTY_TQ, NULL, (yyvsp[(3) - (4)].n), (yyvsp[(2) - (4)].tok))); }
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1713 "ANSI-C.y"
    { 
              SyntaxError("array declaration with illegal empty dimension");
              (yyval.n) = SetBaseType((yyvsp[(1) - (3)].n), MakeAdclCoord(EMPTY_TQ, NULL, SintOne, (yyvsp[(2) - (3)].tok))); 
            }
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1729 "ANSI-C.y"
    { 
              (yyval.n) = SetSUdclNameFields((yyvsp[(1) - (4)].n), NULL, (yyvsp[(3) - (4)].L), (yyvsp[(2) - (4)].tok), (yyvsp[(4) - (4)].tok));
            }
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1734 "ANSI-C.y"
    { 
              (yyval.n) = SetSUdclNameFields((yyvsp[(1) - (5)].n), (yyvsp[(2) - (5)].n), (yyvsp[(4) - (5)].L), (yyvsp[(3) - (5)].tok), (yyvsp[(5) - (5)].tok));
	    }
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1738 "ANSI-C.y"
    { 
              (yyval.n) = SetSUdclName((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n)->coord);
	    }
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1743 "ANSI-C.y"
    { 
              if (ANSIOnly)
                 Warning(1, "empty structure declaration");
              (yyval.n) = SetSUdclNameFields((yyvsp[(1) - (3)].n), NULL, NULL, (yyvsp[(2) - (3)].tok), (yyvsp[(3) - (3)].tok)); 
	    }
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1749 "ANSI-C.y"
    { 
              if (ANSIOnly)
                 Warning(1, "empty structure declaration");
              (yyval.n) = SetSUdclNameFields((yyvsp[(1) - (4)].n), (yyvsp[(2) - (4)].n), NULL, (yyvsp[(3) - (4)].tok), (yyvsp[(4) - (4)].tok)); 
	    }
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1758 "ANSI-C.y"
    { (yyval.n) = MakeSdclCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1759 "ANSI-C.y"
    { (yyval.n) = MakeUdclCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1766 "ANSI-C.y"
    { (yyval.L) = JoinLists((yyvsp[(1) - (2)].L), (yyvsp[(2) - (2)].L)); }
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1779 "ANSI-C.y"
    { 
	      (yyval.L) = MakeNewList(SetDeclType((yyvsp[(2) - (2)].n),
					    MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
					    SU)); 
	    }
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1785 "ANSI-C.y"
    { (yyval.L) = AppendDecl((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n), SU); }
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1791 "ANSI-C.y"
    { (yyval.L) = MakeNewList(SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), SU)); }
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1793 "ANSI-C.y"
    { (yyval.L) = AppendDecl((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n), SU); }
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1800 "ANSI-C.y"
    { SetDeclAttribs((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].L));
              (yyval.n) = SetDeclBitSize((yyvsp[(1) - (3)].n), (yyvsp[(2) - (3)].n)); }
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1803 "ANSI-C.y"
    { (yyval.n) = MakeDeclCoord(NULL, EMPTY_TQ, NULL, NULL, (yyvsp[(1) - (2)].n), (yyvsp[(1) - (2)].n)->coord);
              SetDeclAttribs((yyval.n), (yyvsp[(2) - (2)].L)); }
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1810 "ANSI-C.y"
    { (yyval.n) = SetDeclBitSize((yyvsp[(1) - (3)].n), (yyvsp[(2) - (3)].n));
              SetDeclAttribs((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].L)); }
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1813 "ANSI-C.y"
    { (yyval.n) = MakeDeclCoord(NULL, EMPTY_TQ, NULL, NULL, (yyvsp[(1) - (2)].n), (yyvsp[(1) - (2)].n)->coord);
              SetDeclAttribs((yyval.n), (yyvsp[(2) - (2)].L)); }
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1819 "ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1825 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (2)].n); }
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1831 "ANSI-C.y"
    { (yyval.n) = BuildEnum(NULL, (yyvsp[(3) - (5)].L), (yyvsp[(1) - (5)].tok), (yyvsp[(2) - (5)].tok), (yyvsp[(5) - (5)].tok)); }
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1833 "ANSI-C.y"
    { (yyval.n) = BuildEnum((yyvsp[(2) - (6)].n), (yyvsp[(4) - (6)].L), (yyvsp[(1) - (6)].tok), (yyvsp[(3) - (6)].tok), (yyvsp[(6) - (6)].tok));   }
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1835 "ANSI-C.y"
    { (yyval.n) = BuildEnum((yyvsp[(2) - (2)].n), NULL, (yyvsp[(1) - (2)].tok), (yyvsp[(2) - (2)].n)->coord, (yyvsp[(2) - (2)].n)->coord); }
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1841 "ANSI-C.y"
    { (yyval.L) = MakeNewList(BuildEnumConst((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n))); }
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1843 "ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (4)].L), BuildEnumConst((yyvsp[(3) - (4)].n), (yyvsp[(4) - (4)].n))); }
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1848 "ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1849 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (2)].n);   }
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1853 "ANSI-C.y"
    { }
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1854 "ANSI-C.y"
    { }
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1875 "ANSI-C.y"
    {  (yyval.n) = NULL; }
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1880 "ANSI-C.y"
    { (yyval.L) = BuildLabel((yyvsp[(1) - (2)].n), NULL); }
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1882 "ANSI-C.y"
    { (yyval.n)->u.label.stmt = (yyvsp[(4) - (4)].n); }
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1885 "ANSI-C.y"
    { (yyval.n) = AddContainee(MakeCaseCoord((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n), NULL, (yyvsp[(1) - (4)].tok))); }
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1887 "ANSI-C.y"
    { (yyval.n) = AddContainee(MakeDefaultCoord((yyvsp[(3) - (3)].n), NULL, (yyvsp[(1) - (3)].tok))); }
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1891 "ANSI-C.y"
    { (yyval.n) = BuildLabel((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n)); }
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1896 "ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, NULL, NULL, (yyvsp[(1) - (2)].tok), (yyvsp[(2) - (2)].tok)); }
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1898 "ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, GrabPragmas((yyvsp[(2) - (3)].L)), NULL, (yyvsp[(1) - (3)].tok), (yyvsp[(3) - (3)].tok)); }
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1900 "ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, NULL, GrabPragmas((yyvsp[(2) - (3)].L)), (yyvsp[(1) - (3)].tok), (yyvsp[(3) - (3)].tok)); }
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1902 "ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, (yyvsp[(2) - (4)].L), GrabPragmas((yyvsp[(3) - (4)].L)), (yyvsp[(1) - (4)].tok), (yyvsp[(4) - (4)].tok)); }
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1905 "ANSI-C.y"
    { EnterScope(); }
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1906 "ANSI-C.y"
    { ExitScope(); }
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1913 "ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, NULL, NULL, (yyvsp[(1) - (2)].tok), (yyvsp[(2) - (2)].tok)); }
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1915 "ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, GrabPragmas((yyvsp[(2) - (3)].L)), NULL, (yyvsp[(1) - (3)].tok), (yyvsp[(3) - (3)].tok)); }
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1917 "ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, NULL, GrabPragmas((yyvsp[(2) - (3)].L)), (yyvsp[(1) - (3)].tok), (yyvsp[(3) - (3)].tok)); }
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1919 "ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, (yyvsp[(2) - (4)].L), GrabPragmas((yyvsp[(3) - (4)].L)), (yyvsp[(1) - (4)].tok), (yyvsp[(4) - (4)].tok)); }
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1925 "ANSI-C.y"
    { (yyval.L) = GrabPragmas((yyvsp[(1) - (1)].L)); }
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1926 "ANSI-C.y"
    { (yyval.L) = JoinLists(GrabPragmas((yyvsp[(1) - (2)].L)),
                                                         (yyvsp[(2) - (2)].L)); }
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1931 "ANSI-C.y"
    { (yyval.L) = GrabPragmas(MakeNewList((yyvsp[(1) - (1)].n))); }
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1932 "ANSI-C.y"
    { (yyval.L) = AppendItem(GrabPragmas((yyvsp[(1) - (2)].L)), 
                                                        (yyvsp[(2) - (2)].n)); }
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1942 "ANSI-C.y"
    { (yyval.n) = MakeIfCoord((yyvsp[(3) - (5)].n), (yyvsp[(5) - (5)].n), (yyvsp[(1) - (5)].tok)); }
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1944 "ANSI-C.y"
    { (yyval.n) = MakeIfElseCoord((yyvsp[(3) - (7)].n), (yyvsp[(5) - (7)].n), (yyvsp[(7) - (7)].n), (yyvsp[(1) - (7)].tok), (yyvsp[(6) - (7)].tok)); }
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1945 "ANSI-C.y"
    { PushContainer(Switch); }
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1946 "ANSI-C.y"
    { (yyval.n) = PopContainer(MakeSwitchCoord((yyvsp[(4) - (6)].n), (yyvsp[(6) - (6)].n), NULL, (yyvsp[(1) - (6)].tok))); }
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1951 "ANSI-C.y"
    { PushContainer(While);}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1953 "ANSI-C.y"
    { (yyval.n) = PopContainer(MakeWhileCoord((yyvsp[(4) - (6)].n), (yyvsp[(6) - (6)].n), (yyvsp[(1) - (6)].tok))); }
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1955 "ANSI-C.y"
    { PushContainer(Do);}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1957 "ANSI-C.y"
    { (yyval.n) = PopContainer(MakeDoCoord((yyvsp[(3) - (8)].n), (yyvsp[(6) - (8)].n), (yyvsp[(1) - (8)].tok), (yyvsp[(4) - (8)].tok))); }
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1959 "ANSI-C.y"
    { PushContainer(For);}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1961 "ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord((yyvsp[(3) - (10)].n), (yyvsp[(5) - (10)].n), (yyvsp[(7) - (10)].n), (yyvsp[(10) - (10)].n), (yyvsp[(1) - (10)].tok))); }
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1965 "ANSI-C.y"
    { PushContainer(For);}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1967 "ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord(NULL, (yyvsp[(5) - (10)].n), (yyvsp[(7) - (10)].n), (yyvsp[(10) - (10)].n), (yyvsp[(1) - (10)].tok))); }
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1969 "ANSI-C.y"
    { PushContainer(For);}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1971 "ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord((yyvsp[(3) - (10)].n), (yyvsp[(5) - (10)].n), NULL, (yyvsp[(10) - (10)].n), (yyvsp[(1) - (10)].tok))); }
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1973 "ANSI-C.y"
    { PushContainer(For);}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1975 "ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord((yyvsp[(3) - (10)].n), NULL, (yyvsp[(7) - (10)].n), (yyvsp[(10) - (10)].n), (yyvsp[(1) - (10)].tok))); }
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1976 "ANSI-C.y"
    { PushContainer(For);}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1977 "ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord(NULL, SintZero, NULL, (yyvsp[(6) - (6)].n), (yyvsp[(1) - (6)].tok))); }
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1982 "ANSI-C.y"
    { (yyval.n) = ResolveGoto((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].tok)); }
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1984 "ANSI-C.y"
    { (yyval.n) = AddContainee(MakeContinueCoord(NULL, (yyvsp[(1) - (2)].tok))); }
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1986 "ANSI-C.y"
    { (yyval.n) = AddContainee(MakeBreakCoord(NULL, (yyvsp[(1) - (2)].tok))); }
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1988 "ANSI-C.y"
    { (yyval.n) = AddReturn(MakeReturnCoord((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].tok))); }
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1992 "ANSI-C.y"
    { (yyval.n) = ResolveGoto((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].tok)); }
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 2004 "ANSI-C.y"
    { (yyval.L) = JoinLists(GrabPragmas((yyvsp[(1) - (2)].L)), (yyvsp[(2) - (2)].L)); }
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 2010 "ANSI-C.y"
    {
              if (yydebug)
                {
                  ///*DEBUG*/printf("external.definition # declaration\n");
                  PrintNode(stdout, FirstItem((yyvsp[(1) - (1)].L)), 0); 
                  ///*DEBUG*/printf("\n\n\n");
				}
              (yyval.L) = (yyvsp[(1) - (1)].L);
            }
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 2020 "ANSI-C.y"
    { 
              if (yydebug)
                {
                  ///*DEBUG*/printf("external.definition # function.definition\n");
                  PrintNode(stdout, (yyvsp[(1) - (1)].n), 0); 
                  ///*DEBUG*/printf("\n\n\n");
                }
              (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); 
            }
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 2030 "ANSI-C.y"
    {
			if (yydebug)
                {

                 // printf("external.definition # composite.definition\n");
                  PrintNode(stdout, (yyvsp[(1) - (1)].n), 0); 
                 // printf("\n\n\n");
                }
              (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); 
		}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 2044 "ANSI-C.y"
    { 
              (yyvsp[(1) - (1)].n) = DefineProc(FALSE, 
                              SetDeclType((yyvsp[(1) - (1)].n),
					  MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (1)].n)->coord),
					  Redecl));
            }
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 2051 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n)); }
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 2057 "ANSI-C.y"
    { (yyvsp[(2) - (2)].n) = DefineProc(FALSE, SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl)); }
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 2059 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 2061 "ANSI-C.y"
    { (yyvsp[(2) - (2)].n) = DefineProc(FALSE, SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl)); }
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 2063 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 2065 "ANSI-C.y"
    { 
              (yyvsp[(2) - (2)].n) = DefineProc(FALSE, 
	                      SetDeclType((yyvsp[(2) - (2)].n),
				 	  MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
				          Redecl));
            }
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 2072 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 2074 "ANSI-C.y"
    { 
              (yyvsp[(2) - (2)].n) = DefineProc(FALSE, 
                              SetDeclType((yyvsp[(2) - (2)].n),
					  MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
					  Redecl));
            }
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 2081 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 2083 "ANSI-C.y"
    { 
              (yyvsp[(1) - (1)].n) = DefineProc(TRUE, 
                              SetDeclType((yyvsp[(1) - (1)].n),
					  MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (1)].n)->coord),
					  Redecl));
            }
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 2090 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n)); }
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 2092 "ANSI-C.y"
    {  Node *decl = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);  

               AddParameterTypes(decl, NULL);
               (yyvsp[(2) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 2098 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 2100 "ANSI-C.y"
    { Node *decl = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);

              AddParameterTypes(decl, NULL);
              (yyvsp[(2) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 2106 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 2108 "ANSI-C.y"
    { Node *type = MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
                   *decl = SetDeclType((yyvsp[(2) - (2)].n), type, Redecl);

              AddParameterTypes(decl, NULL);
              (yyvsp[(2) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 2115 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 2117 "ANSI-C.y"
    { Node *type = MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
                   *decl = SetDeclType((yyvsp[(2) - (2)].n), type, Redecl);

              AddParameterTypes(decl, NULL);
              (yyvsp[(2) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 2124 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 2126 "ANSI-C.y"
    { Node *type = MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (2)].n)->coord),
                   *decl = SetDeclType((yyvsp[(1) - (2)].n), type, Redecl);

              AddParameterTypes(decl, (yyvsp[(2) - (2)].L));
              (yyvsp[(1) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 2133 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(1) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 2135 "ANSI-C.y"
    { Node *decl = SetDeclType((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].n), Redecl);

              AddParameterTypes(decl, (yyvsp[(3) - (3)].L));
              (yyvsp[(2) - (3)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 2141 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (5)].n), (yyvsp[(5) - (5)].n)); }
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 2143 "ANSI-C.y"
    { Node *decl = SetDeclType((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].n), Redecl);

              AddParameterTypes(decl, (yyvsp[(3) - (3)].L));
              (yyvsp[(2) - (3)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 2149 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (5)].n), (yyvsp[(5) - (5)].n)); }
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 2151 "ANSI-C.y"
    { Node *type = MakeDefaultPrimType((yyvsp[(1) - (3)].tq).tq, (yyvsp[(1) - (3)].tq).coord),
                   *decl = SetDeclType((yyvsp[(2) - (3)].n), type, Redecl);

              AddParameterTypes(decl, (yyvsp[(3) - (3)].L));
              (yyvsp[(2) - (3)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 2158 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (5)].n), (yyvsp[(5) - (5)].n)); }
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 2160 "ANSI-C.y"
    { Node *type = MakeDefaultPrimType((yyvsp[(1) - (3)].tq).tq, (yyvsp[(1) - (3)].tq).coord), 
                   *decl = SetDeclType((yyvsp[(2) - (3)].n), type, Redecl);
				       

              AddParameterTypes(decl, (yyvsp[(3) - (3)].L));
              (yyvsp[(2) - (3)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 2168 "ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (5)].n), (yyvsp[(5) - (5)].n)); }
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 2173 "ANSI-C.y"
    { OldStyleFunctionDefinition = TRUE; }
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 2175 "ANSI-C.y"
    { OldStyleFunctionDefinition = FALSE; 
               (yyval.L) = (yyvsp[(2) - (2)].L); }
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 2190 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 2191 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 2192 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 2193 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 2194 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 2199 "ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 2201 "ANSI-C.y"
    { const char *first_text  = (yyvsp[(1) - (2)].n)->u.Const.text;
              const char *second_text = (yyvsp[(2) - (2)].n)->u.Const.text;
              int   length = strlen(first_text) + strlen(second_text) + 1;
              char *buffer = HeapNewArray(char, length);
              char *new_text, *new_val;
	
              /* since text (which includes quotes and escape codes)
		 is always longer than value, it's safe to use buffer
		 to concat both */
              strcpy(buffer, NodeConstantStringValue((yyvsp[(1) - (2)].n)));
	      strcat(buffer, NodeConstantStringValue((yyvsp[(2) - (2)].n)));
              new_val = UniqueString(buffer);

              strcpy(buffer, first_text);
	      strcat(buffer, second_text);
              new_text = buffer;
              (yyval.n) = MakeStringTextCoord(new_text, new_val, (yyvsp[(1) - (2)].n)->coord);
	     }
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 2222 "ANSI-C.y"
    { (yyval.tq).tq = T_CONST;    (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 2223 "ANSI-C.y"
    { (yyval.tq).tq = T_VOLATILE; (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 2224 "ANSI-C.y"
    { (yyval.tq).tq = T_INLINE;   (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 2228 "ANSI-C.y"
    { (yyval.tq).tq = T_CONST;    (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 2229 "ANSI-C.y"
    { (yyval.tq).tq = T_VOLATILE; (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 2233 "ANSI-C.y"
    { (yyval.tq).tq = T_TYPEDEF;  (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 2234 "ANSI-C.y"
    { (yyval.tq).tq = T_EXTERN;   (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 2235 "ANSI-C.y"
    { (yyval.tq).tq = T_STATIC;   (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 2236 "ANSI-C.y"
    { (yyval.tq).tq = T_AUTO;     (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 2237 "ANSI-C.y"
    { (yyval.tq).tq = T_REGISTER; (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 2241 "ANSI-C.y"
    { (yyval.n) = StartPrimType(Void, (yyvsp[(1) - (1)].tok));    }
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2242 "ANSI-C.y"
    { (yyval.n) = StartPrimType(Char, (yyvsp[(1) - (1)].tok));     }
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2243 "ANSI-C.y"
    { (yyval.n) = StartPrimType(Int_ParseOnly, (yyvsp[(1) - (1)].tok));     }
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2244 "ANSI-C.y"
    { (yyval.n) = StartPrimType(Float, (yyvsp[(1) - (1)].tok));   }
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2245 "ANSI-C.y"
    { (yyval.n) = StartPrimType(Double, (yyvsp[(1) - (1)].tok));  }
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2247 "ANSI-C.y"
    { (yyval.n) = StartPrimType(Signed, (yyvsp[(1) - (1)].tok));  }
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2248 "ANSI-C.y"
    { (yyval.n) = StartPrimType(Unsigned, (yyvsp[(1) - (1)].tok));}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2250 "ANSI-C.y"
    { (yyval.n) = StartPrimType(Short, (yyvsp[(1) - (1)].tok));   }
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2251 "ANSI-C.y"
    { (yyval.n) = StartPrimType(Long, (yyvsp[(1) - (1)].tok));    }
    break;



/* Line 1455 of yacc.c  */
#line 6697 "y.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 2254 "ANSI-C.y"

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



