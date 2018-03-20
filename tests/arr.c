
int i;

int *pi;

int **ppi;

void foo()
{
  *pi = 0;   /* legal */
  pi[0] = 0; /* legal */
  *i = 0;    /* illegal */
  i[0] = 0;  /* illegal */

  **ppi = 0;     /* legal */
  ppi[0][0] = 0; /* legal */
  **pi = 0;      /* illegal */
  pi[0][0] = 0;  /* illegal */
}
