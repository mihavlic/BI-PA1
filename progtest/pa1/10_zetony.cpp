#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
  char take1;
  char take2;
} MoveValue;

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

void try_move(Ctx *memo, char prev_start, char prev_end, char take1, char take2,
              bool prev_take_two, MoveValue *out_value) {
  int curr_score = memo->coins[(int)take1];
  bool current_take_two = false;
  if (take2 > 0) {
    curr_score += memo->coins[(int)take2];
    current_take_two = true;
  }

  short encoded = -1;
  int prev_score = 0;
  if (prev_start != prev_end) {
    MoveKey move = MoveKey{
        prev_start,
        prev_end,
        prev_take_two,
        current_take_two,
    };

    assert(move_is_valid(memo, move));
    encoded = encode_move(move);
    const MoveValue *value = &memo->table[encoded];

    prev_score = value->p1_score;
    curr_score += value->p2_score;
  }

  if (curr_score > out_value->p1_score) {
    *out_value = MoveValue{curr_score, prev_score, encoded, take1, take2};
  }
}

void find_count(Ctx *memo) {
  int *coins = memo->coins;
  char coins_len = memo->coins_len;

  for (char size = 1; size <= coins_len; size++) {
    for (char i = 0; i + size <= coins_len; i++) {
      for (int m = 0; m < 4; m++) {
        bool p1_take_two = m % 2;
        bool p2_take_two = m / 2;
        char j = i + size;

        MoveValue value = {INT_MIN, 0, -1, -1, -1};

        if (size >= 1) {
          try_move(memo, i + 1, j, i, -1, p2_take_two, &value);
          try_move(memo, i, j - 1, j - 1, -1, p2_take_two, &value);
        }

        if (!p1_take_two) {
          if (size >= 2) {
            try_move(memo, i, j - 2, j - 1, j - 2, p2_take_two, &value);
          }
          if (size >= 3) {
            try_move(memo, i + 2, j, i, i + 1, p2_take_two, &value);
            try_move(memo, i + 1, j - 1, i, j - 1, p2_take_two, &value);
          }
        }

        MoveKey key = {i, j, p1_take_two, p2_take_two};

        assert(value.take1 != -1);
        *memo_get(memo, key) = value;
      }
    }
  }

  short next_move = encode_move(MoveKey{0, coins_len, false, false});

  bool p1 = true;
  int p1_score = 0;
  int p2_score = 0;
  while (next_move > 0) {
    MoveValue *value = &memo->table[next_move];

    char p = p1 ? 'A' : 'B';
    printf("%c ", p);

    printf("[%d]", value->take1);
    if (value->take2 > 0) {
      printf(", [%d]", value->take2);
    }
    printf(": ");

    int sum = coins[(int)value->take1];
    printf("%d", coins[(int)value->take1]);
    if (value->take2 > 0) {
      sum += coins[(int)value->take2];
      printf(" + %d", coins[(int)value->take2]);
    }
    printf("\n");

    if (p1) {
      p1_score += sum;
    } else {
      p2_score += sum;
    }
    p1 = !p1;
    next_move = value->prev_move;
  }

  printf("A: %d, B: %d\n", p1_score, p2_score);
}

int bad() {
  printf("Nespravny vstup.\n");
  return 0;
}

int main() {
  printf("Zetony:\n");
  int coins[100] = {};
  int coins_len = 0;

  while (true) {
    int coin = 0;
    int count = scanf("%d", &coin);
    if (count != 1 && feof(stdin)) {
      break;
    }
    if (count != 1 || coins_len >= 100) {
      return bad();
    }
    coins[coins_len++] = coin;
  }

  if (coins_len == 0) {
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
      coins,
      coins_len,
  };

  find_count(&memo);
  free(table);
  return 0;
}