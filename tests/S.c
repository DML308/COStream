# 1 "strings.c"
 





















































#pragma ident "S.c,v 1.3 1995/05/05 19:19:05 randall Exp Copyright 1994 Massachusetts Institute of Technology"

# 1 "../basics.h" 1
 




















































































#pragma ident "S.c,v 1.3 1995/05/05 19:19:05 randall Exp Copyright 1994 Massachusetts Institute of Technology"









# 1 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/stdio.h" 1 3











# 1 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/stdarg.h" 1 3



 

































# 1 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/va-sparc.h" 1 3
 


 





 

typedef char * __gnuc_va_list;







 

# 84 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/va-sparc.h" 3


# 38 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/stdarg.h" 2 3

# 99 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/stdarg.h" 3







# 159 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/stdarg.h" 3








# 12 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/stdio.h" 2 3

 




extern	struct	_iobuf {
	int	_cnt;
	unsigned char *_ptr;
	unsigned char *_base;
	int	_bufsiz;
	short	_flag;
	char	_file;		 
} _iob[];








































extern struct _iobuf 	*fopen  (const char *, const char *)  ;
extern struct _iobuf 	*fdopen  (int, const char *)  ;
extern struct _iobuf 	*freopen  (const char *, const char *, struct _iobuf  *)  ;
extern struct _iobuf 	*popen  (const char *, const char *)  ;
extern struct _iobuf 	*tmpfile();
extern long	ftell  (struct _iobuf  *)  ;
extern char	*fgets  (char *, int, struct _iobuf  *)  ;
extern char	*gets  (char *)  ;
extern char	*sprintf  (char *, const char *, ...)  ;
extern char	*ctermid  (char *)  ;
extern char	*cuserid  (char *)  ;
extern char	*tempnam  (const char *, const char *)  ;
extern char	*tmpnam  (char *)  ;






# 120 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/stdio.h" 3




# 96 "../basics.h" 2

# 1 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/stdlib.h" 1 3
 

 
















# 1 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/sys/stdtypes.h" 1 3
 

 










typedef	int		sigset_t;	 

typedef	unsigned int	speed_t;	 
typedef	unsigned long	tcflag_t;	 
typedef	unsigned char	cc_t;		 
typedef	int		pid_t;		 

typedef	unsigned short	mode_t;		 
typedef	short		nlink_t;	 

typedef	long		clock_t;	 
typedef	long		time_t;		 



typedef long unsigned int size_t;		 



typedef int		ptrdiff_t;	 




typedef	unsigned short	wchar_t;	 



# 20 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/stdlib.h" 2 3


extern unsigned int _mb_cur_max;




 
extern void	abort( );
extern int	abs  (int)  ;
extern double	atof  (const char *)  ;
extern int	atoi  (const char *)  ;
extern long int	atol  (const char *)  ;
extern char *	bsearch  (const void *, const void *, long unsigned int , long unsigned int , int (*) (const void *, const void *))  ;
extern void *	calloc  (long unsigned int , long unsigned int )  ;
extern void	exit  (int)  ;
extern void	free  (void *)  ;
extern char *	getenv  (const char *)  ;
extern void *	malloc  (long unsigned int )  ;
extern int	qsort  (void *, long unsigned int , long unsigned int , int (*) (const void *, const void *))  ;
extern int	rand( );
extern void *	realloc  (void *, long unsigned int )  ;
extern int	srand  (unsigned int)  ;

extern int    mbtowc  (unsigned short *, const char *, long unsigned int )  ;
extern int    wctomb  (char *, unsigned short )  ;
extern size_t mbstowcs  (unsigned short *, const char *, long unsigned int )  ;
extern size_t wcstombs  (char *, const unsigned short *, long unsigned int )  ;













# 97 "../basics.h" 2

# 1 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/string.h" 1 3
 





















extern char *	strcat  (char *, const char *)  ;
extern char *	strchr  (const char *, int)  ;
extern int	strcmp  (const char *, const char *)  ;
extern char *	strcpy  (char *, const char *)  ;
extern size_t	strcspn  (const char *, const char *)  ;

extern char *	strdup( );

extern size_t	strlen  (const char *)  ;
extern char *	strncat  (char *, const char *, long unsigned int )  ;
extern int	strncmp  (const char *, const char *, long unsigned int )  ;
extern char *	strncpy  (char *, const char *, long unsigned int )  ;
extern char *	strpbrk  (const char *, const char *)  ;
extern char *	strrchr  (const char *, int)  ;
extern size_t	strspn  (const char *, const char *)  ;
extern char *	strstr  (const char *, const char *)  ;
extern char *	strtok  (char *, const char *)  ;

