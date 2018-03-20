
typedef struct unknown U;

U *foo(U *p, void b[])  /* warn about array of voids */
{
  int i = sizeof(b[1]);  /* illegal: can't take sizeof(void) */
  int j = sizeof(*p);    /* illegal: can't take sizeof incomplete type */
  void *x;
  x+=5;  /* illegal */
  b[i];   /* illegal: can't compute index into array of voids */
  x = b + j;  /* illegal */
  return p + 4; /* illegal */
  

  {
   U**bleah;

    bleah = bleah + 5;  /* legal! */
  }
}

