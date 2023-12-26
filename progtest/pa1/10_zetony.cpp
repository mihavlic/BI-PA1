#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// We use dynamic programming to solve the problem from the bottom up.
// Each possible move is memoized using the two following structs:

// Concretely describes the state of the game,
// p1 is the player currently making a move.
typedef struct {
  // interval of coins remaining when this move is played
  char start, end;
  // p1 took two coins in their previous move
  bool p1_take_two;
  // p2 took two coins in their previous move (the move immediatelly before this
  // one)
  bool p2_take_two;
} MoveKey;

// The "best" move for a MoveKey
typedef struct {
  // the cumulative score of p1 including this move
  int p1_score;
  // the cumulative score of p2 (this is the same as p1_score of prev_move)
  int p2_score;
  // an encoded MoveKey of the previous move done by the oponent
  short prev_move;
  // the index of the coin taken in this move
  char take1;
  // the index of the second coin taken in this move
  //  -1 if it wasn't used
  char take2;
} MoveValue;

// Pack a MoveKey into an integer, occupies the lower bits so can be indexed with.
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

// Unpack a MoveKey from a previously encoded integer.
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

// Context containing the game's coins and the memoization table.
typedef struct {
  // memoization table indexed by encoded MoveKeys
  MoveValue *table;
  int table_len;
  int *coins;
  int coins_len;
} Ctx;

bool move_is_valid(Ctx *ctx, MoveKey move) {
  return 0 <= move.start && move.start < move.end && move.end <= ctx->coins_len;
}

// Get the MoveValue for an encoded MoveKey.
MoveValue *get_move_encoded(Ctx *ctx, short encoded) {
  assert(encoded < ctx->table_len);
  return &ctx->table[encoded];
}

// Get the MoveValue for a MoveKey. Does not check that the MoveValue has been
// properly initialized.
MoveValue *get_move(Ctx *ctx, MoveKey move) {
  assert(move_is_valid(ctx, move));
  int encoded = encode_move(move);
  return get_move_encoded(ctx, encoded);
}

// Get a coin value from its index.
int get_coin(Ctx *ctx, char index) {
  assert(index < ctx->coins_len);
  return ctx->coins[(int)index];
}

// Try a move, overwrite out_move if it results in a better score.
//
// A move taking the first two coins (2 5)
//  2 5 6 8 7 8
//  | | |      |
//  | | |      prev_end = 6
//  | | prev_start = 2
//  | take2 = 1
//  take1 = 0
void try_move(Ctx *ctx,
              // interval of the coins remaining after this move
              char prev_start, char prev_end,
              // index of the first coin taken in this move
              char take1,
              // index of the second taken in this move
              char take2,
              // the oponent's move before this one took two
              bool prev_take_two,
              // output move value
              MoveValue *out_move) {
  // take1 is always valid
  int curr_score = get_coin(ctx, take1);
  bool current_take_two = false;

  // update the sum if take2 is also valid
  if (take2 >= 0) {
    curr_score += get_coin(ctx, take2);
    current_take_two = true;
  }

  short prev_move = -1;
  int prev_score = 0;
  // lookup the oponent's previous move if the remaining interval is nonempty
  if (prev_start < prev_end) {
    MoveKey move = MoveKey{
        prev_start,
        prev_end,
        prev_take_two,
        // this move is from the oponent's perspective
        current_take_two,
    };

    assert(move_is_valid(ctx, move));
    prev_move = encode_move(move);
    const MoveValue *value = get_move_encoded(ctx, prev_move);

    // add the score achieved by the "current" player in its previous moves
    curr_score += value->p2_score;
    // pass the oponent's score through
    prev_score = value->p1_score;
  }

  // overwrite out_move if this move results in a better score
  if (curr_score > out_move->p1_score) {
    *out_move = MoveValue{curr_score, prev_score, prev_move, take1, take2};
  }
}

// Populate the memoization table with all valid MoveKeys.
void populate_table(Ctx *ctx) {
  char coins_len = ctx->coins_len;

  // compute the best move for every coin interval and *_take_two combination
  // try_move references previous moves, so we need the intervals ordered by size
  for (char size = 1; size <= coins_len; size++) {
    for (char i = 0; i + size <= coins_len; i++) {
      char j = i + size;
      for (int m = 0; m < 4; m++) {
        bool p1_take_two = m % 2;
        bool p2_take_two = m / 2;

        MoveValue move = {INT_MIN, 0, 0, 0, 0};

        // for sizes <= 2, we try the same move multiple times, this is fine

        // try taking one coin, this is always valid so the move will get initialized
        try_move(ctx, i + 1, j, i, -1, p2_take_two, &move);
        try_move(ctx, i, j - 1, j - 1, -1, p2_take_two, &move);

        // try taking two coins
        if (!p1_take_two && size >= 2) {
          try_move(ctx, i, j - 2, j - 1, j - 2, p2_take_two, &move);
          try_move(ctx, i + 2, j, i, i + 1, p2_take_two, &move);
          try_move(ctx, i + 1, j - 1, i, j - 1, p2_take_two, &move);
        }

        MoveKey key = {i, j, p1_take_two, p2_take_two};
        *get_move(ctx, key) = move;
      }
    }
  }
}

// Print the output required by progtest by following the linked list of moves.
void print_results(Ctx *ctx) {
  MoveValue *start_move =
      get_move(ctx, MoveKey{0, (char)ctx->coins_len, false, false});

  MoveValue *move = start_move;
  bool p1 = true;
  while (true) {
    char p = p1 ? 'A' : 'B';
    printf("%c [%d]", p, move->take1);
    if (move->take2 >= 0) {
      printf(", [%d]", move->take2);
    }

    printf(": %d", get_coin(ctx, move->take1));
    if (move->take2 >= 0) {
      printf(" + %d", get_coin(ctx, move->take2));
    }
    printf("\n");

    if (move->prev_move < 0) {
      break;
    }

    p1 = !p1;
    move = get_move_encoded(ctx, move->prev_move);
  }

  printf("A: %d, B: %d\n", start_move->p1_score, start_move->p2_score);
}

// Get the coin values from stdin.
int load_input(int *coins, int maxlen) {
  int coins_len = 0;
  while (true) {
    int coin = 0;
    int count = scanf("%d", &coin);
    if (count != 1 && feof(stdin)) {
      break;
    }
    if (count != 1 || coins_len >= maxlen) {
      return -1;
    }
    coins[coins_len++] = coin;
  }

  return coins_len;
}

int main() {
  printf("Zetony:\n");

  int coins[100] = {};
  int coins_len = load_input(coins, 100);

  if (coins_len <= 0) {
    printf("Nespravny vstup.\n");
    return 0;
  }

  int max_index = encode_move(MoveKey{
      (char)(coins_len - 1),
      (char)coins_len,
      true,
      true,
  });
  MoveValue *table = (MoveValue *)calloc(max_index + 1, sizeof(MoveValue));
  Ctx ctx = Ctx{
      table,
      max_index + 1,
      coins,
      coins_len,
  };

  populate_table(&ctx);
  print_results(&ctx);

  free(table);
  return 0;
}