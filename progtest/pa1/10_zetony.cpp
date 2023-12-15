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
  assert(0 <= move.start && move.start < move.end);
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
  MoveValue *table;
  int table_size;

  int *coins;
  int coins_len;
} Ctx;

bool move_is_valid(Ctx *memo, MoveKey move) {
  return 0 <= move.start && move.start < move.end &&
         move.end <= memo->coins_len;
}

MoveValue *memo_get(Ctx *memo, MoveKey move) {
  int encoded = encode_move(move);
  return &memo->table[encoded];
}

void try_move(Ctx *memo, char move_start, char move_end, char prev_start,
              char prev_end, MoveKey *out_key, MoveValue *out_value) {
  int sum = 0;
  for (int i = move_start; i < move_end; i++) {
    sum += memo->coins[i];
  }

  bool current_is_double = (move_end - move_start) == 2;
  if (prev_start == prev_end) {
    if (sum > out_value->p1_score) {
      out_key->p1_take_two = current_is_double;
      out_key->p2_take_two = false;
      *out_value = MoveValue{
          sum,
          0,
          -1,
          true,
      };
    }
    return;
  }

  for (int i = 0; i < 4; i++) {
    bool oponent_is_double = i % 2;
    bool next_current_is_double = i / 2;

    if (current_is_double && next_current_is_double) {
      continue;
    }

    MoveKey move = MoveKey{
        prev_start,
        prev_end,
        oponent_is_double,
        next_current_is_double,
    };

    assert(move_is_valid(memo, move));
    short encoded = encode_move(move);
    const MoveValue *value = &memo->table[encoded];

    if (!value->valid) {
      continue;
    }

    int total_score = sum + value->p2_score;
    if (total_score > value->p1_score) {
      out_key->p1_take_two = current_is_double;
      out_key->p2_take_two = oponent_is_double;
      *out_value = MoveValue{
          total_score,
          value->p1_score,
          encoded,
          true,
      };
    }
  }
}

void find_count(Ctx *memo) {
  int *coins = memo->coins;
  char coins_len = memo->coins_len;

  for (char size = 1; size <= coins_len; size++) {
    for (char i = 0; i + size <= coins_len; i++) {
      char j = i + size;

      MoveKey key = {i, j, false, false};
      MoveValue value = {
          INT_MIN,
          0,
          -1,
          false,
      };

      try_move(memo, i, i + 1, i + 1, j, &key, &value);
      try_move(memo, j - 1, j, i, j - 1, &key, &value);

      if (size >= 2) {
        try_move(memo, i, i + 2, i + 2, j, &key, &value);
        try_move(memo, j - 2, j, i, j - 2, &key, &value);
      }

      assert(value.p1_score != INT_MIN);
      assert(value.p2_score != INT_MIN);
      assert(value.valid);
      MoveValue *get = memo_get(memo, key);
      *get = value;
    }
  }

  // a b a b . .
  // |     |

  int max = INT_MIN;
  short encoded = -1;
  for (int i = 0; i < 4; i++) {
    bool oponent_is_double = i % 2;
    bool next_current_is_double = i / 2;

    MoveKey move = MoveKey{
        0,
        coins_len,
        oponent_is_double,
        next_current_is_double,
    };

    const MoveValue *value = memo_get(memo, move);
    if (value->p1_score > max) {
      max = value->p1_score;
      encoded = encode_move(move);
    }
  }

  bool p1 = true;
  int p1_score = 0;
  int p2_score = 0;
  while (encoded > 0) {
    MoveKey move = decode_move(encoded);
    MoveValue *value = &memo->table[encoded];

    int start = move.start;
    int end = move.end;
    int d = 1;
    if (value->prev_move > 0) {
      MoveKey prev = decode_move(value->prev_move);
      if (move.start == prev.start) {
        start = end;
        end = prev.end;
        d = -1;
      } else {
        end = prev.start;
      }
    }

    char p = p1 ? 'A' : 'B';
    printf("%c ", p);

    bool first = true;
    for (int i = start; i < end; i += d) {
      if (!first) {
        printf(", ");
      }
      printf("[%d]", i);
      first = false;
    }

    printf(": ");

    int sum = 0;
    first = true;
    for (int i = start; i < end; i += d) {
      if (!first) {
        printf(" + ");
      }
      sum += coins[i];
      printf("%d", coins[i]);
      first = false;
    }
    printf("\n");

    if (p1) {
      p1_score += sum;
    } else {
      p2_score += sum;
    }
    p1 = !p1;
    encoded = value->prev_move;
  }

  printf("A: %d, B: %d\n", p1_score, p2_score);
}

void bad() { printf("Nespravny vstup.\n"); }

void run(ArrayList *coins) {
  printf("Zetony:\n");
  while (true) {
    int coin = 0;
    int count = scanf("%d", &coin);
    if (count != 1 && feof(stdin)) {
      break;
    }
    if (count != 1 || coin < 0) {
      return bad();
    }
    list_push(coins, &coin, sizeof(int));
  }

  int coins_len = coins->size / sizeof(int);
  if (coins_len == 0 || coins_len > 100) {
    return bad();
  }

  int max_index = encode_move(MoveKey{
      (char)(coins_len - 1),
      (char)coins_len,
      true,
      true,
  });
  MoveValue *table = (MoveValue *)calloc(max_index + 1, sizeof(MoveValue));
  Ctx memo = Ctx{
      table,
      max_index,
      (int *)coins->allocation,
      coins_len,
  };

  find_count(&memo);
}

int main() {
  ArrayList coins = {};
  run(&coins);
  list_free(&coins);
  return 0;
}