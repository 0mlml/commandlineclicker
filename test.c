#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>

int main()
{
  POINT p;
  GetCursorPos(&p);
  printf("x: %ld, y: %ld", p.x, p.y);
  return 0;
}