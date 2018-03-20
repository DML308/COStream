#line 1 "init.c"

int xxx[] = (0,0,1);   

int yyy[] = {0,0,1};   




struct baz {
  int a;
} baz1;

int funco()
{
  struct baz y = baz1;   
  return y.a;
}



struct foo {
  int sdfdsf;
  int a;
};
int zz;
struct foo bar[5];
int *xx = &((bar[0]).a);   




struct fool { int x, y; };
struct foom { char *s, *t; };
struct fool a;
union un1 { int x, y; };

void barbar()
{
  extern char msg[10];
  struct fool f1 = a;     
  struct fool f2 = { a.x };   
  union un1 g1 = { a.x };   
  struct foom f3 = { msg };   

   
  f1.x = f2.x;
  f3.s[0] = f1.x;
  g1.x = f3.s[0];
  f2.x = g1.x;
}





 


int yoff =   (   (char *)&((( struct fool  *)0)->  y ) - (char *)(( struct fool  *)0)  ) ;    





union {
	int a[10];
	int b[10];
} u;
int c[10];

struct {
  int d[10];
  int e[10];
} s;


 
int right1 = &(u.a[1]) - u.b;
int right2 = &(c[1]) - c;
int diff = &(s.e[5]) - &(s.d[2]);   
int diff2 = s.e - s.d;

 
int wrong1 = &(u.a[1]) - c;


