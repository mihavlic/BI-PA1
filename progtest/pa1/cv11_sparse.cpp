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

TROWCOL *new_trowcol(TROWCOL *next, int index) {
  TROWCOL *add = (TROWCOL *)malloc(sizeof(TROWCOL));
  add->m_Next = next;
  add->m_Cells = NULL;
  add->m_Idx = index;
  return add;
}

TCELL *new_cell() {
  TCELL *add = (TCELL *)malloc(sizeof(TCELL));
  *add = {};
  return add;
}

TROWCOL *list_find_or_add(TROWCOL **start, int index) {
  if (!*start || index < (*start)->m_Idx) {
    TROWCOL *add = new_trowcol(*start, index);
    *start = add;
    return add;
  }

  TROWCOL *prev = NULL;
  TROWCOL *curr = *start;
  while (curr) {
    if (curr->m_Idx == index) {
      return curr;
    }
    if (curr->m_Idx >= index) {
      break;
    }
    prev = curr;
    curr = curr->m_Next;
  }

  TROWCOL *add = new_trowcol(curr, index);
  if (prev) {
    prev->m_Next = add;
  }
  return add;
}

int get_index(TCELL *cell, bool row) {
  if (row) {
    return cell->m_Row;
  } else {
    return cell->m_Col;
  }
}

TCELL *cell_find_or_add_row(TCELL **start, int row_index, int col_index,
                            bool *inserted) {
  if (!*start || row_index < (*start)->m_Row) {
    TCELL *add = new_cell();
    add->m_Down = *start;
    add->m_Col = col_index;
    *start = add;
    *inserted = true;
    return add;
  }

  TCELL *prev = NULL;
  TCELL *curr = *start;
  while (curr) {
    if (curr->m_Row == row_index) {
      *inserted = false;
      return curr;
    }
    if (curr->m_Row >= row_index) {
      break;
    }
    prev = curr;
    curr = curr->m_Down;
  }

  TCELL *add = new_cell();
  add->m_Down = curr;
  add->m_Col = col_index;

  if (prev) {
    add->m_Down = add;
  }
  *inserted = true;
  return add;
}

TROWCOL *list_find(TROWCOL *start, int index) {
  TROWCOL *curr = start;
  while (curr) {
    if (curr->m_Idx == index) {
      return curr;
    }
    curr = curr->m_Next;
  }
  return NULL;
}

TCELL *cell_find(TCELL *start, int index, bool row) {
  TCELL *curr = start;
  while (curr) {
    if (get_index(curr, row) == index) {
      return curr;
    }
    if (row) {
      curr = curr->m_Down;
    } else {
      curr = curr->m_Right;
    }
  }
  return NULL;
}

TCELL *cell_find_before(TCELL *start, int index, bool row) {
  TCELL *prev = NULL;
  TCELL *curr = start;
  while (curr && get_index(curr, row) < index) {
    prev = curr;
    if (row) {
      curr = curr->m_Down;
    } else {
      curr = curr->m_Right;
    }
  }
  return prev;
}

