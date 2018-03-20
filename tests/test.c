
#define fldoff(str, fld)        ((char *)&(((struct str *)0)->fld) - (char *)0)



extern int      printf();

typedef struct coord {
  unsigned file:8;
  unsigned line:24;
} Coord;

struct s {
    enum { A, B, C } a;
    Coord b;
    double d;
    int c;
};

typedef enum { X, Y=X-1, Z,
	       B, Y } ted;
    
int main()
{
    int Z;

    printf("%d %d %d %d\n",
	   fldoff(s, a),
	   fldoff(s, b),
	   fldoff(s, c),
	   fldoff(s, d));
    return(0);
}
