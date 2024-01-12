#include <cstdlib>
#ifndef __PROGTEST__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TCell {
  struct TCell *m_Right;
  struct TCell *m_Down;
  int m_Row;
  int m_Col;
  int m_Data;
} TCELL;

typedef struct TRowCol {
  struct TRowCol *m_Next;
  TCELL *m_Cells;
  int m_Idx;
} TROWCOL;

typedef struct TSparseMatrix {
  TROWCOL *m_Rows;
  TROWCOL *m_Cols;
} TSPARSEMATRIX;
#endif /* __PROGTEST__ */

TROWCOL *new_trowcol() { return (TROWCOL *)malloc(sizeof(TROWCOL)); }
TCELL *new_cell() { return (TCELL *)malloc(sizeof(TCELL)); }

int get_index(TCELL *cell, bool down) {
  if (down) {
    return cell->m_Row;
  } else {
    return cell->m_Col;
  }
}

TCELL *get_next(TCELL *cell, bool down) {
  if (down) {
    return cell->m_Down;
  } else {
    return cell->m_Right;
  }
}

void list_find(TROWCOL *start, int index, TROWCOL **out_prev,
               TROWCOL **out_curr) {
  TROWCOL *prev = NULL;
  TROWCOL *curr = start;
  while (curr && curr->m_Idx <= index) {
    if (curr->m_Idx == index) {
      *out_prev = prev;
      *out_curr = curr;
      return;
    }
    prev = curr;
    curr = curr->m_Next;
  }
  *out_prev = prev;
  *out_curr = NULL;
}

void cell_find(TCELL *start, int index, bool down, TCELL **out_prev,
               TCELL **out_curr) {
  TCELL *prev = NULL;
  TCELL *curr = start;
  while (curr && get_index(curr, down) <= index) {
    if (get_index(curr, down) == index) {
      *out_prev = prev;
      *out_curr = curr;
      return;
    }
    prev = curr;
    curr = get_next(curr, down);
  }
  *out_prev = prev;
  *out_curr = NULL;
}

TROWCOL *list_find_or_add(TROWCOL **start, int index) {
  TROWCOL *prev = NULL;
  TROWCOL *curr = NULL;
  list_find(*start, index, &prev, &curr);

  if (curr) {
    return curr;
  }

  TROWCOL *add = new_trowcol();
  *add = TROWCOL{NULL, NULL, index};

  if (prev) {
    TROWCOL *next = prev->m_Next;
    prev->m_Next = add;
    add->m_Next = next;
  } else {
    add->m_Next = *start;
    *start = add;
  }

  return add;
}

TCELL *cell_find_or_add(TCELL **start, int row_index, int col_index, bool down,
                        TCELL *use, bool *inserted) {
  TCELL *prev = NULL;
  TCELL *curr = NULL;
  cell_find(*start, down ? row_index : col_index, down, &prev, &curr);

  if (curr) {
    *inserted = false;
    return curr;
  }

  TCELL *add = use;
  if (!use) {
    add = new_cell();
    *add = TCELL{NULL, NULL, row_index, col_index, -1};
  }

  if (prev) {
    if (down) {
      TCELL *next = prev->m_Down;
      prev->m_Down = add;
      add->m_Down = next;
    } else {
      TCELL *next = prev->m_Right;
      prev->m_Right = add;
      add->m_Right = next;
    }
  } else {
    if (down) {
      add->m_Down = *start;
    } else {
      add->m_Right = *start;
    }
    *start = add;
  }

  *inserted = true;
  return add;
}

void cell_free(TCELL *start, bool down) {
  TCELL *curr = start;
  while (curr) {
    TCELL *tmp = curr;
    curr = get_next(curr, down);
    free(tmp);
  }
}

void list_free(TROWCOL *start, bool is_rows) {
  TROWCOL *curr = start;
  while (curr) {
    TROWCOL *tmp = curr;
    if (is_rows) {
      cell_free(curr->m_Cells, false);
    }
    curr = curr->m_Next;
    free(tmp);
  }
}