# 51 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/string.h" 3





# 98 "../basics.h" 2


# 1 "../config.h" 1
 





































































#pragma ident "S.c,v 1.3 1995/05/05 19:19:05 randall Exp Copyright 1994 Massachusetts Institute of Technology"




# 1 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/limits.h" 1 3
 


 





 
# 1 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/syslimits.h" 1 3
 




# 1 "/usr/include/limits.h" 1 3
 





















 
















# 6 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/syslimits.h" 2 3

# 11 "/usr/local/gnu/lib/gcc-lib/sparc-sun-sunos4.1.3/2.5.8/include/limits.h" 2 3





 



 



 




 





 



 












 





 



 








 



 









 




 








 






 



# 76 "../config.h" 2



 



 




 


 


# 105 "../config.h"


 


typedef char            TARGET_CHAR;
typedef int             TARGET_INT;
typedef unsigned int    TARGET_UINT;
typedef long            TARGET_LONG;
typedef unsigned long   TARGET_ULONG;

 






 







































# 100 "../basics.h" 2



extern int      fprintf();
extern int      fputc();
extern int      fputs();
extern struct _iobuf      *fopen();
extern int      fclose();
extern int      printf();
extern int	sscanf();
extern int      _flsbuf();
extern void     bcopy();
extern int      toupper();
extern char *   memcpy();
extern int      fflush();






 






 
typedef void Generic;
typedef void **GenericREF;   

typedef int Bool;




 




 























 

typedef struct nodeStruct Node;
typedef struct tablestruct SymbolTable;
typedef int OpType;


typedef struct coord {
    int line;
    short offset;
    short file;
    Bool includedp;   
} Coord;

  Coord UnknownCoord;
















 

# 1 "../heap.h" 1
 

























































#pragma ident "S.c,v 1.3 1995/05/05 19:19:05 randall Exp Copyright 1994 Massachusetts Institute of Technology"







  inline void *HeapAllocate(int number, int size);
  inline void HeapFree(void *ptr);


# 199 "../basics.h" 2

# 1 "../list.h" 1
 
































































#pragma ident "S.c,v 1.3 1995/05/05 19:19:05 randall Exp Copyright 1994 Massachusetts Institute of Technology"




typedef struct liststruct List;

typedef struct {
    List *first, *current, *tail;
} ListMarker;

  Generic *FirstItem(List *list);
  Generic *LastItem(List *list);
  Generic *SetItem(List *list, Generic *element);
  List    *Rest(List *list);
  List    *Last(List *list);
  int      ListLength(List *list);

  List *FindItem(List *list, Generic *item);
  List *RemoveItem(List *list, Generic *item);

  List *MakeNewList(Generic *item);
  List *ConsItem(Generic *item, List *list);
  List *AppendItem(List *list, Generic *item);
  List *JoinLists(List *list1, List *list2);
  List *ListCopy(List *list);


  List *List2(Generic *x1, Generic *x2);
  List *List3(Generic *x1, Generic *x2, Generic *x3);
  List *List4(Generic *x1, Generic *x2, Generic *x3, Generic *x4);
  List *List5(Generic *x1, Generic *x2, Generic *x3, Generic *x4, Generic *x5);


 
  void IterateList(ListMarker *, List *);   
  Bool NextOnList(ListMarker *, GenericREF itemref);
  List *InsertList(ListMarker *marker, List *list);
  List *SplitList(ListMarker *marker);
  void SetCurrentOnList(ListMarker *marker, Generic *handle);
  List *NextChunkOnList(ListMarker *, int chunksize);


# 200 "../basics.h" 2

# 1 "../symbol.h" 1
 



























































#pragma ident "S.c,v 1.3 1995/05/05 19:19:05 randall Exp Copyright 1994 Massachusetts Institute of Technology"





 
typedef enum { Nested, Flat } TableType;

  extern short Level;   

