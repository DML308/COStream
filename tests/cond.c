

struct boo *p;
struct boo *q;

void foo()
{
  p = (1 ? 0 : q);
  p = (0 ? q : 0);
}
