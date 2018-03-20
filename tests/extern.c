
 static int x;

 int x;   /* illegal */

extern int x;


extern int y;

static int y;


 extern int redundant;  /* non-redundant */
 extern int redundant;  /* redundant */
 extern int redundant;  /* redundant */

 int redundant = 5;  /* non-redundant */

 extern int redundant;  /* redundant */

 int redundant = 6;  /* illegal */

 void foo()
 {
   extern int redundant;  /* redundant */

   {
     extern int redundant2;  /* non-redundant */
   }
   {
     extern int redundant2;  /* non-redundant */
   }

 }




char bar=5;
char bar=6;
