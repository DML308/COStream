
extern int a[3];
extern int a[3];  /* legal */
extern int a[4];  /* illegal: extern 'a' redeclared */

enum rid
{
  RID_UNUSED,
  RID_INT,
  RID_CHAR,
  RID_FLOAT,
  RID_DOUBLE,
  RID_VOID,
  RID_UNUSED1,

  RID_UNSIGNED,
  RID_SHORT,
  RID_LONG,
  RID_AUTO,
  RID_STATIC,
  RID_EXTERN,
  RID_REGISTER,
  RID_TYPEDEF,
  RID_SIGNED,
  RID_CONST,
  RID_VOLATILE,
  RID_INLINE,
  RID_NOALIAS,
  RID_ITERATOR,
  RID_COMPLEX,

  RID_IN,
  RID_OUT,
  RID_INOUT,
  RID_BYCOPY,
  RID_ONEWAY,
  RID_ID,

  RID_MAX
};

extern int arr[(int) RID_MAX];
extern int arr[(int) RID_MAX];  /* legal */
extern int arr[3];  /* illegal: extern 'arr' redeclared */
