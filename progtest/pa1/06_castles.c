#ifndef __PROGTEST__
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
constexpr int MAP_MAX = 200;
#endif /* __PROGTEST__ */

int find_x_bound(int x, int y, int dx, int xend, int size, int alt,
                 int altitude[][MAP_MAX]) {
  int prev = x;
  while (true) {
    x += dx;
    if (x < 0 || x >= size || altitude[x][y] >= alt) {
      return prev;
    }
    if (x == xend) {
      return x;
    }

    prev = x;
  }
}

int find_y_bound(int x, int y, int dy, int yend, int size, int alt,
                 int altitude[][MAP_MAX]) {
  int prev = y;
  while (true) {
    y += dy;
    if (y < 0 || y >= size || altitude[x][y] >= alt) {
      return prev;
    }
    if (y == yend) {
      return y;
    }

    prev = y;
  }
}

int rectangle_size(int x, int y, int size, int altitude[][MAP_MAX]) {
  int alt = altitude[x][y];

  int x_min_arr[MAP_MAX] = {};
  int x_max_arr[MAP_MAX] = {};
  int y_min_arr[MAP_MAX] = {};
  int y_max_arr[MAP_MAX] = {};

  int x_min = find_x_bound(x, y, -1, 0, size, alt, altitude);
  int x_max = find_x_bound(x, y, 1, 0, size, alt, altitude);

  int prev_min = -1;
  int prev_max = -1;
  for (int x_ = x_min; x_ <= x_max; x_++) {
    prev_min = find_y_bound(x_, y, -1, prev_min, size, alt, altitude);
    x_min_arr[x_] = prev_min;
    prev_max = find_y_bound(x_, y, 1, prev_max, size, alt, altitude);
    x_max_arr[x_] = prev_max;
  }

  int y_min = x_min_arr[x];
  int y_max = x_max_arr[x];

  prev_min = -1;
  prev_max = -1;
  for (int y_ = y_min; y_ <= y_max; y_++) {
    prev_min = find_x_bound(x, y_, -1, prev_min, size, alt, altitude);
    y_min_arr[y_] = prev_min;
    prev_max = find_x_bound(x, y_, 1, prev_max, size, alt, altitude);
    y_max_arr[y_] = prev_max;
  }

  printf("\n");
  printf("Raw\n");
  for (int y_ = 0; y_ < size; y_++) {
    for (int x_ = 0; x_ < size; x_++) {
      if (x_ == x && y_ == y) {
        printf(" x");
      } else if (altitude[x_][y_] < alt) {
        printf(" .");
      } else {
        printf(" *");
      }
    }
    printf("\n");
  }
  printf("Convex\n");
  for (int y_ = 0; y_ < size; y_++) {
    for (int x_ = 0; x_ < size; x_++) {
      if (x_ == x && y_ == y) {
        printf(" x");
      } else if (x_ >= x_min && x_ <= x_max && y_ >= y_min && y_ <= y_max &&
                 x_ >= y_min_arr[y_] && x_ <= y_max_arr[y_] &&
                 y_ >= x_min_arr[x_] && y_ <= x_max_arr[x_]) {
        printf(" .");
      } else {
        printf(" *");
      }
    }
    printf("\n");
  }
  // printf("\n");
  // for (int y_ = 0; y_ < size; y_++) {
  //   for (int x_ = 0; x_ < size; x_++) {
  //     if (x_ == x && y_ == y) {
  //       printf("  x  ");
  //     } else if (x_ == x) {
  //       printf("%2d/%-2d", y_min_arr[y_], y_max_arr[y_]);
  //     } else if (y_ == y) {
  //       printf("%2d/%-2d", x_min_arr[x_], x_max_arr[x_]);
  //     } else {
  //       printf("     ");
  //     }
  //   }
  //   printf("\n");
  // }
  // printf("\n");

  int area = 0;
  for (int i = x_min; i <= x; i++) {
    for (int j = x; j <= x_max; j++) {
      int min = 0;
      int max = 0;
      for (int y_ = y; y_ >= y_min; y_--) {
        if (y_min_arr[y_] > i || y_max_arr[y_] < j) {
          break;
        }
        min = y_;
      }
      for (int y_ = y; y_ <= y_min; y_++) {
        if (y_min_arr[y_] > i || y_max_arr[y_] < j) {
          break;
        }
        max = y_;
      }
      int area_ = (max - min + 1) * (j - i + 1);
      if (area_ > area) {
        area = area_;
      }
    }
  }

  return area;
}

