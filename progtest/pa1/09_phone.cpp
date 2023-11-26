#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <stdio.h>

#ifndef __PROGTEST__
#define DEBUG(fmt) fprintf(stderr, "%s:%d " fmt, __FILE__, __LINE__);
#define DEBUGF(fmt, ...)                                                       \
  fprintf(stderr, "%s:%d " fmt, __FILE__, __LINE__, __VA_ARGS__);
#else
#define DEBUG(fmt)
#define DEBUGF(fmt, ...)
#endif

typedef struct {
  void *allocation;
  int size;
  int capacity;
} ArrayList;

// note size is not updated
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

// index is not a byte offset
void list_insert(ArrayList *list, int index, void *element, int size) {
  int new_size = list->size + size;
  list_reserve(list, new_size);
  int start_offset = index * size;
  char *start = (char *)list->allocation + start_offset;
  memmove(start + size, start, list->size - start_offset);
  memcpy(start, element, size);
  list->size = new_size;
}

void list_reset(ArrayList *list) { list->size = 0; }
void list_free(ArrayList *list) { free(list->allocation); }

typedef struct {
  char c;
  ArrayList children;
  int leaf_data;
} TrieNode;

void node_free(TrieNode *node) {
  int children_count = node->children.size / sizeof(TrieNode);
  TrieNode *children = (TrieNode *)node->children.allocation;
  for (int j = 0; j < children_count; j++) {
    node_free(children + j);
  }
  list_free(&node->children);
}

TrieNode *find_child(TrieNode *node, char key) {
  int children_count = node->children.size / sizeof(TrieNode);
  TrieNode *children = (TrieNode *)node->children.allocation;

  for (int j = 0; j < children_count; j++) {
    TrieNode *child = children + j;
    if (child->c == key) {
      return child;
    }
  }
  return NULL;
}

TrieNode *node_add_child(TrieNode *node) {
  int offset = node->children.size / sizeof(TrieNode);
  TrieNode empty = {};
  empty.leaf_data = -1;
  list_push(&node->children, &empty, sizeof(TrieNode));
  return ((TrieNode *)node->children.allocation) + offset;
}

TrieNode *find_or_insert_child(TrieNode *node, char c) {
  if (node->c == 0) {
    node->c = c;
    return node;
  }

  TrieNode *child = find_child(node, c);
  if (child) {
    return child;
  } else {
    TrieNode *added = node_add_child(node);
    added->c = c;
    return added;
  }
}

TrieNode *node_insert(TrieNode *node, const char *key) {
  assert(*key != 0);
  TrieNode *current = node;
  for (int i = 0;; i++) {
    char c = key[i];
    if (c == 0) {
      break;
    }
    current = find_or_insert_child(current, c);
  }
  return current;
}

TrieNode *node_find(TrieNode *node, const char *key, bool *found) {
  assert(*key != 0);
  *found = false;

  TrieNode *current = node;
  for (int i = 0;; i++) {
    char c = key[i];
    if (c == 0) {
      break;
    }
    TrieNode *child = find_child(current, c);
    if (child) {
      current = child;
    } else {
      return current;
    }
  }

  *found = true;
  return current;
}

const char *get_T9(char c) {
  static const char *T9[9] = {" ",   "ABC",  "DEF", "GHI", "JKL",
                              "MNO", "PQRS", "TUV", "WXYZ"};
  if (c < '1' || '9' < c) {
    return NULL;
  }
  return T9[c - '1'];
}

void visit_children(TrieNode *node, void (*callback)(TrieNode *, void *),
                    void *callback_data) {
  callback(node, callback_data);

  int children_count = node->children.size / sizeof(TrieNode);
  TrieNode *children = (TrieNode *)node->children.allocation;

  for (int i = 0; i < children_count; i++) {
    TrieNode *child = children + i;
    visit_children(child, callback, callback_data);
  }
}

void visit_children_t9(TrieNode *node, const char *key, int level,
                       void (*callback)(TrieNode *, void *),
                       void *callback_data) {
  callback(node, callback_data);

  char c = key[level];
  if (c == 0) {
    return visit_children(node, callback, callback_data);
  }

  const char *decoded = get_T9(c);
  if (!decoded) {
    return;
  }

  for (int i = 0; decoded[i] != 0; i++) {
    TrieNode *child = find_child(node, decoded[i]);
    if (child) {
      visit_children_t9(child, key, level + 1, callback, callback_data);
    }
  }
}

typedef struct {
  char *number;
  char *name;
} Contact;

void exists() { printf("Kontakt jiz existuje.\n"); }

void bad() { printf("Nespravny vstup.\n"); }