void initMatrix(TSPARSEMATRIX *m) { *m = {}; }
void addSetCell(TSPARSEMATRIX *m, int rowIdx, int colIdx, int data) {
  TROWCOL *col = list_find_or_add(&m->m_Cols, colIdx);
  bool inserted = false;
  TCELL *cell = cell_find_or_add_row(&col->m_Cells, rowIdx, colIdx, &inserted);
  cell->m_Data = data;

  if (!inserted) {
    return;
  }

  TROWCOL *row = list_find_or_add(&m->m_Rows, rowIdx);
}
bool removeCell(TSPARSEMATRIX *m, int rowIdx, int colIdx) {
  // todo
}
void freeMatrix(TSPARSEMATRIX *m) {
  // todo
}
#ifndef __PROGTEST__
int main(int argc, char *argv[]) {
  TSPARSEMATRIX m;
  initMatrix(&m);
  addSetCell(&m, 0, 1, 10);
  addSetCell(&m, 1, 0, 20);
  addSetCell(&m, 1, 5, 30);
  addSetCell(&m, 2, 1, 40);
  assert(m.m_Rows && m.m_Rows->m_Idx == 0 && m.m_Rows->m_Cells &&
         m.m_Rows->m_Cells->m_Row == 0 && m.m_Rows->m_Cells->m_Col == 1 &&
         m.m_Rows->m_Cells->m_Data == 10 &&
         m.m_Rows->m_Cells->m_Right == nullptr);
  assert(m.m_Rows->m_Next && m.m_Rows->m_Next->m_Idx == 1 &&
         m.m_Rows->m_Next->m_Cells && m.m_Rows->m_Next->m_Cells->m_Row == 1 &&
         m.m_Rows->m_Next->m_Cells->m_Col == 0 &&
         m.m_Rows->m_Next->m_Cells->m_Data == 20 &&
         m.m_Rows->m_Next->m_Cells->m_Right &&
         m.m_Rows->m_Next->m_Cells->m_Right->m_Row == 1 &&
         m.m_Rows->m_Next->m_Cells->m_Right->m_Col == 5 &&
         m.m_Rows->m_Next->m_Cells->m_Right->m_Data == 30 &&
         m.m_Rows->m_Next->m_Cells->m_Right->m_Right == nullptr);
  assert(m.m_Rows->m_Next->m_Next && m.m_Rows->m_Next->m_Next->m_Idx == 2 &&
         m.m_Rows->m_Next->m_Next->m_Cells &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Row == 2 &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Col == 1 &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Data == 40 &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Right == nullptr);
  assert(m.m_Rows->m_Next->m_Next->m_Next == nullptr);
  assert(m.m_Cols && m.m_Cols->m_Idx == 0 && m.m_Cols->m_Cells &&
         m.m_Cols->m_Cells->m_Row == 1 && m.m_Cols->m_Cells->m_Col == 0 &&
         m.m_Cols->m_Cells->m_Data == 20 &&
         m.m_Cols->m_Cells->m_Down == nullptr);
  assert(m.m_Cols->m_Next && m.m_Cols->m_Next->m_Idx == 1 &&
         m.m_Cols->m_Next->m_Cells && m.m_Cols->m_Next->m_Cells->m_Row == 0 &&
         m.m_Cols->m_Next->m_Cells->m_Col == 1 &&
         m.m_Cols->m_Next->m_Cells->m_Data == 10 &&
         m.m_Cols->m_Next->m_Cells->m_Down &&
         m.m_Cols->m_Next->m_Cells->m_Down->m_Row == 2 &&
         m.m_Cols->m_Next->m_Cells->m_Down->m_Col == 1 &&
         m.m_Cols->m_Next->m_Cells->m_Down->m_Data == 40 &&
         m.m_Cols->m_Next->m_Cells->m_Down->m_Down == nullptr);
  assert(m.m_Cols->m_Next->m_Next && m.m_Cols->m_Next->m_Next->m_Idx == 5 &&
         m.m_Cols->m_Next->m_Next->m_Cells &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Row == 1 &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Col == 5 &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Data == 30 &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Down == nullptr);
  assert(m.m_Cols->m_Next->m_Next->m_Next == nullptr);
  assert(m.m_Rows->m_Cells == m.m_Cols->m_Next->m_Cells);
  assert(m.m_Rows->m_Next->m_Cells == m.m_Cols->m_Cells);
  assert(m.m_Rows->m_Next->m_Cells->m_Right ==
         m.m_Cols->m_Next->m_Next->m_Cells);
  assert(m.m_Rows->m_Next->m_Next->m_Cells ==
         m.m_Cols->m_Next->m_Cells->m_Down);
  addSetCell(&m, 230, 190, 50);
  assert(m.m_Rows && m.m_Rows->m_Idx == 0 && m.m_Rows->m_Cells &&
         m.m_Rows->m_Cells->m_Row == 0 && m.m_Rows->m_Cells->m_Col == 1 &&
         m.m_Rows->m_Cells->m_Data == 10 &&
         m.m_Rows->m_Cells->m_Right == nullptr);
  assert(m.m_Rows->m_Next && m.m_Rows->m_Next->m_Idx == 1 &&
         m.m_Rows->m_Next->m_Cells && m.m_Rows->m_Next->m_Cells->m_Row == 1 &&
         m.m_Rows->m_Next->m_Cells->m_Col == 0 &&
         m.m_Rows->m_Next->m_Cells->m_Data == 20 &&
         m.m_Rows->m_Next->m_Cells->m_Right &&
         m.m_Rows->m_Next->m_Cells->m_Right->m_Row == 1 &&
         m.m_Rows->m_Next->m_Cells->m_Right->m_Col == 5 &&
         m.m_Rows->m_Next->m_Cells->m_Right->m_Data == 30 &&
         m.m_Rows->m_Next->m_Cells->m_Right->m_Right == nullptr);
  assert(m.m_Rows->m_Next->m_Next && m.m_Rows->m_Next->m_Next->m_Idx == 2 &&
         m.m_Rows->m_Next->m_Next->m_Cells &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Row == 2 &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Col == 1 &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Data == 40 &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Right == nullptr);
  assert(m.m_Rows->m_Next->m_Next->m_Next &&
         m.m_Rows->m_Next->m_Next->m_Next->m_Idx == 230 &&
         m.m_Rows->m_Next->m_Next->m_Next->m_Cells &&
         m.m_Rows->m_Next->m_Next->m_Next->m_Cells->m_Row == 230 &&
         m.m_Rows->m_Next->m_Next->m_Next->m_Cells->m_Col == 190 &&
         m.m_Rows->m_Next->m_Next->m_Next->m_Cells->m_Data == 50 &&
         m.m_Rows->m_Next->m_Next->m_Next->m_Cells->m_Right == nullptr);
  assert(m.m_Rows->m_Next->m_Next->m_Next->m_Next == nullptr);
  assert(m.m_Cols && m.m_Cols->m_Idx == 0 && m.m_Cols->m_Cells &&
         m.m_Cols->m_Cells->m_Row == 1 && m.m_Cols->m_Cells->m_Col == 0 &&
         m.m_Cols->m_Cells->m_Data == 20 &&
         m.m_Cols->m_Cells->m_Down == nullptr);
  assert(m.m_Cols->m_Next && m.m_Cols->m_Next->m_Idx == 1 &&
         m.m_Cols->m_Next->m_Cells && m.m_Cols->m_Next->m_Cells->m_Row == 0 &&
         m.m_Cols->m_Next->m_Cells->m_Col == 1 &&
         m.m_Cols->m_Next->m_Cells->m_Data == 10 &&
         m.m_Cols->m_Next->m_Cells->m_Down &&
         m.m_Cols->m_Next->m_Cells->m_Down->m_Row == 2 &&
         m.m_Cols->m_Next->m_Cells->m_Down->m_Col == 1 &&
         m.m_Cols->m_Next->m_Cells->m_Down->m_Data == 40 &&
         m.m_Cols->m_Next->m_Cells->m_Down->m_Down == nullptr);
  assert(m.m_Cols->m_Next->m_Next && m.m_Cols->m_Next->m_Next->m_Idx == 5 &&
         m.m_Cols->m_Next->m_Next->m_Cells &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Row == 1 &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Col == 5 &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Data == 30 &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Down == nullptr);
  assert(m.m_Cols->m_Next->m_Next->m_Next &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Idx == 190 &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Row == 230 &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Col == 190 &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Data == 50 &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Down == nullptr);
  assert(m.m_Cols->m_Next->m_Next->m_Next->m_Next == nullptr);
  assert(m.m_Rows->m_Cells == m.m_Cols->m_Next->m_Cells);
  assert(m.m_Rows->m_Next->m_Cells == m.m_Cols->m_Cells);
  assert(m.m_Rows->m_Next->m_Cells->m_Right ==
         m.m_Cols->m_Next->m_Next->m_Cells);
  assert(m.m_Rows->m_Next->m_Next->m_Cells ==
         m.m_Cols->m_Next->m_Cells->m_Down);
  assert(m.m_Rows->m_Next->m_Next->m_Next->m_Cells ==
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells);
  assert(removeCell(&m, 0, 1));
  assert(!removeCell(&m, 0, 1));
  assert(!removeCell(&m, 1, 2));
  assert(m.m_Rows && m.m_Rows->m_Idx == 1 && m.m_Rows->m_Cells &&
         m.m_Rows->m_Cells->m_Row == 1 && m.m_Rows->m_Cells->m_Col == 0 &&
         m.m_Rows->m_Cells->m_Data == 20 && m.m_Rows->m_Cells->m_Right &&
         m.m_Rows->m_Cells->m_Right->m_Row == 1 &&
         m.m_Rows->m_Cells->m_Right->m_Col == 5 &&
         m.m_Rows->m_Cells->m_Right->m_Data == 30 &&
         m.m_Rows->m_Cells->m_Right->m_Right == nullptr);
  assert(m.m_Rows->m_Next && m.m_Rows->m_Next->m_Idx == 2 &&
         m.m_Rows->m_Next->m_Cells && m.m_Rows->m_Next->m_Cells->m_Row == 2 &&
         m.m_Rows->m_Next->m_Cells->m_Col == 1 &&
         m.m_Rows->m_Next->m_Cells->m_Data == 40 &&
         m.m_Rows->m_Next->m_Cells->m_Right == nullptr);
  assert(m.m_Rows->m_Next->m_Next && m.m_Rows->m_Next->m_Next->m_Idx == 230 &&
         m.m_Rows->m_Next->m_Next->m_Cells &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Row == 230 &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Col == 190 &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Data == 50 &&
         m.m_Rows->m_Next->m_Next->m_Cells->m_Right == nullptr);
  assert(m.m_Rows->m_Next->m_Next->m_Next == nullptr);
  assert(m.m_Cols && m.m_Cols->m_Idx == 0 && m.m_Cols->m_Cells &&
         m.m_Cols->m_Cells->m_Row == 1 && m.m_Cols->m_Cells->m_Col == 0 &&
         m.m_Cols->m_Cells->m_Data == 20 &&
         m.m_Cols->m_Cells->m_Down == nullptr);
  assert(m.m_Cols->m_Next && m.m_Cols->m_Next->m_Idx == 1 &&
         m.m_Cols->m_Next->m_Cells && m.m_Cols->m_Next->m_Cells->m_Row == 2 &&
         m.m_Cols->m_Next->m_Cells->m_Col == 1 &&
         m.m_Cols->m_Next->m_Cells->m_Data == 40 &&
         m.m_Cols->m_Next->m_Cells->m_Down == nullptr);
  assert(m.m_Cols->m_Next->m_Next && m.m_Cols->m_Next->m_Next->m_Idx == 5 &&
         m.m_Cols->m_Next->m_Next->m_Cells &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Row == 1 &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Col == 5 &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Data == 30 &&
         m.m_Cols->m_Next->m_Next->m_Cells->m_Down == nullptr);
  assert(m.m_Cols->m_Next->m_Next->m_Next &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Idx == 190 &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Row == 230 &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Col == 190 &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Data == 50 &&
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells->m_Down == nullptr);
  assert(m.m_Cols->m_Next->m_Next->m_Next->m_Next == nullptr);
  assert(m.m_Rows->m_Cells == m.m_Cols->m_Cells);
  assert(m.m_Rows->m_Cells->m_Right == m.m_Cols->m_Next->m_Next->m_Cells);
  assert(m.m_Rows->m_Next->m_Cells == m.m_Cols->m_Next->m_Cells);
  assert(m.m_Rows->m_Next->m_Next->m_Cells ==
         m.m_Cols->m_Next->m_Next->m_Next->m_Cells);
  freeMatrix(&m);
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
