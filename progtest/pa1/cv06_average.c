#ifndef __PROGTEST__
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#endif /* __PROGTEST__ */

long long avg3(long long a, long long b, long long c) {
  long long rem1 = a % 3;
  long long rem2 = b % 3;
  long long rem3 = c % 3;

  long long remainder = rem1 + rem2 + rem3;
  long long average = a / 3 + b / 3 + c / 3 + remainder / 3;

  long long remrem = remainder % 3;
  if (remrem < 0 && average > 0) {
    average--;
  } else if (remrem > 0 && average < 0) {
    average++;
  }

  return average;
}

#ifndef __PROGTEST__
#define ASSERT_EQ(a, b)                                                        \
  {                                                                            \
    long long _a = a;                                                          \
    long long _b = b;                                                          \
    if (_a != _b) {                                                            \
      printf("%s:%d Assertion failed:\n  %s != %s\n  %lld != %lld\n",          \
             __FILE__, __LINE__, #a, #b, _a, _b);                              \
      return 1;                                                                \
    }                                                                          \
  }
int main(int argc, char *argv[]) {
  ASSERT_EQ(avg3(1, 2, 3), 2);
  ASSERT_EQ(avg3(-100, 100, 30), 10);
  ASSERT_EQ(avg3(1, 2, 2), 1);
  ASSERT_EQ(avg3(-1, -2, -2), -1);
  ASSERT_EQ(avg3(9223372036854775800, 9223372036854775800, -8),
            6148914691236517197);
  ASSERT_EQ(avg3(9223372036854775800, -9223372036854775807, 2), -1);
  ASSERT_EQ(avg3(LLONG_MAX, LLONG_MAX, LLONG_MAX), LLONG_MAX);
  return 0;
}
#endif /* __PROGTEST__ */
