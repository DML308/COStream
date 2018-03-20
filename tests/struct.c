
struct foo {
  struct bar *x;
  struct baz {
    struct baz *k;
  } *y;
} a, b;

int size = sizeof(struct baz);

struct bar {
  struct foo f;
};

struct {
  int x, y;
} d, e;

void foo(void)
{
  d = e;
}




