#include <cassert>
#include <cctype>
#include <cstdarg>
#include <cstddef>
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

typedef struct {
  char sum_down;
  char sum_right;
} Cell;

// special sum values:
//  X  CELL_X_VALUE
//  .  CELL_EMPTY_VALUE
constexpr char CELL_X_VALUE = 0;
constexpr char CELL_EMPTY_VALUE = -1;
constexpr Cell CELL_X = Cell{CELL_X_VALUE, CELL_X_VALUE};
constexpr Cell CELL_EMPTY = Cell{CELL_EMPTY_VALUE, CELL_EMPTY_VALUE};

typedef struct {
  int width;
  int height;
  ArrayList values;
} Field;

Cell *field_get(Field *field, int x, int y) {
  assert(x < field->width);
  assert(y < field->height);
  int index = x + y * field->width;
  return ((Cell *)field->values.allocation) + index;
}

void field_free(Field *field) { list_free(&field->values); }

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
    *cell = CELL_EMPTY;
    return RESULT_OK;
  }
  if (**line == 'X' && *(*line + 1) != '\\') {
    (*line)++;
    *cell = CELL_X;
    return RESULT_OK;
  }
  if (**line == 'X' || isdigit(**line)) {
    int sum1 = 0;
    int sum2 = 0;
    TRY(parse_cell_expr(line, &sum1));
    ENSURE(consume_single(line, '\\'));
    TRY(parse_cell_expr(line, &sum2));
    *cell = Cell{(char)sum1, (char)sum2};
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

void format_append(char *buffer, int maxlen, int *cursor, const char *format,
                   ...) {
  int remaining = maxlen - *cursor;
  // 1 because the last byte in the buffer will be null anyway
  if (remaining <= 1) {
    return;
  }

  va_list va_args;
  va_start(va_args, format);
  int n = vsnprintf(buffer + *cursor, remaining, format, va_args);
  if (n > 0) {
    *cursor += n;
  }
}

void field_debug(Field *field) {
  assert(field->width > 0);
  for (int y = 0; y < field->height; y++) {
    for (int x = 0; x < field->width; x++) {
      Cell *cell = field_get(field, x, y);
      char buffer[16] = {};
      int cursor = 0;

      if (cell->sum_down == -1) {
        snprintf(buffer, 16, ".");
      } else if (cell->sum_down == 0 && cell->sum_right == 0) {
        snprintf(buffer, 16, "X");
      } else {
        if (cell->sum_down == 0) {
          format_append(buffer, 16, &cursor, "X");
        } else {
          format_append(buffer, 16, &cursor, "%d", cell->sum_down);
        }
        format_append(buffer, 16, &cursor, "\\");
        if (cell->sum_right == 0) {
          format_append(buffer, 16, &cursor, "X");
        } else {
          format_append(buffer, 16, &cursor, "%d", cell->sum_right);
        }
      }
      if (x != 0) {
        printf(" ");
      }
      printf("%-5s", buffer);
    }
    printf("\n");
  }
}

Result check_cell(int x, int y, int sum_value, int *previous_empty) {
  // if we're at an empty cell, just add it to the counter
  if (sum_value == CELL_EMPTY_VALUE) {
    *previous_empty += 1;
    return RESULT_OK;
  }

  // if there are more than 9 empty cells in a row,
  // we cannot assign them 9 unique digits
  if (*previous_empty > 9) {
    DEBUGF("cell at [%d, %d] too many empties\n", x, y);
    return RESULT_ERR;
  }

  // X\X . . .    there is no sum header for these empties
  if (sum_value == CELL_X_VALUE && *previous_empty != 0) {
    DEBUGF("X cell at [%d, %d] has empties\n", x, y);
    return RESULT_ERR;
  }

  // X\2    X     there are no empties for this header
  if (sum_value != CELL_EMPTY_VALUE && sum_value != CELL_X_VALUE) {
    if (*previous_empty == 0) {
      DEBUGF("cell at [%d, %d] no empties\n", x, y);
      return RESULT_ERR;
    }
    *previous_empty = 0;
  }

  return RESULT_OK;
}

Result field_validate(Field *field) {
  for (int y = 0; y < field->height; y++) {
    int previous_empty = 0;
    for (int x = field->width - 1; x >= 0; x--) {
      Cell *cell = field_get(field, x, y);
      TRY(check_cell(x, y, cell->sum_right, &previous_empty));
    }
    TRY(check_cell(0, y, 0, &previous_empty));
  }
  for (int x = 0; x < field->width; x++) {
    int previous_empty = 0;
    for (int y = field->height - 1; y >= 0; y--) {
      Cell *cell = field_get(field, x, y);
      TRY(check_cell(x, y, cell->sum_down, &previous_empty));
    }
    TRY(check_cell(x, 0, 0, &previous_empty));
  }
  return RESULT_OK;
}

Result run(Field *field, char **line_buf, size_t *line_len) {
  while (getline(line_buf, line_len, stdin) > 0) {
    TRY(field_parse_line(field, *line_buf));
  }

  if ((1 > field->width || field->width > 32) ||
      (1 > field->height || field->height > 32)) {
    return RESULT_ERR;
  }

  field_debug(field);
  TRY(field_validate(field));

  return RESULT_OK;
}

int main() {
  char *line_buf = NULL;
  size_t line_len = 0;

  Field field = {};

  Result result = run(&field, &line_buf, &line_len);
  if (result != RESULT_OK) {
    printf("Nespravny vstup.");
  }

  field_free(&field);
  free(line_buf);
  return 0;
}