#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int euclid(int m, int n)
{
  #ifndef NDEBUG 
  assert(n > 0 && m >= n);
  #endif
  
  int remainder = m % n;
  if (remainder == 0)
    return n;
  else
    return euclid(n, remainder);
}

int main(int argc, char **argv)
{
  int gcd;
  gcd = euclid(-5,-1); // this would cause abort.

  gcd = euclid(100350, 23094);
  printf("%d\n",gcd);
  return EXIT_SUCCESS;
}
