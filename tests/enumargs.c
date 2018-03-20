

typedef enum try {
  at,bt,ct
  } Try;

typedef enum fail {
  df, ef, ff
  } Fail;

Try y;

void proc1(Try x);

void proc1(Fail x)  /* illegal: redeclaration */
{
  y = x;  /* should be legal */
}
