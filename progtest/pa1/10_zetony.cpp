#include <cassert>
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

// typedef struct {
//   void *allocation;
//   int size;
//   int capacity;
// } ArrayList;

// void list_reserve(ArrayList *list, int new_size) {
//   if (new_size > list->capacity) {
//     int new_capacity = list->capacity * 2;
//     if (new_capacity < new_size) {
//       new_capacity = new_size;
//     }
//     list->capacity = new_capacity;
//     list->allocation = realloc(list->allocation, new_capacity);
//   }
// }

// void list_push(ArrayList *list, void *element, int size) {
//   int new_size = list->size + size;
//   list_reserve(list, new_size);
//   memcpy((char *)list->allocation + list->size, element, size);
//   list->size = new_size;
// }

// void list_free(ArrayList *list) { free(list->allocation); }

typedef struct {
  char start;
  char end;
  bool p1_take_two;
  bool p2_take_two;
} MoveKey;

typedef struct {
  int p1_score;
  int p2_score;
  short prev_move;
  bool valid;
} MoveValue;

MoveValue new_value(int p1_score, int p2_score, short prev_move) {
  return MoveValue{p1_score, p2_score, prev_move, true};
}

short encode_move(MoveKey move) {
  assert(0 < move.start && move.start < move.end);
  assert(move.end < 100);

  short encoded = 0;
  encoded += move.start;
  encoded *= 100;
  encoded += move.end;
  encoded *= 2;
  encoded += (short)move.p1_take_two;
  encoded *= 2;
  encoded += (short)move.p2_take_two;

  return encoded;
}

MoveKey decode_move(short encoded) {
  MoveKey key = {};

  key.p2_take_two = encoded % 2;
  encoded /= 2;
  key.p1_take_two = encoded % 2;
  encoded /= 2;
  key.end = encoded % 100;
  encoded /= 100;
  key.start = encoded;

  return key;
}

typedef struct {
  MoveValue *values;
  int size;
} Memo;

MoveValue *memo_get(Memo *memo, char start, char end, bool p1_take_two,
                    bool p2_take_two) {
  if (0 > start || start > end || end >= memo->size) {
    return NULL;
  }

  int encoded = encode_move(MoveKey{start, end, p1_take_two, p2_take_two});
  MoveValue *move = &memo->values[encoded];

  if (!move->valid) {
    return NULL;
  }

  return move;
}

// dynamic programming
// see https://en.wikipedia.org/wiki/Change-making_problem
int find_count(int *coins, int coins_len, MoveValue *memo) {
  for (char i = 0; i < coins_len; i++) {
    MoveKey move = MoveKey{
        .start = i,
        .end = i,
        .p1_take_two = false,
        .p2_take_two = false,
    };

    *memo_get(memo, move) = MoveValue{
        .p1_score = coins[i],
        .p2_score = 0,
        .prev_move = -1,
        .valid = true,
    };
  }

  for (int size = 2; size < coins_len; size++) {
    int maximum = INT_MIN;
    for (int i = 0; i + size <= coins_len; i++) {
      int end = i + size;

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

// 3 5 3 6 1

// 5 5 6 6
// 5 5 6 6