#line 2 "signed.c"
int        *psi;
unsigned     *pui;
int *pi;

signed char *psch;
unsigned char *puch;
char *pch;

void foo(void)
{
  psi = pi;
  pui = pi;
  pi = psi;

  psch = pch;
  puch = pch;
  pch = psch;
  pch = puch;

  psch = "testing";
  puch = "testing";
  pch = "testing";
}
