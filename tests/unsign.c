int x = 0;    /* bug: parser turns 0 into 0U */

long int eval;

long l;

int bar = 0xFF;

int qux = 0506;

unsigned k = -5;

int unsigned m;
char unsigned uc;

void foo(unsigned int u)
{
  l = 0L - eval;  /* bug: parser turns 0L into 0UL */
  if (u == 256U)  /* bug: parser turns 256U into 256UL */
    l = u;
}
