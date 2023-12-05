#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef __PROGTEST__
#define DEBUG(fmt) fprintf(stderr, "%s:%d " fmt, __FILE__, __LINE__)
#define DEBUGF(fmt, ...)                                                       \
  fprintf(stderr, "%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(fmt)
#define DEBUGF(fmt, ...)
#endif

typedef struct {
  void *allocation;
  int size;
  int capacity;
} ArrayList;

void list_reserve(ArrayList *list, int new_size) {
  if (new_size > list->capacity) {
    int new_capacity = list->capacity * 2;
    if (new_capacity < new_size) {
      new_capacity = new_size;
    }
    list->capacity = new_capacity;
    list->allocation = realloc(list->allocation, new_capacity);
  }
}

void list_push(ArrayList *list, void *element, int size) {
  int new_size = list->size + size;
  list_reserve(list, new_size);
  memcpy((char *)list->allocation + list->size, element, size);
  list->size = new_size;
}

void list_free(ArrayList *list) { free(list->allocation); }

// dynamic programming
// see https://en.wikipedia.org/wiki/Change-making_problem
int find_count(ArrayList *coins_list, ArrayList *scratch, int sum) {
  list_reserve(scratch, (sum + 1) * sizeof(int));

  int *table = (int *)scratch->allocation;
  int *coins = (int *)coins_list->allocation;
  int coin_count = coins_list->size / sizeof(int);

  table[0] = 0;

  for (int current_sum = 1; current_sum <= sum; current_sum++) {
    int minimum = INT_MAX;
    table[current_sum] = INT_MAX;
    for (int c = 0; c < coin_count; c++) {
      int current_coin = coins[c];
      // the sum can be formed only with smaller-or-equal values
      if (current_coin > current_sum) {
        continue;
      }

      // the number of coins used to form the sum excluding the current coin
      int prev_count = table[current_sum - current_coin];
      if (prev_count == INT_MAX) {
        continue;
      }

      int new_count = prev_count + 1;
      // keep track of global minimum for this sum
      if (new_count < minimum) {
        minimum = new_count;
        table[current_sum] = minimum;
      }
    }
  }

  return table[sum];
}

void bad() { printf("Nespravny vstup.\n"); }

void run(ArrayList *coins, ArrayList *scratch) {
  printf("Mince:\n");
  while (true) {
    int num = 0;
    int count = scanf("%d", &num);
    if (count != 1 || num < 0) {
      return bad();
    }

    if (num == 0) {
      break;
    }
    list_push(coins, &num, sizeof(int));
  }

  if (coins->size == 0) {
    return bad();
  }

  printf("Castky:\n");
  while (true) {
    int sum = 0;
    int count = scanf("%d", &sum);
    if (count != 1 && feof(stdin)) {
      return;
    }
    if (count != 1 || sum < 0) {
      return bad();
    }
    int min = find_count(coins, scratch, sum);
    if (min == INT_MAX) {
      printf("= nema reseni\n");
    } else {
      printf("= %d\n", min);
    }
  }
}

int main() {
  ArrayList coins = {};
  ArrayList scratch = {};
  run(&coins, &scratch);
  list_free(&coins);
  list_free(&scratch);
  return 0;
}