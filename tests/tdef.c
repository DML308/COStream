
typedef int foo;
typedef foo bar;

bar k;
foo l;


bar f(foo x); 
foo f(bar x); /* bug: should be legal, since signatures do not conflict after
		 typedefs are replaced with their true types */

foo f(foo x)
{
  k = x;
  return k;
}


typedef void Generic;
typedef void **GenericREF;  /* address of a Generic, for pass by reference */


typedef struct nodeStruct Node;
struct nodeStruct {
	int x;
	int y;
      };

void appendit(Generic *lst, int x)
{
  lst = 0;
  x = 5;
}

void doit()
{
  Node *ls;

  appendit(ls, 1);
}