void add_number(char *line, ArrayList *contacts, TrieNode *number_trie,
                TrieNode *name_trie) {
  // + 123456 Vagner Ladislav
  if (*(line++) != '+')
    return bad();
  if (*(line++) != ' ')
    return bad();

  char *number_start = line;
  while (true) {
    char c = *(line++);
    if ('0' <= c && c <= '9') {
    } else if (c == ' ') {
      break;
    } else {
      DEBUGF("Unexpected character when reading number '%c'\n", c);
      return bad();
    }
  }
  ptrdiff_t number_size = (ptrdiff_t)(line - number_start);

  char *name_start = line;
  while (true) {
    char c = *(line++);
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == ' ') {
    } else if (c == '\n') {
      break;
    } else {
      DEBUGF("Unexpected character when reading number '%c'\n", c);
      return bad();
    }
  }
  ptrdiff_t name_size = (ptrdiff_t)(line - name_start);

  char *number = (char *)malloc(number_size);
  memcpy(number, number_start, number_size - 1);
  number[number_size - 1] = 0;

  char *name = (char *)malloc(name_size);
  memcpy(name, name_start, name_size - 1);
  name[name_size - 1] = 0;

  Contact contact = {number, name};
  int new_contact = contacts->size / sizeof(Contact);
  list_push(contacts, &contact, sizeof(Contact));

  TrieNode *node = NULL;
  // Contact *contacts_ptr = (Contact *)contacts->allocation;
  node = node_insert(number_trie, number);
  node->leaf_data = new_contact;
  node = node_insert(name_trie, name);
  node->leaf_data = new_contact;

  printf("OK\n");
}

void add_contact(TrieNode *node, void *data) {
  ArrayList *list = (ArrayList *)data;
  if (node->c != 0 && node->leaf_data != -1) {
    list_push(list, &node->leaf_data, sizeof(int));
  }
}

int compare_ints(const void *a, const void *b) {
  int arg1 = *(const int *)a;
  int arg2 = *(const int *)b;

  if (arg1 < arg2)
    return -1;
  if (arg1 > arg2)
    return 1;
  return 0;
}

int int_dedup(int *array, int size) {
  if (size < 1) {
    return size;
  }

  int dst = 1;
  int prev = array[0];
  for (int src = 1; src < size; src++) {
    if (array[src] == prev) {
      continue;
    }
    prev = array[src];
    array[dst] = array[src];
    dst++;
  }

  return dst;
}

void do_query(char *line, ArrayList *contacts, ArrayList *scratch,
              TrieNode *number_trie, TrieNode *name_trie) {
  // ? 1234567
  if (*(line++) != '?')
    return bad();
  if (*(line++) != ' ')
    return bad();

  const char *number_start = line;
  while (true) {
    char c = *(line++);
    if ('0' <= c && c <= '9') {
    } else if (c == '\n') {
      break;
    } else {
      DEBUGF("Unexpected character when reading number '%c'\n", c);
      return bad();
    }
  }
  *(line - 1) = 0;

  bool bah = false;
  TrieNode *found = node_find(number_trie, number_start, &bah);

  list_reset(scratch);
  visit_children(found, add_contact, scratch);
  visit_children_t9(name_trie, number_start, 0, add_contact, scratch);

  qsort(scratch->allocation, scratch->size / sizeof(int), sizeof(int),
        compare_ints);
  int scratch_size =
      int_dedup((int *)scratch->allocation, scratch->size / sizeof(int));

  for (int i = 0; i < scratch_size; i++) {
    int index = ((int *)scratch->allocation)[i];
    Contact *contact = ((Contact *)contacts->allocation) + index;
    printf("%s %s\n", contact->number, contact->name);
  }

  printf("Celkem: %d\n", scratch_size);
}

void dispatch(char **line, size_t *line_len, ArrayList *contacts,
              ArrayList *scratch, TrieNode *number_trie, TrieNode *name_trie) {
  while (true) {
    if (getline(line, line_len, stdin) <= 0) {
      return;
    }
    switch (**line) {
    case '+':
      add_number(*line, contacts, number_trie, name_trie);
      break;
    case '?':
      do_query(*line, contacts, scratch, number_trie, name_trie);
      break;
    case '\0':
      DEBUG("EOF\n");
      return;
    default:
      DEBUGF("unexpected character '%c'\n", **line);
      return;
    }
  }
}

int main() {
  ArrayList contacts = {};
  ArrayList scratch = {};
  TrieNode number_trie = {};
  TrieNode name_trie = {};

  char *line = NULL;
  size_t line_len = 0;

  dispatch(&line, &line_len, &contacts, &scratch, &number_trie, &name_trie);

  int contacts_size = contacts.size / sizeof(Contact);
  Contact *contacts_ptr = (Contact *)contacts.allocation;
  for (int i = 0; i < contacts_size; i++) {
    free(contacts_ptr[i].number);
    free(contacts_ptr[i].name);
  }

  free(line);
  list_free(&contacts);
  list_free(&scratch);
  node_free(&number_trie);
  node_free(&name_trie);
  return 0;
}
