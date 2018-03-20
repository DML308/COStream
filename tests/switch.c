
void foo()
{

  switch (1) {
  case 1: { }  /* bug: parser turns this empty block into "0;" */
    break;
  }

  switch (2) {  /* bug: asserts on empty switch */
  }

}

void bar()
{
  char *s = "hi there";
  int a;

  switch (s) {  /* bug: doesn't check switch expr for scalarness */
  case s:  /* bug: doesn't check that case expr is scalar and constant */
    break;
  default:
  case a+1: /* bug: not constant, no error */
    break;
  case 1:
  case 1:   /* bug: duplicate case labels */
    break;
  default:  /* bug: redundant default, no error */
    break;
  }
}
