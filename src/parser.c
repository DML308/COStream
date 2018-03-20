
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
#line 1 "./src/ANSI-C.y"


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
     UNCERTAINTY = 309,
     UPLUS = 310,
     UMINUS = 311,
     INDIR = 312,
     ADDRESS = 313,
     POSTINC = 314,
     POSTDEC = 315,
     PREINC = 316,
     PREDEC = 317,
     BOGUS = 318,
     IDENTIFIER = 319,
     STRINGliteral = 320,
     FLOATINGconstant = 321,
     INTEGERconstant = 322,
     OCTALconstant = 323,
     HEXconstant = 324,
     WIDECHARconstant = 325,
     CHARACTERconstant = 326,
     TYPEDEFname = 327,
     ARROW = 328,
     ICR = 329,
     DECR = 330,
     LS = 331,
     RS = 332,
     LE = 333,
     GE = 334,
     EQ = 335,
     NE = 336,
     ANDAND = 337,
     OROR = 338,
     ELLIPSIS = 339,
     MULTassign = 340,
     DIVassign = 341,
     MODassign = 342,
     PLUSassign = 343,
     MINUSassign = 344,
     LSassign = 345,
     RSassign = 346,
     ANDassign = 347,
     ERassign = 348,
     ORassign = 349,
     INLINE = 350,
     ATTRIBUTE = 351
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
#define UNCERTAINTY 309
#define UPLUS 310
#define UMINUS 311
#define INDIR 312
#define ADDRESS 313
#define POSTINC 314
#define POSTDEC 315
#define PREINC 316
#define PREDEC 317
#define BOGUS 318
#define IDENTIFIER 319
#define STRINGliteral 320
#define FLOATINGconstant 321
#define INTEGERconstant 322
#define OCTALconstant 323
#define HEXconstant 324
#define WIDECHARconstant 325
#define CHARACTERconstant 326
#define TYPEDEFname 327
#define ARROW 328
#define ICR 329
#define DECR 330
#define LS 331
#define RS 332
#define LE 333
#define GE 334
#define EQ 335
#define NE 336
#define ANDAND 337
#define OROR 338
#define ELLIPSIS 339
#define MULTassign 340
#define DIVassign 341
#define MODassign 342
#define PLUSassign 343
#define MINUSassign 344
#define LSassign 345
#define RSassign 346
#define ANDassign 347
#define ERassign 348
#define ORassign 349
#define INLINE 350
#define ATTRIBUTE 351




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 211 "./src/ANSI-C.y"

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
#line 485 "y.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 497 "y.tab.c"

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
#define YYLAST   5063

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  121
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  181
/* YYNRULES -- Number of rules.  */
#define YYNRULES  476
/* YYNRULES -- Number of states.  */
#define YYNSTATES  840

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   351

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
       9,   108,    10,    18,     2,     2,     2,     2,     2,     2,
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
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120
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
     230,   234,   238,   243,   248,   252,   254,   256,   258,   262,
     268,   275,   277,   282,   286,   291,   296,   302,   307,   313,
     323,   334,   346,   354,   363,   368,   372,   376,   379,   382,
     386,   390,   392,   396,   398,   401,   404,   407,   410,   415,
     417,   419,   421,   423,   425,   427,   429,   434,   436,   440,
     444,   448,   450,   454,   458,   460,   464,   468,   470,   474,
     478,   482,   486,   488,   492,   496,   498,   502,   504,   508,
     510,   514,   516,   520,   522,   526,   528,   534,   536,   540,
     542,   544,   546,   548,   550,   552,   554,   556,   558,   560,
     562,   564,   568,   570,   571,   573,   576,   579,   582,   585,
     586,   587,   594,   595,   596,   603,   604,   605,   613,   614,
     620,   621,   627,   631,   632,   633,   640,   643,   644,   645,
     652,   653,   654,   662,   663,   669,   670,   676,   680,   682,
     684,   686,   689,   692,   695,   698,   701,   704,   707,   710,
     713,   716,   718,   721,   724,   726,   728,   730,   732,   734,
     736,   739,   742,   745,   747,   750,   753,   755,   758,   761,
     763,   766,   768,   771,   773,   775,   777,   779,   781,   783,
     785,   788,   793,   799,   803,   807,   812,   817,   819,   823,
     825,   828,   830,   832,   835,   839,   843,   848,   850,   852,
     854,   856,   859,   862,   866,   870,   874,   878,   883,   885,
     888,   892,   894,   896,   898,   901,   905,   908,   912,   917,
     919,   923,   925,   928,   932,   937,   941,   946,   948,   952,
     954,   956,   958,   961,   963,   966,   967,   969,   971,   974,
     981,   983,   987,   988,   990,   995,   997,   999,  1001,  1002,
    1005,  1009,  1014,  1016,  1018,  1022,  1024,  1028,  1030,  1032,
    1036,  1040,  1044,  1046,  1049,  1052,  1055,  1057,  1060,  1063,
    1065,  1068,  1071,  1074,  1076,  1079,  1082,  1085,  1089,  1094,
    1098,  1103,  1109,  1112,  1116,  1121,  1123,  1125,  1127,  1130,
    1133,  1136,  1139,  1143,  1146,  1150,  1154,  1157,  1161,  1164,
    1165,  1167,  1170,  1176,  1183,  1186,  1189,  1194,  1195,  1198,
    1199,  1201,  1203,  1205,  1207,  1209,  1211,  1213,  1215,  1217,
    1220,  1221,  1226,  1231,  1235,  1239,  1242,  1246,  1250,  1255,
    1257,  1259,  1262,  1266,  1270,  1275,  1277,  1280,  1282,  1285,
    1288,  1294,  1302,  1303,  1310,  1311,  1318,  1319,  1328,  1329,
    1340,  1341,  1352,  1353,  1364,  1365,  1376,  1377,  1384,  1388,
    1391,  1394,  1398,  1402,  1404,  1407,  1409,  1411,  1413,  1414,
    1418,  1421,  1422,  1427,  1428,  1433,  1434,  1439,  1440,  1445,
    1446,  1450,  1451,  1456,  1457,  1462,  1463,  1468,  1469,  1474,
    1475,  1480,  1481,  1487,  1488,  1494,  1495,  1501,  1502,  1508,
    1509,  1512,  1514,  1516,  1518,  1520,  1522,  1524,  1527,  1529,
    1531,  1533,  1535,  1537,  1539,  1541,  1543,  1545,  1547,  1549,
    1551,  1553,  1555,  1557,  1559,  1561,  1563
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     122,     0,    -1,   276,    -1,    61,     9,   124,    10,    -1,
     200,   221,    -1,   200,   221,   217,    -1,   124,    22,   200,
     221,    -1,   124,    22,   200,   221,   217,    -1,    -1,   127,
     126,   131,    -1,    58,    87,    15,   128,    16,    -1,    -1,
      59,   129,    22,    60,   129,    -1,    59,   129,    -1,    60,
     129,    -1,    60,   129,    22,    59,   129,    -1,   130,    -1,
     129,    22,   130,    -1,   123,    87,    -1,    20,   132,   133,
      21,    -1,    20,   132,   262,   133,    21,    -1,    -1,    62,
     237,    19,    -1,   254,    -1,   133,   254,    -1,   136,    -1,
     135,    -1,    76,    15,    87,    16,   145,    19,    -1,    63,
     144,    -1,    63,   137,    -1,    63,   138,    -1,    74,   259,
     140,   260,    -1,    74,   259,   262,   140,   260,    -1,    69,
     259,   139,   140,   141,   260,    -1,    69,   259,   262,   139,
     140,   141,   260,    -1,    69,   259,   262,   263,   139,   140,
     141,   260,    -1,    70,   143,    -1,    70,   142,    -1,   255,
      -1,   136,    -1,   140,   255,    -1,   140,   136,    -1,    71,
     142,    -1,    73,    15,    16,    19,    -1,    73,    15,   155,
      16,    19,    -1,    72,    15,    16,    19,    -1,    72,    15,
     170,    16,    19,    -1,    87,   145,    19,    -1,    15,   155,
      16,    -1,    15,    16,    -1,   259,   147,   148,   149,   260,
      -1,   259,   262,   147,   148,   149,   260,    -1,    -1,    64,
     258,    -1,    65,   258,    -1,    -1,    66,    20,   150,    21,
      -1,   151,    -1,   150,   151,    -1,    87,   152,    19,    -1,
      68,    15,    16,    -1,    67,    15,    16,    -1,    68,    15,
     172,    16,    -1,    67,    15,   172,    16,    -1,    77,    15,
      16,    -1,    87,    -1,   296,    -1,   297,    -1,    15,   172,
      16,    -1,    15,   259,   263,   260,    16,    -1,    15,   259,
     262,   263,   260,    16,    -1,   153,    -1,   154,    23,   172,
      24,    -1,   154,    15,    16,    -1,   154,    15,   155,    16,
      -1,   154,    15,    16,   146,    -1,   154,    15,   155,    16,
     146,    -1,   154,    15,    16,   145,    -1,   154,    15,   155,
      16,   145,    -1,    69,    15,    87,    16,   259,   139,   140,
     141,   260,    -1,    69,    15,    87,    16,   259,   262,   139,
     140,   141,   260,    -1,    69,    15,    87,    16,   259,   262,
     263,   139,   140,   141,   260,    -1,    74,    15,    87,    16,
     259,   140,   260,    -1,    74,    15,    87,    16,   259,   262,
     140,   260,    -1,    75,    15,    16,   145,    -1,   154,    17,
      87,    -1,   154,    96,    87,    -1,   154,    97,    -1,   154,
      98,    -1,   154,    17,    95,    -1,   154,    96,    95,    -1,
     170,    -1,   155,    22,   170,    -1,   154,    -1,    97,   156,
      -1,    98,   156,    -1,   157,   158,    -1,    52,   156,    -1,
      52,    15,   226,    16,    -1,     3,    -1,     4,    -1,     5,
      -1,     6,    -1,     7,    -1,     8,    -1,   156,    -1,    15,
     226,    16,   158,    -1,   158,    -1,   159,     4,   158,    -1,
     159,    14,   158,    -1,   159,    13,   158,    -1,   159,    -1,
     160,     5,   159,    -1,   160,     6,   159,    -1,   160,    -1,
     161,    99,   160,    -1,   161,   100,   160,    -1,   161,    -1,
     162,     9,   161,    -1,   162,    10,   161,    -1,   162,   101,
     161,    -1,   162,   102,   161,    -1,   162,    -1,   163,   103,
     162,    -1,   163,   104,   162,    -1,   163,    -1,   164,     3,
     163,    -1,   164,    -1,   165,    12,   164,    -1,   165,    -1,
     166,    11,   165,    -1,   166,    -1,   167,   105,   166,    -1,
     167,    -1,   168,   106,   167,    -1,   168,    -1,   168,    18,
     172,    25,   169,    -1,   169,    -1,   156,   171,   170,    -1,
     108,    -1,   109,    -1,   110,    -1,   111,    -1,   112,    -1,
     113,    -1,   114,    -1,   115,    -1,   116,    -1,   117,    -1,
     118,    -1,   170,    -1,   172,    22,   170,    -1,   169,    -1,
      -1,   172,    -1,   176,    19,    -1,   185,    19,    -1,   196,
      19,    -1,   202,    19,    -1,    -1,    -1,   194,   207,   177,
     227,   178,   233,    -1,    -1,    -1,   200,   207,   179,   227,
     180,   233,    -1,    -1,    -1,   176,    22,   207,   181,   227,
     182,   233,    -1,    -1,   194,     1,   183,   227,   233,    -1,
      -1,   200,     1,   184,   227,   233,    -1,   176,    22,     1,
      -1,    -1,    -1,   198,   218,   186,   227,   187,   233,    -1,
     123,   218,    -1,    -1,    -1,   204,   218,   188,   227,   189,
     233,    -1,    -1,    -1,   185,    22,   218,   190,   227,   191,
     233,    -1,    -1,   198,     1,   192,   227,   233,    -1,    -1,
     204,     1,   193,   227,   233,    -1,   185,    22,     1,    -1,
     195,    -1,   196,    -1,   197,    -1,   201,   300,    -1,   198,
     301,    -1,   195,   199,    -1,   195,   301,    -1,   202,   300,
      -1,   198,   206,    -1,   196,   199,    -1,   203,   300,    -1,
     198,    95,    -1,   197,   199,    -1,   300,    -1,   204,   300,
      -1,   198,   199,    -1,   298,    -1,   300,    -1,   201,    -1,
     202,    -1,   203,    -1,   301,    -1,   204,   301,    -1,   201,
     298,    -1,   201,   301,    -1,   206,    -1,   204,   206,    -1,
     202,   298,    -1,    95,    -1,   204,    95,    -1,   203,   298,
      -1,   298,    -1,   204,   298,    -1,   299,    -1,   205,   299,
      -1,   240,    -1,   250,    -1,   208,    -1,   211,    -1,   218,
      -1,   222,    -1,   209,    -1,     4,   208,    -1,     4,    15,
     210,    16,    -1,     4,   205,    15,   210,    16,    -1,     4,
     205,   208,    -1,    15,   208,    16,    -1,    15,   210,   217,
      16,    -1,    15,   208,    16,   217,    -1,    95,    -1,    15,
     210,    16,    -1,    95,    -1,    95,   217,    -1,   212,    -1,
     213,    -1,     4,   211,    -1,     4,   205,   211,    -1,    15,
     212,    16,    -1,    15,   212,    16,   217,    -1,   215,    -1,
     216,    -1,   217,    -1,     4,    -1,     4,   205,    -1,     4,
     214,    -1,     4,   205,   214,    -1,    15,   215,    16,    -1,
      15,   216,    16,    -1,    15,   217,    16,    -1,    15,   215,
      16,   217,    -1,   239,    -1,    15,    16,    -1,    15,   236,
      16,    -1,   219,    -1,   221,    -1,   220,    -1,     4,   218,
      -1,     4,   205,   218,    -1,   221,   217,    -1,    15,   219,
      16,    -1,    15,   219,    16,   217,    -1,    87,    -1,    15,
     221,    16,    -1,   223,    -1,     4,   222,    -1,     4,   205,
     222,    -1,   221,    15,   224,    16,    -1,    15,   222,    16,
      -1,    15,   222,    16,   217,    -1,    87,    -1,   224,    22,
      87,    -1,    87,    -1,    95,    -1,   200,    -1,   200,   214,
      -1,   204,    -1,   204,   214,    -1,    -1,   228,    -1,   229,
      -1,   228,   229,    -1,   120,    15,    15,   230,    16,    16,
      -1,   231,    -1,   230,    22,   231,    -1,    -1,   232,    -1,
     232,    15,   172,    16,    -1,    87,    -1,    95,    -1,    42,
      -1,    -1,   108,   234,    -1,    20,   235,    21,    -1,    20,
     235,    22,    21,    -1,   170,    -1,   234,    -1,   235,    22,
     234,    -1,   237,    -1,   237,    22,   107,    -1,   107,    -1,
     238,    -1,   237,    22,   238,    -1,   238,   108,   234,    -1,
     237,    22,     1,    -1,   194,    -1,   194,   214,    -1,   194,
     218,    -1,   194,   211,    -1,   198,    -1,   198,   214,    -1,
     198,   218,    -1,   200,    -1,   200,   214,    -1,   200,   218,
      -1,   200,   211,    -1,   204,    -1,   204,   214,    -1,   204,
     218,    -1,    23,    24,    -1,    23,   173,    24,    -1,   239,
      23,   173,    24,    -1,   239,    23,    24,    -1,   241,    20,
     242,    21,    -1,   241,   225,    20,   242,    21,    -1,   241,
     225,    -1,   241,    20,    21,    -1,   241,   225,    20,    21,
      -1,    29,    -1,    41,    -1,   243,    -1,   242,   243,    -1,
     245,    19,    -1,   244,    19,    -1,   204,   247,    -1,   244,
      22,   247,    -1,   200,   246,    -1,   245,    22,   246,    -1,
     207,   248,   227,    -1,   249,   227,    -1,   218,   248,   227,
      -1,   249,   227,    -1,    -1,   249,    -1,    25,   173,    -1,
      35,    20,   251,   253,    21,    -1,    35,   225,    20,   251,
     253,    21,    -1,    35,   225,    -1,   225,   252,    -1,   251,
      22,   225,   252,    -1,    -1,   108,   173,    -1,    -1,    22,
      -1,   134,    -1,   255,    -1,   256,    -1,   258,    -1,   264,
      -1,   265,    -1,   267,    -1,   275,    -1,     1,    19,    -1,
      -1,    87,    25,   257,   255,    -1,    34,   173,    25,   255,
      -1,    50,    25,   255,    -1,    95,    25,   255,    -1,   259,
     260,    -1,   259,   262,   260,    -1,   259,   133,   260,    -1,
     259,   262,   133,   260,    -1,    20,    -1,    21,    -1,    20,
      21,    -1,    20,   262,    21,    -1,    20,   263,    21,    -1,
      20,   262,   263,    21,    -1,   175,    -1,   262,   175,    -1,
     255,    -1,   263,   255,    -1,   174,    19,    -1,    55,    15,
     172,    16,   254,    -1,    55,    15,   172,    16,   254,    31,
     254,    -1,    -1,    33,   266,    15,   172,    16,   255,    -1,
      -1,    57,   268,    15,   172,    16,   255,    -1,    -1,    54,
     269,   255,    57,    15,   172,    16,    19,    -1,    -1,    47,
      15,   174,    19,   174,    19,   174,    16,   270,   254,    -1,
      -1,    47,    15,     1,    19,   174,    19,   174,    16,   271,
     254,    -1,    -1,    47,    15,   174,    19,   174,    19,     1,
      16,   272,   254,    -1,    -1,    47,    15,   174,    19,     1,
      19,   174,    16,   273,   254,    -1,    -1,    47,    15,     1,
      16,   274,   254,    -1,    51,    87,    19,    -1,    46,    19,
      -1,    30,    19,    -1,    40,   174,    19,    -1,    51,    95,
      19,    -1,   277,    -1,   276,   277,    -1,   175,    -1,   278,
      -1,   125,    -1,    -1,   218,   279,   261,    -1,   218,    86,
      -1,    -1,   194,   218,   280,   261,    -1,    -1,   200,   218,
     281,   261,    -1,    -1,   198,   218,   282,   261,    -1,    -1,
     204,   218,   283,   261,    -1,    -1,   222,   284,   261,    -1,
      -1,   194,   222,   285,   261,    -1,    -1,   200,   222,   286,
     261,    -1,    -1,   198,   222,   287,   261,    -1,    -1,   204,
     222,   288,   261,    -1,    -1,   222,   294,   289,   261,    -1,
      -1,   194,   222,   294,   290,   261,    -1,    -1,   200,   222,
     294,   291,   261,    -1,    -1,   198,   222,   294,   292,   261,
      -1,    -1,   204,   222,   294,   293,   261,    -1,    -1,   295,
     262,    -1,    89,    -1,    90,    -1,    91,    -1,    92,    -1,
      94,    -1,    88,    -1,   297,    88,    -1,    42,    -1,    53,
      -1,   119,    -1,    42,    -1,    53,    -1,    37,    -1,    39,
      -1,    56,    -1,    26,    -1,    36,    -1,    49,    -1,    38,
      -1,    28,    -1,    43,    -1,    27,    -1,    48,    -1,    45,
      -1,    44,    -1,    32,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   388,   388,   399,   406,   410,   416,   420,   432,   431,
     446,   456,   457,   461,   465,   469,   476,   480,   487,   495,
     500,   509,   510,   519,   523,   533,   537,   545,   552,   556,
     560,   567,   571,   579,   583,   587,   594,   599,   607,   611,
     615,   619,   626,   634,   638,   645,   649,   657,   667,   673,
     682,   687,   695,   696,   704,   713,   714,   721,   725,   732,
     740,   745,   750,   755,   760,   782,   783,   784,   785,   789,
     793,   800,   801,   803,   805,   808,   815,   822,   829,   836,
     840,   844,   848,   852,   856,   862,   864,   866,   868,   872,
     874,   879,   881,   886,   888,   890,   892,   895,   897,   902,
     903,   904,   905,   906,   907,   911,   912,   916,   917,   919,
     921,   926,   927,   929,   934,   935,   937,   942,   943,   945,
     947,   949,   954,   955,   957,   962,   963,   968,   969,   977,
     978,   985,   986,   991,   992,   999,  1000,  1005,  1006,  1025,
    1026,  1027,  1028,  1029,  1030,  1031,  1032,  1033,  1034,  1035,
    1039,  1040,  1055,  1059,  1060,  1096,  1098,  1100,  1102,  1109,
    1112,  1108,  1118,  1121,  1117,  1127,  1130,  1126,  1140,  1139,
    1150,  1149,  1158,  1165,  1168,  1164,  1173,  1179,  1182,  1178,
    1188,  1189,  1187,  1196,  1195,  1206,  1205,  1214,  1219,  1221,
    1222,  1228,  1230,  1232,  1234,  1241,  1243,  1245,  1252,  1254,
    1256,  1263,  1264,  1267,  1274,  1275,  1280,  1282,  1283,  1288,
    1289,  1291,  1293,  1299,  1300,  1302,  1309,  1311,  1313,  1319,
    1320,  1326,  1327,  1334,  1335,  1340,  1341,  1342,  1343,  1353,
    1354,  1357,  1360,  1363,  1371,  1374,  1377,  1384,  1386,  1393,
    1395,  1397,  1406,  1407,  1410,  1417,  1420,  1427,  1428,  1429,
    1434,  1436,  1438,  1441,  1448,  1451,  1454,  1457,  1464,  1465,
    1466,  1471,  1472,  1477,  1478,  1481,  1488,  1490,  1493,  1500,
    1502,  1509,  1514,  1517,  1524,  1526,  1529,  1544,  1546,  1552,
    1553,  1558,  1560,  1562,  1564,  1573,  1574,  1579,  1581,  1586,
    1593,  1595,  1601,  1602,  1604,  1610,  1611,  1612,  1617,  1618,
    1623,  1624,  1625,  1630,  1632,  1642,  1643,  1646,  1655,  1657,
    1661,  1666,  1672,  1674,  1677,  1680,  1683,  1685,  1687,  1689,
    1691,  1694,  1697,  1700,  1702,  1704,  1710,  1712,  1714,  1719,
    1735,  1739,  1744,  1749,  1755,  1765,  1766,  1771,  1772,  1778,
    1779,  1785,  1791,  1797,  1799,  1806,  1809,  1816,  1819,  1826,
    1827,  1832,  1837,  1839,  1841,  1847,  1849,  1855,  1856,  1860,
    1861,  1870,  1871,  1874,  1875,  1876,  1877,  1878,  1879,  1881,
    1887,  1886,  1891,  1893,  1897,  1902,  1904,  1906,  1908,  1912,
    1913,  1919,  1921,  1923,  1925,  1932,  1933,  1938,  1939,  1944,
    1948,  1950,  1952,  1952,  1958,  1957,  1962,  1961,  1966,  1965,
    1972,  1971,  1976,  1975,  1980,  1979,  1983,  1983,  1988,  1990,
    1992,  1994,  1998,  2009,  2010,  2016,  2026,  2036,  2051,  2050,
    2059,  2064,  2063,  2068,  2067,  2072,  2071,  2081,  2080,  2090,
    2089,  2099,  2098,  2107,  2106,  2115,  2114,  2124,  2123,  2133,
    2132,  2142,  2141,  2150,  2149,  2158,  2157,  2167,  2166,  2180,
    2180,  2197,  2198,  2199,  2200,  2201,  2206,  2207,  2229,  2230,
    2231,  2235,  2236,  2240,  2241,  2242,  2243,  2244,  2248,  2249,
    2250,  2251,  2252,  2254,  2255,  2257,  2258
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
  "ROUNDROBIN", "PIPELINE", "FILEREADER", "FILEWRITER", "UNCERTAINTY",
  "UPLUS", "UMINUS", "INDIR", "ADDRESS", "POSTINC", "POSTDEC", "PREINC",
  "PREDEC", "BOGUS", "IDENTIFIER", "STRINGliteral", "FLOATINGconstant",
  "INTEGERconstant", "OCTALconstant", "HEXconstant", "WIDECHARconstant",
  "CHARACTERconstant", "TYPEDEFname", "ARROW", "ICR", "DECR", "LS", "RS",
  "LE", "GE", "EQ", "NE", "ANDAND", "OROR", "ELLIPSIS", "'='",
  "MULTassign", "DIVassign", "MODassign", "PLUSassign", "MINUSassign",
  "LSassign", "RSassign", "ANDassign", "ERassign", "ORassign", "INLINE",
  "ATTRIBUTE", "$accept", "prog.start", "stream.type.specifier",
  "stream.declaration.list", "composite.definition", "$@1",
  "composite.head", "composite.head.inout",
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
     332,   333,   334,   335,   336,   337,   338,   339,    61,   340,
     341,   342,   343,   344,   345,   346,   347,   348,   349,   350,
     351
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   121,   122,   123,   124,   124,   124,   124,   126,   125,
     127,   128,   128,   128,   128,   128,   129,   129,   130,   131,
     131,   132,   132,   133,   133,   134,   134,   135,   136,   136,
     136,   137,   137,   138,   138,   138,   139,   139,   140,   140,
     140,   140,   141,   142,   142,   143,   143,   144,   145,   145,
     146,   146,   147,   147,   148,   149,   149,   150,   150,   151,
     152,   152,   152,   152,   152,   153,   153,   153,   153,   153,
     153,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   154,   154,   154,   154,   154,   154,   154,   154,   154,
     154,   155,   155,   156,   156,   156,   156,   156,   156,   157,
     157,   157,   157,   157,   157,   158,   158,   159,   159,   159,
     159,   160,   160,   160,   161,   161,   161,   162,   162,   162,
     162,   162,   163,   163,   163,   164,   164,   165,   165,   166,
     166,   167,   167,   168,   168,   169,   169,   170,   170,   171,
     171,   171,   171,   171,   171,   171,   171,   171,   171,   171,
     172,   172,   173,   174,   174,   175,   175,   175,   175,   177,
     178,   176,   179,   180,   176,   181,   182,   176,   183,   176,
     184,   176,   176,   186,   187,   185,   185,   188,   189,   185,
     190,   191,   185,   192,   185,   193,   185,   185,   194,   194,
     194,   195,   195,   195,   195,   196,   196,   196,   197,   197,
     197,   198,   198,   198,   199,   199,   200,   200,   200,   201,
     201,   201,   201,   202,   202,   202,   203,   203,   203,   204,
     204,   205,   205,   206,   206,   207,   207,   207,   207,   208,
     208,   208,   208,   208,   209,   209,   209,   210,   210,   211,
     211,   211,   212,   212,   212,   213,   213,   214,   214,   214,
     215,   215,   215,   215,   216,   216,   216,   216,   217,   217,
     217,   218,   218,   219,   219,   219,   220,   220,   220,   221,
     221,   222,   222,   222,   223,   223,   223,   224,   224,   225,
     225,   226,   226,   226,   226,   227,   227,   228,   228,   229,
     230,   230,   231,   231,   231,   232,   232,   232,   233,   233,
     234,   234,   234,   235,   235,   236,   236,   236,   237,   237,
     237,   237,   238,   238,   238,   238,   238,   238,   238,   238,
     238,   238,   238,   238,   238,   238,   239,   239,   239,   239,
     240,   240,   240,   240,   240,   241,   241,   242,   242,   243,
     243,   244,   244,   245,   245,   246,   246,   247,   247,   248,
     248,   249,   250,   250,   250,   251,   251,   252,   252,   253,
     253,   254,   254,   255,   255,   255,   255,   255,   255,   255,
     257,   256,   256,   256,   256,   258,   258,   258,   258,   259,
     260,   261,   261,   261,   261,   262,   262,   263,   263,   264,
     265,   265,   266,   265,   268,   267,   269,   267,   270,   267,
     271,   267,   272,   267,   273,   267,   274,   267,   275,   275,
     275,   275,   275,   276,   276,   277,   277,   277,   279,   278,
     278,   280,   278,   281,   278,   282,   278,   283,   278,   284,
     278,   285,   278,   286,   278,   287,   278,   288,   278,   289,
     278,   290,   278,   291,   278,   292,   278,   293,   278,   295,
     294,   296,   296,   296,   296,   296,   297,   297,   298,   298,
     298,   299,   299,   300,   300,   300,   300,   300,   301,   301,
     301,   301,   301,   301,   301,   301,   301
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
       3,     3,     4,     4,     3,     1,     1,     1,     3,     5,
       6,     1,     4,     3,     4,     4,     5,     4,     5,     9,
      10,    11,     7,     8,     4,     3,     3,     2,     2,     3,
       3,     1,     3,     1,     2,     2,     2,     2,     4,     1,
       1,     1,     1,     1,     1,     1,     4,     1,     3,     3,
       3,     1,     3,     3,     1,     3,     3,     1,     3,     3,
       3,     3,     1,     3,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     1,     5,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     0,     1,     2,     2,     2,     2,     0,
       0,     6,     0,     0,     6,     0,     0,     7,     0,     5,
       0,     5,     3,     0,     0,     6,     2,     0,     0,     6,
       0,     0,     7,     0,     5,     0,     5,     3,     1,     1,
       1,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     1,     2,     2,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     1,     2,     2,     1,     2,     2,     1,
       2,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       2,     4,     5,     3,     3,     4,     4,     1,     3,     1,
       2,     1,     1,     2,     3,     3,     4,     1,     1,     1,
       1,     2,     2,     3,     3,     3,     3,     4,     1,     2,
       3,     1,     1,     1,     2,     3,     2,     3,     4,     1,
       3,     1,     2,     3,     4,     3,     4,     1,     3,     1,
       1,     1,     2,     1,     2,     0,     1,     1,     2,     6,
       1,     3,     0,     1,     4,     1,     1,     1,     0,     2,
       3,     4,     1,     1,     3,     1,     3,     1,     1,     3,
       3,     3,     1,     2,     2,     2,     1,     2,     2,     1,
       2,     2,     2,     1,     2,     2,     2,     3,     4,     3,
       4,     5,     2,     3,     4,     1,     1,     1,     2,     2,
       2,     2,     3,     2,     3,     3,     2,     3,     2,     0,
       1,     2,     5,     6,     2,     2,     4,     0,     2,     0,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       0,     4,     4,     3,     3,     2,     3,     3,     4,     1,
       1,     2,     3,     3,     4,     1,     2,     1,     2,     2,
       5,     7,     0,     6,     0,     6,     0,     8,     0,    10,
       0,    10,     0,    10,     0,    10,     0,     6,     3,     2,
       2,     3,     3,     1,     2,     1,     1,     1,     0,     3,
       2,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       3,     0,     4,     0,     4,     0,     4,     0,     4,     0,
       4,     0,     5,     0,     5,     0,     5,     0,     5,     0,
       2,     1,     1,     1,     1,     1,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,   466,   472,   470,   335,   476,     0,   467,
     463,   469,   464,   336,   458,   471,   475,   474,   473,   468,
     459,   465,     0,     0,   269,   216,   460,     0,     0,   417,
       8,   415,     0,     0,     0,   188,   189,   190,     0,     0,
     206,   207,   208,     0,   213,   418,   261,   263,   262,   449,
     271,   223,     0,   224,     2,   413,   416,   219,   201,   209,
     461,   462,     0,   264,   272,   221,     0,     0,     0,     0,
     279,   280,   354,     0,     0,     1,     0,     0,   176,   262,
       0,   155,     0,   156,     0,   168,     0,     0,   239,   159,
     225,   229,   226,   241,   242,   227,   449,   193,   204,   205,
     194,   157,   197,   200,   183,   199,   203,   196,   173,   449,
     192,   170,   162,   227,   449,   211,   191,   212,   158,   215,
     195,   218,   198,   185,   217,   214,   177,   449,   220,   202,
     210,   420,     0,     0,     0,   266,   258,     0,   439,     0,
       0,   332,   414,   265,   273,   222,   267,   270,   275,   357,
     359,     0,    11,     0,     0,   206,   207,   208,     0,     0,
       0,     0,    21,     9,   172,   165,   227,   228,   187,   180,
     285,     0,     0,   230,   243,     0,   237,     0,     0,     0,
     240,   285,     0,     0,   441,   285,   285,     0,     0,   445,
     285,   285,     0,     0,   443,   285,   285,     0,     0,   447,
       0,   419,   259,   277,   307,   312,   189,   316,   319,   207,
     323,     0,     0,   305,   308,    99,   100,   101,   102,   103,
     104,     0,   326,     0,     0,     0,     0,    65,   456,   451,
     452,   453,   454,   455,     0,     0,    71,    93,   105,     0,
     107,   111,   114,   117,   122,   125,   127,   129,   131,   133,
     135,   152,     0,    66,    67,     0,   430,     0,   385,     0,
       0,     0,     0,   450,   333,     0,     0,     0,   337,     0,
       0,     0,   268,   276,     0,   355,   360,     0,   359,     0,
       0,     0,     3,     0,     0,     4,     0,     0,   285,   285,
       0,   298,   286,   287,     0,     0,   233,   244,     0,   234,
       0,   245,   160,   422,   432,     0,   298,   174,   426,   436,
       0,   298,   163,   424,   434,     0,   298,   178,   428,   438,
       0,     0,   379,   381,     0,   392,     0,   153,     0,     0,
       0,     0,   396,     0,   394,    65,   216,   105,   137,   150,
     154,     0,   387,   363,   364,     0,     0,     0,   365,   366,
     367,   368,   250,     0,   315,   313,   247,   248,   249,   314,
     250,     0,   317,   318,   322,   320,   321,   324,   325,   274,
       0,   260,     0,     0,     0,   281,   283,     0,     0,     0,
      97,     0,     0,     0,     0,    94,    95,     0,     0,     0,
       0,    87,    88,    96,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   327,   457,   329,     0,   440,   173,   177,
     386,     0,   349,   343,   285,   349,   341,   285,   330,   338,
     340,     0,   339,     0,   334,     0,   358,   357,   352,     0,
       0,    13,    16,    14,    10,     0,     0,     5,     0,     0,
       0,     0,   361,    26,    25,    23,   362,     0,   166,   181,
       0,     0,   169,   288,   231,     0,   238,   236,   235,   246,
     298,   442,   184,   298,   446,   171,   298,   444,   186,   298,
     448,   369,   410,     0,     0,     0,   409,     0,     0,     0,
       0,     0,     0,     0,   370,     0,   139,   140,   141,   142,
     143,   144,   145,   146,   147,   148,   149,     0,     0,   389,
     380,     0,   375,     0,   382,     0,   383,     0,   388,   251,
     252,     0,     0,     0,   251,   278,   311,   306,   309,     0,
     302,   310,    68,   250,     0,   282,   284,     0,     0,     0,
       0,     0,     0,     0,    73,     0,    91,    85,    89,     0,
      86,    90,   108,   110,   109,   112,   113,   115,   116,   118,
     119,   120,   121,   123,   124,   126,   128,   130,   132,     0,
     134,   328,   351,   285,   350,   346,   285,   348,   342,   344,
     331,   356,   353,    18,     0,     0,     6,    22,     0,     0,
       0,     0,    29,    30,    28,     0,    19,    24,     0,   298,
     298,   292,   299,   232,   161,   175,   164,   179,     0,     0,
     411,     0,     0,   373,   408,   412,     0,     0,     0,     0,
     374,   138,   151,   377,     0,   376,   384,   253,   254,   255,
     256,   303,     0,   251,   106,     0,     0,    98,     0,     0,
       0,    84,    77,    75,    52,    74,     0,    72,     0,   345,
     347,     0,    17,     0,     7,     0,     0,     0,     0,    20,
     167,   182,   297,   295,   296,     0,   290,   293,     0,   372,
     406,   153,     0,     0,     0,     0,   371,   378,   257,   300,
       0,     0,    69,     0,     0,    49,     0,     0,     0,    52,
      78,    76,    92,   136,    12,    15,     0,     0,     0,    39,
       0,    38,     0,    47,     0,     0,   292,     0,     0,     0,
       0,     0,     0,     0,   390,     0,   301,   304,    70,     0,
       0,     0,     0,    48,    53,     0,    55,     0,     0,     0,
       0,    37,    36,     0,     0,     0,    41,    40,    31,     0,
       0,   289,   291,     0,   393,   407,   153,   153,     0,     0,
       0,   395,     0,     0,     0,    82,     0,    54,     0,     0,
      55,     0,     0,     0,     0,     0,     0,    32,    27,   294,
       0,     0,     0,     0,     0,   391,     0,     0,     0,    83,
       0,    50,     0,     0,     0,     0,     0,    42,    33,     0,
       0,   400,   404,   402,   398,   397,    79,     0,     0,     0,
       0,    57,    51,    45,     0,    43,     0,    34,     0,     0,
       0,     0,     0,    80,     0,     0,     0,     0,     0,    56,
      58,    46,    44,    35,   401,   405,   403,   399,    81,     0,
       0,     0,    59,    61,     0,    60,     0,    64,    63,    62
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    27,    28,   153,    29,    80,    30,   281,   441,   442,
     163,   287,   451,   452,   453,   454,   592,   593,   697,   700,
     764,   731,   732,   594,   641,   643,   688,   726,   759,   800,
     801,   818,   236,   237,   545,   337,   239,   240,   241,   242,
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
#define YYPACT_NINF -651
static const yytype_int16 yypact[] =
{
    2298,   730,    63,  -651,  -651,  -651,  -651,  -651,    65,  -651,
    -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,
    -651,  -651,   -44,    53,  -651,  -651,  -651,    66,   308,  -651,
    -651,  -651,   102,   160,   172,  2128,   700,    91,  1788,   419,
    2128,   787,    91,  1850,  -651,    -5,  -651,  -651,    72,    78,
    -651,  -651,   125,  -651,  2298,  -651,  -651,  -651,  -651,  -651,
    -651,  -651,   730,  -651,  -651,  -651,    98,   424,   110,   109,
    -651,  -651,    87,   139,  4916,  -651,   946,   308,  -651,   292,
     145,  -651,   584,  -651,   267,  -651,   390,   315,   292,  -651,
    -651,  -651,  -651,  -651,  -651,   157,    54,  -651,  -651,  -651,
    -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,   238,   258,
    -651,  -651,  -651,   265,    64,  -651,  -651,  -651,  -651,  -651,
    -651,  -651,  -651,  -651,  -651,  -651,   305,   317,  -651,  -651,
    -651,  -651,   331,  2570,  4126,  -651,   169,   331,  -651,  4855,
    2600,   336,  -651,  -651,  -651,  -651,   292,  -651,   292,   126,
     232,   109,   310,    67,    88,   804,   234,   234,  4944,   946,
     430,  2643,   298,  -651,  -651,  -651,  -651,  -651,  -651,  -651,
     245,   315,   465,  -651,  -651,   315,  -651,   367,   292,   399,
    -651,   245,   331,   331,  -651,   245,   245,   331,   331,  -651,
     245,   245,   331,   331,  -651,   245,   245,   331,   331,  -651,
    1298,  -651,  -651,  -651,  -651,   455,    91,  2377,   455,    91,
    2439,   195,   448,   384,   375,  -651,  -651,  -651,  -651,  -651,
    -651,  2100,  -651,  4607,   477,   479,   490,  -651,  -651,  -651,
    -651,  -651,  -651,  -651,  4648,  4648,  -651,   548,  -651,  4703,
    -651,   318,   391,   447,    96,   550,   509,   410,   505,   429,
      23,  -651,   519,  -651,   460,  4167,  -651,   331,  -651,   172,
    1912,   419,  1974,  4855,  -651,   138,  2475,  2673,  -651,   325,
     407,  2704,  -651,  -651,  4703,  -651,   109,   530,   232,   500,
     500,   557,  -651,  4916,    88,   292,  4886,  1197,   245,   245,
     549,   458,   245,  -651,   580,   315,  -651,  -651,   597,   292,
     559,   292,  -651,  -651,  -651,   331,   458,  -651,  -651,  -651,
     331,   458,  -651,  -651,  -651,   331,   458,  -651,  -651,  -651,
     331,   583,  -651,  -651,   592,  -651,  4703,  4703,   622,   571,
     631,   240,  -651,   650,  -651,   648,   653,  1266,  -651,  -651,
     674,   656,  -651,  -651,  -651,  1099,  1399,  3385,  -651,  -651,
    -651,  -651,   717,  2189,  -651,  -651,  -651,  -651,  -651,  -651,
     199,  2225,  -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,
     613,  -651,  2007,  4223,   358,    97,  2536,   691,  1693,  2100,
    -651,   629,   638,   702,  4264,  -651,  -651,  4319,   262,  4703,
     332,  -651,  -651,  -651,  4703,  4703,  4703,  4703,  4703,  4703,
    4703,  4703,  4703,  4703,  4703,  4703,  4703,  4703,  4703,  4703,
    4703,  4703,  4703,  -651,  -651,  -651,   699,  -651,  -651,  -651,
    -651,  4703,   704,  -651,   245,   704,  -651,   245,  -651,  -651,
    -651,   330,  -651,   138,  -651,  2737,  -651,   126,  -651,   712,
     651,   724,  -651,   726,  -651,    88,   733,  -651,   511,   214,
     737,  2797,  -651,  -651,  -651,  -651,  -651,  1197,  -651,  -651,
     739,  4223,  -651,  -651,  -651,   636,  -651,  -651,  -651,  -651,
     458,  -651,  -651,   458,  -651,  -651,   458,  -651,  -651,   458,
    -651,  -651,  -651,   740,   744,   752,  -651,  3932,  3875,   755,
     758,  3875,  4703,   763,  -651,  3875,  -651,  -651,  -651,  -651,
    -651,  -651,  -651,  -651,  -651,  -651,  -651,  4703,  4703,  -651,
    -651,  2895,  -651,  1099,  -651,  3483,  -651,   653,  -651,   717,
    -651,   757,   765,   766,   199,  -651,  -651,  -651,  -651,  4223,
    -651,  -651,  -651,   555,  2334,  -651,  -651,  4703,  1693,  3581,
     768,   770,   771,   773,   136,   371,  -651,  -651,  -651,   208,
    -651,  -651,  -651,  -651,  -651,   318,   318,   391,   391,   447,
     447,   447,   447,    96,    96,   550,   509,   410,   505,    69,
     429,  -651,  -651,   245,  -651,  -651,   245,  -651,  -651,  -651,
    -651,  -651,  -651,  -651,   624,   372,   292,  -651,  2039,   774,
     774,   773,  -651,  -651,  -651,   706,  -651,  -651,  2993,   458,
     458,    21,  -651,  -651,  -651,  -651,  -651,  -651,  4703,  3875,
    -651,   202,   780,  -651,  -651,  -651,   748,   414,  4703,  3875,
    -651,  -651,  -651,  -651,  2895,  -651,  -651,  -651,   292,  -651,
    -651,  -651,   673,   555,  -651,  3581,   795,  -651,   774,   774,
    4360,  -651,  -651,  -651,  4776,   136,  4703,  -651,  4703,  -651,
    -651,   500,  -651,   500,  -651,  4807,  1497,   797,   798,  -651,
    -651,  -651,  -651,  -651,  -651,   432,  -651,   803,   433,  -651,
    -651,  4703,  3973,   807,  3091,   459,  -651,  -651,  -651,  -651,
    4071,   809,  -651,  4807,  1497,  -651,   487,   774,   762,  4776,
    -651,  -651,  -651,  -651,   808,   808,   633,  3679,  1595,  -651,
    3189,  -651,  1497,  -651,   773,   817,    21,  4703,  3875,  3091,
     815,   816,   818,  4703,   814,  3875,  -651,  -651,  -651,  3679,
    1595,  3189,  1497,  -651,  -651,   774,   772,   762,   500,   835,
     836,  -651,  -651,  3287,  3679,  3777,  -651,  -651,  -651,  3189,
     839,  -651,  -651,   567,  -651,  -651,  4703,  4703,  4030,   568,
    3091,  -651,  3287,  3679,  3777,  -651,  3189,  -651,   834,   838,
     772,  4415,  4456,   782,   838,  3287,  3679,  -651,  -651,  -651,
     844,   845,   846,   848,   847,  -651,   838,  3287,  3679,  -651,
     781,  -651,   838,   855,   851,   858,   585,  -651,  -651,   838,
    3287,  -651,  -651,  -651,  -651,  -651,  -651,   838,  3287,   183,
      32,  -651,  -651,  -651,   859,  -651,   860,  -651,   838,  3091,
    3091,  3091,  3091,  -651,   838,   869,   870,   871,   868,  -651,
    -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,  4511,
    4552,   874,  -651,  -651,   645,  -651,   660,  -651,  -651,  -651
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -651,  -651,  -209,  -651,  -651,  -651,  -651,  -651,  -267,  -535,
    -651,  -651,  -325,  -651,  -651,   319,  -651,  -651,  -319,   205,
     -97,   129,  -651,  -651,  -523,   248,   206,   170,   143,  -651,
     101,  -651,  -651,  -651,  -604,   227,  -651,  -195,   312,   314,
     406,   322,   491,   497,   502,   499,   501,  -651,  -119,   111,
    -651,  -220,  -216,  -258,     0,  -651,  -651,  -651,  -651,  -651,
    -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,
    -651,  -651,  -651,     5,  -651,  -116,  -651,    10,    20,     6,
     141,   -43,   441,    28,   -54,   -24,   -30,   -79,  -651,  -110,
     -68,   -40,  -651,  -181,  -328,  -315,   -37,   266,    38,  -651,
     604,   649,  -651,  -651,    27,   533,    84,  -651,   627,  -651,
     215,  -651,   271,  -457,  -651,  -651,   640,  -349,  -651,  -651,
    -651,   657,  -233,  -651,  -651,   489,   498,   495,  -178,  -651,
     779,   496,   662,   106,  -198,  -651,  -651,  -650,    25,   204,
     754,  -188,  -330,  -651,  -651,  -651,  -651,  -651,  -651,  -651,
    -651,  -651,  -651,  -651,  -651,  -651,   884,  -651,  -651,  -651,
    -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,  -651,
    -651,  -651,  -651,   212,  -651,  -651,  -651,   725,   -59,   760,
     634
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -438
static const yytype_int16 yytable[] =
{
      31,   374,   342,   145,   602,    34,    39,   173,   177,   112,
      38,   135,   346,   443,   107,   251,   515,   206,   174,   125,
     511,   642,   159,   528,   355,   521,   362,   365,    43,   367,
     135,   156,   172,   521,   429,    72,   686,   724,   522,   416,
      66,   411,   135,    73,   393,   206,   522,   179,   539,   652,
     652,   180,   165,   819,    31,    97,   102,   103,   436,    34,
      39,   294,    74,   662,    38,   298,    75,     1,   657,   485,
     440,   440,   631,  -228,  -431,   757,  -228,   282,     2,   141,
     154,   131,    43,  -228,  -433,    69,  -228,   133,   427,   283,
     209,   508,   177,   296,   648,   134,   177,   156,  -429,   457,
     145,   533,   158,   284,   297,   401,   402,   151,   663,   272,
     484,   273,   534,   145,   146,    66,   664,     3,   209,   799,
     134,    81,   690,   135,    82,    66,   148,     9,    10,   412,
      12,   179,   598,    14,   125,   179,   251,   354,   205,   208,
     364,   300,    86,   207,    20,   140,   265,    21,   342,   518,
      24,   640,    70,    87,   152,   251,   322,   513,   786,   374,
      71,   210,  -228,   421,   374,   162,   205,   208,   266,   549,
     206,   207,  -228,    85,  -228,    24,    86,  -421,   156,    83,
     342,   740,    84,   107,  -228,   465,   125,    87,   624,   210,
     538,   569,   255,   652,   535,   536,    70,   403,   404,   552,
     553,   554,   429,   360,    71,   572,   521,   251,   635,    66,
      26,   369,    70,    66,   361,   155,   177,   370,   670,   522,
      71,   671,   134,   717,   156,    24,   102,   375,   156,   612,
     508,   112,   647,    88,   274,   422,   107,   206,   125,   528,
     156,    60,   125,   209,   574,   206,   378,   574,   447,   376,
     815,   816,    61,   427,   276,   179,   206,   300,  -425,    24,
     817,   300,   467,   420,   469,   302,    45,    88,   168,   306,
     307,    76,   617,   265,   311,   312,    14,   265,  -435,   316,
     317,   155,    77,   589,   174,  -423,    24,    20,   590,   445,
     613,   205,   208,   616,    78,   266,   207,   620,   519,   266,
      95,   591,   251,   437,   108,   113,   524,   161,   184,   126,
     209,   158,    76,   179,   210,   134,   523,   518,   209,    86,
      45,   189,   394,    77,   523,  -427,   194,   489,   143,   209,
     175,   395,   396,    66,    76,   490,   156,  -437,   627,   199,
     342,   518,   634,   627,   430,    77,   420,   431,   166,   547,
     169,   200,   125,    26,    24,   421,   271,   548,   205,   208,
     286,   238,   155,   207,   719,   290,   205,   208,   735,   279,
     280,   207,   458,   459,   532,   440,   440,   205,   208,   734,
     508,   210,   207,   299,   694,   375,   695,   645,   668,   210,
     754,    66,   156,   646,    86,    24,   397,   398,   675,    66,
     210,   753,    24,   422,   378,   171,   372,   376,   155,   378,
     176,   669,   155,   710,   712,   301,   766,    24,   206,   550,
     111,   676,   408,    86,   155,   143,   432,   551,   300,   433,
     674,   653,    60,    23,    87,   778,   508,   518,   143,   133,
     147,   265,   440,    61,   440,   161,   147,   134,   705,   708,
     380,   297,   627,   134,   706,   508,   689,   420,   701,   352,
     145,   385,   386,   266,   371,   145,   238,   698,   702,    86,
     353,   359,   206,   363,   366,   715,   368,    24,   134,   633,
     295,   508,   238,   373,   530,    88,   701,   743,   770,   771,
     773,   209,   381,   749,   382,   720,   722,   523,   546,   701,
     342,   238,   737,   723,   701,   383,    24,    60,   575,   646,
     744,   577,   407,   420,    88,   157,   409,   751,    61,   440,
     155,   701,   342,   737,   701,   166,   418,   166,   419,   693,
     587,   166,   425,   588,   410,   737,   701,   518,   420,   205,
     208,   737,    24,   413,   207,   209,   399,   400,   414,   654,
      88,   438,    24,   238,   737,   701,   518,   597,   737,   533,
      88,    23,   210,   387,   460,   388,   461,   737,   701,   644,
     534,   389,   530,   444,   145,   468,   155,   472,   134,   737,
     701,   157,   475,   769,   774,   164,   487,   478,    86,   508,
     508,   678,   737,   205,   208,   161,   464,    60,   207,    87,
     737,   806,   481,   134,    48,    48,    67,   646,    61,   834,
     836,   482,   161,   466,   655,   656,   210,   597,   621,   622,
     134,   238,   238,   238,   238,   238,   238,   238,   238,   238,
     238,   238,   238,   238,   238,   238,   238,   238,    48,   238,
     530,   486,    48,    48,   390,   391,   392,    48,   238,    49,
      64,   161,   603,   405,   406,   776,   488,   649,    48,   134,
     650,   838,   157,   683,   684,   492,    48,   508,   789,   100,
     644,    24,   110,   494,   117,   509,   839,   130,   495,    88,
     797,   160,   508,    96,   651,    23,    48,   109,   114,   420,
      48,    67,   127,   808,   679,   680,   508,   425,   420,   166,
     525,   814,   420,    49,   597,   729,   730,   537,   157,   555,
     556,   144,   157,   557,   558,   623,   541,   625,   543,   101,
     420,   352,   420,   571,   157,   542,     3,   563,   564,   421,
     597,   167,   353,   582,     1,    64,     9,    10,   583,    12,
     134,   604,    14,   636,   605,     2,   584,   606,   585,   147,
     607,   546,   595,    20,   601,   608,    21,   692,   285,    60,
      98,    98,    98,    98,   238,   115,   119,   121,   128,   609,
      61,   610,    60,   628,   614,    67,    48,   615,   618,    67,
     714,   629,   630,    61,   637,   143,   638,   639,   640,   117,
     143,   530,   130,   658,   322,    99,    99,    99,    99,   672,
     116,   120,   122,   129,    24,   673,   118,   559,   560,   561,
     562,   682,    88,     3,   704,   745,   703,    24,   707,    26,
     157,   144,   713,     9,    10,   718,    12,   725,   677,    14,
     728,     4,     5,   741,   746,   747,     7,   748,   758,   681,
      20,   110,    11,    21,   130,   750,    14,    15,    16,    17,
     761,   762,    18,    19,   780,   730,   775,    20,   768,   510,
     791,   792,   793,    48,   794,    48,   795,   804,   799,    48,
     660,   661,   784,   546,   803,   238,   157,   805,   821,   822,
     115,   119,   121,   128,   829,   830,   831,   832,   446,   721,
     837,   256,   787,   691,   110,   727,   130,   760,   565,    67,
     130,   820,   733,   782,   738,   566,    26,   739,   167,   568,
     167,   567,   540,   570,   167,   824,   825,   826,   827,   463,
     576,   742,   579,    26,   752,   755,   448,   756,   435,   578,
     278,    98,    98,   581,   119,   128,   303,   304,   142,   765,
     439,   308,   309,   767,     0,     0,   313,   314,     0,     0,
      76,   318,   319,     0,     0,     0,     0,   160,   777,     0,
     779,    77,     0,   781,     0,   160,    99,    99,   788,   120,
     129,   790,     0,     0,     0,   699,     0,     0,     0,     0,
     796,     0,     0,   798,     0,    98,   802,   128,    60,     0,
       0,   128,     0,   807,     0,     0,     0,     0,     0,    61,
       0,   813,     0,   699,     0,     0,     0,     0,     0,     0,
     130,   417,   823,     0,     0,     0,   699,     0,   828,   736,
      99,   699,   129,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    24,     0,     0,     0,    48,   699,     0,
     736,   699,     0,     0,     0,     0,     0,     0,     0,   586,
       0,     0,   736,   699,     0,     0,     0,     0,   736,   471,
       0,     0,     0,     0,   474,     0,     0,     0,     0,   477,
       0,   736,   699,     0,   480,   736,     0,     0,     0,     0,
       0,     0,   167,     0,   736,   699,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   736,   699,     0,     0,
     321,   128,   215,   216,   217,   218,   219,   220,     0,   736,
       0,     0,     0,     0,   221,     0,     0,   736,  -153,   322,
     510,     0,     0,     0,     0,     3,     4,     5,     6,   324,
       0,     7,   325,   326,     8,     9,    10,    11,    12,   327,
      13,    14,    15,    16,    17,   328,   329,    18,    19,   330,
     331,   223,    20,   332,   333,    21,   334,     0,     0,     0,
      23,     0,   449,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,   450,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   335,   228,   229,   230,
     231,   232,     0,   233,   336,     0,   234,   235,   321,     0,
     215,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,   221,     0,     0,     0,  -153,   322,    26,     0,
       0,     0,     0,     3,     4,     5,     6,   324,     0,     7,
     325,   326,     8,     9,    10,    11,    12,   327,    13,    14,
      15,    16,    17,   328,   329,    18,    19,   330,   331,   223,
      20,   332,   333,    21,   334,     0,     0,     0,    23,     0,
     449,     0,     0,     0,     0,     0,   224,     0,     0,     0,
       0,   225,   226,   450,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   335,   228,   229,   230,   231,   232,
       0,   233,   336,     0,   234,   235,     0,     0,     0,   321,
       0,   215,   216,   217,   218,   219,   220,     0,     0,     0,
       0,     0,     0,   221,     0,     0,    26,  -153,   322,   323,
       0,     0,     0,     0,     3,     4,     5,     6,   324,     0,
       7,   325,   326,     8,     9,    10,    11,    12,   327,    13,
      14,    15,    16,    17,   328,   329,    18,    19,   330,   331,
     223,    20,   332,   333,    21,   334,     0,     0,     0,    23,
       0,     0,     0,     0,     0,     0,     0,   224,     0,     0,
       0,     0,   225,   226,   496,   497,   498,   499,   500,   501,
     502,   503,   504,   505,   506,   335,   228,   229,   230,   231,
     232,     0,   233,   336,     0,   234,   235,     0,     0,     0,
     321,     0,   215,   216,   217,   218,   219,   220,     0,     0,
       0,     0,     0,     0,   221,     0,     0,    26,  -153,   322,
     514,     0,     0,     0,     0,     3,     4,     5,     6,   324,
       0,     7,   325,   326,     8,     9,    10,    11,    12,   327,
      13,    14,    15,    16,    17,   328,   329,    18,    19,   330,
     331,   223,    20,   332,   333,    21,   334,     0,     0,     0,
      23,     0,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   335,   228,   229,   230,
     231,   232,     0,   233,   336,     0,   234,   235,   321,     0,
     215,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,   221,     0,     0,     0,  -153,   322,    26,     0,
       0,     0,     0,     3,     4,     5,     6,   324,     0,     7,
     325,   326,     8,     9,    10,    11,    12,   327,    13,    14,
      15,    16,    17,   328,   329,    18,    19,   330,   331,   223,
      20,   332,   333,    21,   334,     0,     0,     0,    23,     0,
     449,     0,     0,     0,     0,     0,   224,     0,     0,     0,
       0,   225,   226,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   335,   228,   229,   230,   231,   232,
       0,   233,   336,     0,   234,   235,   321,     0,   215,   216,
     217,   218,   219,   220,     0,     0,     0,     0,     0,     0,
     221,     0,     0,     0,  -153,   322,    26,     0,     0,     0,
       0,     3,     4,     5,     6,   324,     0,     7,   325,   326,
       8,     9,    10,    11,    12,   327,    13,    14,    15,    16,
      17,   328,   329,    18,    19,   330,   331,   223,    20,   332,
     333,    21,   334,     0,     0,     0,    23,     0,     0,     0,
       0,     0,     0,     0,   224,   696,     0,     0,     0,   225,
     226,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   335,   228,   229,   230,   231,   232,     0,   233,
     336,     0,   234,   235,   321,     0,   215,   216,   217,   218,
     219,   220,     0,     0,     0,     0,     0,     0,   221,     0,
       0,     0,  -153,   322,    26,     0,     0,     0,     0,     3,
       4,     5,     6,   324,     0,     7,   325,   326,     8,     9,
      10,    11,    12,   327,    13,    14,    15,    16,    17,   328,
     329,    18,    19,   330,   331,   223,    20,   332,   333,    21,
     334,     0,     0,     0,    23,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   225,   226,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     335,   228,   229,   230,   231,   232,     0,   233,   336,   104,
     234,   235,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     2,     0,     0,     0,     0,     0,     0,
       0,     0,    26,     0,     3,     4,     5,     6,     0,     0,
       7,     0,     0,     8,     9,    10,    11,    12,     0,    13,
      14,    15,    16,    17,     0,     0,    18,    19,     0,     0,
       0,    20,     0,     0,    21,     0,     0,     0,     0,     0,
       0,   123,     0,     0,     1,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    24,     3,     4,     5,     6,
       0,     0,     7,   105,     0,     8,     9,    10,    11,    12,
       0,    13,    14,    15,    16,    17,     0,     0,    18,    19,
       0,     0,     0,    20,     0,     0,    21,    26,     0,     0,
       0,     0,     0,   104,     0,     0,    76,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    77,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    24,     3,     4,
       5,     6,     0,     0,     7,   124,     0,     8,     9,    10,
      11,    12,     0,    13,    14,    15,    16,    17,     0,     0,
      18,    19,     0,     0,     0,    20,     0,     0,    21,    26,
       0,     0,     0,     0,     0,   123,     0,     0,    76,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    77,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
       3,     4,     5,     6,     0,     0,     7,   105,   526,     8,
       9,    10,    11,    12,     0,    13,    14,    15,    16,    17,
       0,     0,    18,    19,     0,     0,     0,    20,     0,     0,
      21,    26,     0,     3,     4,     5,     6,     0,     0,     7,
     526,     0,     8,     9,    10,    11,    12,     0,    13,    14,
      15,    16,    17,     0,     0,    18,    19,     0,     0,     0,
      20,    24,     0,    21,     0,     3,     4,     5,     6,   124,
       0,     7,     0,     0,     8,     9,    10,    11,    12,     0,
      13,    14,    15,    16,    17,     0,     0,    18,    19,     0,
       0,     0,    20,    26,     0,    21,     0,     0,     0,     0,
       0,     0,    25,   215,   216,   217,   218,   219,   220,     0,
       0,     0,     0,     0,   527,   221,     0,     0,     0,     0,
     322,     0,     0,     0,     0,     0,    26,     4,     5,     6,
       0,     0,     7,     0,    25,     8,     0,     0,    11,     0,
       0,    13,    14,    15,    16,    17,     0,     0,    18,    19,
       0,     0,   223,    20,     3,     4,     5,     0,    26,     0,
       7,     0,     0,     0,     9,    10,    11,    12,     0,   224,
      14,    15,    16,    17,   225,   226,    18,    19,     0,     0,
       0,    20,     0,     0,    21,     0,     0,   227,   228,   229,
     230,   231,   232,   352,   233,    25,     0,   234,   235,     0,
       0,     0,     0,     0,   353,   202,     0,     0,     0,     0,
       0,     0,   134,     0,     0,     3,     4,     5,     6,    26,
       0,     7,     0,     0,     8,     9,    10,    11,    12,   360,
      13,    14,    15,    16,    17,     0,     0,    18,    19,     0,
     361,   202,    20,     0,     0,    21,     0,    26,   134,     0,
       0,     3,     4,     5,     6,     0,     0,     7,     0,     0,
       8,     9,    10,    11,    12,     0,    13,    14,    15,    16,
      17,     0,     0,    18,    19,     0,    24,     0,    20,     0,
       0,    21,     0,     0,    25,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   204,     0,     0,     0,
       0,     0,     1,     0,     0,     0,     0,     0,    26,     0,
       0,     0,    24,     2,     0,     0,     0,     0,     0,     0,
      25,     0,     0,     0,     3,     4,     5,     6,     0,     0,
       7,     0,   204,     8,     9,    10,    11,    12,   533,    13,
      14,    15,    16,    17,    26,     0,    18,    19,     0,   534,
     202,    20,     0,     0,    21,     0,    22,   134,     0,    23,
       3,     4,     5,     6,     0,     0,     7,     0,     0,     8,
       9,    10,    11,    12,     0,    13,    14,    15,    16,    17,
       0,   360,    18,    19,     0,    24,     0,    20,     0,     0,
      21,     0,   361,    25,     0,     0,     0,     0,     0,     0,
     134,     0,     0,     3,     4,     5,     6,     0,     0,     7,
       0,     0,     8,     9,    10,    11,    12,    26,    13,    14,
      15,    16,    17,     0,     0,    18,    19,     0,     0,    25,
      20,     0,     0,    21,     0,     0,     0,     0,     0,     0,
       0,   204,     0,   360,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    26,   361,     0,     0,     0,     0,     0,
       0,     0,   134,     0,    24,     3,     4,     5,     6,     0,
       0,     7,   105,     0,     8,     9,    10,    11,    12,    76,
      13,    14,    15,    16,    17,     0,     0,    18,    19,     0,
      77,     0,    20,     0,     0,    21,    26,     0,     0,     0,
     421,     0,     4,     5,     6,     0,     0,     7,     0,     0,
       8,     0,     0,    11,     0,     0,    13,    14,    15,    16,
      17,     0,     0,    18,    19,     0,    24,     0,    20,     0,
       0,     0,     0,     0,   124,     0,     0,     0,     0,     0,
     533,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   534,     0,     0,     0,     0,     0,     0,    26,   134,
       0,     0,    24,     4,     5,     6,     0,     0,     7,     0,
     124,     8,     0,     0,    11,     0,     0,    13,    14,    15,
      16,    17,     0,     0,    18,    19,   202,     0,     0,    20,
       0,     0,     0,     0,    26,     0,     3,     4,     5,     6,
       0,     0,     7,     0,     0,     8,     9,    10,    11,    12,
       0,    13,    14,    15,    16,    17,     0,     0,    18,    19,
       0,   264,     0,    20,     0,     0,    21,     4,     5,     6,
       0,   124,     7,     0,     0,     8,     0,     0,    11,     0,
       0,    13,    14,    15,    16,    17,     0,     0,    18,    19,
       0,     0,     0,    20,     0,    26,     0,   203,     0,   202,
       0,     0,     0,     0,     0,    25,     0,     0,     0,     3,
       4,     5,     6,     0,     0,     7,     0,   204,     8,     9,
      10,    11,    12,     0,    13,    14,    15,    16,    17,    26,
       0,    18,    19,     0,   428,    25,    20,     0,     0,    21,
       4,     5,     6,     0,     0,     7,     0,     0,     8,     0,
       0,    11,     0,     0,    13,    14,    15,    16,    17,    26,
       0,    18,    19,     0,     0,   434,    20,     0,     0,     0,
       0,     4,     5,     6,     0,     0,     7,     0,    25,     8,
       0,     0,    11,     0,     0,    13,    14,    15,    16,    17,
     204,     0,    18,    19,     0,     0,     0,    20,   580,     0,
       0,     0,    26,     0,     4,     5,     6,     0,    25,     7,
       0,     0,     8,     0,     0,    11,     0,     0,    13,    14,
      15,    16,    17,     0,     0,    18,    19,     0,     0,     0,
      20,     0,    26,     0,     0,     0,     0,     0,   321,    25,
     215,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,   221,     0,     0,     0,  -153,   322,   596,     0,
       0,     0,     0,    26,     0,     0,     0,   324,     0,     0,
     325,   326,    25,     0,     0,     0,     0,   327,     0,     0,
       0,     0,     0,   328,   329,     0,     0,   330,   331,   223,
       0,   332,   333,     0,   334,     0,    26,     0,     0,     0,
     449,     0,     0,     0,     0,     0,   224,     0,     0,     0,
       0,   225,   226,   450,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   335,   228,   229,   230,   231,   232,
       0,   233,   517,     0,   234,   235,   321,     0,   215,   216,
     217,   218,   219,   220,     0,     0,     0,     0,     0,     0,
     221,     0,     0,     0,  -153,   322,   510,     0,     0,     0,
       0,     0,     0,     0,     0,   324,     0,     0,   325,   326,
       0,     0,     0,     0,     0,   327,     0,     0,     0,     0,
       0,   328,   329,     0,     0,   330,   331,   223,     0,   332,
     333,     0,   334,     0,     0,     0,     0,     0,   449,     0,
       0,     0,     0,     0,   224,     0,     0,     0,     0,   225,
     226,   450,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   335,   228,   229,   230,   231,   232,     0,   233,
     517,     0,   234,   235,   321,     0,   215,   216,   217,   218,
     219,   220,     0,     0,     0,     0,     0,     0,   221,     0,
       0,     0,  -153,   322,   659,     0,     0,     0,     0,     0,
       0,     0,     0,   324,     0,     0,   325,   326,     0,     0,
       0,     0,     0,   327,     0,     0,     0,     0,     0,   328,
     329,     0,     0,   330,   331,   223,     0,   332,   333,     0,
     334,     0,     0,     0,     0,     0,   449,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   225,   226,   450,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     335,   228,   229,   230,   231,   232,     0,   233,   517,     0,
     234,   235,   321,     0,   215,   216,   217,   218,   219,   220,
       0,     0,     0,     0,     0,     0,   221,     0,     0,     0,
    -153,   322,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   324,     0,     0,   325,   326,     0,     0,     0,     0,
       0,   327,     0,     0,     0,     0,     0,   328,   329,     0,
       0,   330,   331,   223,     0,   332,   333,     0,   334,     0,
       0,     0,     0,     0,   449,     0,     0,     0,     0,     0,
     224,     0,     0,     0,     0,   225,   226,   450,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   335,   228,
     229,   230,   231,   232,     0,   233,   517,     0,   234,   235,
     321,     0,   215,   216,   217,   218,   219,   220,     0,     0,
       0,     0,     0,     0,   221,     0,     0,     0,  -153,   322,
     510,     0,     0,     0,     0,     0,     0,     0,     0,   324,
       0,     0,   325,   326,     0,     0,     0,     0,     0,   327,
       0,     0,     0,     0,     0,   328,   329,     0,     0,   330,
     331,   223,     0,   332,   333,     0,   334,     0,     0,     0,
       0,     0,   449,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   335,   228,   229,   230,
     231,   232,     0,   233,   517,     0,   234,   235,   321,     0,
     215,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,   221,     0,     0,     0,  -153,   322,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   324,     0,     0,
     325,   326,     0,     0,     0,     0,     0,   327,     0,     0,
       0,     0,     0,   328,   329,     0,     0,   330,   331,   223,
       0,   332,   333,     0,   334,     0,     0,     0,     0,     0,
     449,     0,     0,     0,     0,     0,   224,     0,   763,     0,
       0,   225,   226,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   335,   228,   229,   230,   231,   232,
       0,   233,   517,     0,   234,   235,   321,     0,   215,   216,
     217,   218,   219,   220,     0,     0,     0,     0,     0,     0,
     221,     0,     0,     0,  -153,   322,   516,     0,     0,     0,
       0,     0,     0,     0,     0,   324,     0,     0,   325,   326,
       0,     0,     0,     0,     0,   327,     0,     0,     0,     0,
       0,   328,   329,     0,     0,   330,   331,   223,     0,   332,
     333,     0,   334,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,     0,     0,   225,
     226,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   335,   228,   229,   230,   231,   232,     0,   233,
     517,     0,   234,   235,   321,     0,   215,   216,   217,   218,
     219,   220,     0,     0,     0,     0,     0,     0,   221,     0,
       0,     0,  -153,   322,   626,     0,     0,     0,     0,     0,
       0,     0,     0,   324,     0,     0,   325,   326,     0,     0,
       0,     0,     0,   327,     0,     0,     0,     0,     0,   328,
     329,     0,     0,   330,   331,   223,     0,   332,   333,     0,
     334,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   225,   226,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     335,   228,   229,   230,   231,   232,     0,   233,   517,     0,
     234,   235,   321,     0,   215,   216,   217,   218,   219,   220,
       0,     0,     0,     0,     0,     0,   221,     0,     0,     0,
    -153,   322,   510,     0,     0,     0,     0,     0,     0,     0,
       0,   324,     0,     0,   325,   326,     0,     0,     0,     0,
       0,   327,     0,     0,     0,     0,     0,   328,   329,     0,
       0,   330,   331,   223,     0,   332,   333,     0,   334,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,     0,     0,     0,     0,   225,   226,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   335,   228,
     229,   230,   231,   232,     0,   233,   517,     0,   234,   235,
     321,     0,   215,   216,   217,   218,   219,   220,     0,     0,
       0,     0,     0,     0,   221,     0,     0,     0,  -153,   322,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   324,
       0,     0,   325,   326,     0,     0,     0,     0,     0,   327,
       0,     0,     0,     0,     0,   328,   329,     0,     0,   330,
     331,   223,     0,   332,   333,     0,   334,     0,     0,     0,
       0,     0,   449,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   335,   228,   229,   230,
     231,   232,     0,   233,   517,     0,   234,   235,   321,     0,
     215,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,     0,   221,     0,     0,     0,  -153,   322,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   324,     0,     0,
     325,   326,     0,     0,     0,     0,     0,   327,     0,     0,
       0,     0,     0,   328,   329,     0,     0,   330,   331,   223,
       0,   332,   333,     0,   334,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   224,   696,     0,     0,
       0,   225,   226,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   335,   228,   229,   230,   231,   232,
       0,   233,   517,     0,   234,   235,   321,     0,   215,   216,
     217,   218,   219,   220,     0,     0,     0,     0,     0,     0,
     221,     0,     0,     0,  -153,   322,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   324,     0,     0,   325,   326,
       0,     0,     0,     0,     0,   327,     0,     0,     0,     0,
       0,   328,   329,     0,     0,   330,   331,   223,     0,   332,
     333,     0,   334,   611,     0,   215,   216,   217,   218,   219,
     220,     0,     0,     0,   224,     0,     0,   221,     0,   225,
     226,  -153,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   335,   228,   229,   230,   231,   232,     0,   233,
     517,     0,   234,   235,   711,     0,   215,   216,   217,   218,
     219,   220,     0,     0,   223,     0,     0,     0,   221,     0,
       0,     0,  -153,     0,     0,     0,     0,     0,     0,     0,
       0,   224,     0,     0,     0,     0,   225,   226,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   227,
     228,   229,   230,   231,   232,   223,   233,     0,     0,   234,
     235,   772,     0,   215,   216,   217,   218,   219,   220,     0,
       0,     0,   224,     0,     0,   221,  -153,   225,   226,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,   228,   229,   230,   231,   232,     0,   233,     0,     0,
     234,   235,     0,     0,   215,   216,   217,   218,   219,   220,
       0,     0,   223,     0,     0,     0,   221,     0,     0,     0,
       0,   529,   716,     0,     0,     0,     0,     0,     0,   224,
       0,     0,     0,     0,   225,   226,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   227,   228,   229,
     230,   231,   232,   223,   233,     0,     0,   234,   235,   215,
     216,   217,   218,   219,   220,     0,     0,     0,     0,     0,
     224,   221,     0,     0,     0,   225,   226,     0,     0,     0,
     222,     0,     0,     0,     0,     0,     0,     0,   227,   228,
     229,   230,   231,   232,     0,   233,     0,     0,   234,   235,
     215,   216,   217,   218,   219,   220,     0,     0,   223,     0,
       0,     0,   221,     0,     0,     0,     0,     0,     0,     0,
       0,   415,     0,     0,     0,   224,     0,     0,     0,     0,
     225,   226,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   227,   228,   229,   230,   231,   232,   223,
     233,     0,     0,   234,   235,     0,   215,   216,   217,   218,
     219,   220,     0,     0,     0,     0,   224,     0,   221,     0,
       0,   225,   226,   529,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   227,   228,   229,   230,   231,   232,
       0,   233,     0,     0,   234,   235,     0,   215,   216,   217,
     218,   219,   220,     0,     0,   223,     0,     0,     0,   221,
       0,     0,     0,     0,   322,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   225,   226,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,   228,   229,   230,   231,   232,   223,   233,     0,     0,
     234,   235,   215,   216,   217,   218,   219,   220,     0,     0,
       0,     0,     0,   224,   221,   544,     0,     0,   225,   226,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   227,   228,   229,   230,   231,   232,     0,   233,     0,
       0,   234,   235,   215,   216,   217,   218,   219,   220,     0,
       0,   223,     0,     0,     0,   221,   685,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   224,     0,
       0,     0,     0,   225,   226,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   227,   228,   229,   230,
     231,   232,   223,   233,     0,     0,   234,   235,   215,   216,
     217,   218,   219,   220,     0,     0,     0,     0,     0,   224,
     221,   783,     0,     0,   225,   226,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   227,   228,   229,
     230,   231,   232,     0,   233,     0,     0,   234,   235,   215,
     216,   217,   218,   219,   220,     0,     0,   223,     0,     0,
       0,   221,   785,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   224,     0,     0,     0,     0,   225,
     226,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   227,   228,   229,   230,   231,   232,   223,   233,
       0,     0,   234,   235,   215,   216,   217,   218,   219,   220,
       0,     0,     0,     0,     0,   224,   221,   833,     0,     0,
     225,   226,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   227,   228,   229,   230,   231,   232,     0,
     233,     0,     0,   234,   235,   215,   216,   217,   218,   219,
     220,     0,     0,   223,     0,     0,     0,   221,   835,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     224,     0,     0,     0,     0,   225,   226,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   227,   228,
     229,   230,   231,   232,   223,   233,     0,     0,   234,   235,
     215,   216,   217,   218,   219,   220,     0,     0,     0,     0,
       0,   224,   379,     0,     0,     0,   225,   226,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   227,
     228,   229,   230,   231,   232,     0,   233,     0,     0,   234,
     235,   215,   216,   217,   218,   219,   220,     0,     0,   223,
       0,     0,     0,   384,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   224,     0,     0,     0,
       0,   225,   226,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   227,   228,   229,   230,   231,   232,
     223,   233,     0,     0,   234,   235,   215,   216,   217,   218,
     219,   220,     0,     0,     0,     0,     0,   224,   221,     0,
       0,     0,   225,   226,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   227,   228,   229,   230,   231,
     232,     0,   233,     0,     0,   234,   235,     0,     0,     0,
       0,     0,     0,     0,     0,   223,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   224,     0,     0,     0,     0,   225,   226,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     227,   228,   229,   230,   231,   232,     0,   233,     0,     0,
     234,   235,     3,     4,     5,     6,     0,     0,     7,     0,
       0,     8,     9,    10,    11,    12,     0,    13,    14,    15,
      16,    17,     0,     0,    18,    19,     0,     0,     0,    20,
       0,     0,    21,     3,     4,     5,     6,    23,     0,     7,
     687,     0,     8,     9,    10,    11,    12,     0,    13,    14,
      15,    16,    17,     0,     0,    18,    19,     0,     0,     0,
      20,     0,     0,    21,     0,     0,     0,     0,    23,     0,
       0,    25,     0,     0,     0,     0,     0,   696,     0,     0,
       0,     3,     4,     5,     6,     0,     0,     7,     0,     0,
       8,     9,    10,    11,    12,    26,    13,    14,    15,    16,
      17,     0,    25,    18,    19,     0,     0,     0,    20,     0,
       0,    21,     3,     4,     5,     6,    23,     0,     7,     0,
       0,     8,     9,    10,    11,    12,    26,    13,    14,    15,
      16,    17,     0,     0,    18,    19,     0,     0,     0,    20,
       0,     0,    21,     4,     5,     6,     0,     0,     7,     0,
      25,     8,     0,     0,    11,     0,     0,    13,    14,    15,
      16,    17,     0,     0,    18,    19,     0,     0,     0,    20,
       0,     4,     5,     6,    26,     0,     7,     0,     0,     8,
       0,    25,    11,     0,     0,    13,    14,    15,    16,    17,
       0,     0,    18,    19,     0,     0,     0,    20,     0,     0,
       0,     0,     0,     0,     0,    26,     0,     0,     0,     0,
       0,    25,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    26,     0,     0,     0,   124,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    26
};

static const yytype_int16 yycheck[] =
{
       0,   221,   200,    62,   461,     0,     0,    86,    87,    39,
       0,    48,   200,   280,    38,   134,   346,   133,    86,    43,
     345,   544,    76,   372,   205,   353,   207,   208,     0,   210,
      67,    74,    86,   361,   267,     8,   640,   687,   353,   255,
       2,    18,    79,    87,   239,   161,   361,    87,   378,   584,
     585,    88,    82,    21,    54,    35,    36,    37,   274,    54,
      54,   171,     9,    42,    54,   175,     0,     4,   591,   327,
     279,   280,   529,    19,    20,   725,    22,    10,    15,    52,
      74,    86,    54,    19,    20,    20,    22,    15,   266,    22,
     133,    22,   171,   172,    25,    23,   175,   140,    20,   287,
     159,     4,    74,    15,   172,     9,    10,    20,    87,   146,
     326,   148,    15,   172,    16,    77,    95,    26,   161,    87,
      23,    19,   645,   160,    22,    87,    16,    36,    37,   106,
      39,   171,   457,    42,   158,   175,   255,   205,   133,   133,
     208,   178,     4,   133,    53,    20,   140,    56,   346,   347,
      87,    15,    87,    15,    15,   274,    20,   345,   762,   379,
      95,   133,   108,    25,   384,    20,   161,   161,   140,   389,
     286,   161,   108,     1,   120,    87,     4,    20,   221,    19,
     378,   704,    22,   207,   120,   295,   210,    15,   513,   161,
     378,   411,    23,   728,   375,   376,    87,   101,   102,   394,
     395,   396,   435,     4,    95,   421,   534,   326,   538,   171,
     119,    16,    87,   175,    15,    74,   295,    22,    16,   534,
      95,    19,    23,   680,   267,    87,   206,   221,   271,   487,
      22,   261,    24,    95,   108,   265,   260,   353,   262,   588,
     283,    42,   266,   286,   422,   361,   221,   425,   285,   221,
      67,    68,    53,   431,    22,   295,   372,   294,    20,    87,
      77,   298,   299,   263,   301,   181,     0,    95,     1,   185,
     186,     4,   492,   267,   190,   191,    42,   271,    20,   195,
     196,   140,    15,    69,   352,    20,    87,    53,    74,   283,
     488,   286,   286,   491,    28,   267,   286,   495,   352,   271,
      34,    87,   421,   276,    38,    39,   360,    15,    96,    43,
     353,   283,     4,   353,   286,    23,   353,   515,   361,     4,
      54,   109,     4,    15,   361,    20,   114,    87,    62,   372,
      15,    13,    14,   295,     4,    95,   379,    20,   519,   127,
     538,   539,   537,   524,    19,    15,   346,    22,    82,    87,
      84,    20,   376,   119,    87,    25,    20,    95,   353,   353,
      62,   134,   221,   353,   683,   120,   361,   361,   698,    59,
      60,   361,   288,   289,    16,   584,   585,   372,   372,   698,
      22,   353,   372,    16,   651,   379,   653,    16,   608,   361,
     720,   353,   435,    22,     4,    87,     5,     6,   618,   361,
     372,   720,    87,   433,   379,    15,    22,   379,   267,   384,
      95,   609,   271,   671,   672,    16,   735,    87,   534,    87,
       1,   619,    12,     4,   283,   159,    19,    95,   465,    22,
      16,    59,    42,    61,    15,   754,    22,   635,   172,    15,
      16,   435,   651,    53,   653,    15,    16,    23,    16,    16,
     223,   519,   633,    23,    22,    22,   644,   457,   656,     4,
     519,   234,   235,   435,    16,   524,   239,   655,   656,     4,
      15,   205,   588,   207,   208,    16,   210,    87,    23,   533,
      15,    22,   255,   108,   373,    95,   684,   707,   746,   747,
     748,   534,    15,   713,    15,   683,   684,   534,   387,   697,
     698,   274,   700,    16,   702,    15,    87,    42,   424,    22,
     708,   427,     3,   513,    95,    74,    11,   715,    53,   728,
     379,   719,   720,   721,   722,   259,   260,   261,   262,   648,
      19,   265,   266,    22,   105,   733,   734,   735,   538,   534,
     534,   739,    87,    24,   534,   588,    99,   100,    88,   586,
      95,    21,    87,   326,   752,   753,   754,   451,   756,     4,
      95,    61,   534,    15,    15,    17,   108,   765,   766,   544,
      15,    23,   461,    16,   633,    16,   435,   306,    23,   777,
     778,   140,   311,    16,    16,     1,    15,   316,     4,    22,
      22,   628,   790,   588,   588,    15,    16,    42,   588,    15,
     798,    16,    19,    23,     0,     1,     2,    22,    53,   829,
     830,    19,    15,    16,   589,   590,   588,   511,   507,   508,
      23,   394,   395,   396,   397,   398,   399,   400,   401,   402,
     403,   404,   405,   406,   407,   408,   409,   410,    34,   412,
     529,    19,    38,    39,    96,    97,    98,    43,   421,     0,
       1,    15,    16,   103,   104,   752,    25,   573,    54,    23,
     576,    16,   221,   638,   639,    15,    62,    22,   765,    35,
     645,    87,    38,    25,    40,    19,    16,    43,    25,    95,
     777,    77,    22,    34,    60,    61,    82,    38,    39,   689,
      86,    87,    43,   790,    21,    22,    22,   431,   698,   433,
      87,   798,   702,    54,   598,    72,    73,    16,   267,   397,
     398,    62,   271,   399,   400,   511,    87,   513,    16,    19,
     720,     4,   722,    24,   283,    87,    26,   405,   406,    25,
     624,    82,    15,    21,     4,    86,    36,    37,    87,    39,
      23,   470,    42,   539,   473,    15,    22,   476,    22,    16,
     479,   640,    15,    53,    15,    15,    56,   646,   154,    42,
      35,    36,    37,    38,   537,    40,    41,    42,    43,    25,
      53,    19,    42,    16,    19,   171,   172,    19,    15,   175,
     674,    16,    16,    53,    16,   519,    16,    16,    15,   155,
     524,   680,   158,    87,    20,    35,    36,    37,    38,    19,
      40,    41,    42,    43,    87,    57,    19,   401,   402,   403,
     404,    16,    95,    26,    16,   709,    19,    87,    15,   119,
     379,   172,    15,    36,    37,    16,    39,    65,   624,    42,
      22,    27,    28,    16,    19,    19,    32,    19,    66,   635,
      53,   207,    38,    56,   210,    31,    42,    43,    44,    45,
      15,    15,    48,    49,    20,    73,   750,    53,    19,    21,
      16,    16,    16,   259,    16,   261,    19,    16,    87,   265,
     599,   600,   761,   762,    19,   648,   435,    19,    19,    19,
     155,   156,   157,   158,    15,    15,    15,    19,   284,   684,
      16,   137,   763,   645,   260,   689,   262,   727,   407,   295,
     266,   800,   697,   760,   700,   408,   119,   702,   259,   410,
     261,   409,   379,   412,   265,   809,   810,   811,   812,   292,
     425,   706,   433,   119,   719,   721,   286,   722,   271,   431,
     151,   206,   207,   437,   209,   210,   182,   183,    54,   734,
     278,   187,   188,   739,    -1,    -1,   192,   193,    -1,    -1,
       4,   197,   198,    -1,    -1,    -1,    -1,   353,   753,    -1,
     756,    15,    -1,   759,    -1,   361,   206,   207,   764,   209,
     210,   766,    -1,    -1,    -1,   656,    -1,    -1,    -1,    -1,
     776,    -1,    -1,   778,    -1,   260,   782,   262,    42,    -1,
      -1,   266,    -1,   789,    -1,    -1,    -1,    -1,    -1,    53,
      -1,   797,    -1,   684,    -1,    -1,    -1,    -1,    -1,    -1,
     376,   257,   808,    -1,    -1,    -1,   697,    -1,   814,   700,
     260,   702,   262,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    -1,    -1,    -1,   433,   719,    -1,
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
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    -1,    97,    98,     1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    -1,    -1,    -1,    19,    20,   119,    -1,
      -1,    -1,    -1,    26,    27,    28,    29,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    -1,    -1,    -1,    61,    -1,
      63,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    -1,    97,    98,    -1,    -1,    -1,     1,
      -1,     3,     4,     5,     6,     7,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    -1,    -1,   119,    19,    20,    21,
      -1,    -1,    -1,    -1,    26,    27,    28,    29,    30,    -1,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    -1,    -1,    -1,    61,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,
      -1,    -1,    74,    75,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,    87,    88,    89,    90,    91,
      92,    -1,    94,    95,    -1,    97,    98,    -1,    -1,    -1,
       1,    -1,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    -1,    -1,   119,    19,    20,
      21,    -1,    -1,    -1,    -1,    26,    27,    28,    29,    30,
      -1,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    -1,    -1,    -1,
      61,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    -1,    97,    98,     1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    -1,    -1,    -1,    19,    20,   119,    -1,
      -1,    -1,    -1,    26,    27,    28,    29,    30,    -1,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    -1,    -1,    -1,    61,    -1,
      63,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    -1,    97,    98,     1,    -1,     3,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    -1,    -1,    -1,    19,    20,   119,    -1,    -1,    -1,
      -1,    26,    27,    28,    29,    30,    -1,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    -1,    61,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    70,    -1,    -1,    -1,    74,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    -1,    94,
      95,    -1,    97,    98,     1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,
      -1,    -1,    19,    20,   119,    -1,    -1,    -1,    -1,    26,
      27,    28,    29,    30,    -1,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    -1,    61,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    -1,    94,    95,     1,
      97,    98,     4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   119,    -1,    26,    27,    28,    29,    -1,    -1,
      32,    -1,    -1,    35,    36,    37,    38,    39,    -1,    41,
      42,    43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,
      -1,    53,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,
      -1,     1,    -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    26,    27,    28,    29,
      -1,    -1,    32,    95,    -1,    35,    36,    37,    38,    39,
      -1,    41,    42,    43,    44,    45,    -1,    -1,    48,    49,
      -1,    -1,    -1,    53,    -1,    -1,    56,   119,    -1,    -1,
      -1,    -1,    -1,     1,    -1,    -1,     4,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    26,    27,
      28,    29,    -1,    -1,    32,    95,    -1,    35,    36,    37,
      38,    39,    -1,    41,    42,    43,    44,    45,    -1,    -1,
      48,    49,    -1,    -1,    -1,    53,    -1,    -1,    56,   119,
      -1,    -1,    -1,    -1,    -1,     1,    -1,    -1,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    15,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      26,    27,    28,    29,    -1,    -1,    32,    95,     1,    35,
      36,    37,    38,    39,    -1,    41,    42,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    -1,    -1,    53,    -1,    -1,
      56,   119,    -1,    26,    27,    28,    29,    -1,    -1,    32,
       1,    -1,    35,    36,    37,    38,    39,    -1,    41,    42,
      43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,    -1,
      53,    87,    -1,    56,    -1,    26,    27,    28,    29,    95,
      -1,    32,    -1,    -1,    35,    36,    37,    38,    39,    -1,
      41,    42,    43,    44,    45,    -1,    -1,    48,    49,    -1,
      -1,    -1,    53,   119,    -1,    56,    -1,    -1,    -1,    -1,
      -1,    -1,    95,     3,     4,     5,     6,     7,     8,    -1,
      -1,    -1,    -1,    -1,   107,    15,    -1,    -1,    -1,    -1,
      20,    -1,    -1,    -1,    -1,    -1,   119,    27,    28,    29,
      -1,    -1,    32,    -1,    95,    35,    -1,    -1,    38,    -1,
      -1,    41,    42,    43,    44,    45,    -1,    -1,    48,    49,
      -1,    -1,    52,    53,    26,    27,    28,    -1,   119,    -1,
      32,    -1,    -1,    -1,    36,    37,    38,    39,    -1,    69,
      42,    43,    44,    45,    74,    75,    48,    49,    -1,    -1,
      -1,    53,    -1,    -1,    56,    -1,    -1,    87,    88,    89,
      90,    91,    92,     4,    94,    95,    -1,    97,    98,    -1,
      -1,    -1,    -1,    -1,    15,    16,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    -1,    -1,    26,    27,    28,    29,   119,
      -1,    32,    -1,    -1,    35,    36,    37,    38,    39,     4,
      41,    42,    43,    44,    45,    -1,    -1,    48,    49,    -1,
      15,    16,    53,    -1,    -1,    56,    -1,   119,    23,    -1,
      -1,    26,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
      35,    36,    37,    38,    39,    -1,    41,    42,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    87,    -1,    53,    -1,
      -1,    56,    -1,    -1,    95,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   107,    -1,    -1,    -1,
      -1,    -1,     4,    -1,    -1,    -1,    -1,    -1,   119,    -1,
      -1,    -1,    87,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      95,    -1,    -1,    -1,    26,    27,    28,    29,    -1,    -1,
      32,    -1,   107,    35,    36,    37,    38,    39,     4,    41,
      42,    43,    44,    45,   119,    -1,    48,    49,    -1,    15,
      16,    53,    -1,    -1,    56,    -1,    58,    23,    -1,    61,
      26,    27,    28,    29,    -1,    -1,    32,    -1,    -1,    35,
      36,    37,    38,    39,    -1,    41,    42,    43,    44,    45,
      -1,     4,    48,    49,    -1,    87,    -1,    53,    -1,    -1,
      56,    -1,    15,    95,    -1,    -1,    -1,    -1,    -1,    -1,
      23,    -1,    -1,    26,    27,    28,    29,    -1,    -1,    32,
      -1,    -1,    35,    36,    37,    38,    39,   119,    41,    42,
      43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,    95,
      53,    -1,    -1,    56,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   107,    -1,     4,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119,    15,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    23,    -1,    87,    26,    27,    28,    29,    -1,
      -1,    32,    95,    -1,    35,    36,    37,    38,    39,     4,
      41,    42,    43,    44,    45,    -1,    -1,    48,    49,    -1,
      15,    -1,    53,    -1,    -1,    56,   119,    -1,    -1,    -1,
      25,    -1,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
      35,    -1,    -1,    38,    -1,    -1,    41,    42,    43,    44,
      45,    -1,    -1,    48,    49,    -1,    87,    -1,    53,    -1,
      -1,    -1,    -1,    -1,    95,    -1,    -1,    -1,    -1,    -1,
       4,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,   119,    23,
      -1,    -1,    87,    27,    28,    29,    -1,    -1,    32,    -1,
      95,    35,    -1,    -1,    38,    -1,    -1,    41,    42,    43,
      44,    45,    -1,    -1,    48,    49,    16,    -1,    -1,    53,
      -1,    -1,    -1,    -1,   119,    -1,    26,    27,    28,    29,
      -1,    -1,    32,    -1,    -1,    35,    36,    37,    38,    39,
      -1,    41,    42,    43,    44,    45,    -1,    -1,    48,    49,
      -1,    21,    -1,    53,    -1,    -1,    56,    27,    28,    29,
      -1,    95,    32,    -1,    -1,    35,    -1,    -1,    38,    -1,
      -1,    41,    42,    43,    44,    45,    -1,    -1,    48,    49,
      -1,    -1,    -1,    53,    -1,   119,    -1,    87,    -1,    16,
      -1,    -1,    -1,    -1,    -1,    95,    -1,    -1,    -1,    26,
      27,    28,    29,    -1,    -1,    32,    -1,   107,    35,    36,
      37,    38,    39,    -1,    41,    42,    43,    44,    45,   119,
      -1,    48,    49,    -1,    21,    95,    53,    -1,    -1,    56,
      27,    28,    29,    -1,    -1,    32,    -1,    -1,    35,    -1,
      -1,    38,    -1,    -1,    41,    42,    43,    44,    45,   119,
      -1,    48,    49,    -1,    -1,    21,    53,    -1,    -1,    -1,
      -1,    27,    28,    29,    -1,    -1,    32,    -1,    95,    35,
      -1,    -1,    38,    -1,    -1,    41,    42,    43,    44,    45,
     107,    -1,    48,    49,    -1,    -1,    -1,    53,    21,    -1,
      -1,    -1,   119,    -1,    27,    28,    29,    -1,    95,    32,
      -1,    -1,    35,    -1,    -1,    38,    -1,    -1,    41,    42,
      43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,    -1,
      53,    -1,   119,    -1,    -1,    -1,    -1,    -1,     1,    95,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    -1,    -1,    -1,    19,    20,    21,    -1,
      -1,    -1,    -1,   119,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    95,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,    52,
      -1,    54,    55,    -1,    57,    -1,   119,    -1,    -1,    -1,
      63,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    74,    75,    76,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    -1,    97,    98,     1,    -1,     3,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    -1,    -1,    -1,    19,    20,    21,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,
      -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    50,    51,    52,    -1,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    63,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    74,
      75,    76,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    -1,    94,
      95,    -1,    97,    98,     1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,
      -1,    -1,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    50,    51,    52,    -1,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,    76,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    -1,    94,    95,    -1,
      97,    98,     1,    -1,     3,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,
      19,    20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    33,    34,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    50,    51,    52,    -1,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    74,    75,    76,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    -1,    94,    95,    -1,    97,    98,
       1,    -1,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,    19,    20,
      21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,
      51,    52,    -1,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    -1,    97,    98,     1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    -1,    -1,    -1,    19,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,    52,
      -1,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      63,    -1,    -1,    -1,    -1,    -1,    69,    -1,    71,    -1,
      -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    -1,    97,    98,     1,    -1,     3,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    -1,    -1,    -1,    19,    20,    21,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,
      -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    50,    51,    52,    -1,    54,
      55,    -1,    57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    74,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    -1,    94,
      95,    -1,    97,    98,     1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,
      -1,    -1,    19,    20,    21,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    -1,    33,    34,    -1,    -1,
      -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,    -1,    46,
      47,    -1,    -1,    50,    51,    52,    -1,    54,    55,    -1,
      57,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    -1,    94,    95,    -1,
      97,    98,     1,    -1,     3,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,
      19,    20,    21,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    -1,    33,    34,    -1,    -1,    -1,    -1,
      -1,    40,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      -1,    50,    51,    52,    -1,    54,    55,    -1,    57,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    -1,    94,    95,    -1,    97,    98,
       1,    -1,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    -1,    15,    -1,    -1,    -1,    19,    20,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,
      -1,    -1,    33,    34,    -1,    -1,    -1,    -1,    -1,    40,
      -1,    -1,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,
      51,    52,    -1,    54,    55,    -1,    57,    -1,    -1,    -1,
      -1,    -1,    63,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    -1,    94,    95,    -1,    97,    98,     1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    -1,    15,    -1,    -1,    -1,    19,    20,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,
      33,    34,    -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,
      -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,    52,
      -1,    54,    55,    -1,    57,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    -1,    -1,
      -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      -1,    94,    95,    -1,    97,    98,     1,    -1,     3,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,    -1,
      15,    -1,    -1,    -1,    19,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    30,    -1,    -1,    33,    34,
      -1,    -1,    -1,    -1,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    46,    47,    -1,    -1,    50,    51,    52,    -1,    54,
      55,    -1,    57,     1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    -1,    -1,    69,    -1,    -1,    15,    -1,    74,
      75,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    -1,    94,
      95,    -1,    97,    98,     1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    52,    -1,    -1,    -1,    15,    -1,
      -1,    -1,    19,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    74,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    52,    94,    -1,    -1,    97,
      98,     1,    -1,     3,     4,     5,     6,     7,     8,    -1,
      -1,    -1,    69,    -1,    -1,    15,    16,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    -1,    94,    -1,    -1,
      97,    98,    -1,    -1,     3,     4,     5,     6,     7,     8,
      -1,    -1,    52,    -1,    -1,    -1,    15,    -1,    -1,    -1,
      -1,    20,    21,    -1,    -1,    -1,    -1,    -1,    -1,    69,
      -1,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,
      90,    91,    92,    52,    94,    -1,    -1,    97,    98,     3,
       4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,
      69,    15,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,
      24,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    -1,    94,    -1,    -1,    97,    98,
       3,     4,     5,     6,     7,     8,    -1,    -1,    52,    -1,
      -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    24,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    89,    90,    91,    92,    52,
      94,    -1,    -1,    97,    98,    -1,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    69,    -1,    15,    -1,
      -1,    74,    75,    20,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      -1,    94,    -1,    -1,    97,    98,    -1,     3,     4,     5,
       6,     7,     8,    -1,    -1,    52,    -1,    -1,    -1,    15,
      -1,    -1,    -1,    -1,    20,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    52,    94,    -1,    -1,
      97,    98,     3,     4,     5,     6,     7,     8,    -1,    -1,
      -1,    -1,    -1,    69,    15,    16,    -1,    -1,    74,    75,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    87,    88,    89,    90,    91,    92,    -1,    94,    -1,
      -1,    97,    98,     3,     4,     5,     6,     7,     8,    -1,
      -1,    52,    -1,    -1,    -1,    15,    16,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,
      91,    92,    52,    94,    -1,    -1,    97,    98,     3,     4,
       5,     6,     7,     8,    -1,    -1,    -1,    -1,    -1,    69,
      15,    16,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,    89,
      90,    91,    92,    -1,    94,    -1,    -1,    97,    98,     3,
       4,     5,     6,     7,     8,    -1,    -1,    52,    -1,    -1,
      -1,    15,    16,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    74,
      75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    87,    88,    89,    90,    91,    92,    52,    94,
      -1,    -1,    97,    98,     3,     4,     5,     6,     7,     8,
      -1,    -1,    -1,    -1,    -1,    69,    15,    16,    -1,    -1,
      74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    87,    88,    89,    90,    91,    92,    -1,
      94,    -1,    -1,    97,    98,     3,     4,     5,     6,     7,
       8,    -1,    -1,    52,    -1,    -1,    -1,    15,    16,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    74,    75,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,    88,
      89,    90,    91,    92,    52,    94,    -1,    -1,    97,    98,
       3,     4,     5,     6,     7,     8,    -1,    -1,    -1,    -1,
      -1,    69,    15,    -1,    -1,    -1,    74,    75,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    87,
      88,    89,    90,    91,    92,    -1,    94,    -1,    -1,    97,
      98,     3,     4,     5,     6,     7,     8,    -1,    -1,    52,
      -1,    -1,    -1,    15,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,    92,
      52,    94,    -1,    -1,    97,    98,     3,     4,     5,     6,
       7,     8,    -1,    -1,    -1,    -1,    -1,    69,    15,    -1,
      -1,    -1,    74,    75,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    87,    88,    89,    90,    91,
      92,    -1,    94,    -1,    -1,    97,    98,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    -1,    74,    75,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      87,    88,    89,    90,    91,    92,    -1,    94,    -1,    -1,
      97,    98,    26,    27,    28,    29,    -1,    -1,    32,    -1,
      -1,    35,    36,    37,    38,    39,    -1,    41,    42,    43,
      44,    45,    -1,    -1,    48,    49,    -1,    -1,    -1,    53,
      -1,    -1,    56,    26,    27,    28,    29,    61,    -1,    32,
      64,    -1,    35,    36,    37,    38,    39,    -1,    41,    42,
      43,    44,    45,    -1,    -1,    48,    49,    -1,    -1,    -1,
      53,    -1,    -1,    56,    -1,    -1,    -1,    -1,    61,    -1,
      -1,    95,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,
      -1,    26,    27,    28,    29,    -1,    -1,    32,    -1,    -1,
      35,    36,    37,    38,    39,   119,    41,    42,    43,    44,
      45,    -1,    95,    48,    49,    -1,    -1,    -1,    53,    -1,
      -1,    56,    26,    27,    28,    29,    61,    -1,    32,    -1,
      -1,    35,    36,    37,    38,    39,   119,    41,    42,    43,
      44,    45,    -1,    -1,    48,    49,    -1,    -1,    -1,    53,
      -1,    -1,    56,    27,    28,    29,    -1,    -1,    32,    -1,
      95,    35,    -1,    -1,    38,    -1,    -1,    41,    42,    43,
      44,    45,    -1,    -1,    48,    49,    -1,    -1,    -1,    53,
      -1,    27,    28,    29,   119,    -1,    32,    -1,    -1,    35,
      -1,    95,    38,    -1,    -1,    41,    42,    43,    44,    45,
      -1,    -1,    48,    49,    -1,    -1,    -1,    53,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    -1,
      -1,    95,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   119,    -1,    -1,    -1,    95,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   119
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     4,    15,    26,    27,    28,    29,    32,    35,    36,
      37,    38,    39,    41,    42,    43,    44,    45,    48,    49,
      53,    56,    58,    61,    87,    95,   119,   122,   123,   125,
     127,   175,   176,   185,   194,   195,   196,   197,   198,   200,
     201,   202,   203,   204,   206,   218,   219,   220,   221,   222,
     223,   240,   241,   250,   276,   277,   278,   298,   300,   301,
      42,    53,   205,   218,   222,   299,   219,   221,   222,    20,
      87,    95,   225,    87,     9,     0,     4,    15,   218,   221,
     126,    19,    22,    19,    22,     1,     4,    15,    95,   207,
     208,   209,   211,   212,   213,   218,   222,   199,   298,   300,
     301,    19,   199,   199,     1,    95,   199,   206,   218,   222,
     301,     1,   207,   218,   222,   298,   300,   301,    19,   298,
     300,   298,   300,     1,    95,   206,   218,   222,   298,   300,
     301,    86,   279,    15,    23,   217,   239,   284,   294,   295,
      20,   225,   277,   218,   222,   299,    16,    16,    16,   225,
     251,    20,    15,   124,   200,   201,   202,   203,   204,   205,
     221,    15,    20,   131,     1,   207,   218,   222,     1,   218,
     183,    15,   205,   208,   211,    15,    95,   208,   210,   212,
     217,   177,   280,   285,   294,   192,   186,   282,   287,   294,
     184,   179,   281,   286,   294,   193,   188,   283,   288,   294,
      20,   261,    16,    87,   107,   194,   196,   198,   200,   202,
     204,   224,   236,   237,   238,     3,     4,     5,     6,     7,
       8,    15,    24,    52,    69,    74,    75,    87,    88,    89,
      90,    91,    92,    94,    97,    98,   153,   154,   156,   157,
     158,   159,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   173,   296,   297,    23,   261,   289,   175,   194,
     198,   200,   204,   262,    21,   200,   204,   242,   243,   244,
     245,    20,   217,   217,   108,   252,    22,   253,   251,    59,
      60,   128,    10,    22,    15,   221,    62,   132,   181,   190,
     120,   227,   228,   229,   210,    15,   208,   211,   210,    16,
     217,    16,   227,   261,   261,   290,   227,   227,   261,   261,
     292,   227,   227,   261,   261,   291,   227,   227,   261,   261,
     293,     1,    20,    21,    30,    33,    34,    40,    46,    47,
      50,    51,    54,    55,    57,    87,    95,   156,   169,   170,
     172,   174,   255,   256,   258,   259,   262,   263,   264,   265,
     267,   275,     4,    15,   211,   214,   215,   216,   217,   218,
       4,    15,   214,   218,   211,   214,   218,   214,   218,    16,
      22,    16,    22,   108,   172,   200,   204,   226,   259,    15,
     156,    15,    15,    15,    15,   156,   156,    15,    17,    23,
      96,    97,    98,   158,     4,    13,    14,     5,     6,    99,
     100,     9,    10,   101,   102,   103,   104,     3,    12,    11,
     105,    18,   106,    24,    88,    24,   173,   261,   218,   218,
     175,    25,   207,   246,   249,   218,   247,   249,    21,   243,
      19,    22,    19,    22,    21,   242,   173,   225,    21,   253,
     123,   129,   130,   129,    16,   200,   221,   217,   237,    63,
      76,   133,   134,   135,   136,   254,   255,   262,   227,   227,
      15,   108,   233,   229,    16,   210,    16,   217,    16,   217,
     178,   261,   233,   187,   261,   233,   180,   261,   233,   189,
     261,    19,    19,   266,   173,   174,    19,    15,    25,    87,
      95,   269,    15,   268,    25,    25,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   171,    22,    19,
      21,   133,   260,   262,    21,   263,    21,    95,   255,   205,
     214,   215,   216,   217,   205,    87,     1,   107,   238,    20,
     170,   234,    16,     4,    15,   214,   214,    16,   262,   263,
     226,    87,    87,    16,    16,   155,   170,    87,    95,   172,
      87,    95,   158,   158,   158,   159,   159,   160,   160,   161,
     161,   161,   161,   162,   162,   163,   164,   165,   166,   172,
     167,    24,   173,   248,   249,   227,   248,   227,   247,   246,
      21,   252,    21,    87,    22,    22,   221,    19,    22,    69,
      74,    87,   137,   138,   144,    15,    21,   254,   133,   182,
     191,    15,   234,    16,   233,   233,   233,   233,    15,    25,
      19,     1,   174,   255,    19,    19,   255,   172,    15,   257,
     255,   170,   170,   260,   133,   260,    21,   214,    16,    16,
      16,   234,   235,   205,   158,   263,   260,    16,    16,    16,
      15,   145,   145,   146,   259,    16,    22,    24,    25,   227,
     227,    60,   130,    59,   217,   259,   259,   145,    87,    21,
     233,   233,    42,    87,    95,   230,   231,   232,   172,   255,
      16,    19,    19,    57,    16,   172,   255,   260,   217,    21,
      22,   260,    16,   259,   259,    16,   155,    64,   147,   262,
     145,   146,   170,   169,   129,   129,    70,   139,   262,   136,
     140,   255,   262,    19,    16,    16,    22,    15,    16,   274,
     174,     1,   174,    15,   254,    16,    21,   234,    16,   139,
     262,   140,   262,    16,   258,    65,   148,   147,    22,    72,
      73,   142,   143,   140,   139,   263,   136,   255,   260,   140,
     145,    16,   231,   172,   255,   254,    19,    19,    19,   172,
      31,   255,   140,   139,   263,   260,   140,   258,    66,   149,
     148,    15,    15,    71,   141,   140,   139,   260,    19,    16,
     174,   174,     1,   174,    16,   254,   141,   140,   139,   260,
      20,   260,   149,    16,   170,    16,   155,   142,   260,   141,
     140,    16,    16,    16,    16,    19,   260,   141,   140,    87,
     150,   151,   260,    19,    16,    19,    16,   260,   141,   271,
     273,   272,   270,   260,   141,    67,    68,    77,   152,    21,
     151,    19,    19,   260,   254,   254,   254,   254,   260,    15,
      15,    15,    19,    16,   172,    16,   172,    16,    16,    16
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
#line 388 "./src/ANSI-C.y"
    { Program = GrabPragmas((yyvsp[(1) - (1)].L)); }
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 400 "./src/ANSI-C.y"
    {
			(yyval.n) = SetSTRdclNameFields(MakeSTRdclCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (4)].tok)), NULL, (yyvsp[(3) - (4)].L), (yyvsp[(2) - (4)].tok), (yyvsp[(4) - (4)].tok));
		}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 407 "./src/ANSI-C.y"
    { 
			(yyval.L) = MakeNewList(SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Stream));	
		}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 411 "./src/ANSI-C.y"
    { 
			(yyval.L) = MakeNewList(SetDeclType( ModifyDeclType((yyvsp[(2) - (3)].n),(yyvsp[(3) - (3)].n)),(yyvsp[(1) - (3)].n), Stream));
			// $$ = MakeNewList(SetDeclTypeMult($2, $4, $1, Stream));
			//$$ = MakeNewList(SetDeclType($2, $1, Stream));	
		}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 417 "./src/ANSI-C.y"
    { 
			(yyval.L) = JoinLists((yyvsp[(1) - (4)].L), MakeNewList(SetDeclType((yyvsp[(4) - (4)].n), (yyvsp[(3) - (4)].n), Stream)));
		}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 421 "./src/ANSI-C.y"
    { 
			(yyval.L) = JoinLists((yyvsp[(1) - (5)].L), MakeNewList(SetDeclType(ModifyDeclType((yyvsp[(4) - (5)].n), (yyvsp[(5) - (5)].n)), (yyvsp[(3) - (5)].n), Stream)));
			//$$ = JoinLists($1, MakeNewList(SetDeclTypeMult($4, $6, $3, Stream)));
			//$$ = JoinLists($1, MakeNewList(SetDeclType($4, $3, Stream)));
		}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 432 "./src/ANSI-C.y"
    {
			IsInCompositeParam = TRUE;
			(yyvsp[(1) - (1)].n) = DefineComposite((yyvsp[(1) - (1)].n));
		}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 437 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found composite.definition!\n\n");
			(yyval.n) = SetCompositeBody((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n));
			
		}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 447 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found composite.head!\n");
			(yyval.n) = SetDeclType(ModifyDeclType(ConvertIdToDecl((yyvsp[(2) - (5)].n), EMPTY_TQ, NULL, NULL, NULL), MakeComdclCoord(EMPTY_TQ, (yyvsp[(4) - (5)].n), (yyvsp[(3) - (5)].tok))),
								MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (5)].tok)), 
								Redecl);
		}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 456 "./src/ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 458 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeComInOutCoord(EMPTY_TQ, (yyvsp[(2) - (5)].L), (yyvsp[(5) - (5)].L), (yyvsp[(1) - (5)].tok));
		}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 462 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeComInOutCoord(EMPTY_TQ, (yyvsp[(2) - (2)].L), NULL, (yyvsp[(1) - (2)].tok));
		}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 466 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeComInOutCoord(EMPTY_TQ, NULL, (yyvsp[(2) - (2)].L), (yyvsp[(1) - (2)].tok));
		}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 470 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeComInOutCoord(EMPTY_TQ, (yyvsp[(5) - (5)].L), (yyvsp[(2) - (5)].L), (yyvsp[(1) - (5)].tok));
		}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 477 "./src/ANSI-C.y"
    {
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 481 "./src/ANSI-C.y"
    {
			(yyval.L) = AppendItem((yyvsp[(1) - (3)].L),(yyvsp[(3) - (3)].n));
		}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 488 "./src/ANSI-C.y"
    {
			(yyval.n) = SetDeclType(ConvertIdToDecl((yyvsp[(2) - (2)].n), EMPTY_TQ, NULL, NULL, NULL), (yyvsp[(1) - (2)].n), Commal);
		}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 496 "./src/ANSI-C.y"
    {
			/////*DEBUG*/printf("have found composite.body!\n");
			(yyval.n) = MakeComBodyCoord(PrimVoid, (yyvsp[(2) - (4)].n), NULL,(yyvsp[(3) - (4)].L), (yyvsp[(1) - (4)].tok), (yyvsp[(4) - (4)].tok));
		}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 501 "./src/ANSI-C.y"
    {
			/////*DEBUG*/printf("have found composite.body!\n");
			(yyval.n) = MakeComBodyCoord(PrimVoid, (yyvsp[(2) - (5)].n), (yyvsp[(3) - (5)].L),(yyvsp[(4) - (5)].L), (yyvsp[(1) - (5)].tok), (yyvsp[(5) - (5)].tok));
		}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 509 "./src/ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 511 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found composite.body.param!\n");
			(yyval.n) = MakeParamCoord((yyvsp[(2) - (3)].L), (yyvsp[(1) - (3)].tok));
			IsInCompositeParam = FALSE;
		}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 520 "./src/ANSI-C.y"
    {
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 524 "./src/ANSI-C.y"
    {
			(yyval.L) = AppendItem((yyvsp[(1) - (2)].L),(yyvsp[(2) - (2)].n));
		}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 534 "./src/ANSI-C.y"
    {
			(yyval.n) = (yyvsp[(1) - (1)].n);
		}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 538 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found operator.files\n");
			(yyval.n) = (yyvsp[(1) - (1)].n);
		}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 546 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeFileWriterOperator((yyvsp[(3) - (6)].n), (yyvsp[(5) - (6)].L), (yyvsp[(1) - (6)].tok));
		}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 553 "./src/ANSI-C.y"
    {
			  (yyval.n) = MakeAddCoord((yyvsp[(2) - (2)].n),(yyvsp[(1) - (2)].tok));
		  }
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 557 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeAddCoord((yyvsp[(2) - (2)].n),(yyvsp[(1) - (2)].tok));
		}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 561 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeAddCoord((yyvsp[(2) - (2)].n),(yyvsp[(1) - (2)].tok));
		}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 568 "./src/ANSI-C.y"
    {
			(yyval.n) = MakePipelineCoord(NULL,NULL,NULL,(yyvsp[(3) - (4)].L),(yyvsp[(1) - (4)].tok));
		}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 572 "./src/ANSI-C.y"
    {
			(yyval.n) = MakePipelineCoord(NULL,NULL,(yyvsp[(3) - (5)].L),(yyvsp[(4) - (5)].L),(yyvsp[(1) - (5)].tok));
		}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 580 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL,NULL,NULL,NULL,(yyvsp[(3) - (6)].n),(yyvsp[(4) - (6)].L),(yyvsp[(5) - (6)].n),(yyvsp[(1) - (6)].tok));
		}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 584 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL,NULL,(yyvsp[(3) - (7)].L),NULL,(yyvsp[(4) - (7)].n),(yyvsp[(5) - (7)].L),(yyvsp[(6) - (7)].n),(yyvsp[(1) - (7)].tok));
		}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 588 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL,NULL,(yyvsp[(3) - (8)].L),(yyvsp[(4) - (8)].L),(yyvsp[(5) - (8)].n),(yyvsp[(6) - (8)].L),(yyvsp[(7) - (8)].n),(yyvsp[(1) - (8)].tok));
		}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 595 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found SPLIT duplicate.statement \n");
			(yyval.n) = MakeSplitCoord((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok));
		}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 600 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found SPLIT roundrobin.statement \n");
			(yyval.n) = MakeSplitCoord((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok));
		}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 608 "./src/ANSI-C.y"
    {
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 612 "./src/ANSI-C.y"
    {
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 616 "./src/ANSI-C.y"
    {
			(yyval.L) = AppendItem((yyvsp[(1) - (2)].L),(yyvsp[(2) - (2)].n));
		}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 620 "./src/ANSI-C.y"
    {
			(yyval.L) = AppendItem((yyvsp[(1) - (2)].L),(yyvsp[(2) - (2)].n));
		}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 627 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found JOIN roundrobin.statement \n");
			(yyval.n) = MakeJoinCoord((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok));
		}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 635 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeRoundRobinCoord(NULL, (yyvsp[(1) - (4)].tok));
		}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 639 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeRoundRobinCoord((yyvsp[(3) - (5)].L), (yyvsp[(1) - (5)].tok));
		}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 646 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeDuplicateCoord(NULL, (yyvsp[(1) - (4)].tok));
		}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 650 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeDuplicateCoord((yyvsp[(3) - (5)].n), (yyvsp[(1) - (5)].tok));
		}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 658 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found operator.default.call\n");
			(yyval.n) = MakeCompositeCallCoord(MakeCompositeIdCoord((yyvsp[(1) - (3)].n)->u.id.text, (yyvsp[(1) - (3)].n)->coord), 
										ModifyOperatorDeclArguments(MakeOperdclCoord(EMPTY_TQ, NULL, NULL, NULL, (yyvsp[(1) - (3)].n)->coord), (yyvsp[(2) - (3)].L)), 
										FALSE, (yyvsp[(1) - (3)].n)->coord);
		}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 668 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found operator.param.list \n");
			(yyval.L) = (yyvsp[(2) - (3)].L);
		}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 674 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found operator.param.list-2 \n");
			(yyval.L) = NULL;
		}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 683 "./src/ANSI-C.y"
    {
			 ///*DEBUG*/printf("have found operator.selfdefine.body\n");
			(yyval.n) = MakeOperBodyCoord(PrimVoid, NULL, (yyvsp[(2) - (5)].n), (yyvsp[(3) - (5)].n), (yyvsp[(4) - (5)].n),(yyvsp[(1) - (5)].tok),(yyvsp[(5) - (5)].tok));
		 }
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 688 "./src/ANSI-C.y"
    {
			 ///*DEBUG*/printf("have found operator.selfdefine.body\n");
			(yyval.n) = MakeOperBodyCoord(PrimVoid, (yyvsp[(2) - (6)].L), (yyvsp[(3) - (6)].n), (yyvsp[(4) - (6)].n), (yyvsp[(5) - (6)].n),(yyvsp[(1) - (6)].tok),(yyvsp[(6) - (6)].tok));
		 }
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 695 "./src/ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 697 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("operator.selfdefine.logic.init.list.opt is not null!!!\n");
			(yyval.n) = (yyvsp[(2) - (2)].n);
		}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 705 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found WORK compound.statement\n");
			(yyval.n) = (yyvsp[(2) - (2)].n);
		}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 713 "./src/ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 715 "./src/ANSI-C.y"
    { 
			(yyval.n) = (yyvsp[(3) - (4)].L); /*MakeWindowCoord*/
		}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 722 "./src/ANSI-C.y"
    { 
			(yyval.L) = MakeNewList((yyvsp[(1) - (1)].n));
		}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 726 "./src/ANSI-C.y"
    { 
			(yyval.L) = AppendItem((yyvsp[(1) - (2)].L),(yyvsp[(2) - (2)].n));
		}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 733 "./src/ANSI-C.y"
    { 
			///*DEBUG*/printf("have found IDENTIFIER ':' window.type ';'\n");
			(yyval.n) = MakeWindowCoord(LookupStreamIdsNode((yyvsp[(1) - (3)].n)), (yyvsp[(2) - (3)].n), (yyvsp[(2) - (3)].n)->coord); 
		}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 741 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found SLIDING ',' window.eviction\n");
			(yyval.n) = MakeWindowSlidingCoord(EMPTY_TQ, NULL, (yyvsp[(2) - (3)].tok));
		}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 746 "./src/ANSI-C.y"
    { 
			///*DEBUG*/printf("have found TUMBLING ',' window.trigger\n");
			(yyval.n) = MakeWindowTumbingCoord(EMPTY_TQ,NULL, (yyvsp[(2) - (3)].tok)); 
		}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 751 "./src/ANSI-C.y"
    {
			///*DEBUG*/printf("have found SLIDING ',' window.eviction\n");
			(yyval.n) = MakeWindowSlidingCoord(EMPTY_TQ, (yyvsp[(3) - (4)].n), (yyvsp[(2) - (4)].tok));
		}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 756 "./src/ANSI-C.y"
    { 
			///*DEBUG*/printf("have found TUMBLING ',' window.trigger\n");
			(yyval.n) = MakeWindowTumbingCoord(EMPTY_TQ, (yyvsp[(3) - (4)].n), (yyvsp[(2) - (4)].tok)); 
		}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 761 "./src/ANSI-C.y"
    {
			printf("have found UNCERTAINTY");
			(yyval.n)=MakeWindowUncertaintyCoord(EMPTY_TQ,NULL,(yyvsp[(2) - (3)].tok));
		}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 782 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 785 "./src/ANSI-C.y"
    { if ((yyvsp[(2) - (3)].n)->typ == Comma) (yyvsp[(2) - (3)].n)->coord = (yyvsp[(1) - (3)].tok);
                                  (yyvsp[(2) - (3)].n)->parenthesized = TRUE;
                                  (yyval.n) = (yyvsp[(2) - (3)].n); }
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 790 "./src/ANSI-C.y"
    { if (ANSIOnly)
	         SyntaxError("statement expressions not allowed with -ansi switch");
               (yyval.n) = MakeBlockCoord(NULL, NULL, GrabPragmas((yyvsp[(3) - (5)].L)), (yyvsp[(1) - (5)].tok), (yyvsp[(4) - (5)].tok)); }
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 794 "./src/ANSI-C.y"
    { if (ANSIOnly)
	         SyntaxError("statement expressions not allowed with -ansi switch");
              (yyval.n) = MakeBlockCoord(NULL, (yyvsp[(3) - (6)].L), GrabPragmas((yyvsp[(4) - (6)].L)), (yyvsp[(1) - (6)].tok), (yyvsp[(5) - (6)].tok)); }
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 802 "./src/ANSI-C.y"
    { (yyval.n) = ExtendArray((yyvsp[(1) - (4)].n), (yyvsp[(3) - (4)].n), (yyvsp[(2) - (4)].tok)); }
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 804 "./src/ANSI-C.y"
    { (yyval.n) = MakeCallCoord((yyvsp[(1) - (3)].n), NULL, (yyvsp[(2) - (3)].tok)); }
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 806 "./src/ANSI-C.y"
    { (yyval.n) = MakeCallCoord((yyvsp[(1) - (4)].n), (yyvsp[(3) - (4)].L), (yyvsp[(2) - (4)].tok)); }
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 809 "./src/ANSI-C.y"
    {
			(yyvsp[(1) - (4)].n) = ModifyDeclType(ConvertIdToDecl((yyvsp[(1) - (4)].n), EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ,NULL, NULL, NULL, (yyvsp[(1) - (4)].n)->coord) );
			(yyvsp[(1) - (4)].n) = SetDeclType((yyvsp[(1) - (4)].n), MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (4)].n)->coord), Redecl) ;
			(yyvsp[(1) - (4)].n) = DefineOperator((yyvsp[(1) - (4)].n));
			(yyval.n) = SetOperatorBody((yyvsp[(1) - (4)].n), (yyvsp[(4) - (4)].n));
		}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 816 "./src/ANSI-C.y"
    {
			(yyvsp[(1) - (5)].n) = ModifyDeclType(ConvertIdToDecl((yyvsp[(1) - (5)].n), EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ,NULL, (yyvsp[(3) - (5)].L), NULL, (yyvsp[(1) - (5)].n)->coord) );
			(yyvsp[(1) - (5)].n) = SetDeclType((yyvsp[(1) - (5)].n), MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (5)].n)->coord), Redecl) ;
			(yyvsp[(1) - (5)].n) = DefineOperator((yyvsp[(1) - (5)].n));
			(yyval.n) = SetOperatorBody((yyvsp[(1) - (5)].n), (yyvsp[(5) - (5)].n));
		}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 823 "./src/ANSI-C.y"
    {
			(yyvsp[(1) - (4)].n) = ModifyDeclType(ConvertIdToDecl((yyvsp[(1) - (4)].n), EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ, NULL, NULL, NULL, (yyvsp[(1) - (4)].n)->coord));

			///*DEBUG*/printf("have found composite template call\n");
			(yyval.n) = MakeCompositeCallCoord(MakeCompositeIdCoord((yyvsp[(1) - (4)].n)->u.decl.name, (yyvsp[(1) - (4)].n)->coord), ModifyOperatorDeclArguments((yyvsp[(1) - (4)].n)->u.decl.type, (yyvsp[(4) - (4)].L)), TRUE, (yyvsp[(1) - (4)].n)->coord);
		}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 830 "./src/ANSI-C.y"
    {
			(yyvsp[(1) - (5)].n) = ModifyDeclType(ConvertIdToDecl((yyvsp[(1) - (5)].n), EMPTY_TQ, NULL, NULL, NULL), MakeOperdclCoord(EMPTY_TQ, NULL, (yyvsp[(3) - (5)].L), NULL, (yyvsp[(1) - (5)].n)->coord));

			///*DEBUG*/printf("have found composite template call\n");
			(yyval.n) = MakeCompositeCallCoord(MakeCompositeIdCoord((yyvsp[(1) - (5)].n)->u.decl.name, (yyvsp[(1) - (5)].n)->coord), ModifyOperatorDeclArguments((yyvsp[(1) - (5)].n)->u.decl.type, (yyvsp[(5) - (5)].L)), TRUE, (yyvsp[(1) - (5)].n)->coord);
		}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 837 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (9)].n)),NULL,NULL,(yyvsp[(6) - (9)].n),GrabPragmas((yyvsp[(7) - (9)].L)),(yyvsp[(8) - (9)].n),(yyvsp[(1) - (9)].tok));
		}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 841 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (10)].n)),(yyvsp[(6) - (10)].L),NULL,(yyvsp[(7) - (10)].n),GrabPragmas((yyvsp[(8) - (10)].L)),(yyvsp[(9) - (10)].n),(yyvsp[(1) - (10)].tok));
		}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 845 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeSplitJoinCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (11)].n)),(yyvsp[(6) - (11)].L),(yyvsp[(7) - (11)].L),(yyvsp[(8) - (11)].n),GrabPragmas((yyvsp[(9) - (11)].L)),(yyvsp[(10) - (11)].n),(yyvsp[(1) - (11)].tok));
		}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 849 "./src/ANSI-C.y"
    {
			(yyval.n) = MakePipelineCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (7)].n)), NULL,(yyvsp[(6) - (7)].L), (yyvsp[(1) - (7)].tok));
		}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 853 "./src/ANSI-C.y"
    {
			(yyval.n) = MakePipelineCoord(NULL, LookupStreamIdsNode((yyvsp[(3) - (8)].n)), (yyvsp[(6) - (8)].L),(yyvsp[(7) - (8)].L), (yyvsp[(1) - (8)].tok));
		}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 857 "./src/ANSI-C.y"
    {
			(yyval.n) = MakeFileReaderOperator(NULL,(yyvsp[(4) - (4)].L),(yyvsp[(1) - (4)].tok));
		}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 863 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('.', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 865 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ARROW, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 867 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(POSTINC, (yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tok)); }
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 869 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(POSTDEC, (yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tok)); }
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 873 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('.', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 875 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ARROW, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 880 "./src/ANSI-C.y"
    { (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); }
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 882 "./src/ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n)); }
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 887 "./src/ANSI-C.y"
    { (yyval.n) = LookupPostfixExpression((yyvsp[(1) - (1)].n)); }
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 889 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(PREINC, (yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok)); }
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 891 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(PREDEC, (yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok)); }
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 893 "./src/ANSI-C.y"
    { (yyvsp[(1) - (2)].n)->u.unary.expr = (yyvsp[(2) - (2)].n);
              (yyval.n) = (yyvsp[(1) - (2)].n); }
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 896 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(SIZEOF, (yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tok)); }
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 898 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord(SIZEOF, (yyvsp[(3) - (4)].n), (yyvsp[(1) - (4)].tok)); }
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 902 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('&', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 903 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('*', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 904 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('+', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 905 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('-', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 906 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('~', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 907 "./src/ANSI-C.y"
    { (yyval.n) = MakeUnaryCoord('!', NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 912 "./src/ANSI-C.y"
    { (yyval.n) = MakeCastCoord((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n), (yyvsp[(1) - (4)].tok)); }
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 918 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('*', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 920 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('/', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 922 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('%', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 928 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('+', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 930 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('-', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 936 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(LS, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 938 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(RS, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 944 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('<', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 946 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('>', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 948 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(LE, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 950 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(GE, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 956 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(EQ, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 958 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(NE, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 964 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('&', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 970 "./src/ANSI-C.y"
    { 
              WarnAboutPrecedence('^', (yyvsp[(1) - (3)].n));
              WarnAboutPrecedence('^', (yyvsp[(3) - (3)].n));
	      (yyval.n) = MakeBinopCoord('^', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 979 "./src/ANSI-C.y"
    { WarnAboutPrecedence('|', (yyvsp[(1) - (3)].n));
              WarnAboutPrecedence('|', (yyvsp[(3) - (3)].n));
              (yyval.n) = MakeBinopCoord('|', (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 987 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ANDAND, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 993 "./src/ANSI-C.y"
    { WarnAboutPrecedence(OROR, (yyvsp[(1) - (3)].n));
              WarnAboutPrecedence(OROR, (yyvsp[(3) - (3)].n));
              (yyval.n) = MakeBinopCoord(OROR, (yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n), (yyvsp[(2) - (3)].tok)); }
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1001 "./src/ANSI-C.y"
    { (yyval.n) = MakeTernaryCoord((yyvsp[(1) - (5)].n), (yyvsp[(3) - (5)].n), (yyvsp[(5) - (5)].n), (yyvsp[(2) - (5)].tok), (yyvsp[(4) - (5)].tok)); }
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1007 "./src/ANSI-C.y"
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

  case 139:

/* Line 1455 of yacc.c  */
#line 1025 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord('=', NULL, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1026 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(MULTassign, NULL, NULL, (yyvsp[(1) - (1)].tok));  }
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1027 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(DIVassign, NULL, NULL, (yyvsp[(1) - (1)].tok));   }
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1028 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(MODassign, NULL, NULL, (yyvsp[(1) - (1)].tok));   }
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1029 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(PLUSassign, NULL, NULL, (yyvsp[(1) - (1)].tok));  }
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1030 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(MINUSassign, NULL, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1031 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(LSassign, NULL, NULL, (yyvsp[(1) - (1)].tok));    }
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1032 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(RSassign, NULL, NULL, (yyvsp[(1) - (1)].tok));    }
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1033 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ANDassign, NULL, NULL, (yyvsp[(1) - (1)].tok));   }
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1034 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ERassign, NULL, NULL, (yyvsp[(1) - (1)].tok));    }
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1035 "./src/ANSI-C.y"
    { (yyval.n) = MakeBinopCoord(ORassign, NULL, NULL, (yyvsp[(1) - (1)].tok));    }
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1041 "./src/ANSI-C.y"
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

  case 152:

/* Line 1455 of yacc.c  */
#line 1055 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1059 "./src/ANSI-C.y"
    { (yyval.n) = (Node *) NULL; }
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1060 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1097 "./src/ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (2)].L); }
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1099 "./src/ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (2)].L); }
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1101 "./src/ANSI-C.y"
    { (yyval.L) = MakeNewList(ForceNewSU((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tok))); }
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1103 "./src/ANSI-C.y"
    { (yyval.L) = MakeNewList(ForceNewSU((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tok))); }
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1109 "./src/ANSI-C.y"
    { 
	      SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);
	    }
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1112 "./src/ANSI-C.y"
    { SetDeclAttribs((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].L)); }
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1114 "./src/ANSI-C.y"
    { 
              (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (6)].n), (yyvsp[(6) - (6)].n))); 
            }
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1118 "./src/ANSI-C.y"
    { 
              SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);
            }
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1121 "./src/ANSI-C.y"
    { SetDeclAttribs((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].L)); }
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1123 "./src/ANSI-C.y"
    { 
              (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (6)].n), (yyvsp[(6) - (6)].n))); 
			}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1127 "./src/ANSI-C.y"
    { 
	      (yyval.L) = AppendDecl((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n), Redecl);
			 }
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1130 "./src/ANSI-C.y"
    { SetDeclAttribs((yyvsp[(3) - (5)].n), (yyvsp[(5) - (5)].L)); }
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1132 "./src/ANSI-C.y"
    { 
              SetDeclInit((yyvsp[(3) - (7)].n), (yyvsp[(7) - (7)].n)); 
            }
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1140 "./src/ANSI-C.y"
    { 
              SyntaxError("declaration without a variable"); 
            }
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1145 "./src/ANSI-C.y"
    { 
              (yyval.L) = NULL; /* empty list */ 
            }
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1150 "./src/ANSI-C.y"
    { 
              SyntaxError("declaration without a variable"); 
            }
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1155 "./src/ANSI-C.y"
    { 
              (yyval.L) = NULL; /* empty list */ 
            }
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1165 "./src/ANSI-C.y"
    { 
              SetDeclType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord), NoRedecl);
            }
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1168 "./src/ANSI-C.y"
    { SetDeclAttribs((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].L)); }
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1170 "./src/ANSI-C.y"
    { 
              (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (6)].n), (yyvsp[(6) - (6)].n))); 
            }
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1174 "./src/ANSI-C.y"
    {
			SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);
            (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (2)].n), NULL)); 
		}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1179 "./src/ANSI-C.y"
    { 
              SetDeclType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord), NoRedecl);
            }
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1182 "./src/ANSI-C.y"
    { SetDeclAttribs((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].L)); }
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1184 "./src/ANSI-C.y"
    { 
              (yyval.L) = MakeNewList(SetDeclInit((yyvsp[(2) - (6)].n), (yyvsp[(6) - (6)].n))); 
	    }
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1188 "./src/ANSI-C.y"
    { (yyval.L) = AppendDecl((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n), NoRedecl); }
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1189 "./src/ANSI-C.y"
    { SetDeclAttribs((yyvsp[(3) - (5)].n), (yyvsp[(5) - (5)].L)); }
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1191 "./src/ANSI-C.y"
    { SetDeclInit((yyvsp[(3) - (7)].n), (yyvsp[(7) - (7)].n)); }
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1196 "./src/ANSI-C.y"
    { 
              SyntaxError("declaration without a variable"); 
	    }
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1201 "./src/ANSI-C.y"
    { 
              (yyval.L) = NULL; /* empty list */ 
	    }
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1206 "./src/ANSI-C.y"
    { 
              SyntaxError("declaration without a variable"); 
	    }
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1211 "./src/ANSI-C.y"
    { 
              (yyval.L) = NULL; /* empty list */ 
            }
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1220 "./src/ANSI-C.y"
    { (yyval.n) = FinishPrimType((yyvsp[(1) - (1)].n)); }
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1229 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1231 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1233 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1235 "./src/ANSI-C.y"
    { (yyval.n) = MergePrimTypes((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n)); }
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1242 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1244 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1246 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1253 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1255 "./src/ANSI-C.y"
    { (yyval.n) = ConvertIdToTdef((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq, GetTypedefType((yyvsp[(2) - (2)].n))); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1257 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1265 "./src/ANSI-C.y"
    { (yyval.tq).tq = MergeTypeQuals((yyvsp[(1) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).coord);
              (yyval.tq).coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1268 "./src/ANSI-C.y"
    { (yyval.tq).tq = MergeTypeQuals((yyvsp[(1) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).coord);
              (yyval.tq).coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1281 "./src/ANSI-C.y"
    { (yyval.n) = FinishPrimType((yyvsp[(1) - (1)].n)); }
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1290 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1292 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1294 "./src/ANSI-C.y"
    { (yyval.n) = MergePrimTypes((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n)); }
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1301 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1303 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1310 "./src/ANSI-C.y"
    { (yyval.n) = ConvertIdToTdef((yyvsp[(1) - (1)].n), EMPTY_TQ, GetTypedefType((yyvsp[(1) - (1)].n))); }
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1312 "./src/ANSI-C.y"
    { (yyval.n) = ConvertIdToTdef((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].tq).tq, GetTypedefType((yyvsp[(2) - (2)].n))); (yyval.n)->coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1314 "./src/ANSI-C.y"
    { (yyval.n) = TypeQualifyNode((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].tq).tq); }
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1321 "./src/ANSI-C.y"
    { (yyval.tq).tq = MergeTypeQuals((yyvsp[(1) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).coord);
              (yyval.tq).coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1328 "./src/ANSI-C.y"
    { (yyval.tq).tq = MergeTypeQuals((yyvsp[(1) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).tq, (yyvsp[(2) - (2)].tq).coord);
              (yyval.tq).coord = (yyvsp[(1) - (2)].tq).coord; }
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1344 "./src/ANSI-C.y"
    {
	      Warning(2, "function prototype parameters must have types");
              (yyval.n) = AddDefaultParameterTypes((yyvsp[(1) - (1)].n));
            }
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1355 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok)), Redecl);
               }
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1358 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(3) - (4)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (4)].tok)), Redecl); 
               }
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1361 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(4) - (5)].n), MakePtrCoord(   (yyvsp[(2) - (5)].tq).tq,    NULL, (yyvsp[(1) - (5)].tok)), Redecl);
               }
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1364 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(3) - (3)].n), MakePtrCoord(   (yyvsp[(2) - (3)].tq).tq,    NULL, (yyvsp[(1) - (3)].tok)), Redecl); 
               }
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1372 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n);  
              }
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1375 "./src/ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(3) - (4)].n)); 
               }
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1378 "./src/ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1385 "./src/ANSI-C.y"
    { (yyval.n) = ConvertIdToDecl((yyvsp[(1) - (1)].n), EMPTY_TQ, NULL, NULL, NULL); }
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1387 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n);  
               }
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1394 "./src/ANSI-C.y"
    { (yyval.n) = ConvertIdToDecl((yyvsp[(1) - (1)].n), EMPTY_TQ, NULL, NULL, NULL); }
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1396 "./src/ANSI-C.y"
    { (yyval.n) = ConvertIdToDecl((yyvsp[(1) - (2)].n), EMPTY_TQ, (yyvsp[(2) - (2)].n), NULL, NULL);   }
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1408 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok)), Redecl); 
               }
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1411 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(3) - (3)].n), MakePtrCoord((yyvsp[(2) - (3)].tq).tq, NULL, (yyvsp[(1) - (3)].tok)), Redecl); 
               }
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1418 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1421 "./src/ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1435 "./src/ANSI-C.y"
    { (yyval.n) = MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1437 "./src/ANSI-C.y"
    { (yyval.n) = MakePtrCoord((yyvsp[(2) - (2)].tq).tq, NULL, (yyvsp[(1) - (2)].tok)); }
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1439 "./src/ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok))); 
               }
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1442 "./src/ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(3) - (3)].n), MakePtrCoord((yyvsp[(2) - (3)].tq).tq, NULL, (yyvsp[(1) - (3)].tok))); 
               }
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1449 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1452 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1455 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1458 "./src/ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1464 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n);                   }
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1465 "./src/ANSI-C.y"
    { (yyval.n) = MakeFdclCoord(EMPTY_TQ, NULL, NULL, (yyvsp[(1) - (2)].tok)); }
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1466 "./src/ANSI-C.y"
    { (yyval.n) = MakeFdclCoord(EMPTY_TQ, (yyvsp[(2) - (3)].L), NULL, (yyvsp[(1) - (3)].tok)); }
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1479 "./src/ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok))); 
               }
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1482 "./src/ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(3) - (3)].n), MakePtrCoord(   (yyvsp[(2) - (3)].tq).tq,    NULL, (yyvsp[(1) - (3)].tok))); 
               }
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1489 "./src/ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n)); }
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1491 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1494 "./src/ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1501 "./src/ANSI-C.y"
    { (yyval.n) = ConvertIdToDecl((yyvsp[(1) - (1)].n), EMPTY_TQ, NULL, NULL, NULL); }
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1503 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1510 "./src/ANSI-C.y"
    { 
/*              OldStyleFunctionDefinition = TRUE; */
              (yyval.n) = (yyvsp[(1) - (1)].n); 
            }
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1515 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakePtrCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (2)].tok)), SU); 
               }
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1518 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(3) - (3)].n), MakePtrCoord((yyvsp[(2) - (3)].tq).tq, NULL, (yyvsp[(1) - (3)].tok)), SU); 
               }
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1525 "./src/ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(1) - (4)].n), MakeFdclCoord(EMPTY_TQ, (yyvsp[(3) - (4)].L), NULL, (yyvsp[(2) - (4)].tok))); }
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1527 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); 
               }
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1530 "./src/ANSI-C.y"
    { (yyval.n) = ModifyDeclType((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); 
               }
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1545 "./src/ANSI-C.y"
    { (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); }
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1547 "./src/ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n)); }
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1559 "./src/ANSI-C.y"
    { (yyval.n) = FinishType((yyvsp[(1) - (1)].n)); }
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1561 "./src/ANSI-C.y"
    { (yyval.n) = FinishType(SetBaseType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n))); }
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1563 "./src/ANSI-C.y"
    { (yyval.n) = MakeDefaultPrimType((yyvsp[(1) - (1)].tq).tq, (yyvsp[(1) - (1)].tq).coord); }
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1565 "./src/ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord)); }
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1573 "./src/ANSI-C.y"
    { (yyval.L) = NULL; }
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1575 "./src/ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (1)].L); }
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1580 "./src/ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (1)].L); }
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1582 "./src/ANSI-C.y"
    { (yyval.L) = JoinLists ((yyvsp[(1) - (2)].L), (yyvsp[(2) - (2)].L)); }
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1587 "./src/ANSI-C.y"
    { if (ANSIOnly)
	            SyntaxError("__attribute__ not allowed with -ansi switch");
                  (yyval.L) = (yyvsp[(4) - (6)].L); }
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1594 "./src/ANSI-C.y"
    { (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); }
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 1596 "./src/ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n)); }
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1601 "./src/ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 1603 "./src/ANSI-C.y"
    { (yyval.n) = ConvertIdToAttrib((yyvsp[(1) - (1)].n), NULL); }
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1605 "./src/ANSI-C.y"
    { (yyval.n) = ConvertIdToAttrib((yyvsp[(1) - (4)].n), (yyvsp[(3) - (4)].n)); }
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 1612 "./src/ANSI-C.y"
    { (yyval.n) = MakeIdCoord(UniqueString("const"), (yyvsp[(1) - (1)].tok)); }
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1617 "./src/ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1618 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (2)].n); }
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1623 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (3)].n); (yyval.n)->coord = (yyvsp[(1) - (3)].tok); }
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1624 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (4)].n); (yyval.n)->coord = (yyvsp[(1) - (4)].tok); }
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1625 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n);}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1631 "./src/ANSI-C.y"
    { (yyval.n) = MakeInitializerCoord(MakeNewList((yyvsp[(1) - (1)].n)), (yyvsp[(1) - (1)].n)->coord);}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1633 "./src/ANSI-C.y"
    { 
              assert((yyvsp[(1) - (3)].n)->typ == Initializer);
			  AppendItem((yyvsp[(1) - (3)].n)->u.initializer.exprs, (yyvsp[(3) - (3)].n));
              (yyval.n) = (yyvsp[(1) - (3)].n); 
            }
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1643 "./src/ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), EllipsisNode); }
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 1647 "./src/ANSI-C.y"
    { Node *n = MakePrimCoord(EMPTY_TQ, Void, (yyvsp[(1) - (1)].tok));
	      SyntaxErrorCoord(n->coord, "First argument cannot be `...'");
              (yyval.L) = MakeNewList(n);
            }
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 1656 "./src/ANSI-C.y"
    { (yyval.L) = MakeNewList((yyvsp[(1) - (1)].n)); }
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1658 "./src/ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n)); }
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1662 "./src/ANSI-C.y"
    { 
	      SyntaxErrorCoord((yyvsp[(1) - (3)].n)->coord, "formals cannot have initializers");
              (yyval.L) = MakeNewList((yyvsp[(1) - (3)].n)); 
            }
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1667 "./src/ANSI-C.y"
    { (yyval.L) = (yyvsp[(1) - (3)].L); }
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1673 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1675 "./src/ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n)); 
            }
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1678 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Formal); 
            }
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1681 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Formal); 
            }
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1684 "./src/ANSI-C.y"
    { (yyval.n) = MakeDefaultPrimType((yyvsp[(1) - (1)].tq).tq, (yyvsp[(1) - (1)].tq).coord); }
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1686 "./src/ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord)); }
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1688 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord), Formal); }
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1690 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1692 "./src/ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n)); 
            }
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1695 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Formal); 
            }
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1698 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Formal); 
            }
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1701 "./src/ANSI-C.y"
    { (yyval.n) = MakeDefaultPrimType((yyvsp[(1) - (1)].tq).tq, (yyvsp[(1) - (1)].tq).coord); }
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1703 "./src/ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord)); }
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1705 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclType((yyvsp[(2) - (2)].n), MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord), Formal); }
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1711 "./src/ANSI-C.y"
    { (yyval.n) = MakeAdclCoord(EMPTY_TQ, NULL, NULL, (yyvsp[(1) - (2)].tok)); }
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1713 "./src/ANSI-C.y"
    { (yyval.n) = MakeAdclCoord(EMPTY_TQ, NULL, (yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].tok)); }
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1715 "./src/ANSI-C.y"
    { (yyval.n) = SetBaseType((yyvsp[(1) - (4)].n), MakeAdclCoord(EMPTY_TQ, NULL, (yyvsp[(3) - (4)].n), (yyvsp[(2) - (4)].tok))); }
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1720 "./src/ANSI-C.y"
    { 
              SyntaxError("array declaration with illegal empty dimension");
              (yyval.n) = SetBaseType((yyvsp[(1) - (3)].n), MakeAdclCoord(EMPTY_TQ, NULL, SintOne, (yyvsp[(2) - (3)].tok))); 
            }
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1736 "./src/ANSI-C.y"
    { 
              (yyval.n) = SetSUdclNameFields((yyvsp[(1) - (4)].n), NULL, (yyvsp[(3) - (4)].L), (yyvsp[(2) - (4)].tok), (yyvsp[(4) - (4)].tok));
            }
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1741 "./src/ANSI-C.y"
    { 
              (yyval.n) = SetSUdclNameFields((yyvsp[(1) - (5)].n), (yyvsp[(2) - (5)].n), (yyvsp[(4) - (5)].L), (yyvsp[(3) - (5)].tok), (yyvsp[(5) - (5)].tok));
	    }
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1745 "./src/ANSI-C.y"
    { 
              (yyval.n) = SetSUdclName((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n)->coord);
	    }
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1750 "./src/ANSI-C.y"
    { 
              if (ANSIOnly)
                 Warning(1, "empty structure declaration");
              (yyval.n) = SetSUdclNameFields((yyvsp[(1) - (3)].n), NULL, NULL, (yyvsp[(2) - (3)].tok), (yyvsp[(3) - (3)].tok)); 
	    }
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1756 "./src/ANSI-C.y"
    { 
              if (ANSIOnly)
                 Warning(1, "empty structure declaration");
              (yyval.n) = SetSUdclNameFields((yyvsp[(1) - (4)].n), (yyvsp[(2) - (4)].n), NULL, (yyvsp[(3) - (4)].tok), (yyvsp[(4) - (4)].tok)); 
	    }
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1765 "./src/ANSI-C.y"
    { (yyval.n) = MakeSdclCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1766 "./src/ANSI-C.y"
    { (yyval.n) = MakeUdclCoord(EMPTY_TQ, NULL, (yyvsp[(1) - (1)].tok)); }
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1773 "./src/ANSI-C.y"
    { (yyval.L) = JoinLists((yyvsp[(1) - (2)].L), (yyvsp[(2) - (2)].L)); }
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1786 "./src/ANSI-C.y"
    { 
	      (yyval.L) = MakeNewList(SetDeclType((yyvsp[(2) - (2)].n),
					    MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
					    SU)); 
	    }
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1792 "./src/ANSI-C.y"
    { (yyval.L) = AppendDecl((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n), SU); }
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1798 "./src/ANSI-C.y"
    { (yyval.L) = MakeNewList(SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), SU)); }
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1800 "./src/ANSI-C.y"
    { (yyval.L) = AppendDecl((yyvsp[(1) - (3)].L), (yyvsp[(3) - (3)].n), SU); }
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1807 "./src/ANSI-C.y"
    { SetDeclAttribs((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].L));
              (yyval.n) = SetDeclBitSize((yyvsp[(1) - (3)].n), (yyvsp[(2) - (3)].n)); }
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1810 "./src/ANSI-C.y"
    { (yyval.n) = MakeDeclCoord(NULL, EMPTY_TQ, NULL, NULL, (yyvsp[(1) - (2)].n), (yyvsp[(1) - (2)].n)->coord);
              SetDeclAttribs((yyval.n), (yyvsp[(2) - (2)].L)); }
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1817 "./src/ANSI-C.y"
    { (yyval.n) = SetDeclBitSize((yyvsp[(1) - (3)].n), (yyvsp[(2) - (3)].n));
              SetDeclAttribs((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].L)); }
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1820 "./src/ANSI-C.y"
    { (yyval.n) = MakeDeclCoord(NULL, EMPTY_TQ, NULL, NULL, (yyvsp[(1) - (2)].n), (yyvsp[(1) - (2)].n)->coord);
              SetDeclAttribs((yyval.n), (yyvsp[(2) - (2)].L)); }
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1826 "./src/ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1832 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (2)].n); }
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1838 "./src/ANSI-C.y"
    { (yyval.n) = BuildEnum(NULL, (yyvsp[(3) - (5)].L), (yyvsp[(1) - (5)].tok), (yyvsp[(2) - (5)].tok), (yyvsp[(5) - (5)].tok)); }
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1840 "./src/ANSI-C.y"
    { (yyval.n) = BuildEnum((yyvsp[(2) - (6)].n), (yyvsp[(4) - (6)].L), (yyvsp[(1) - (6)].tok), (yyvsp[(3) - (6)].tok), (yyvsp[(6) - (6)].tok));   }
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1842 "./src/ANSI-C.y"
    { (yyval.n) = BuildEnum((yyvsp[(2) - (2)].n), NULL, (yyvsp[(1) - (2)].tok), (yyvsp[(2) - (2)].n)->coord, (yyvsp[(2) - (2)].n)->coord); }
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1848 "./src/ANSI-C.y"
    { (yyval.L) = MakeNewList(BuildEnumConst((yyvsp[(1) - (2)].n), (yyvsp[(2) - (2)].n))); }
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1850 "./src/ANSI-C.y"
    { (yyval.L) = AppendItem((yyvsp[(1) - (4)].L), BuildEnumConst((yyvsp[(3) - (4)].n), (yyvsp[(4) - (4)].n))); }
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1855 "./src/ANSI-C.y"
    { (yyval.n) = NULL; }
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1856 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(2) - (2)].n);   }
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1860 "./src/ANSI-C.y"
    { }
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1861 "./src/ANSI-C.y"
    { }
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 1882 "./src/ANSI-C.y"
    {  (yyval.n) = NULL; }
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 1887 "./src/ANSI-C.y"
    { (yyval.n) = BuildLabel((yyvsp[(1) - (2)].n), NULL); }
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1889 "./src/ANSI-C.y"
    { (yyval.n)->u.label.stmt = (yyvsp[(4) - (4)].n); }
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1892 "./src/ANSI-C.y"
    { (yyval.n) = AddContainee(MakeCaseCoord((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n), NULL, (yyvsp[(1) - (4)].tok))); }
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1894 "./src/ANSI-C.y"
    { (yyval.n) = AddContainee(MakeDefaultCoord((yyvsp[(3) - (3)].n), NULL, (yyvsp[(1) - (3)].tok))); }
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1898 "./src/ANSI-C.y"
    { (yyval.n) = BuildLabel((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n)); }
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1903 "./src/ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, NULL, NULL, (yyvsp[(1) - (2)].tok), (yyvsp[(2) - (2)].tok)); }
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1905 "./src/ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, GrabPragmas((yyvsp[(2) - (3)].L)), NULL, (yyvsp[(1) - (3)].tok), (yyvsp[(3) - (3)].tok)); }
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1907 "./src/ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, NULL, GrabPragmas((yyvsp[(2) - (3)].L)), (yyvsp[(1) - (3)].tok), (yyvsp[(3) - (3)].tok)); }
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1909 "./src/ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, (yyvsp[(2) - (4)].L), GrabPragmas((yyvsp[(3) - (4)].L)), (yyvsp[(1) - (4)].tok), (yyvsp[(4) - (4)].tok)); }
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1912 "./src/ANSI-C.y"
    { EnterScope(); }
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1913 "./src/ANSI-C.y"
    { ExitScope(); }
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1920 "./src/ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, NULL, NULL, (yyvsp[(1) - (2)].tok), (yyvsp[(2) - (2)].tok)); }
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1922 "./src/ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, GrabPragmas((yyvsp[(2) - (3)].L)), NULL, (yyvsp[(1) - (3)].tok), (yyvsp[(3) - (3)].tok)); }
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1924 "./src/ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, NULL, GrabPragmas((yyvsp[(2) - (3)].L)), (yyvsp[(1) - (3)].tok), (yyvsp[(3) - (3)].tok)); }
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1926 "./src/ANSI-C.y"
    { (yyval.n) = MakeBlockCoord(PrimVoid, (yyvsp[(2) - (4)].L), GrabPragmas((yyvsp[(3) - (4)].L)), (yyvsp[(1) - (4)].tok), (yyvsp[(4) - (4)].tok)); }
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1932 "./src/ANSI-C.y"
    { (yyval.L) = GrabPragmas((yyvsp[(1) - (1)].L)); }
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1933 "./src/ANSI-C.y"
    { (yyval.L) = JoinLists(GrabPragmas((yyvsp[(1) - (2)].L)),
                                                         (yyvsp[(2) - (2)].L)); }
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1938 "./src/ANSI-C.y"
    { (yyval.L) = GrabPragmas(MakeNewList((yyvsp[(1) - (1)].n))); }
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1939 "./src/ANSI-C.y"
    { (yyval.L) = AppendItem(GrabPragmas((yyvsp[(1) - (2)].L)), 
                                                        (yyvsp[(2) - (2)].n)); }
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1949 "./src/ANSI-C.y"
    { (yyval.n) = MakeIfCoord((yyvsp[(3) - (5)].n), (yyvsp[(5) - (5)].n), (yyvsp[(1) - (5)].tok)); }
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1951 "./src/ANSI-C.y"
    { (yyval.n) = MakeIfElseCoord((yyvsp[(3) - (7)].n), (yyvsp[(5) - (7)].n), (yyvsp[(7) - (7)].n), (yyvsp[(1) - (7)].tok), (yyvsp[(6) - (7)].tok)); }
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1952 "./src/ANSI-C.y"
    { PushContainer(Switch); }
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1953 "./src/ANSI-C.y"
    { (yyval.n) = PopContainer(MakeSwitchCoord((yyvsp[(4) - (6)].n), (yyvsp[(6) - (6)].n), NULL, (yyvsp[(1) - (6)].tok))); }
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1958 "./src/ANSI-C.y"
    { PushContainer(While);}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1960 "./src/ANSI-C.y"
    { (yyval.n) = PopContainer(MakeWhileCoord((yyvsp[(4) - (6)].n), (yyvsp[(6) - (6)].n), (yyvsp[(1) - (6)].tok))); }
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1962 "./src/ANSI-C.y"
    { PushContainer(Do);}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1964 "./src/ANSI-C.y"
    { (yyval.n) = PopContainer(MakeDoCoord((yyvsp[(3) - (8)].n), (yyvsp[(6) - (8)].n), (yyvsp[(1) - (8)].tok), (yyvsp[(4) - (8)].tok))); }
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1966 "./src/ANSI-C.y"
    { PushContainer(For);}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1968 "./src/ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord((yyvsp[(3) - (10)].n), (yyvsp[(5) - (10)].n), (yyvsp[(7) - (10)].n), (yyvsp[(10) - (10)].n), (yyvsp[(1) - (10)].tok))); }
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1972 "./src/ANSI-C.y"
    { PushContainer(For);}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1974 "./src/ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord(NULL, (yyvsp[(5) - (10)].n), (yyvsp[(7) - (10)].n), (yyvsp[(10) - (10)].n), (yyvsp[(1) - (10)].tok))); }
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1976 "./src/ANSI-C.y"
    { PushContainer(For);}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 1978 "./src/ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord((yyvsp[(3) - (10)].n), (yyvsp[(5) - (10)].n), NULL, (yyvsp[(10) - (10)].n), (yyvsp[(1) - (10)].tok))); }
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 1980 "./src/ANSI-C.y"
    { PushContainer(For);}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 1982 "./src/ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord((yyvsp[(3) - (10)].n), NULL, (yyvsp[(7) - (10)].n), (yyvsp[(10) - (10)].n), (yyvsp[(1) - (10)].tok))); }
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1983 "./src/ANSI-C.y"
    { PushContainer(For);}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 1984 "./src/ANSI-C.y"
    { (yyval.n) = PopContainer(MakeForCoord(NULL, SintZero, NULL, (yyvsp[(6) - (6)].n), (yyvsp[(1) - (6)].tok))); }
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 1989 "./src/ANSI-C.y"
    { (yyval.n) = ResolveGoto((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].tok)); }
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 1991 "./src/ANSI-C.y"
    { (yyval.n) = AddContainee(MakeContinueCoord(NULL, (yyvsp[(1) - (2)].tok))); }
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 1993 "./src/ANSI-C.y"
    { (yyval.n) = AddContainee(MakeBreakCoord(NULL, (yyvsp[(1) - (2)].tok))); }
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 1995 "./src/ANSI-C.y"
    { (yyval.n) = AddReturn(MakeReturnCoord((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].tok))); }
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 1999 "./src/ANSI-C.y"
    { (yyval.n) = ResolveGoto((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].tok)); }
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 2011 "./src/ANSI-C.y"
    { (yyval.L) = JoinLists(GrabPragmas((yyvsp[(1) - (2)].L)), (yyvsp[(2) - (2)].L)); }
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 2017 "./src/ANSI-C.y"
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

  case 416:

