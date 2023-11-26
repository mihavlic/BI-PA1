#include <stdio.h>

int main() {
  printf("Zadejte barvu v RGB formatu:\n");

  int r, g, b = 0;
  char c[2] = {};
  int n = 0;
  scanf(" rgb (%d ,%d ,%d )%n %1s", &r, &g, &b, &n);
  
  if (
    n == 0
    || c[0] != 0
    || 0 > r || r > 255
    || 0 > g || g > 255
    || 0 > b || b > 255
  ) {
    printf("Nespravny vstup.\n");
    return 1;
  }

  printf("#%02X%02X%02X\n", r, g, b);
  return 0;
}