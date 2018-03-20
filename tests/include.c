
typedef int foo;

#include "include.h"


void f()
{
  int x;
#include "include.h"
  
  x = y;
}
