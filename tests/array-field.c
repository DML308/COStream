
struct {
  int a[10];
  int b[10];
} foo;

int b[10];

void bar()
{
  foo.a = foo.b;  /* bug: should be error, since foo.a not an lvalue */
}
