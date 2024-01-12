#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __PROGTEST__
#endif /* __PROGTEST__ */

// rotate array with reversals
// https://dotnettutorials.net/lesson/rotate-an-array-by-k-position-using-reversal-algorithm-in-csharp/
// https://betterprogramming.pub/3-ways-to-rotate-an-array-2a45b39f7bec
//
// rotate length l=5 by r=2
//
// [1, 2, 3, 4, 5] / reverse whole array
// [5, 4, 3, 2, 1] / divide into two parts and reverse both
//  <-A-> <--B-->    A are the elements that will wrap around (size=r)
//                   B is the remaining part (size=(l-r))
// [4, 5, 1, 2, 3]
void reverse(int array[], int start, int end) {
  while (start < end) {
    int temp = array[start];
    array[start] = array[end];
    array[end] = temp;
    start++;
    end--;
  }
}

void rotateArray(int array[], int arrayLen, int rotateBy) {
  int rotate = rotateBy % arrayLen;
  if (rotate < 0) {
    rotate += arrayLen;
  }

  if (rotate == 0) {
    return;
  }

  reverse(array, 0, arrayLen - 1);
  reverse(array, 0, rotate - 1);
  reverse(array, rotate, arrayLen - 1);
}

#ifndef __PROGTEST__
void test_rotate(int in[], int out[], int len, int rotateBy) {
  rotateArray(in, len, rotateBy);
  if (memcmp(in, out, len * sizeof(int)) != 0) {
    printf("Rotate test failed\n");
    printf("Expected\n");
    for (int i = 0; i < len; i++) {
      printf(" %d,", out[i]);
    }
    printf("\nGot\n");
    for (int i = 0; i < len; i++) {
      printf(" %d,", in[i]);
    }
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  int in[] = {1, 2, 3, 4};
  int out[] = {4, 1, 2, 3};
  test_rotate(in, out, 4, 1);
}
#endif /* __PROGTEST__ */
