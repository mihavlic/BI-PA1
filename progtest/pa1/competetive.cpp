#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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

void list_push(ArrayList *list, const void *element, int size) {
  int new_size = list->size + size;
  list_reserve(list, new_size);
  memcpy((char *)list->allocation + list->size, element, size);
  list->size = new_size;
}

void list_free(ArrayList *list) { free(list->allocation); }

enum Result {
  RESULT_OK,
  RESULT_ERR,
  RESULT_EMPTY,
};

#ifndef __PROGTEST__
#define DEBUG(fmt) fprintf(stderr, "%s:%d " fmt, __FILE__, __LINE__)
#define DEBUGF(fmt, ...)                                                       \
  fprintf(stderr, "%s:%d " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(fmt)
#define DEBUGF(fmt, ...)
#endif

// function to place breakpoints at to observe ENSURE failures
void debug_breakpoint_hook() { return; }

#define ENSURE(x)                                                              \
  if (!(x)) {                                                                  \
    debug_breakpoint_hook();                                                   \
    DEBUG("Ensure failed: " #x "\n");                                          \
    return RESULT_ERR;                                                         \
  };

#define TRY(x)                                                                 \
  {                                                                            \
    Result _res = (x);                                                         \
    if (_res != RESULT_OK) {                                                   \
      return _res;                                                             \
    };                                                                         \
  }

typedef int GroupIndex;

enum CellKind {
  KIND_EMPTY,  // .
  KIND_HEADER, // 12/X
  KIND_NUMBER  // 1-9
};

struct CellHeader {
  int sum_down;
  int sum_right;
};

struct CellBlank {
  GroupIndex group_down;
  GroupIndex group_right;
};

typedef struct {
  CellKind kind;
  union {
    CellHeader header;
    CellBlank blank;
    int cell_number;
  } data;
} Cell;

typedef uint16_t Bitset;
inline bool bitset_get(Bitset bitset, int i) {
  Bitset mask = 1 << i;
  return (bitset & mask) == mask;
}

inline bool bitset_contains(Bitset bitset, Bitset other) {
  return (bitset & other) == other;
}

inline Bitset bitset_subtract(Bitset bitset, Bitset other) {
  return bitset & ~other;
}

int bitset_digit_sum(Bitset bitset) {
  int sum = 0;
  for (int i = 1; i < 10; i++) {
    if (bitset_get(bitset, i)) {
      sum += i;
    }
  }
  return sum;
}

inline int bitset_popcount(Bitset bitset) { return __builtin_popcount(bitset); }

typedef struct {
  Bitset present_values;
  int cell_count;
  int target_sum;
} Group;

typedef struct {
  int start_offset;
  int end_offset;
} TableHeader;

typedef struct {
  Bitset *start;
  Bitset *end;
} TableRow;

constexpr int SUM_TABLE_SIZE = 45 * 9 + 8 + 1;
typedef struct {
  int width;
  int height;
  ArrayList values;
  ArrayList groups;
  TableHeader sum_table[SUM_TABLE_SIZE];
  Bitset *table_storage;
} Field;

int encode_table_sum(int target_sum, int cell_count) {
  assert(0 < target_sum && target_sum <= 45);
  assert(0 < cell_count && cell_count <= 9);
  return target_sum * 9 + cell_count - 1;
}

void field_populate_table(Field *field) {
  int entry_counts[SUM_TABLE_SIZE] = {};
  // generate permutations of bits 1-9 and count how many entries each
  // combination has 00xx xxxx xxx0
  for (Bitset mask = 2; mask <= 1022; mask += 2) {
    int sum = bitset_digit_sum(mask);
    int count = bitset_popcount(mask);

    int offset = encode_table_sum(sum, count);
    entry_counts[offset] += 1;
  }

  int start = 0;
  for (int i = 0; i < SUM_TABLE_SIZE; i++) {
    // TableHeader.end_offset will be increased when filling the table entries
    // in the next loop
    field->sum_table[i] = TableHeader{start, start};
    start += entry_counts[i];
  }

  Bitset *storage = (Bitset *)malloc(start * sizeof(Bitset));
  for (Bitset mask = 2; mask <= 1022; mask += 2) {
    int sum = bitset_digit_sum(mask);
    int count = bitset_popcount(mask);

    int offset = encode_table_sum(sum, count);
    int entry_offset = field->sum_table[offset].end_offset++;
    storage[entry_offset] = mask;
  }

  field->table_storage = storage;
}

TableRow field_table_get(Field *field, int target_sum, int cell_count) {
  int offset = encode_table_sum(target_sum, cell_count);
  TableHeader header = field->sum_table[offset];
  return TableRow{
      field->table_storage + header.start_offset,
      field->table_storage + header.end_offset,
  };
}

Cell *field_get(Field *field, int x, int y) {
  assert(x < field->width);
  assert(y < field->height);
  int index = x + y * field->width;
  return ((Cell *)field->values.allocation) + index;
}

void field_free(Field *field) {
  list_free(&field->values);
  list_free(&field->groups);
  free(field->table_storage);
}

bool consume_with(char **line, int (*fun)(int)) {
  if (fun(**line)) {
    (*line)++;
    return true;
  }
  return false;
}

bool consume_single(char **line, char c) {
  if (**line == c) {
    (*line)++;
    return true;
  }
  return false;
}

// we need to parse a string that is not null-terminated
// so we need to roll our own function
// https://stackoverflow.com/q/8716293
int substring_to_int(char *string, int len) {
  int n = 0;
  while (len--) {
    n = n * 10;
    n += *string++ - '0';
  }
  return n;
}

// parses the `X/22` expression
Result parse_cell_expr(char **line, int *sum) {
  if (consume_single(line, 'X')) {
    *sum = 0;
  } else {
    char *start = *line;
    while (consume_with(line, isdigit)) {
    }
    char *end = *line;

    ptrdiff_t len = end - start;
    ENSURE(len > 0);
    *sum = substring_to_int(start, len);
    ENSURE(0 < *sum && *sum <= 45);
  }
  return RESULT_OK;
}

Result parse_cell(Cell *cell, char **line) {
  while (consume_with(line, isspace)) {
  }
  // return if we're at the end of line
  if (**line == '\0') {
    return RESULT_EMPTY;
  }
  if (consume_single(line, '.')) {
    cell->kind = KIND_EMPTY;
    cell->data.blank = CellBlank{-1, -1};
    return RESULT_OK;
  }
  if (**line == 'X' && *(*line + 1) != '\\') {
    (*line)++;
    cell->kind = KIND_HEADER;
    cell->data.header = CellHeader{0, 0};
    return RESULT_OK;
  }
  if (**line == 'X' || isdigit(**line)) {
    int sum1 = 0;
    int sum2 = 0;
    TRY(parse_cell_expr(line, &sum1));
    ENSURE(consume_single(line, '\\'));
    TRY(parse_cell_expr(line, &sum2));
    cell->kind = KIND_HEADER;
    cell->data.header = CellHeader{sum1, sum2};
    return RESULT_OK;
  }
  // we haven't matched any valid characters
  return RESULT_ERR;
}

Result field_parse_line(Field *field, char *line) {
  int start = field->values.size / sizeof(Cell);
  while (*line) {
    Cell cell = {};
    Result result = parse_cell(&cell, &line);
    if (result == RESULT_EMPTY) {
      break;
    }
    if (result != RESULT_OK) {
      return RESULT_ERR;
    }
    list_push(&field->values, &cell, sizeof(Cell));
  }
  int end = field->values.size / sizeof(Cell);
  int line_len = end - start;

  // set the initial width when on the first line
  if (start == 0) {
    field->width = line_len;
  } else {
    ENSURE(field->width == line_len);
  }
  field->height += 1;

  return RESULT_OK;
}

int print_cell(const Cell *cell) {
  int c = 0;
  if (cell->kind == KIND_HEADER) {
    CellHeader header = cell->data.header;
    if (header.sum_down == 0 && header.sum_right == 0) {
      c += printf("X");
    } else {
      if (header.sum_down == 0) {
        c += printf("X");
      } else {
        c += printf("%d", header.sum_down);
      }
      c += printf("\\");
      if (header.sum_right == 0) {
        c += printf("X");
      } else {
        c += printf("%d", header.sum_right);
      }
    }
  }
  if (cell->kind == KIND_EMPTY) {
    c += printf(".");
  }
  if (cell->kind == KIND_NUMBER) {
    c += printf("%d", cell->data.cell_number);
  }

  assert(c > 0);
  return c;
}

void field_print(Field *field) {
  assert(field->width > 0);
  for (int y = 0; y < field->height; y++) {
    for (int x = 0; x < field->width; x++) {
      if (x != 0) {
        printf(" ");
      }

      const Cell *cell = field_get(field, x, y);
      int written = print_cell(cell);

      // manually pad to 5
      for (; written < 5; written++) {
        printf(" ");
      }
    }
    printf("\n");
  }
}

Result check_cell(int x, int y, Cell *cell, bool is_down, int *previous_empty) {
  CellKind kind = cell->kind;

  // if we're at an empty cell, just add it to the counter
  if (kind == KIND_EMPTY) {
    *previous_empty += 1;
    return RESULT_OK;
  }

  // if there are more than 9 empty cells in a row,
  // we cannot assign them 9 unique digits
  if (*previous_empty > 9) {
    DEBUGF("cell at [%d, %d] too many empties\n", x, y);
    return RESULT_ERR;
  }

  CellHeader header = cell->data.header;
  char sum = is_down ? header.sum_down : header.sum_right;

  // X\X . . .    there is no sum header for these empties
  if (sum == 0 && *previous_empty != 0) {
    DEBUGF("X cell at [%d, %d] has empties\n", x, y);
    return RESULT_ERR;
  }

  // X\2    X     there are no empties for this header
  if (sum != 0 && *previous_empty == 0) {
    DEBUGF("cell at [%d, %d] no empties\n", x, y);
    return RESULT_ERR;
  }

  *previous_empty = 0;
  return RESULT_OK;
}

Result field_validate(Field *field) {
  Cell black = Cell{KIND_HEADER, {}};
  for (int y = 0; y < field->height; y++) {
    int previous_empty = 0;
    for (int x = field->width - 1; x >= 0; x--) {
      Cell *cell = field_get(field, x, y);
      TRY(check_cell(x, y, cell, false, &previous_empty));
    }
    TRY(check_cell(-1, y, &black, false, &previous_empty));
  }
  for (int x = 0; x < field->width; x++) {
    int previous_empty = 0;
    for (int y = field->height - 1; y >= 0; y--) {
      Cell *cell = field_get(field, x, y);
      TRY(check_cell(x, y, cell, true, &previous_empty));
    }
    TRY(check_cell(x, -1, &black, true, &previous_empty));
  }
  return RESULT_OK;
}

GroupIndex field_add_group(Field *field, int target_sum) {
  GroupIndex offset = field->groups.size / sizeof(Group);
  Group group = {};
  group.target_sum = target_sum;
  list_push(&field->groups, &group, sizeof(Group));
  return offset;
}

Group *field_get_group(Field *field, GroupIndex group) {
  return (Group *)field->groups.allocation + group;
}

void add_group(Field *field, GroupIndex *current_group, Cell *cell,
               bool is_down) {
  if (cell->kind == KIND_EMPTY) {
    if (is_down) {
      cell->data.blank.group_down = *current_group;
    } else {
      cell->data.blank.group_right = *current_group;
    }
    field_get_group(field, *current_group)->cell_count += 1;
  } else if (cell->kind == KIND_HEADER) {
    CellHeader header = cell->data.header;
    char sum = is_down ? header.sum_down : header.sum_right;
    if (sum > 0) {
      *current_group = field_add_group(field, sum);
    }
  }
}

void field_populate_groups(Field *field) {
  for (int y = 0; y < field->height; y++) {
    GroupIndex current_group = -1;
    for (int x = 0; x < field->width; x++) {
      Cell *cell = field_get(field, x, y);
      add_group(field, &current_group, cell, false);
    }
  }
  for (int x = 0; x < field->width; x++) {
    GroupIndex current_group = -1;
    for (int y = 0; y < field->height; y++) {
      Cell *cell = field_get(field, x, y);
      add_group(field, &current_group, cell, true);
    }
  }
}

typedef struct {
  int *first_solution;
  int solution_count;
} KakuroSolution;

typedef struct {
  Group *group_down;
  Group *group_right;
  TableRow row_down;
  TableRow row_right;
} GroupPair;

inline Bitset get_available_group_values(Group *group, TableRow row) {
  Bitset result = 0;
  for (Bitset *set = row.start; set < row.end; set++) {
    // all full bitsets must be a subset of the already chosen values
    if (bitset_contains(*set, group->present_values)) {
      result |= *set;
    }
  }

  // can only choose values not already present
  return bitset_subtract(result, group->present_values);
}

inline Bitset get_available_cell_values(GroupPair *pair) {
  Bitset down = get_available_group_values(pair->group_down, pair->row_down);
  Bitset right = get_available_group_values(pair->group_right, pair->row_right);
  return down & right;
}

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// the domain pruning approached taken from
// https://github.com/zvrba/yass/blob/master/kakuro.cc
void backtracking_search(Field *field, GroupPair *cells, int *cells_out,
                         int cells_len, KakuroSolution *out_solution,
                         int offset) {

  // all variables have been assigned, a solution is found
  if (unlikely(offset == cells_len)) {
    if (unlikely(out_solution->solution_count == 0)) {
      int size = cells_len * sizeof(int);
      int *solution = (int *)malloc(size);
      memcpy(solution, cells_out, size);
      out_solution->first_solution = solution;
    }
    out_solution->solution_count += 1;
    return;
  }

  GroupPair *cell = cells + offset;
  int *cell_value = cells_out + offset;
  Bitset available = get_available_cell_values(cell);

  if (available == 0) {
    return;
  }

  for (unsigned i = 1; i < 10; i++) {
    int mask = 1 << i;
    if ((available & mask) == mask) {
      *cell_value = i;

      Bitset *down = &cell->group_down->present_values;
      Bitset *right = &cell->group_right->present_values;

      *down |= mask;
      *right |= mask;
      backtracking_search(field, cells, cells_out, cells_len, out_solution,
                          offset + 1);
      *down &= ~mask;
      *right &= ~mask;
    }
  }
}

KakuroSolution field_find_solutions(Field *field) {
  // the backtracking search is the most time consuming part of this
  // so we put all the blank cells/variables into a compact array
  ArrayList cells = {};
  for (int y = 0; y < field->height; y++) {
    for (int x = 0; x < field->width; x++) {
      Cell *cell = field_get(field, x, y);
      if (cell->kind == KIND_EMPTY) {
        CellBlank blank = cell->data.blank;
        assert(blank.group_down >= 0);
        assert(blank.group_right >= 0);

        Group *group_down = field_get_group(field, blank.group_down);
        Group *group_right = field_get_group(field, blank.group_right);

        GroupPair pair = GroupPair{
            group_down,
            group_right,
            field_table_get(field, group_down->target_sum,
                            group_down->cell_count),
            field_table_get(field, group_right->target_sum,
                            group_right->cell_count),
        };
        list_push(&cells, &pair, sizeof(GroupPair));
      }
    }
  }

  KakuroSolution solution = KakuroSolution{0, 0};
  {
    GroupPair *pairs = (GroupPair *)cells.allocation;
    int len = cells.size / sizeof(GroupPair);
    int *values = (int *)calloc(len, sizeof(int));

    backtracking_search(field, pairs, values, len, &solution, 0);

    free(values);
    list_free(&cells);
  }

  int offset = 0;
  if (solution.first_solution) {
    for (int y = 0; y < field->height; y++) {
      for (int x = 0; x < field->width; x++) {
        Cell *cell = field_get(field, x, y);
        if (cell->kind == KIND_EMPTY) {
          cell->kind = KIND_NUMBER;
          cell->data.cell_number = solution.first_solution[offset++];
        }
      }
    }
  }

  return solution;
}

Result run(Field *field, char **line_buf, size_t *line_len) {
  printf("Zadejte kakuro:\n");

  while (getline(line_buf, line_len, stdin) > 0) {
    TRY(field_parse_line(field, *line_buf));
  }

  ENSURE((1 <= field->width && field->width <= 32) &&
         (1 <= field->height && field->height <= 32) &&
         field_validate(field) == RESULT_OK);

  field_populate_groups(field);
  field_populate_table(field);
  KakuroSolution solution = field_find_solutions(field);

  if (solution.solution_count == 0) {
    printf("Reseni neexistuje.\n");
  } else if (solution.solution_count == 1) {
    printf("Kakuro ma jedno reseni:\n");
    field_print(field);
  } else if (solution.solution_count > 1) {
    printf("Celkem ruznych reseni: %d\n", solution.solution_count);
  }

  free(solution.first_solution);
  return RESULT_OK;
}

int main() {
  char *line_buf = NULL;
  size_t line_len = 0;

  Field field = {};

  Result result = run(&field, &line_buf, &line_len);
  if (result != RESULT_OK) {
    printf("Nespravny vstup.\n");
  }

  field_free(&field);
  free(line_buf);
  return 0;
}