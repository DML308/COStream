
struct bar {int x;};

int foo(struct bar b)
{
  return foo(1) + foo(1,2) + foo();
}