void castleArea(int altitude[][MAP_MAX], int size, int area[][MAP_MAX]) {
  for (int j = 0; j < size; j++) {
    for (int i = 0; i < size; i++) {
      area[i][j] = rectangle_size(i, j, size, altitude);
    }
  }
}

#ifndef __PROGTEST__
bool identicalMap(const int a[][MAP_MAX], const int b[][MAP_MAX], int size) {
  for (int j = 0; j < size; j++) {
    for (int i = 0; i < size; i++) {
      if (a[i][j] != b[i][j]) {
        return false;
      }
    }
  }
  return true;
}

void print_grid(int grid[][MAP_MAX], int size) {
  for (int j = 0; j < size; j++) {
    for (int i = 0; i < size; i++) {
      printf(" %2d", grid[i][j]);
    }
    printf("\n");
  }
}

void compare_area(int input[][MAP_MAX], int output[][MAP_MAX],
                  int expected[][MAP_MAX], int size) {
  if (!identicalMap(output, expected, size)) {
    printf("\nInput:\n");
    print_grid(input, size);

    printf("Expected:\n");
    print_grid(expected, size);

    printf("Got:\n");
    print_grid(output, size);
  }
}

int main(int argc, char *argv[]) {
  // clang-format off
  static int result[MAP_MAX][MAP_MAX];

  static int alt0[MAP_MAX][MAP_MAX] =
  {
    { 1, 2 },
    { 3, 4 }
  };
  static int area0[MAP_MAX][MAP_MAX] =
  {
    { 1, 2 },
    { 2, 4 }
  };
  castleArea ( alt0, 2, result );
  compare_area ( alt0,  result, area0, 2 );
  static int alt1[MAP_MAX][MAP_MAX] =
  {
    { 2, 7, 1, 9 },
    { 3, 5, 0, 2 },
    { 1, 6, 3, 5 },
    { 1, 2, 2, 8 }
  };
  static int area1[MAP_MAX][MAP_MAX] =
  {
    { 1, 12, 2, 16 },
    { 4, 4, 1, 2 },
    { 1, 9, 4, 4 },
    { 1, 2, 1, 12 }
  };
  castleArea ( alt1, 4, result );
  compare_area ( alt1,  result, area1, 4 );
  static int alt2[MAP_MAX][MAP_MAX] =
  {
    { 1, 2, 3, 4 },
    { 2, 3, 4, 5 },
    { 3, 4, 5, 6 },
    { 4, 5, 6, 7 }
  };
  static int area2[MAP_MAX][MAP_MAX] =
  {
    { 1, 2, 3, 4 },
    { 2, 4, 6, 8 },
    { 3, 6, 9, 12 },
    { 4, 8, 12, 16 }
  };
  castleArea ( alt2, 4, result );
  compare_area ( alt2,  result, area2, 4 );
  static int alt3[MAP_MAX][MAP_MAX] =
  {
    { 7, 6, 5, 4 },
    { 6, 5, 4, 5 },
    { 5, 4, 5, 6 },
    { 4, 5, 6, 7 }
  };
  static int area3[MAP_MAX][MAP_MAX] =
  {
    { 12, 6, 2, 1 },
    { 6, 2, 1, 2 },
    { 2, 1, 2, 6 },
    { 1, 2, 6, 12 }
  };
  castleArea ( alt3, 4, result );
  compare_area ( alt3,  result, area3, 4 );
  static int alt4[MAP_MAX][MAP_MAX] =
  {
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 2, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 }
  };
  static int area4[MAP_MAX][MAP_MAX] =
  {
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 25, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 }
  };
  castleArea ( alt4, 5, result );
  compare_area ( alt4,  result, area4, 5 );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
