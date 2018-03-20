
struct {
  int x;
  int y;
} a, b;

struct {
  int x;
  int y;
} c;

int foo(void)
{
  a = b;
  a = c; /* bug: should be error, actually core-dumps */
}