/* Line 1455 of yacc.c  */
#line 2027 "./src/ANSI-C.y"
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

  case 417:

/* Line 1455 of yacc.c  */
#line 2037 "./src/ANSI-C.y"
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

  case 418:

/* Line 1455 of yacc.c  */
#line 2051 "./src/ANSI-C.y"
    { 
              (yyvsp[(1) - (1)].n) = DefineProc(FALSE, 
                              SetDeclType((yyvsp[(1) - (1)].n),
					  MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (1)].n)->coord),
					  Redecl));
            }
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 2058 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n)); }
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 2064 "./src/ANSI-C.y"
    { (yyvsp[(2) - (2)].n) = DefineProc(FALSE, SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl)); }
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 2066 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 2068 "./src/ANSI-C.y"
    { (yyvsp[(2) - (2)].n) = DefineProc(FALSE, SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl)); }
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 2070 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 2072 "./src/ANSI-C.y"
    { 
              (yyvsp[(2) - (2)].n) = DefineProc(FALSE, 
	                      SetDeclType((yyvsp[(2) - (2)].n),
				 	  MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
				          Redecl));
            }
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 2079 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 2081 "./src/ANSI-C.y"
    { 
              (yyvsp[(2) - (2)].n) = DefineProc(FALSE, 
                              SetDeclType((yyvsp[(2) - (2)].n),
					  MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
					  Redecl));
            }
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 2088 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 2090 "./src/ANSI-C.y"
    { 
              (yyvsp[(1) - (1)].n) = DefineProc(TRUE, 
                              SetDeclType((yyvsp[(1) - (1)].n),
					  MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (1)].n)->coord),
					  Redecl));
            }
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 2097 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(1) - (3)].n), (yyvsp[(3) - (3)].n)); }
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 2099 "./src/ANSI-C.y"
    {  Node *decl = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);  

               AddParameterTypes(decl, NULL);
               (yyvsp[(2) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 2105 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 2107 "./src/ANSI-C.y"
    { Node *decl = SetDeclType((yyvsp[(2) - (2)].n), (yyvsp[(1) - (2)].n), Redecl);

              AddParameterTypes(decl, NULL);
              (yyvsp[(2) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 2113 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 2115 "./src/ANSI-C.y"
    { Node *type = MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
                   *decl = SetDeclType((yyvsp[(2) - (2)].n), type, Redecl);

              AddParameterTypes(decl, NULL);
              (yyvsp[(2) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 2122 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 2124 "./src/ANSI-C.y"
    { Node *type = MakeDefaultPrimType((yyvsp[(1) - (2)].tq).tq, (yyvsp[(1) - (2)].tq).coord),
                   *decl = SetDeclType((yyvsp[(2) - (2)].n), type, Redecl);

              AddParameterTypes(decl, NULL);
              (yyvsp[(2) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 2131 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 2133 "./src/ANSI-C.y"
    { Node *type = MakeDefaultPrimType(EMPTY_TQ, (yyvsp[(1) - (2)].n)->coord),
                   *decl = SetDeclType((yyvsp[(1) - (2)].n), type, Redecl);

              AddParameterTypes(decl, (yyvsp[(2) - (2)].L));
              (yyvsp[(1) - (2)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 2140 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(1) - (4)].n), (yyvsp[(4) - (4)].n)); }
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 2142 "./src/ANSI-C.y"
    { Node *decl = SetDeclType((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].n), Redecl);

              AddParameterTypes(decl, (yyvsp[(3) - (3)].L));
              (yyvsp[(2) - (3)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 2148 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (5)].n), (yyvsp[(5) - (5)].n)); }
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 2150 "./src/ANSI-C.y"
    { Node *decl = SetDeclType((yyvsp[(2) - (3)].n), (yyvsp[(1) - (3)].n), Redecl);

              AddParameterTypes(decl, (yyvsp[(3) - (3)].L));
              (yyvsp[(2) - (3)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 2156 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (5)].n), (yyvsp[(5) - (5)].n)); }
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 2158 "./src/ANSI-C.y"
    { Node *type = MakeDefaultPrimType((yyvsp[(1) - (3)].tq).tq, (yyvsp[(1) - (3)].tq).coord),
                   *decl = SetDeclType((yyvsp[(2) - (3)].n), type, Redecl);

              AddParameterTypes(decl, (yyvsp[(3) - (3)].L));
              (yyvsp[(2) - (3)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 2165 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (5)].n), (yyvsp[(5) - (5)].n)); }
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 2167 "./src/ANSI-C.y"
    { Node *type = MakeDefaultPrimType((yyvsp[(1) - (3)].tq).tq, (yyvsp[(1) - (3)].tq).coord), 
                   *decl = SetDeclType((yyvsp[(2) - (3)].n), type, Redecl);
				       

              AddParameterTypes(decl, (yyvsp[(3) - (3)].L));
              (yyvsp[(2) - (3)].n) = DefineProc(TRUE, decl);
            }
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 2175 "./src/ANSI-C.y"
    { (yyval.n) = SetProcBody((yyvsp[(2) - (5)].n), (yyvsp[(5) - (5)].n)); }
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 2180 "./src/ANSI-C.y"
    { OldStyleFunctionDefinition = TRUE; }
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 2182 "./src/ANSI-C.y"
    { OldStyleFunctionDefinition = FALSE; 
               (yyval.L) = (yyvsp[(2) - (2)].L); }
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 2197 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 2198 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 2199 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 2200 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 2201 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 2206 "./src/ANSI-C.y"
    { (yyval.n) = (yyvsp[(1) - (1)].n); }
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 2208 "./src/ANSI-C.y"
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

  case 458:

/* Line 1455 of yacc.c  */
#line 2229 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_CONST;    (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 2230 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_VOLATILE; (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 2231 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_INLINE;   (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 2235 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_CONST;    (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 2236 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_VOLATILE; (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 2240 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_TYPEDEF;  (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 2241 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_EXTERN;   (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 2242 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_STATIC;   (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 2243 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_AUTO;     (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 2244 "./src/ANSI-C.y"
    { (yyval.tq).tq = T_REGISTER; (yyval.tq).coord = (yyvsp[(1) - (1)].tok); }
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2248 "./src/ANSI-C.y"
    { (yyval.n) = StartPrimType(Void, (yyvsp[(1) - (1)].tok));    }
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2249 "./src/ANSI-C.y"
    { (yyval.n) = StartPrimType(Char, (yyvsp[(1) - (1)].tok));     }
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2250 "./src/ANSI-C.y"
    { (yyval.n) = StartPrimType(Int_ParseOnly, (yyvsp[(1) - (1)].tok));     }
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2251 "./src/ANSI-C.y"
    { (yyval.n) = StartPrimType(Float, (yyvsp[(1) - (1)].tok));   }
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2252 "./src/ANSI-C.y"
    { (yyval.n) = StartPrimType(Double, (yyvsp[(1) - (1)].tok));  }
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2254 "./src/ANSI-C.y"
    { (yyval.n) = StartPrimType(Signed, (yyvsp[(1) - (1)].tok));  }
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2255 "./src/ANSI-C.y"
    { (yyval.n) = StartPrimType(Unsigned, (yyvsp[(1) - (1)].tok));}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2257 "./src/ANSI-C.y"
    { (yyval.n) = StartPrimType(Short, (yyvsp[(1) - (1)].tok));   }
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2258 "./src/ANSI-C.y"
    { (yyval.n) = StartPrimType(Long, (yyvsp[(1) - (1)].tok));    }
    break;



/* Line 1455 of yacc.c  */
#line 6733 "y.tab.c"
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
#line 2261 "./src/ANSI-C.y"

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



