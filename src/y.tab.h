
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

/* Line 1676 of yacc.c  */
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



/* Line 1676 of yacc.c  */
#line 260 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


