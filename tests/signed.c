
signed int *psi;
unsigned int *pui;
int *pi;

signed char *psch;
unsigned char *puch;
char *pch;

void foo()
{
  psi = pi;   /* no warning */
  pui = pi;   /* expect warning */
  pi = psi;   /* no warning */

  psch = pch;  /* expect warning */
  puch = pch;  /* expect warning */
  pch = psch;  /* expect warning */
  pch = puch;  /* expect warning */

  psch = "testing";   /* this should give a warning (different signedness) */
  puch = "testing";   /* this should give a warning (different signedness) */
  pch = "testing";   /* no warning */
}
