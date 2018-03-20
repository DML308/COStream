
char *pch;
const char *pchc;

char **ppch;
const char *ppchc;
char *const *ppcch;
const char *const *ppcchc;

void foo()
{

/* K&R p. 209 (top): [assignment is legal when] both operands are pointers to
   functions or objects whose types are the same except for the possible 
   absence of const or volatile in the right operand. */

  pch = pchc;  /* illegal without cast */
  pchc = pch;  /* legal */

  ppch = ppchc;  /* illegal: different base types: pch and pchc */
  ppch = ppcch;  /* bug: illegal */
  ppch = ppcchc; /* illegal: different base types: pch and pchc */

  ppchc = ppch;  /* illegal: different base types */
  ppcch = ppch;  /* legal */
  ppcchc = ppch;  /* bug: illegal: different base types */
  ppcchc = ppcch; /* bug: illegal: different base types */
}

