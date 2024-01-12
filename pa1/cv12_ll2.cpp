#include <climits>
#ifndef __PROGTEST__
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct TItem {
  struct TItem *m_Next;
  struct TItem *m_Prev;
  int m_Val;
} TITEM;

typedef struct TData {
  TITEM *m_First;
  TITEM *m_Last;
} TDATA;

#endif /* __PROGTEST__ */

TITEM *new_node(int x, TITEM *prev, TITEM *next) {
  TITEM *node = (TITEM *)malloc(sizeof(TITEM));
  node->m_Next = next;
  node->m_Prev = prev;
  node->m_Val = x;
  return node;
}

void insertStart(TDATA *l, int x) {
  TITEM *node = new_node(x, NULL, l->m_First);
  if (l->m_First) {
    l->m_First->m_Prev = node;
  }
  l->m_First = node;
  if (!l->m_Last) {
    l->m_Last = node;
  }
}
void insertEnd(TDATA *l, int x) {
  TITEM *node = new_node(x, l->m_Last, NULL);
  if (l->m_Last) {
    l->m_Last->m_Next = node;
  }
  if (!l->m_First) {
    l->m_First = node;
  }
  l->m_Last = node;
}
int removeMax(TDATA *l) { /* TODO */
  TITEM *curr = l->m_First;
  int max = INT_MIN;
  while (curr) {
    if (curr->m_Val > max) {
      max = curr->m_Val;
    }
    curr = curr->m_Next;
  }

  int count = 0;
  TITEM *prev = NULL;
  curr = l->m_First;
  bool set_first = false;
  while (curr) {
    TITEM *next = curr->m_Next;
    if (curr->m_Val == max) {
      count++;
      if (prev) {
        prev->m_Next = next;
      } else {
        l->m_First = next;
      }
      free(curr);
      if (next) {
        next->m_Prev = prev;
      }
    } else {
      prev = curr;
    }
    if (!set_first && prev != NULL) {
      l->m_First = prev;
      set_first = true;
    }
    if (next == NULL) {
      l->m_Last = prev;
    }
    curr = next;
  }

  return count;
}
void destroyAll(TDATA *l) {
  TITEM *curr = l->m_First;
  while (curr) {
    TITEM *copy = curr;
    curr = curr->m_Next;
    free(copy);
  }
}

