#line 2 "analysis.c"
void foo(void)
{
  int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;
  int ***m;
  int n;int o;int p;
  void( *q) (int, int, int, int);
  int r;int s;int t;int u;

  a+b; /* Live: c d e f g h i j k l m n o p q r s t u  */
  !c; /* Live: d e f g h i j k l m n o p q r s t u  */
  (void)d; /* Live: e f g h i j k l m n o p q r s t u  */
  e, f, g, h, i; /* Live: j k l m n o p q r s t u  */
  j ? k : l; /* Live: m n o p q r s t u  */
  m[n][o][p]; /* Live: q r s t u  */
  q(r,s,t,u);
}


int bar(void)
{
  int a;int b;int c;int d;int f;int g;int i;int k;int l;int m;int n;int o;int p;int q;

 L1:
  a; /* Live: a b c f i k n o p q  */ /* Live: a b c f i k n o p q  */

  if (b) 
    goto L2; /* Live: c f i k n o p q  */ /* Live: a b c f i k n o p q  */
  goto L1; /* Live: a b c f i k n o p q  */

 L2:
  switch (c) {
    d; /* Live: f n o p q  */
  case 0: 
    m = 5; /* Live: f m n o p q  */ /* Live: f m n o p q  */
    f; /* Live: m n o p q  */
    break; /* Live: m n o p q  */
    g; /* Live: i k n o p q  */
  case 1: 
    m = 5; /* Live: i k n o p q  */ /* Live: i k n o p q  */
    i; /* Live: k n o p q  */
  case 2: 
  default: 
    m = 5; /* Live: k m n o p q  */ /* Live: k m n o p q  */ /* Live: k m n o p q  */
    k; /* Live: m n o p q  */
    break; /* Live: m n o p q  */
    l; /* Live: m n o p q  */
  } /* Live: m n o p q  */ /* Live: m n o p q  */ /* Live: m n o p q  */

  if (m) 
    n; /* Live: o p q  */ /* Live: o p q  */

  if (o) 
    p;
  else q;
}

int qux(void)
{
  int a;int b;int c;int d;int e;int f;int g;int h;int i;int j;int k;int l;

  while (a) {
    b; /* Live: a b f g h i j  */
    continue; /* Live: a b f g h i j  */
    c; /* Live: a b f g h i j  */
  } /* Live: a b f g h i j  */ /* Live: f g h i j  */

  do {
    break; /* Live: f g h i j  */
    d; /* Live: e f g h i j  */
  } /* Live: e f g h i j  */while (e); /* Live: f g h i j  */

  for (f; g; h) 
    i; /* Live: g h i j  */ /* Live: j  */

  return j;

  {
    int decl=  k;
    decl+l;
  }
}


char boo(void)
{
}

typedef struct ___sue1 {int x;} somestruct;

int aliasing(void)
{
  int r1;int r2;int r3;

  int w;int x;int y;int z;int arr[22];
  somestruct s;

  r1; /* Live: r2 r3  */
  r2; /* Live: r3  */
  r3;
  &w, &x, &y, &z;
  s.x = 5;
  arr[5] = 0;

  if (w) 
    return x;
}
