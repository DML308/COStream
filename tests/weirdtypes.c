

/* Hungarian notation is used below to indicate the intended type of
   each declaration.  The meanings of the Hungarian prefixes are as
   follows:

       p     pointer to
       a     array of
       f     function returning

       i     int
       ch    char

*/


void test()
{
  char *pch;
  int i;

  int *pi;
  int ai[3];
  int fi(int,int);

  int *api[3];
  int (*pai)[3];
  int *(*papi)[3];

  int *fpi(int, int);
  int (*pfi)(int, int);

  int aai[2][4];

  /* int *afpi[3](int, int);   illegal, since arrays cannot store fns */
  int *(*apfpi[3])(int, int);
  /* int *fapi(int, int)[3];   illegal, since functions must return scalar */
  int *(*fpapi(int, int))[3];
  int *(*(*pfpapi)(int, int))[3];

  int (*(*pfpfi)(int,int)) (int,int);
 
  /* test whether decls were parsed correctly ---------------------- */
  i = 0;
  
  pi = &i;
  ai[1] = i;
  *ai = i;
  i = fi(i,i);
  
  api[1] = pi;
  pai = &ai;  /* note: pai = ai should NOT typecheck */
  papi = &api;

  pi = fpi(i,i);
  pfi = &fi;  pfi = fi;    /* bug: should type-check with and without the & */
  
  aai[1][1] = i;
  
  apfpi[1] = &fpi;   apfpi[1] = fpi;
  papi = fpapi(i, i);
  pfpapi = &fpapi;  pfpapi = fpapi;

  i = (&(*pfi))(i,i);

  /* test parsing of abstract types (casts) ---------------------- */
  i = (int) pch;
  
  pi = (int *) pch;
  /*((int [3]) pch)[1] = i; illegal */
  /*i = ((int (int,int)) pch) (i, i);  illegal */

  /*((int *[3]) pch)[1] = pi;  illegal */
  pai = (int (*) [3]) pch;
  papi = (int *(*) [3]) pch;

  /* pi = ((int *(int,int)) pch) (i,i); illegal */
  i = ((int (*) (int,int)) pch) (i,i);
  
  /* ((int [2][4]) pch)[1][1] = i;  illegal */

  /*  ((int *(*[3])(int,int)) pch)[1] = &fpi;  illegal */
  /*  papi = ((int *(*(int, int))[3]) pch) (i, i);  illegal */
  papi = ((int *(*(*)(int, int))[3]) pch) (i, i);

  pfi = pfpfi(i,i);
  pfi = (*pfpfi(i,i));
  pfi = (********pfpfi(i,i));   /* Bug: ANSI C says this is legal, though weird */
}
    

