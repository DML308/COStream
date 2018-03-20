
void foo() /* no warning about return value */
{
  int a, b, c, d, e, f, g, h, i, j, k, l;
  int ***m;
  int n, o, p;
  void (*q) (int, int, int, int);
  int r, s, t, u;

  a+b;
  !c;
  (void)d;
  e, f, g, h, i;
  j ? k : l;
  m[n][o][p];
  q(r,s,t,u);
}


int bar()  /* no warning about return value */
{
  int a, b, c, d, f, g, i, k, l, m, n, o, p, q;

 L1:
  a;

  if (b) 
    goto L2;
  goto L1;

 L2:
  switch (c) {
    d;  /* unreachable */
  case 0:
    m = 5;
    f;
    break;
    g;  /* unreachable */
  case 1:
    m = 5;
    i;
  case 2:
  default:
    m = 5;
    k;
    break;
    l;  /* unreachable */
  }

  if (m)
    n;
  
  if (o)
    p;
  else q;
}

int qux()
{
  int a, b, c, d, e, f, g, h, i, j, k, l;

  while (a) {
    b;
    continue;
    c;  /* unreachable */
  }

  do {
    break;
    d;  /* unreachable */
  } while (e);

  for (f; g; h)
    i;

  return j;

  {
    int decl = k;  /* unreachable */
    decl+l;
  }
}


char boo()   /* expect warning about return value */
{
}

typedef struct { int x; } somestruct;

int aliasing()
{
  int r1, r2, r3;
  /* these variables aren't register candidates, shouldn't be analyzed */
  int w, x, y, z, arr[22];  
  somestruct s;

  r1;
  r2;
  r3;
  &w, &x, &y, &z;
  s.x = 5;
  arr[5] = 0;

  if (w)
    return x;
}
