
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* Line 1676 of yacc.c  */
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



/* Line 1676 of yacc.c  */
#line 258 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


