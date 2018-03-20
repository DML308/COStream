
#define max(a,b) \
 ( {int _a=(a); int _b=(b); _a > _b ? _a : _b; } )

typedef unsigned long long int big;

big foo(int a)
{
   return (big) 0 + (((big)0x12345678 << 32) | (big)0x12345678);
}

int bar()
{
  return max(1,2);
}



union u {
  int i;
  float f;
  double d;
};

union u u1 = (union u) 5.4;

union u uboo()
{
 union u u2;
 int i = 5;
 double d;

 u2 = d;
 return i;
}
