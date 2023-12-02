#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <math.h>
#include <stdio.h>

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

void list_pop(ArrayList *list, void *element, int size) {
  int new_size = list->size - size;
  assert(new_size >= 0);
  if (element) {
    memcpy(element, (char *)list->allocation + new_size, size);
  }
  list->size = new_size;
}

// decrements list size by removing the last element but returns a pointer to
// the popped memory, you must not call list_push while holding this pointer
void *unsafe_list_pop(ArrayList *list, int size) {
  int new_size = list->size - size;
  assert(new_size >= 0);
  list->size = new_size;
  return (char *)list->allocation + new_size;
}

void *list_peek(ArrayList *list, int size) {
  int offset = list->size - size;
  if (offset < 0) {
    return NULL;
  }
  return (char *)list->allocation + offset;
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

// a bitset of characters, stores [a-z0-9 ]
typedef uint64_t CharBitSet;

static unsigned char CHAR_BIT_SET_LUT[256] = {};
static unsigned char T9_LUT[256] = {};

void populate_luts() {
  unsigned char counter = 0;
  for (char i = 'a'; i <= 'z'; i++) {
    CHAR_BIT_SET_LUT[(int)i] = counter++;
  }
  for (char i = '0'; i <= '9'; i++) {
    CHAR_BIT_SET_LUT[(int)i] = counter++;
  }
  CHAR_BIT_SET_LUT[(int)' '] = counter++;

  const char *T9[10] = {"",    " ",   "abc",  "def", "ghi",
                        "jkl", "mno", "pqrs", "tuv", "wxyz"};
  for (int i = 0; i < 10; i++) {
    const char *str = T9[i];
    for (int j = 0; str[j] != 0; j++) {
      char jc = str[j];
      T9_LUT[(int)jc] = '0' + i;
      T9_LUT['A' + (jc - 'a')] = '0' + i;
    }
  }
}

bool bitset_contains(CharBitSet set, char c) {
  unsigned char bit_index = CHAR_BIT_SET_LUT[(int)c];
  return ((set >> (CharBitSet)bit_index) & 1) == 1;
}

CharBitSet bitset_set(CharBitSet set, char c) {
  unsigned char bit_index = CHAR_BIT_SET_LUT[(int)c];
  return set | ((CharBitSet)1 << bit_index);
}

typedef struct {
  char c;
  CharBitSet set;
  ArrayList children;
  ArrayList leaf_data;
} TrieNode;

void node_free(TrieNode *node) {
  ArrayList stack = {};
  list_push(&stack, &node, sizeof(TrieNode *));

  while (stack.size > 0) {
    TrieNode *last = *((TrieNode **)list_peek(&stack, sizeof(TrieNode *)));
    if (last->children.size == 0) {
      list_free(&last->children);
      list_free(&last->leaf_data);
      list_pop(&stack, NULL, sizeof(TrieNode *));
    } else {
      TrieNode *pop =
          (TrieNode *)unsafe_list_pop(&last->children, sizeof(TrieNode));
      list_push(&stack, &pop, sizeof(TrieNode *));
    }
  }
  list_free(&stack);
}

// binary search an index to insert an element so that the array remains sorted
// stolen from the rust standard library
// https://doc.rust-lang.org/src/core/slice/mod.rs.html#2771-2773
int get_insert_index(TrieNode *array, int size, bool *found, char x) {
  int left = 0;
  int right = size;
  while (left < right) {
    int mid = left + size / 2;
    char c = array[mid].c;

    if (c < x) {
      left = mid + 1;
    } else if (c > x) {
      right = mid;
    } else {
      *found = true;
      return mid;
    }

    size = right - left;
  }
  *found = false;
  return left;
}

TrieNode *find_child(TrieNode *node, char key) {
  assert(key != 0);
  if (!bitset_contains(node->set, key)) {
    return NULL;
  }

  int children_count = node->children.size / sizeof(TrieNode);
  TrieNode *children = (TrieNode *)node->children.allocation;

  bool found = false;
  int index = get_insert_index(children, children_count, &found, key);
  assert(found);
  assert(index < children_count);

  return children + index;
}

TrieNode *find_or_insert_child(TrieNode *node, char c) {
  int children_count = node->children.size / sizeof(TrieNode);
  TrieNode *children = (TrieNode *)node->children.allocation;

  bool found = false;
  int index = get_insert_index(children, children_count, &found, c);

  if (!found) {
    TrieNode empty = {};
    empty.c = c;
    list_insert(&node->children, index, &empty, sizeof(TrieNode));

    node->set = bitset_set(node->set, c);
    children = (TrieNode *)node->children.allocation;
    children_count = node->children.size / sizeof(TrieNode);
  }
  assert(index < children_count);
  return children + index;
}

TrieNode *node_insert(TrieNode *node, const char *key) {
  assert(*key != 0);
  TrieNode *current = node;
  for (int i = 0; key[i] != 0; i++) {
    char c = key[i];
    current = find_or_insert_child(current, c);
  }
  return current;
}

TrieNode *node_find(TrieNode *node, const char *key) {
  assert(*key != 0);
  TrieNode *current = node;
  for (int i = 0;; i++) {
    char c = key[i];
    if (c == 0) {
      return current;
    }
    TrieNode *child = find_child(current, c);
    if (child) {
      current = child;
    } else {
      return NULL;
    }
  }

  return NULL;
}

void encode_t9(char *string) {
  for (int i = 0; string[i] != 0; i++) {
    string[i] = T9_LUT[(int)string[i]];
  }
}

void collect_leaf_data(TrieNode *node, ArrayList *collected) {
  if (node->c != 0 && node->leaf_data.size > 0) {
    list_push(collected, node->leaf_data.allocation, node->leaf_data.size);
  }
}

void collect_children(TrieNode *node, ArrayList *stack, ArrayList *collected) {
  int start_size = stack->size;
  list_push(stack, &node, sizeof(TrieNode *));

  while (stack->size > start_size) {
    TrieNode *pop;
    list_pop(stack, &pop, sizeof(TrieNode *));
    while (true) {
      collect_leaf_data(pop, collected);

      int children_count = pop->children.size / sizeof(TrieNode);
      TrieNode *children = (TrieNode *)pop->children.allocation;

      // avoid pushing and popping a single element
      if (children_count == 1) {
        pop = children;
        continue;
      }

      for (int i = 0; i < children_count; i++) {
        TrieNode *child = children + i;
        list_push(stack, &child, sizeof(TrieNode *));
      }
      break;
    }
  }
}

typedef struct {
  char *number;
  char *name;
} Contact;

void exists() { printf("Kontakt jiz existuje.\n"); }
void bad() { printf("Nespravny vstup.\n"); }

bool leaf_add_data(TrieNode *node, ArrayList *contacts, const char *number,
                   const char *name, int new_contact) {
  int leaf_contacts_size = node->leaf_data.size / sizeof(int);
  int *leaf_contacts = (int *)node->leaf_data.allocation;
  Contact *contacts_ptr = (Contact *)contacts->allocation;
  for (int i = 0; i < leaf_contacts_size; i++) {
    Contact *contact = contacts_ptr + leaf_contacts[i];
    if (strcmp(contact->number, number) == 0 &&
        strcmp(contact->name, name) == 0) {
      return true;
    }
  }

  list_push(&node->leaf_data, &new_contact, sizeof(int));
  return false;
}

void add_number(char *line, ArrayList *contacts, TrieNode *number_trie,
                TrieNode *t9_name_trie) {
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
  if ((number_size - 1) < 1 || (number_size - 1) > 20) {
    return bad();
  }

  if (*line == ' ') {
    return bad();
  }

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
  *(line - 1) = 0;

  ptrdiff_t name_size = (ptrdiff_t)(line - name_start);
  if ((name_size - 1) < 1) {
    return bad();
  }

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
  node = node_insert(number_trie, number);
  if (leaf_add_data(node, contacts, number, name, new_contact))
    return exists();

  encode_t9(name_start);
  node = node_insert(t9_name_trie, name_start);
  if (leaf_add_data(node, contacts, number, name, new_contact))
    return exists();

  printf("OK\n");
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

void do_query(char *line, ArrayList *contacts, ArrayList *stack,
              ArrayList *collected, TrieNode *number_trie,
              TrieNode *t9_name_trie) {
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

  ptrdiff_t number_size = line - number_start - 1;
  if (number_size < 1) {
    return bad();
  }

  list_reset(stack);
  list_reset(collected);

  TrieNode *found = {};
  found = node_find(number_trie, number_start);
  if (found) {
    collect_children(found, stack, collected);
  }

  found = node_find(t9_name_trie, number_start);
  if (found) {
    collect_children(found, stack, collected);
  }

  qsort(collected->allocation, collected->size / sizeof(int), sizeof(int),
        compare_ints);
  int collected_size =
      int_dedup((int *)collected->allocation, collected->size / sizeof(int));

  if (collected_size <= 10) {
    for (int i = 0; i < collected_size; i++) {
      int index = ((int *)collected->allocation)[i];
      Contact *contact = ((Contact *)contacts->allocation) + index;
      printf("%s %s\n", contact->number, contact->name);
    }
  }

  printf("Celkem: %d\n", collected_size);
}

int main() {
  populate_luts();

  ArrayList contacts = {};
  ArrayList stack = {};
  ArrayList collected = {};
  TrieNode number_trie = {};
  TrieNode t9_name_trie = {};

  char *line = NULL;
  size_t line_len = 0;

  while (getline(&line, &line_len, stdin) > 0) {
    switch (*line) {
    case '+':
      add_number(line, &contacts, &number_trie, &t9_name_trie);
      break;
    case '?':
      do_query(line, &contacts, &stack, &collected, &number_trie,
               &t9_name_trie);
      break;
    case '\0':
      DEBUG("EOF\n");
      break;
    default:
      bad();
      DEBUGF("unexpected character '%c'\n", *line);
      break;
    }
  }

  int contacts_size = contacts.size / sizeof(Contact);
  Contact *contacts_ptr = (Contact *)contacts.allocation;
  for (int i = 0; i < contacts_size; i++) {
    free(contacts_ptr[i].number);
    free(contacts_ptr[i].name);
  }

  free(line);
  list_free(&contacts);
  list_free(&stack);
  list_free(&collected);
  node_free(&number_trie);
  node_free(&t9_name_trie);
  return 0;
}