void initMatrix(TSPARSEMATRIX *m) { *m = {}; }
void addSetCell(TSPARSEMATRIX *m, int rowIdx, int colIdx, int data) {
  bool inserted = false;
  TROWCOL *col = list_find_or_add(&m->m_Cols, colIdx);
  TCELL *cell =
      cell_find_or_add(&col->m_Cells, rowIdx, colIdx, true, NULL, &inserted);

  cell->m_Data = data;
  if (!inserted) {
    return;
  }

  TROWCOL *row = list_find_or_add(&m->m_Rows, rowIdx);
  cell_find_or_add(&row->m_Cells, rowIdx, colIdx, false, cell, &inserted);
}
bool removeCell(TSPARSEMATRIX *m, int rowIdx, int colIdx) {
  TROWCOL *col_prev;
  TROWCOL *col;
  list_find(m->m_Cols, colIdx, &col_prev, &col);
  if (!col) {
    return false;
  }
  TROWCOL *row_prev;
  TROWCOL *row;
  list_find(m->m_Rows, rowIdx, &row_prev, &row);
  if (!row) {
    return false;
  }

  TCELL *col_prev_cell;
  TCELL *col_curr_cell;
  cell_find(col->m_Cells, rowIdx, true, &col_prev_cell, &col_curr_cell);

  TCELL *row_prev_cell;
  TCELL *row_curr_cell;
  cell_find(row->m_Cells, colIdx, false, &row_prev_cell, &row_curr_cell);

  if (!col_curr_cell || !row_curr_cell) {
    return false;
  }
  assert(col_curr_cell == row_curr_cell);

  if (col_prev_cell) {
    col_prev_cell->m_Down = col_curr_cell->m_Down;
  } else {
    col->m_Cells = col_curr_cell->m_Down;
  }

  if (row_prev_cell) {
    row_prev_cell->m_Right = row_curr_cell->m_Right;
  } else {
    row->m_Cells = row_curr_cell->m_Right;
  }

  free(col_curr_cell);

  if (col->m_Cells == NULL) {
    if (col_prev) {
      col_prev->m_Next = col->m_Next;
    } else {
      m->m_Cols = col->m_Next;
    }
    free(col);
  }

  if (row->m_Cells == NULL) {
    if (row_prev) {
      row_prev->m_Next = row->m_Next;
    } else {
      m->m_Rows = row->m_Next;
    }
    free(row);
  }

  return true;
}
void freeMatrix(TSPARSEMATRIX *m) {
  list_free(m->m_Cols, false);
  list_free(m->m_Rows, true);
}
#ifndef __PROGTEST__
#define ASSERT_EQ(a, b)                                                        \
  {                                                                            \
    auto _a = a;                                                               \
    auto _b = b;                                                               \
    if (_a != _b) {                                                            \
      printf("%s:%d Assertion failed:\n  %s != %s\n  %ld != %ld\n", __FILE__,  \
             __LINE__, #a, #b, (size_t)_a, (size_t)_b);                        \
      abort();                                                                 \
    }                                                                          \
  }

void fuzz() {
  int dense[4][4] = {};
  TSPARSEMATRIX sparse = {};

  srand(2);

  for (int i = 0; i <= 14; i++) {
    int x = rand() % 4;
    int y = rand() % 4;
    int val = rand() % 4 + 1;

    if (rand() % 2 == 0) {
      addSetCell(&sparse, y, x, val);
      dense[y][x] = val;
    } else {
      removeCell(&sparse, y, x);
      dense[y][x] = 0;
    }

    int temp[4][4] = {};
    TROWCOL *col = sparse.m_Cols;
    while (col) {
      TCELL *cell = col->m_Cells;
      while (cell) {
        int *dense_ptr = &temp[cell->m_Row][cell->m_Col];
        if (*dense_ptr) {
          printf("Non-zero value when adding [%d, %d] %d  (i: %d)\n",
                 cell->m_Col, cell->m_Row, val, i);
        }
        *dense_ptr = cell->m_Data;
        cell = cell->m_Down;
      }
      col = col->m_Next;
    }

    for (int x_ = 0; x_ < 4; x_++) {
      for (int y_ = 0; y_ < 4; y_++) {
        if (temp[y][x] != dense[y][x]) {
          printf("Value mismatch value when adding [%d, %d] %d  (i: %d)\n", x,
                 y, val, i);
        }
      }
    }
  }

  freeMatrix(&sparse);
}

int main() {
  fuzz();
  return 0;

  TSPARSEMATRIX m;
  initMatrix(&m);
  addSetCell(&m, 0, 1, 10);
  addSetCell(&m, 1, 0, 20);
  addSetCell(&m, 1, 5, 30);
  addSetCell(&m, 2, 1, 40);

  assert(m.m_Rows);
  ASSERT_EQ(m.m_Rows->m_Idx, 0);
  assert(m.m_Rows->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Row, 0);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Col, 1);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Data, 10);
  assert(m.m_Rows->m_Cells->m_Right == nullptr);

  assert(m.m_Rows->m_Next);
  ASSERT_EQ(m.m_Rows->m_Next->m_Idx, 1);
  assert(m.m_Rows->m_Next->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Row, 1);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Col, 0);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Data, 20);
  assert(m.m_Rows->m_Next->m_Cells->m_Right);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Right->m_Row, 1);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Right->m_Col, 5);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Right->m_Data, 30);
  assert(m.m_Rows->m_Next->m_Cells->m_Right->m_Right == nullptr);

  assert(m.m_Rows->m_Next->m_Next);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Idx, 2);
  assert(m.m_Rows->m_Next->m_Next->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells->m_Row, 2);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells->m_Col, 1);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells->m_Data, 40);
  assert(m.m_Rows->m_Next->m_Next->m_Cells->m_Right == nullptr);

  assert(m.m_Rows->m_Next->m_Next->m_Next == nullptr);

  assert(m.m_Cols);
  ASSERT_EQ(m.m_Cols->m_Idx, 0);
  assert(m.m_Cols->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Cells->m_Row, 1);
  ASSERT_EQ(m.m_Cols->m_Cells->m_Col, 0);
  ASSERT_EQ(m.m_Cols->m_Cells->m_Data, 20);
  assert(m.m_Cols->m_Cells->m_Down == nullptr);

  assert(m.m_Cols->m_Next);
  ASSERT_EQ(m.m_Cols->m_Next->m_Idx, 1);
  assert(m.m_Cols->m_Next->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Row, 0);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Col, 1);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Data, 10);
  assert(m.m_Cols->m_Next->m_Cells->m_Down);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Down->m_Row, 2);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Down->m_Col, 1);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Down->m_Data, 40);
  assert(m.m_Cols->m_Next->m_Cells->m_Down->m_Down == nullptr);

  assert(m.m_Cols->m_Next->m_Next);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Idx, 5);
  assert(m.m_Cols->m_Next->m_Next->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Cells->m_Row, 1);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Cells->m_Col, 5);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Cells->m_Data, 30);
  assert(m.m_Cols->m_Next->m_Next->m_Cells->m_Down == nullptr);

  assert(m.m_Cols->m_Next->m_Next->m_Next == nullptr);

  ASSERT_EQ(m.m_Rows->m_Cells, m.m_Cols->m_Next->m_Cells);

  ASSERT_EQ(m.m_Rows->m_Next->m_Cells, m.m_Cols->m_Cells);

  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Right,
            m.m_Cols->m_Next->m_Next->m_Cells);

  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells,
            m.m_Cols->m_Next->m_Cells->m_Down);
  addSetCell(&m, 230, 190, 50);

  assert(m.m_Rows);
  ASSERT_EQ(m.m_Rows->m_Idx, 0);
  assert(m.m_Rows->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Row, 0);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Col, 1);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Data, 10);
  assert(m.m_Rows->m_Cells->m_Right == nullptr);

  assert(m.m_Rows->m_Next);
  ASSERT_EQ(m.m_Rows->m_Next->m_Idx, 1);
  assert(m.m_Rows->m_Next->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Row, 1);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Col, 0);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Data, 20);
  assert(m.m_Rows->m_Next->m_Cells->m_Right);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Right->m_Row, 1);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Right->m_Col, 5);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Right->m_Data, 30);
  assert(m.m_Rows->m_Next->m_Cells->m_Right->m_Right == nullptr);

  assert(m.m_Rows->m_Next->m_Next);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Idx, 2);
  assert(m.m_Rows->m_Next->m_Next->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells->m_Row, 2);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells->m_Col, 1);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells->m_Data, 40);
  assert(m.m_Rows->m_Next->m_Next->m_Cells->m_Right == nullptr);

  assert(m.m_Rows->m_Next->m_Next->m_Next);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Next->m_Idx, 230);
  assert(m.m_Rows->m_Next->m_Next->m_Next->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Next->m_Cells->m_Row, 230);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Next->m_Cells->m_Col, 190);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Next->m_Cells->m_Data, 50);
  assert(m.m_Rows->m_Next->m_Next->m_Next->m_Cells->m_Right == nullptr);

  assert(m.m_Rows->m_Next->m_Next->m_Next->m_Next == nullptr);

  assert(m.m_Cols);
  ASSERT_EQ(m.m_Cols->m_Idx, 0);
  assert(m.m_Cols->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Cells->m_Row, 1);
  ASSERT_EQ(m.m_Cols->m_Cells->m_Col, 0);
  ASSERT_EQ(m.m_Cols->m_Cells->m_Data, 20);
  assert(m.m_Cols->m_Cells->m_Down == nullptr);

  assert(m.m_Cols->m_Next);
  ASSERT_EQ(m.m_Cols->m_Next->m_Idx, 1);
  assert(m.m_Cols->m_Next->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Row, 0);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Col, 1);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Data, 10);
  assert(m.m_Cols->m_Next->m_Cells->m_Down);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Down->m_Row, 2);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Down->m_Col, 1);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Down->m_Data, 40);
  assert(m.m_Cols->m_Next->m_Cells->m_Down->m_Down == nullptr);

  assert(m.m_Cols->m_Next->m_Next);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Idx, 5);
  assert(m.m_Cols->m_Next->m_Next->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Cells->m_Row, 1);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Cells->m_Col, 5);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Cells->m_Data, 30);
  assert(m.m_Cols->m_Next->m_Next->m_Cells->m_Down == nullptr);

  assert(m.m_Cols->m_Next->m_Next->m_Next);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Next->m_Idx, 190);
  assert(m.m_Cols->m_Next->m_Next->m_Next->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Row, 230);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Col, 190);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Data, 50);
  assert(m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Down == nullptr);

  assert(m.m_Cols->m_Next->m_Next->m_Next->m_Next == nullptr);

  ASSERT_EQ(m.m_Rows->m_Cells, m.m_Cols->m_Next->m_Cells);

  ASSERT_EQ(m.m_Rows->m_Next->m_Cells, m.m_Cols->m_Cells);

  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Right,
            m.m_Cols->m_Next->m_Next->m_Cells);

  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells,
            m.m_Cols->m_Next->m_Cells->m_Down);

  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Next->m_Cells,
            m.m_Cols->m_Next->m_Next->m_Next->m_Cells);

  assert(removeCell(&m, 0, 1));

  assert(!removeCell(&m, 0, 1));

  assert(!removeCell(&m, 1, 2));

  assert(m.m_Rows);
  ASSERT_EQ(m.m_Rows->m_Idx, 1);
  assert(m.m_Rows->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Row, 1);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Col, 0);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Data, 20);
  assert(m.m_Rows->m_Cells->m_Right);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Right->m_Row, 1);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Right->m_Col, 5);
  ASSERT_EQ(m.m_Rows->m_Cells->m_Right->m_Data, 30);
  assert(m.m_Rows->m_Cells->m_Right->m_Right == nullptr);

  assert(m.m_Rows->m_Next);
  ASSERT_EQ(m.m_Rows->m_Next->m_Idx, 2);
  assert(m.m_Rows->m_Next->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Row, 2);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Col, 1);
  ASSERT_EQ(m.m_Rows->m_Next->m_Cells->m_Data, 40);
  assert(m.m_Rows->m_Next->m_Cells->m_Right == nullptr);

  assert(m.m_Rows->m_Next->m_Next);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Idx, 230);
  assert(m.m_Rows->m_Next->m_Next->m_Cells);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells->m_Row, 230);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells->m_Col, 190);
  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells->m_Data, 50);
  assert(m.m_Rows->m_Next->m_Next->m_Cells->m_Right == nullptr);

  assert(m.m_Rows->m_Next->m_Next->m_Next == nullptr);

  assert(m.m_Cols);
  ASSERT_EQ(m.m_Cols->m_Idx, 0);
  assert(m.m_Cols->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Cells->m_Row, 1);
  ASSERT_EQ(m.m_Cols->m_Cells->m_Col, 0);
  ASSERT_EQ(m.m_Cols->m_Cells->m_Data, 20);
  assert(m.m_Cols->m_Cells->m_Down == nullptr);

  assert(m.m_Cols->m_Next);
  ASSERT_EQ(m.m_Cols->m_Next->m_Idx, 1);
  assert(m.m_Cols->m_Next->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Row, 2);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Col, 1);
  ASSERT_EQ(m.m_Cols->m_Next->m_Cells->m_Data, 40);
  assert(m.m_Cols->m_Next->m_Cells->m_Down == nullptr);

  assert(m.m_Cols->m_Next->m_Next);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Idx, 5);
  assert(m.m_Cols->m_Next->m_Next->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Cells->m_Row, 1);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Cells->m_Col, 5);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Cells->m_Data, 30);
  assert(m.m_Cols->m_Next->m_Next->m_Cells->m_Down == nullptr);

  assert(m.m_Cols->m_Next->m_Next->m_Next);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Next->m_Idx, 190);
  assert(m.m_Cols->m_Next->m_Next->m_Next->m_Cells);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Row, 230);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Col, 190);
  ASSERT_EQ(m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Data, 50);
  assert(m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Down == nullptr);

  assert(m.m_Cols->m_Next->m_Next->m_Next->m_Next == nullptr);

  ASSERT_EQ(m.m_Rows->m_Cells, m.m_Cols->m_Cells);

  ASSERT_EQ(m.m_Rows->m_Cells->m_Right, m.m_Cols->m_Next->m_Next->m_Cells);

  ASSERT_EQ(m.m_Rows->m_Next->m_Cells, m.m_Cols->m_Next->m_Cells);

  ASSERT_EQ(m.m_Rows->m_Next->m_Next->m_Cells,
            m.m_Cols->m_Next->m_Next->m_Next->m_Cells);
  freeMatrix(&m);
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
