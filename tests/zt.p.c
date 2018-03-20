#line 1 "./tests/zt.c"
int main(void)
{
int a[5]={1,2,3,4,5};
int i;
int b[3][3]={{4,5,6},{7,8,9},{10,11,12}};

for (i=0; i<3; i=i+1) 
 {
  int t;
  t=i;
  a[i]=t;
 }

}
