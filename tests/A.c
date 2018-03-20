int *fopen();
int *tmp[3];
int (*tmp2)[3], (**f)(void);
typedef int **fred;
volatile fred (*tmp3[5])(int);

void f(int x, int y)
{
    int c;

  top:
    x = 3;
    goto third;
/*next:  omitted because it caused indeterminism in error message output */
    y = 4;
/*  goto exit;    omitted because it caused indeterminism in error message output */
  third:
    y = 5;
}


int g(int c)
{
    goto third;
}
