#include <cstdint>
#include <cstdlib>
#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
typedef struct {
  int m_TotalDays;
  int m_WorkDays;
} TResult;
#endif /* __PROGTEST__ */

bool is_leap(int year) {
  if (year % 4 == 0) {
    if (year % 100 == 0) {
      if (year % 400 == 0) {
        if (year % 4000 == 0) {
          return false;
        }
        return true;
      }
      return false;
    }
    return true;
  } else {
    return false;
  }
}

const int moth_density[13] = {0,  31, 28, 31, 30, 31, 30,
                              31, 31, 30, 31, 30, 31};
const int days_by_month[13] = {0,   31,  59,  90,  120, 151, 181,
                               212, 243, 273, 304, 334, 365};
const int start_2000_1_1 = 730120; // == days_since(2000, 1, 1)

int days_since(int y, int m, int d) {
  int days = d;
  days += days_by_month[m - 1];
  if (m > 2 && is_leap(y)) {
    days++;
  }
  int prev = y - 1;
  days += prev * 365;
  days += prev / 4;
  days -= prev / 100;
  days += prev / 400;
  days -= prev / 4000;
  return days;
}

int make_timestamp(int y, int m, int d) {
  return days_since(y, m, d) - start_2000_1_1;
}

int days_in_moth(int y, int m) {
  if (m == 2 && is_leap(y)) {
    return 29;
  } else {
    return moth_density[m];
  }
}

bool date_correct(int y, int m, int d) {
  return 0 < m && m <= 12 && 2000 <= y && 0 < d && d <= days_in_moth(y, m);
}

typedef struct {
  int month;
  int day;
} DayMonth;

DayMonth holidays[] = {{1, 1},   {5, 1},   {5, 8},   {7, 5},   {7, 6},  {9, 28},
                       {10, 28}, {11, 17}, {12, 24}, {12, 25}, {12, 26}};

bool is_holiday(int m, int d) {
  for (int i = 0; i < 11; i++) {
    DayMonth holiday = holidays[i];
    if (holiday.month == m && holiday.day == d) {
      return true;
    }
  }
  return false;
}

bool isWorkDay(int y, int m, int d) {
  if (!date_correct(y, m, d)) {
    return false;
  }
  int weekday = make_timestamp(y, m, d) % 7;
  return weekday >= 2 && !is_holiday(m, d);
}

// TODO for whole years this functions returns only 14 distinct values since
// - is the year leap? 0-1
// - which weekday does the year start on? 0-6
int holidays_which_are_workdays(int y, int max_timestamp) {
  int count = 0;
  for (int i = 0; i < 11; i++) {
    DayMonth holiday = holidays[i];
    int timestamp = make_timestamp(y, holiday.month, holiday.day);
    if (timestamp >= max_timestamp) {
      break;
    }
    if (timestamp % 7 >= 2) {
      count++;
    }
  }
  return count;
}

// we can estimate a conservative table size, because we're returning number of
// days in a i32 so the maximum year is about `2^31 / 365 ~= 6 000 000` years
// (I've experimentally found that the maximum year is 5879490)
constexpr int TABLE_SIZE = 5879490 - 2000;
void fill_table(int table[TABLE_SIZE]) {
  // the count of days which have been both a holiday and a workday
  int holiday_bias = 0;
  for (int entry = 0; entry < TABLE_SIZE; entry++) {
    int year = entry + 2000;
    holiday_bias += holidays_which_are_workdays(year, INT32_MAX);
    table[entry] = holiday_bias;
  }
}

int work_days_since(int y, int timestamp, int table[TABLE_SIZE]) {
  // Sa Su Mo Tu We Th Fr
  // 0  1  2  3  4  5  6
  int partial = timestamp % 7 - 2;
  int whole = (timestamp / 7) * 5;
  // the number of work days since the epoch, this doesn't handle holidays
  int workdays = whole + (partial > 0 ? partial : 0);

  if (y > 2000) {
    workdays -= table[y - 2001];
  }
  workdays -= holidays_which_are_workdays(y, timestamp);
  return workdays;
}

int *table = NULL;

