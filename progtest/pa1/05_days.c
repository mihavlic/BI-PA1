#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
typedef struct
{
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

// 🦋🦋🦋🦋
int days_in_moth(int y, int m) {
  int moth_density[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (m == 2 && is_leap(y)) {
    return 29;
  } else {
    return moth_density[m - 1];
  }
}

// https://en.wikipedia.org/wiki/Zeller%27s_congruence#Common_simplification
int zellers_congruence(int y, int m, int d) {
  if (m < 3) {
    m += 12;
    y -= 1;
  }
  return d + 13 * (m + 1)/5 + y + y/4 - y/100 + y/400 - y/4000;
}

bool is_holiday(int m, int d) {
  switch (m) {
    case(1): return d == 1 ;
    case(5): return d == 1 || d == 8;
    case(7): return d == 5 || d == 6;
    case(9): return d == 28;
    case(10): return d == 28;
    case(11): return d == 17;
    case(12): return d == 24 || d == 25 || d == 26;
    default: return false;
  }
}

bool date_correct(int y, int m, int d) {
  return
    0 < m && m <= 12 &&
    2000 <= y &&
    0 < d && d <= days_in_moth(y, m);
}

bool isWorkDay(int y, int m, int d) {
  if (!date_correct(y, m, d)) {
    return false;
  }
  int weekday = zellers_congruence(y, m, d) % 7;
  return weekday >= 2 && !is_holiday(m, d);
}

void count_days(int y, int m1, int d1, int m2, int d2, int *weekdays, int *total_days, int *work_days) {
  for (int m = m1; m <= m2; m++) {
    int start_day = m == m1 ? d1 : 1;
    int end_day = m == m2 ? d2 : days_in_moth(y, m);
    for (int d = start_day; d <= end_day; d++) {
      if (*weekdays % 7 >= 2 && !is_holiday(m, d)) {
        *work_days += 1;
      }
      *total_days += 1;
      *weekdays += 1;
    }
  }
}

void count_days_year_cached(int y, int *weekdays, int *total_days, int *work_days, TResult *cache) {
  bool leap = is_leap(y);
  int weekday = *weekdays % 7;

  int key = (int)leap + 2*weekday;
  printf("%d\n", key);
  TResult *entry = &cache[key];

  if (entry->m_TotalDays == 0) {
    int total_days = 0;
    int work_days = 0;
    int tmp = *weekdays;
    count_days(y, 1, 1, 12, 31, &tmp, &total_days, &work_days);
    entry->m_TotalDays = total_days;
    entry->m_WorkDays = work_days;
  }
  
  *weekdays += entry->m_TotalDays;
  *total_days += entry->m_TotalDays;
  *work_days += entry->m_WorkDays;
}

TResult global_year_cache[14] = {0};

TResult countDays(int y1, int m1, int d1, int y2, int m2, int d2) {
  if (!date_correct(y1, m1, d1) || !date_correct(y2, m2, d2)) {
    return TResult {-1, -1};
  }
  
  int weekdays = zellers_congruence(y1, m1, d1);
  int total_days = 0;
  int work_days = 0;

  int cached = 0;
  int non_cached = 0;

  for (int y = y1; y <= y2; y++) {
    int start_month = (y == y1) ? m1 : 1;
    int end_month = (y == y2) ? m2 : 12;    
    int start_day = (y == y1) ? d1 : 1;
    int end_day = (y == y2) ? d2 : 31;
    if (start_month == 1 && end_month == 12 && start_day == 1 && end_day == 31) {
      count_days_year_cached(y, &weekdays, &total_days, &work_days, global_year_cache);
      cached++;
    } else {
      count_days(y, start_month, start_day, end_month, end_day, &weekdays, &total_days, &work_days);
      non_cached++;
    }
  }

  printf("Cached %g\n", ((double) cached) / ((double) (cached + non_cached)));

  if (total_days == 0) {
    return TResult {-1, -1};
  }

  return TResult {
    total_days,
    work_days,
  };
}

#ifndef __PROGTEST__
#define ASSERT_EQ(a, b) \
  _a = a; \
  _b = b; \
  if (_a != _b) { \
    printf("%s:%d Assertion failed:\n  %s != %s\n  %d != %d\n", __FILE__, __LINE__, #a, #b, _a, _b); \
    return 1; \
  }

int main (int argc, char **argv) {  
  TResult r;
  int _a, _b;

  ASSERT_EQ(true, isWorkDay  ( 2023, 10, 10 ) );
  ASSERT_EQ(true, isWorkDay  ( 2008,  2, 29 ) );
  ASSERT_EQ(false, isWorkDay ( 2023, 11, 31 ) );
  ASSERT_EQ(false, isWorkDay ( 2023, 11, 17 ) );
  ASSERT_EQ(false, isWorkDay ( 2023, 11, 11 ) );
  ASSERT_EQ(false, isWorkDay ( 2023,  2, 29 ) );
  ASSERT_EQ(false, isWorkDay ( 2004,  2, 29 ) );
  ASSERT_EQ(false, isWorkDay ( 2001,  2, 29 ) );
  ASSERT_EQ(false, isWorkDay ( 1996,  1,  2 ) );

  r = countDays ( 2023, 11,  1,
                  2023, 11, 30 );
  ASSERT_EQ( r.m_TotalDays, 30 );
  ASSERT_EQ( r.m_WorkDays, 21 );

  r = countDays ( 2023, 11,  1,
                  2023, 11, 17 );
  ASSERT_EQ( r.m_TotalDays, 17 );
  ASSERT_EQ( r.m_WorkDays, 12 );

  r = countDays ( 2023, 11,  1,
                  2023, 11,  1 );
  ASSERT_EQ( r.m_TotalDays, 1 );
  ASSERT_EQ( r.m_WorkDays, 1 );

  r = countDays ( 2023, 11, 17,
                  2023, 11, 17 );
  ASSERT_EQ( r.m_TotalDays, 1 );
  ASSERT_EQ( r.m_WorkDays, 0 );

  r = countDays ( 2023,  1,  1,
                  2023, 12, 31 );
  ASSERT_EQ( r.m_TotalDays, 365 );
  ASSERT_EQ( r.m_WorkDays, 252 );

  r = countDays ( 2024,  1,  1,
                  2024, 12, 31 );
  ASSERT_EQ( r.m_TotalDays, 366 );
  ASSERT_EQ( r.m_WorkDays, 254 );

  r = countDays ( 2000,  1,  1,
                  2023, 12, 31 );
  ASSERT_EQ( r.m_TotalDays, 8766 );
  ASSERT_EQ( r.m_WorkDays, 6072 );

  r = countDays ( 2001,  2,  3,
                  2023,  7, 18 );
  ASSERT_EQ( r.m_TotalDays, 8201 );
  ASSERT_EQ( r.m_WorkDays, 5682 );

  r = countDays ( 2021,  3, 31,
                  2023, 11, 12 );
  ASSERT_EQ( r.m_TotalDays, 957 );
  ASSERT_EQ( r.m_WorkDays, 666 );

  r = countDays ( 2001,  1,  1,
                  2000,  1,  1 );
  ASSERT_EQ( r.m_TotalDays, -1 );
  ASSERT_EQ( r.m_WorkDays, -1 );

  r = countDays ( 2001,  1,  1,
                  2023,  2, 29 );
  ASSERT_EQ( r.m_TotalDays, -1 );
  ASSERT_EQ( r.m_WorkDays, -1 );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */