#define KTSIZE 2048

typedef long long DoubleWord;


inline DoubleWord
make_key(int x, int y, int z){
  DoubleWord result;
  result = KeyTable[ ( (z<<6) + (x<<3) + y )&(KTSIZE-1) ) ];
                             /* added extra paren here  ^   */
  result =| result<<32;
  result += ((x<<21)|(z<<14)|(y<<7)| ( (x&5)+(y&5)+(z&5)));
  return(result);
}
