static int static k;

int x[2][2.3]["hi"]['a'];

int **y[3][4];

typedef int T;

struct { };

struct hi { int x; };
union there { int x; };
struct hi { int y; };
union there { int y; };
enum test { Hi, There };
enum test { Second, Def };

const static volatile int h;
static const typedef volatile int h2;
static const const int h3;
const int const h4;

unsigned unsigned int q;
signed signed char p;
long p1; short p2; char p3;
signed p4; unsigned p5;
int static p6;
int volatile p6b;
unsigned int unsigned p7;
signed long signed p8;
static int static p9;

struct hello { };

int (*x1);
int (x2);

typedef int X;
X const p10;

int y = { 3, 4, 5, };

g(int x)
{
    x = (int * const) 10;
    x = (int * []) 20;
    x = (int * const []) 30;
    x = (int (* const)[]) 32;
    x = (int * const([])) 34;
    x = (int (*)(register int[], auto int x, auto int T, register,
		 register [], register const x, const, const *, const x)) 40;
}

double f(extern T y) {
    int x = -10;
    auto x1;

  top:
    do {
	x++;
	x %= sizeof x / sizeof ++x;
	x |= 0; x &= 1;
	x ^= x |= 4 &= ~4;
    } while (x < 0);
  top:
    goto top;
    goto top;
}

int f2(int x) {
    struct { int y; } p;
    struct { int z; } register q;
    register struct { int z; } const q2;
    volatile register struct { int z; } const q3;
    int;

    p.y = 4;
}

static  f3() {
    register struct { int z; } p2;
}

const f4() { }

f5() { }

static int f6() {}

long f7() {}

static const f8() {}

g1(x) int x; {}
static g2(x) long x; {}
int g3(x) long x; {}
const static *g4(x) int x; {}
const * const g5(x) int x; {}