#ifndef __PROGTEST__
int main(void) {
  TDATA a;
  a.m_First = a.m_Last = NULL;
  insertEnd(&a, 1);
  insertEnd(&a, 2);
  insertEnd(&a, 1);
  insertEnd(&a, 3);
  insertEnd(&a, 4);
  insertEnd(&a, 2);
  insertEnd(&a, 5);
  assert(a.m_First != NULL);
  assert(a.m_First->m_Val == 1);
  assert(a.m_First->m_Next != NULL);
  assert(a.m_First->m_Prev == NULL);
  assert(a.m_First->m_Next->m_Val == 2);
  assert(a.m_First->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Prev == a.m_First);
  assert(a.m_First->m_Next->m_Next->m_Val == 1);
  assert(a.m_First->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Prev == a.m_First->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Val == 3);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Val == 4);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Val == 2);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Val == 5);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next ==
         NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next);
  assert(a.m_Last == a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next);
  assert(removeMax(&a) == 1);
  assert(a.m_First != NULL);
  assert(a.m_First->m_Val == 1);
  assert(a.m_First->m_Next != NULL);
  assert(a.m_First->m_Prev == NULL);
  assert(a.m_First->m_Next->m_Val == 2);
  assert(a.m_First->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Prev == a.m_First);
  assert(a.m_First->m_Next->m_Next->m_Val == 1);
  assert(a.m_First->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Prev == a.m_First->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Val == 3);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Val == 4);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Val == 2);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next == NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next->m_Next);
  assert(a.m_Last == a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next);
  destroyAll(&a);

  a.m_First = a.m_Last = NULL;
  insertEnd(&a, 1);
  insertEnd(&a, 2);
  insertEnd(&a, 3);
  insertEnd(&a, 0);
  insertEnd(&a, 2);
  insertEnd(&a, 3);
  insertStart(&a, 1);
  insertStart(&a, 2);
  insertStart(&a, 3);
  assert(a.m_First != NULL);
  assert(a.m_First->m_Val == 3);
  assert(a.m_First->m_Next != NULL);
  assert(a.m_First->m_Prev == NULL);
  assert(a.m_First->m_Next->m_Val == 2);
  assert(a.m_First->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Prev == a.m_First);
  assert(a.m_First->m_Next->m_Next->m_Val == 1);
  assert(a.m_First->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Prev == a.m_First->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Val == 1);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Val == 2);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Val == 3);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Val == 0);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next !=
         NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next
             ->m_Val == 2);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next
             ->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next
             ->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next
             ->m_Next->m_Val == 3);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next
             ->m_Next->m_Next == NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next
             ->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next);
  assert(a.m_Last == a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next
                         ->m_Next->m_Next);
  assert(removeMax(&a) == 3);
  assert(a.m_First != NULL);
  assert(a.m_First->m_Val == 2);
  assert(a.m_First->m_Next != NULL);
  assert(a.m_First->m_Prev == NULL);
  assert(a.m_First->m_Next->m_Val == 1);
  assert(a.m_First->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Prev == a.m_First);
  assert(a.m_First->m_Next->m_Next->m_Val == 1);
  assert(a.m_First->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Prev == a.m_First->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Val == 2);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Val == 0);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Val == 2);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Next == NULL);
  assert(a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next->m_Prev ==
         a.m_First->m_Next->m_Next->m_Next->m_Next);
  assert(a.m_Last == a.m_First->m_Next->m_Next->m_Next->m_Next->m_Next);
  assert(removeMax(&a) == 3);
  assert(a.m_First != NULL);
  assert(a.m_First->m_Val == 1);
  assert(a.m_First->m_Next != NULL);
  assert(a.m_First->m_Prev == NULL);
  assert(a.m_First->m_Next->m_Val == 1);
  assert(a.m_First->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Prev == a.m_First);
  assert(a.m_First->m_Next->m_Next->m_Val == 0);
  assert(a.m_First->m_Next->m_Next->m_Next == NULL);
  assert(a.m_First->m_Next->m_Next->m_Prev == a.m_First->m_Next);
  assert(a.m_Last == a.m_First->m_Next->m_Next);
  assert(removeMax(&a) == 2);
  assert(a.m_First != NULL);
  assert(a.m_First->m_Val == 0);
  assert(a.m_First->m_Next == NULL);
  assert(a.m_First->m_Prev == NULL);
  assert(a.m_Last == a.m_First);
  destroyAll(&a);

  a.m_First = a.m_Last = NULL;
  insertEnd(&a, -1);
  insertEnd(&a, -2);
  insertEnd(&a, -10000);
  insertEnd(&a, -1);
  insertEnd(&a, -300);
  assert(removeMax(&a) == 2);
  assert(a.m_First != NULL);
  assert(a.m_First->m_Val == -2);
  assert(a.m_First->m_Next != NULL);
  assert(a.m_First->m_Prev == NULL);
  assert(a.m_First->m_Next->m_Val == -10000);
  assert(a.m_First->m_Next->m_Next != NULL);
  assert(a.m_First->m_Next->m_Prev == a.m_First);
  assert(a.m_First->m_Next->m_Next->m_Val == -300);
  assert(a.m_First->m_Next->m_Next->m_Next == NULL);
  assert(a.m_First->m_Next->m_Next->m_Prev == a.m_First->m_Next);
  assert(a.m_Last == a.m_First->m_Next->m_Next);
  destroyAll(&a);

  return 0;
}
#endif /* __PROGTEST__ */
