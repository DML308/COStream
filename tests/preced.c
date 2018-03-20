
void foo(int x, int y, int z)
{
  x = x+1 || y+1;

  if (x && y || z)
    x = x | y + z ^ x;

  x = x | y * z;  /* no warning */
  x = x | y >> z;  /* no warning */
  x = x | y / z;  /* no warning */

  if (x & y == z)
    z = y;
}