typedef void (*ConflictProc)(Generic *orig, Generic *create);
typedef void (*ShadowProc)(Generic *create, Generic *shadowed);
typedef void (*ExitscopeProc)(Generic *dead);


 




  SymbolTable *NewSymbolTable(const char *name, TableType kind,
				   ShadowProc, ExitscopeProc);

  void ResetSymbolTable(SymbolTable *table);

  void PrintSymbolTable(struct _iobuf  *out, SymbolTable *table);

  void EnterScope(void);
  void ExitScope(void);

  Bool LookupSymbol(SymbolTable *, const char *name, Generic **var);

  Generic *InsertSymbol(SymbolTable *, const char *name, Generic *var,
			     ConflictProc);

  void MoveToOuterScope(SymbolTable *, const char *name);

  const char *InsertUniqueSymbol(SymbolTable *table, Generic *var, const char *root);


typedef struct {
  SymbolTable *table;
  int i;
  void *chain;
} SymbolTableMarker;

  void IterateSymbolTable(SymbolTableMarker *, SymbolTable *);
  Bool NextInSymbolTable(SymbolTableMarker *, const char **name, GenericREF itemref);



# 201 "../basics.h" 2





 

  extern const float VersionNumber;      
  extern const char *const VersionDate;  
  extern const char * Executable;        
  extern List *Program;                  
  extern int WarningLevel;               
  extern int Line, LineOffset, Errors, Warnings;     
  extern unsigned int CurrentFile;       
  extern char *Filename;                 
  extern char *FileNames[      256 ];     
  extern const char *PhaseName;          

 
  extern Bool FileIncludedp[      256 ];  
  extern Bool CurrentIncludedp;          

 
  extern SymbolTable *Identifiers, *Labels, *Tags;

 
  extern SymbolTable *Externals;

 
  extern Bool DebugLex;                  
  extern Bool PrintLineOffset;           
  extern Bool IgnoreLineDirectives;      
  extern Bool ANSIOnly;                  
  extern Bool FormatReadably;            
  extern Bool PrintLiveVars;             

 

 
  void DPN(Node *n);
  void DPL(List *list);
  void PrintNode(struct _iobuf  *out, Node *node, int tab_depth);
  int PrintConstant(struct _iobuf  *out, Node *c, Bool with_name);
  void PrintCRSpaces(struct _iobuf  *out, int spaces);
  void PrintSpaces(struct _iobuf  *out, int spaces);
  void PrintList(struct _iobuf  *out, List *list, int tab_depth);
  int PrintOp(struct _iobuf  *out, OpType op);   
  void CharToText(char *array, unsigned char value);
  inline int PrintChar(struct _iobuf  *out, int c);     
  int PrintString(struct _iobuf  *out, const char *string);  

 
  volatile void  Fail(const char *file, int line, const char *msg);
  void SyntaxError(const char *fmt, ...);
  void Warning(int level, const char *fmt, ...);
  void SyntaxErrorCoord(Coord c, const char *fmt, ...);
  void WarningCoord(int level, Coord c, const char *fmt, ...);

 
  int yyparse(void), yylex(void);
  char *UniqueString(const char *string);    
  void SetFile(const char *filename, int line);          

 
  void VerifyParse(List *program);           

 
  List *SemanticCheckProgram(List *program);
  Node *SemCheckNode(Node *node);
  List *SemCheckList(List *list);

 
  List *TransformProgram(List *program);

 
  void OutputProgram(struct _iobuf  *out, List *program);





# 57 "strings.c" 2




typedef struct eT {
    char *string;
    struct eT *next;
} entryType;

static  entryType *hash_table[ 231 ];

 


static  char *copy_string(const char *string)
{
    char *new_string;

    new_string = (( char  *)HeapAllocate(  strlen(string)+1 , sizeof( char ))) ;
    return strcpy(new_string, string);
}

 
static  short hash_function(const char *string)
{
    unsigned short i, k;
    unsigned long val;

     (( string != 0  ) ? (void)0 : (void)Fail("strings.c", 85, "string != NULL")) ;
    
    val = (short) string[0] + 1;
    for(i = 1; i < 8; i++) {
	if (string[i] == 0) break;
	k = string[i] & 0x3f;
	val *= k + 7;
    }
    return((short)(val %  231 ));
}


  char *UniqueString(const char *string)
{
    short bucket = hash_function(string);
    entryType *entry;

    for (entry = hash_table[bucket]; entry != 0 ; entry = entry->next)
      if (strcmp(string, entry->string) == 0)
	return(entry->string);

     
    entry = (( entryType  *)HeapAllocate(1, sizeof( entryType ))) ;

    entry->string = copy_string(string);
    entry->next = hash_table[bucket];
    hash_table[bucket] = entry;

    return(entry->string);
}

