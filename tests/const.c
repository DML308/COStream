
struct glum {
  const int id;
  double val;
} test[] = { 
  { 1, 2.32},
  { 2, 5.67}
};

const int rot[5] = { 1, 2, 3, 4, 0 };	       

int tt[3];
int ss[3];

typedef const char *CPCH;

void foo(const char *p, char const *q, char * const r, CPCH p2)
{
  ++p;  /* legal */
  ++q;  /* legal */
  ++r;  /* error or warning */

  (*p)++; /* bug: should be error/warning */
  (*q)++; /* bug: should be error/warning */
  (*r)++; /* legal */

  test[0].val = .00354; /* legal */
  test[0].id = 3; /* bug: should be error/warning */
  test->id = 3;   /* bug: error should be "left operand must be modifiable
		     lvalue."  Actually is "assignment type mismatch." */

  rot[0] = 810;  /* should be error/warning */

  tt = ss;  /* bug: error should be "left operand must be modifiable
	       lvalue."  Actually is "assignment type mismatch." */  

  ++p2; /* legal */
  ++(*p2); /* Bug: should be error/warning */
}

