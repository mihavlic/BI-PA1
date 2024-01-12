#ifndef __PROGTEST__
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
constexpr int MAP_MAX = 200;
#endif /* __PROGTEST__ */

int find_x_bound(int x, int y, int dx, int size, int alt,
                 int altitude[][MAP_MAX]) {
  int prev = x;
  while (true) {
    x += dx;
    if (x < 0 || x >= size || altitude[x][y] >= alt) {
      return prev;
    }
    prev = x;
  }
}

int find_y_bound(int x, int y, int dy, int size, int alt,
                 int altitude[][MAP_MAX]) {
  int prev = y;
  while (true) {
    y += dy;
    if (y < 0 || y >= size || altitude[x][y] >= alt) {
      return prev;
    }
    prev = y;
  }
}

void make_convex(int min_array[], int max_array[], int start, int end,
                 int middle) {
  int prev_min = min_array[middle];
  int prev_max = max_array[middle];
  for (int i = middle - 1; i >= start; i--) {
    if (prev_min > min_array[i]) {
      min_array[i] = prev_min;
    }
    if (prev_max < max_array[i]) {
      max_array[i] = prev_max;
    }
    prev_min = min_array[i];
    prev_max = max_array[i];
  }

  prev_min = min_array[middle];
  prev_max = max_array[middle];
  for (int i = middle + 1; i <= end; i++) {
    if (prev_min > min_array[i]) {
      min_array[i] = prev_min;
    }
    if (prev_max < max_array[i]) {
      max_array[i] = prev_max;
    }
    prev_min = min_array[i];
    prev_max = max_array[i];
  }
}

int rectangle_size(int x, int y, int size, int altitude[][MAP_MAX]) {
  int alt = altitude[x][y];

  int x_min_arr[MAP_MAX] = {};
  int x_max_arr[MAP_MAX] = {};
  int y_min_arr[MAP_MAX] = {};
  int y_max_arr[MAP_MAX] = {};

  int x_min = find_x_bound(x, y, -1, size, alt, altitude);
  int x_max = find_x_bound(x, y, 1, size, alt, altitude);
  for (int x_ = x_min; x_ <= x_max; x_++) {
    x_min_arr[x_] = find_y_bound(x_, y, -1, size, alt, altitude);
    x_max_arr[x_] = find_y_bound(x_, y, 1, size, alt, altitude);
  }
  make_convex(x_min_arr, x_max_arr, x_min, x_max, x);

  int y_min = x_min_arr[x];
  int y_max = x_max_arr[x];
  for (int y_ = y_min; y_ <= y_max; y_++) {
    y_min_arr[y_] = find_x_bound(x, y_, -1, size, alt, altitude);
    y_max_arr[y_] = find_x_bound(x, y_, 1, size, alt, altitude);
  }
  make_convex(y_min_arr, y_max_arr, y_min, y_max, y);

  // printf("\n");
  // printf("Raw\n");
  // for (int y_ = 0; y_ < size; y_++) {
  //   for (int x_ = 0; x_ < size; x_++) {
  //     if (x_ == x && y_ == y) {
  //       printf(" x");
  //     } else if (altitude[x_][y_] < alt) {
  //       printf(" .");
  //     } else {
  //       printf(" *");
  //     }
  //   }
  //   printf("\n");
  // }
  // printf("Convex\n");
  // for (int y_ = 0; y_ < size; y_++) {
  //   for (int x_ = 0; x_ < size; x_++) {
  //     if (x_ == x && y_ == y) {
  //       printf(" x");
  //     } else if (x_ >= x_min && x_ <= x_max && y_ >= y_min && y_ <= y_max &&
  //                x_ >= y_min_arr[y_] && x_ <= y_max_arr[y_] &&
  //                y_ >= x_min_arr[x_] && y_ <= x_max_arr[x_]) {
  //       printf(" .");
  //     } else {
  //       printf(" *");
  //     }
  //   }
  //   printf("\n");
  // }
  // printf("\n");
  // for (int y_ = 0; y_ < size; y_++) {
  //   for (int x_ = 0; x_ < size; x_++) {
  //     if (x_ == x) {
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
      for (int y_ = y; y_ <= y_max; y_++) {
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
      printf(" %3d", grid[i][j]);
    }
    printf("\n");
  }
}

