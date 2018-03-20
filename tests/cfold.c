
struct {
  char c;
  char d;
} goodbye;

/* Concern: outputs folded constant, rather than original expression.
   What if front-end and back-end disagree */
int hello = sizeof(goodbye) * 2;


const int x;     /* should be warning: const obj should have initializer */
const int y = 5;

typedef const int CI;  /* Bug: spurious warning (constant not initialized!) */

CI z = 9;

int foo[] = {   x, 
		y, 
		z };  /* Bug: these are all non-constant, so should be error */


enum {
  a, b, c
  };

int test = a;  /* Spurious error: a considered non-constant */
int test2 = 0 ? 1 : 2;  /* Spurious error: ternary expression not recognized as constant */
int test3 = (0,0,0,1);  /* Bug: should be error */
int boo[0? 2 : 4]; /* Bug: should be allowed */
int test4 = 1 + 2 + 3 * (5 / 3) + 8 - 12;

int apple[(int) 1.0];  /* bug: should be allowed */
const int nil = 0;
enum {zero = 0};

void testing(char *p)
{
  p != nil;  /* Bug: should be a warning, since nil is not constant 0 */
  p != !1;   /* Bug: should be legal, but evaluates !1 to 1 instead of 0 */
  p != 0;   /* legal */
  p != zero; /* legal */
}


int bar[y];  /* Bug: should be error, since y is not constant expr.
	        Actually causes assertion failure! */


int *bogus = &foo[1];

int *boguser = foo + 2;
