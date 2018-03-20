
typedef int foo;
typedef foo bar;

struct bleah {
  foo x;
  bar y;
} z;

int a = sizeof(z);