void print_compare(int grid1[][MAP_MAX], int grid2[][MAP_MAX], int size) {
  for (int j = 0; j < size; j++) {
    for (int i = 0; i < size; i++) {
      if (false && grid1[i][j] != grid2[i][j]) {
        printf("\033[0;31m");
        printf(" %3d", grid1[i][j]);
        printf("\033[0m");
      } else {
        printf(" %3d", grid1[i][j]);
      }
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
    print_compare(expected, output, size);

    printf("\n Got:\n");
    print_compare(output, expected, size);
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
  static int alt5[MAP_MAX][MAP_MAX] = {
    { 687, 760, 795, 503, 870, 947, 836, 358, 125, 501, 712, 792, 501, 225, 988, 776, 809, 767, 859, 409, 61, 20, 239, 973, 684 },
    { 888, 297, 693, 954, 630, 659, 641, 742, 454, 496, 612, 753, 333, 322, 878, 186, 387, 22, 687, 612, 363, 815, 421, 130, 675 },
    { 183, 544, 695, 422, 517, 379, 663, 166, 424, 617, 148, 84, 611, 890, 890, 107, 854, 644, 440, 177, 874, 626, 564, 249, 665 },
    { 528, 612, 481, 949, 94, 508, 132, 990, 203, 907, 507, 934, 570, 25, 358, 539, 173, 794, 150, 415, 685, 610, 270, 681, 50 },
    { 799, 907, 29, 715, 156, 46, 243, 120, 879, 544, 215, 387, 29, 205, 942, 936, 65, 876, 858, 90, 587, 397, 264, 733, 548 },
    { 31, 418, 158, 301, 451, 560, 452, 359, 941, 167, 515, 988, 410, 636, 219, 955, 203, 607, 984, 408, 901, 272, 825, 130, 130 },
    { 916, 717, 527, 532, 802, 427, 563, 221, 937, 217, 672, 850, 669, 383, 791, 189, 899, 131, 951, 887, 703, 906, 90, 310, 242 },
    { 850, 211, 514, 676, 341, 996, 944, 410, 524, 828, 565, 303, 391, 138, 241, 960, 162, 91, 982, 546, 234, 523, 445, 718, 474 },
    { 684, 421, 381, 774, 83, 975, 624, 646, 842, 652, 340, 838, 596, 102, 362, 424, 667, 666, 168, 805, 259, 480, 968, 702, 814 },
    { 514, 288, 337, 311, 358, 812, 995, 779, 545, 121, 214, 520, 97, 861, 362, 102, 553, 201, 698, 655, 915, 475, 323, 933, 995 },
    { 480, 192, 827, 800, 246, 642, 666, 535, 979, 329, 893, 791, 324, 25, 336, 797, 239, 209, 247, 452, 571, 349, 5, 124, 47 },
    { 661, 40, 874, 336, 325, 221, 168, 518, 49, 969, 116, 43, 635, 651, 22, 965, 897, 166, 641, 922, 854, 439, 513, 63, 38 },
    { 966, 987, 387, 971, 111, 786, 984, 503, 661, 672, 829, 234, 841, 699, 283, 810, 815, 678, 445, 467, 701, 762, 716, 219, 404 },
    { 990, 73, 195, 503, 137, 233, 469, 124, 620, 793, 587, 406, 129, 91, 419, 802, 272, 654, 643, 971, 289, 805, 786, 968, 250 },
    { 605, 21, 365, 673, 240, 769, 663, 313, 316, 167, 802, 901, 988, 278, 521, 133, 218, 279, 263, 661, 51, 417, 933, 57, 412 },
    { 256, 698, 217, 42, 666, 467, 0, 39, 184, 673, 631, 953, 337, 945, 621, 856, 99, 522, 844, 730, 395, 330, 948, 27, 593 },
    { 961, 78, 362, 894, 487, 774, 150, 185, 991, 544, 204, 810, 896, 243, 995, 922, 875, 300, 611, 172, 922, 467, 623, 444, 663 },
    { 353, 192, 993, 653, 219, 938, 614, 649, 300, 860, 136, 74, 362, 321, 417, 907, 525, 228, 155, 121, 223, 77, 996, 523, 688 },
    { 520, 797, 507, 143, 594, 523, 849, 786, 516, 854, 357, 807, 469, 6, 107, 681, 142, 534, 44, 815, 951, 951, 693, 179, 458 },
    { 814, 754, 536, 162, 630, 576, 34, 427, 436, 529, 21, 959, 378, 807, 827, 233, 164, 634, 54, 522, 94, 735, 16, 980, 779 },
    { 832, 931, 82, 525, 463, 893, 691, 569, 781, 205, 199, 709, 239, 627, 145, 768, 0, 456, 147, 160, 636, 732, 324, 622, 138 },
    { 199, 716, 225, 215, 48, 357, 399, 980, 791, 924, 795, 684, 967, 364, 465, 172, 916, 175, 411, 895, 672, 532, 895, 481, 31 },
    { 55, 469, 115, 732, 91, 253, 931, 160, 478, 146, 208, 835, 546, 540, 979, 822, 687, 663, 142, 404, 129, 314, 320, 656, 78 },
    { 215, 680, 962, 110, 513, 993, 518, 982, 108, 250, 74, 713, 181, 234, 191, 679, 794, 379, 577, 335, 358, 752, 22, 373, 894 },
    { 426, 854, 560, 98, 510, 990, 313, 543, 952, 776, 56, 297, 294, 39, 405, 896, 465, 118, 77, 699, 662, 108, 845, 393, 38 }
  };
  static int area5[MAP_MAX][MAP_MAX] = {
    { 1, 4, 10, 1, 25, 30, 21, 2, 1, 4, 10, 18, 4, 1, 176, 4, 6, 3, 20, 4, 2, 1, 3, 85, 8 },
    { 12, 1, 2, 60, 5, 10, 1, 14, 4, 2, 5, 18, 2, 2, 21, 2, 3, 1, 6, 6, 2, 16, 2, 1, 7 },
    { 1, 2, 13, 1, 6, 1, 15, 1, 2, 6, 2, 1, 6, 28, 33, 1, 15, 3, 4, 1, 32, 6, 5, 2, 6 },
    { 2, 4, 3, 50, 1, 6, 1, 272, 1, 22, 3, 51, 5, 1, 2, 4, 2, 13, 1, 4, 8, 4, 2, 9, 1 },
    { 4, 32, 1, 12, 2, 1, 4, 1, 12, 5, 1, 4, 1, 2, 68, 40, 1, 36, 14, 1, 4, 2, 1, 14, 5 },
    { 1, 4, 2, 2, 5, 9, 4, 3, 33, 1, 4, 208, 2, 6, 1, 36, 3, 3, 175, 2, 30, 1, 21, 1, 1 },
    { 40, 10, 7, 4, 24, 1, 9, 1, 32, 2, 9, 18, 12, 3, 12, 1, 42, 2, 25, 11, 3, 42, 1, 4, 2 },
    { 20, 1, 6, 6, 2, 550, 27, 4, 2, 12, 5, 1, 4, 2, 2, 60, 2, 1, 136, 4, 1, 5, 2, 6, 3 },
    { 8, 5, 2, 15, 1, 45, 1, 5, 18, 4, 2, 13, 6, 1, 4, 4, 8, 7, 1, 18, 2, 4, 70, 1, 16 },
    { 5, 3, 3, 1, 8, 16, 288, 14, 5, 1, 2, 4, 1, 32, 4, 1, 4, 1, 14, 2, 39, 4, 2, 32, 288 },
    { 2, 2, 24, 10, 1, 4, 8, 7, 102, 2, 40, 8, 2, 1, 3, 15, 2, 3, 3, 4, 9, 4, 1, 4, 2 },
    { 6, 1, 28, 4, 5, 2, 1, 6, 1, 75, 2, 1, 5, 6, 1, 78, 39, 1, 4, 39, 15, 2, 8, 2, 1 },
    { 13, 96, 5, 51, 1, 14, 92, 5, 10, 3, 16, 2, 24, 10, 2, 9, 18, 20, 1, 2, 4, 12, 9, 3, 6 },
    { 150, 2, 2, 7, 2, 2, 4, 1, 8, 27, 5, 3, 2, 1, 7, 18, 3, 9, 5, 119, 2, 11, 5, 35, 1 },
    { 3, 1, 4, 9, 3, 24, 10, 4, 4, 1, 30, 22, 180, 2, 8, 1, 2, 4, 1, 9, 1, 4, 18, 2, 5 },
    { 1, 12, 2, 1, 18, 4, 1, 2, 3, 9, 6, 33, 1, 30, 9, 20, 1, 4, 34, 6, 3, 1, 30, 1, 6 },
    { 24, 1, 2, 28, 2, 16, 2, 4, 175, 2, 2, 6, 15, 1, 375, 55, 28, 2, 9, 2, 36, 4, 3, 3, 7 },
    { 2, 2, 182, 10, 1, 40, 3, 8, 1, 20, 2, 1, 4, 3, 5, 32, 6, 5, 4, 1, 4, 1, 475, 5, 10 },
    { 2, 10, 2, 1, 4, 1, 18, 17, 3, 22, 5, 10, 6, 1, 2, 8, 1, 6, 1, 16, 66, 25, 3, 1, 2 },
    { 12, 10, 8, 2, 12, 6, 1, 2, 3, 5, 1, 76, 2, 9, 24, 2, 3, 12, 2, 4, 1, 14, 1, 119, 13 },
    { 8, 33, 1, 6, 3, 27, 4, 2, 19, 2, 2, 6, 1, 5, 1, 16, 1, 4, 3, 2, 10, 9, 2, 4, 3 },
    { 2, 7, 3, 2, 1, 4, 5, 115, 7, 26, 10, 1, 70, 1, 3, 1, 51, 1, 6, 33, 6, 2, 25, 2, 1 },
    { 1, 3, 1, 16, 2, 2, 25, 1, 6, 1, 3, 15, 6, 4, 112, 18, 9, 18, 1, 5, 1, 2, 3, 7, 2 },
    { 3, 4, 35, 2, 5, 175, 2, 133, 1, 4, 2, 10, 1, 3, 1, 4, 18, 2, 8, 1, 2, 15, 1, 2, 27 },
    { 4, 8, 3, 1, 2, 25, 1, 2, 34, 12, 1, 4, 4, 1, 6, 32, 3, 2, 1, 12, 3, 1, 24, 2, 1 }
  };
  castleArea ( alt5, 25, result );
  compare_area ( alt5,  result, area5, 25 );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
