
int foo __attribute__ ((aligned(8))),
    bar __attribute__ ((packed,aligned(4)));

void _exit() __attribute__ ((noreturn));
void access() __attribute__ ((const));

struct bleah {
  char goober[25] __attribute__ ((aligned(16)));
};

typedef long long DoubleWord;
typedef int SWORD;
typedef short S16;

struct searchstate
 { 
   DoubleWord   key  __attribute__ ((aligned (8)));      /* current hash key */
   SWORD   depth;      /* depth to search                           */
   S16   ply_in_game; /* corresponds to the index of the pointer. */
 } sss;

