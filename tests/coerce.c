char *p = 5;  /* bug: should be error */
typedef int fi(int x);
char *foo()
{
 int a;
 int x[5];
 void *q;

 a = (int)foo;  /* bug: should be permissible */
 a = (q < 0);   /* bug: should be illegal */
 ~x;  /* bug: should be error, actually core dumps */

 q = (char []) x; /* casting to array, should be error */
 q = (fi) x; /* casting to function, should be error */
 return 5; /* bug: should give warning */ 
}
