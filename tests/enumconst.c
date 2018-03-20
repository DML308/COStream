

typedef enum try {
  /* Bug: assigns b:=1 rather than b:=a+1 */
  a = 20,
  b,
  c
  } Try;

Try x = a;
