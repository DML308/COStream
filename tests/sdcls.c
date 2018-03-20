
struct empty { };

struct empty e1;

struct empty2 {} e2;

void foo()
{
 struct empty;
 struct empty2 {
   struct empty *x;
 };
 struct empty {
   struct empty2 *y;
 };

 struct incomplete *pi;

 pi->test = 0;  /* illegal */
}

struct empty a[5]; /* legal */

/* now the same thing, but with unions */

union uempty {};

union uempty ue1;

union uempty2 {} ue2;

void ufoo()
{
 union uempty;
 union uempty2 {
   union uempty *x;
 };
 union uempty {
   union uempty2 *y;
 };
}

union uempty ua[5]; /* legal */



/* now unsized arrays */


int b[0];
int sb = sizeof(b); /* legal */

int c[];
int sc = sizeof(c); /* illegal */
