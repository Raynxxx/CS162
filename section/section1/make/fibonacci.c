#include "functions.h"

int fibonacci (int n) {
  if (n == 0) return 0;
  if (n == 1) return 1;
  int current = 1, previous = 0, temp;
  while (n >= 2) {
    temp = current;
    current = current + previous;
    previous = temp;
    n--;
  }
  return current;
}