TResult countDays(int y1, int m1, int d1, int y2, int m2, int d2) {
  if (!date_correct(y1, m1, d1) || !date_correct(y2, m2, d2)) {
    return TResult{-1, -1};
  }

  if (table == NULL) {
    table = (int *)malloc(TABLE_SIZE * sizeof(int));
    fill_table(table);
  }

  int start = make_timestamp(y1, m1, d1);
  int end = make_timestamp(y2, m2, d2) + 1;
  if (m2 == 12 && d2 == 31) {
    y2 += 1;
  }

  int total_days = end - start;
  int work_days =
      work_days_since(y2, end, table) - work_days_since(y1, start, table);

  if (total_days <= 0) {
    return TResult{-1, -1};
  }

  return TResult{
      total_days,
      work_days,
  };
}

#ifndef __PROGTEST__
#define ASSERT_EQ(a, b)                                                        \
  {                                                                            \
    int _a = a;                                                                \
    int _b = b;                                                                \
    if (_a != _b) {                                                            \
      printf("%s:%d Assertion failed:\n  %s != %s\n  %d != %d\n", __FILE__,    \
             __LINE__, #a, #b, _a, _b);                                        \
      return 1;                                                                \
    }                                                                          \
  }

int main(int argc, char **argv) {
  TResult r;

  ASSERT_EQ(true, isWorkDay(2023, 10, 10));
  ASSERT_EQ(true, isWorkDay(2008, 2, 29));
  ASSERT_EQ(false, isWorkDay(2023, 11, 31));
  ASSERT_EQ(false, isWorkDay(2023, 11, 17));
  ASSERT_EQ(false, isWorkDay(2023, 11, 11));
  ASSERT_EQ(false, isWorkDay(2023, 2, 29));
  ASSERT_EQ(false, isWorkDay(2004, 2, 29));
  ASSERT_EQ(false, isWorkDay(2001, 2, 29));
  ASSERT_EQ(false, isWorkDay(1996, 1, 2));

  r = countDays(2023, 11, 1, 2023, 11, 30);
  ASSERT_EQ(r.m_TotalDays, 30);
  ASSERT_EQ(r.m_WorkDays, 21);

  r = countDays(2023, 11, 1, 2023, 11, 17);
  ASSERT_EQ(r.m_TotalDays, 17);
  ASSERT_EQ(r.m_WorkDays, 12);

  r = countDays(2023, 11, 1, 2023, 11, 1);
  ASSERT_EQ(r.m_TotalDays, 1);
  ASSERT_EQ(r.m_WorkDays, 1);

  r = countDays(2023, 11, 17, 2023, 11, 17);
  ASSERT_EQ(r.m_TotalDays, 1);
  ASSERT_EQ(r.m_WorkDays, 0);

  r = countDays(2023, 1, 1, 2023, 12, 31);
  ASSERT_EQ(r.m_TotalDays, 365);
  ASSERT_EQ(r.m_WorkDays, 252);

  r = countDays(2024, 1, 1, 2024, 12, 31);
  ASSERT_EQ(r.m_TotalDays, 366);
  ASSERT_EQ(r.m_WorkDays, 254);

  r = countDays(2000, 1, 1, 2023, 12, 31);
  ASSERT_EQ(r.m_TotalDays, 8766);
  ASSERT_EQ(r.m_WorkDays, 6072);

  r = countDays(2001, 2, 3, 2023, 7, 18);
  ASSERT_EQ(r.m_TotalDays, 8201);
  ASSERT_EQ(r.m_WorkDays, 5682);

  r = countDays(2021, 3, 31, 2023, 11, 12);
  ASSERT_EQ(r.m_TotalDays, 957);
  ASSERT_EQ(r.m_WorkDays, 666);

  r = countDays(2001, 1, 1, 2000, 1, 1);
  ASSERT_EQ(r.m_TotalDays, -1);
  ASSERT_EQ(r.m_WorkDays, -1);

  r = countDays(2001, 1, 1, 2023, 2, 29);
  ASSERT_EQ(r.m_TotalDays, -1);
  ASSERT_EQ(r.m_WorkDays, -1);

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */