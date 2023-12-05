#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
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

static unsigned char ALPHABET_LUT[256] = {};
static unsigned char T9_LUT[256] = {};

void populate_luts() {
  // ascii to reduced alphabet
  unsigned char counter = 0;
  for (char i = 'a'; i <= 'z'; i++) {
    ALPHABET_LUT[(int)i] = counter;
    ALPHABET_LUT['A' + (i - 'a')] = counter++;
  }
  ALPHABET_LUT[(int)' '] = counter++;

  for (char i = '0'; i <= '9'; i++) {
    ALPHABET_LUT[(int)i] = counter++;
  }
  // ascii to t9
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

constexpr int ALPHABET_SIZE = 26 + 1;

typedef int NodeHandle;
constexpr NodeHandle TRIE_ROOT = 0;
constexpr NodeHandle TRIE_NULL = -1;

typedef struct {
  char c;
  ArrayList leaf_data;
  NodeHandle alphabet[ALPHABET_SIZE];
} TrieNode;

typedef struct {
  ArrayList nodes;
} Trie;

void node_init(TrieNode *node, char c) {
  node->c = c;
  node->leaf_data = {};
  for (int i = 0; i < ALPHABET_SIZE; i++) {
    node->alphabet[i] = -1;
  }
}

void trie_init(Trie *trie) {
  *trie = {};
  assert(trie->nodes.size == 0);
  TrieNode empty = {};
  node_init(&empty, 0);
  list_push(&trie->nodes, &empty, sizeof(TrieNode));
}

void trie_free(Trie *trie) {
  int node_count = trie->nodes.size / sizeof(TrieNode);
  TrieNode *nodes = (TrieNode *)trie->nodes.allocation;
  for (int i = 0; i < node_count; i++) {
    TrieNode *node = nodes + i;
    list_free(&node->leaf_data);
  }
  list_free(&trie->nodes);
}

NodeHandle trie_new_node(Trie *trie, char c) {
  NodeHandle offset = trie->nodes.size / sizeof(TrieNode);
  TrieNode empty = {};
  node_init(&empty, c);
  list_push(&trie->nodes, &empty, sizeof(TrieNode));
  return offset;
}

TrieNode *trie_get(Trie *trie, NodeHandle handle) {
  // assert(handle >= 0 && handle < (trie->nodes.size / (int)sizeof(TrieNode)));
  TrieNode *nodes = (TrieNode *)trie->nodes.allocation;
  return nodes + handle;
}

int get_alphabet_index(char c) {
  unsigned char index = ALPHABET_LUT[(int)c];
  if (index >= ALPHABET_SIZE) {
    index -= ALPHABET_SIZE;
  }
  return index;
}

NodeHandle find_child(Trie *trie, NodeHandle handle, char key) {
  assert(key != 0);
  TrieNode *node = trie_get(trie, handle);

  int index = get_alphabet_index(key);
  return node->alphabet[index];
}

NodeHandle find_or_insert_child(Trie *trie, NodeHandle handle, char key) {
  assert(key != 0);
  TrieNode *node = trie_get(trie, handle);

  int index = get_alphabet_index(key);
  NodeHandle child = node->alphabet[index];

  if (child == TRIE_NULL) {
    NodeHandle inserted = trie_new_node(trie, key);
    node = trie_get(trie, handle);
    child = inserted;
    node->alphabet[index] = inserted;
  }

  return child;
}

NodeHandle node_insert(Trie *trie, const char *key) {
  assert(*key != 0);
  NodeHandle current = TRIE_ROOT;
  for (int i = 0; key[i] != 0; i++) {
    char c = key[i];
    current = find_or_insert_child(trie, current, c);
  }
  return current;
}

NodeHandle node_find(Trie *trie, const char *key) {
  assert(*key != 0);
  NodeHandle current = TRIE_ROOT;
  for (int i = 0; key[i] != 0; i++) {
    current = find_child(trie, current, key[i]);
    if (current == TRIE_NULL) {
      return TRIE_NULL;
    }
  }

  return current;
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

void collect_children(Trie *trie, NodeHandle node, ArrayList *stack,
                      ArrayList *collected) {
  int start_size = stack->size;
  list_push(stack, &node, sizeof(NodeHandle));

  while (stack->size > start_size) {
    NodeHandle handle;
    list_pop(stack, &handle, sizeof(NodeHandle));

    TrieNode *pop = trie_get(trie, handle);
    collect_leaf_data(pop, collected);

    for (int i = 0; i < ALPHABET_SIZE; i++) {
      NodeHandle child = pop->alphabet[i];
      if (child != TRIE_NULL) {
        list_push(stack, &child, sizeof(NodeHandle));
      }
    }
  }
}

typedef struct {
  char *number;
  char *name;
} Contact;

void exists() { printf("Kontakt jiz existuje.\n"); }
void bad() { printf("Nespravny vstup.\n"); }

bool leaf_add_data(Trie *trie, NodeHandle handle, ArrayList *contacts,
                   const char *number, const char *name, int new_contact) {
  TrieNode *node = trie_get(trie, handle);

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

#define EXPECT(char)                                                           \
  if (*(line++) != char) {                                                     \
    DEBUGF("Unexpected character '%c'", char);                                 \
    return bad();                                                              \
  }

int expect_sequence(char **line, int (*function)(int), char end) {
  char *start = *line;
  while (function(**line)) {
    *line += 1;
  }
  if (**line == end) {
    ptrdiff_t size = *line - start;
    **line = '\0';
    *line += 1;
    return size;
  }
  DEBUGF("Unexpected character '%c'", **line);
  return -1;
}

void add_number(char *line, ArrayList *contacts, Trie *number_trie,
                Trie *t9_name_trie) {
  // + 123456 Vagner Ladislav
  EXPECT('+')
  EXPECT(' ')

  char *number_start = line;
  int number_size = expect_sequence(&line, isdigit, ' ');
  if (number_size < 1 || number_size > 20) {
    return bad();
  }

  if (*line == ' ') {
    return bad();
  }

  char *name_start = line;
  int name_size = expect_sequence(
      &line, [](int c) -> int { return isalpha(c) || c == ' '; }, '\n');
  if (name_size < 1) {
    return bad();
  }

  char *number = (char *)malloc(number_size + 1);
  memcpy(number, number_start, number_size + 1);

  char *name = (char *)malloc(name_size + 1);
  memcpy(name, name_start, name_size + 1);

  Contact contact = {number, name};
  int new_contact = contacts->size / sizeof(Contact);
  list_push(contacts, &contact, sizeof(Contact));

  NodeHandle node = TRIE_NULL;
  node = node_insert(number_trie, number);
  if (leaf_add_data(number_trie, node, contacts, number, name, new_contact)) {
    return exists();
  }

  encode_t9(name_start);
  node = node_insert(t9_name_trie, name_start);
  if (leaf_add_data(t9_name_trie, node, contacts, number, name, new_contact)) {
    return exists();
  }

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
              ArrayList *collected, Trie *number_trie, Trie *t9_name_trie) {
  // ? 1234567
  EXPECT('?')
  EXPECT(' ')

  const char *number_start = line;
  int number_size = expect_sequence(&line, isdigit, '\n');
  if (number_size < 1) {
    return bad();
  }

  list_reset(stack);
  list_reset(collected);

  NodeHandle found = TRIE_NULL;
  found = node_find(number_trie, number_start);
  if (found != TRIE_NULL) {
    collect_children(number_trie, found, stack, collected);
  }

  found = node_find(t9_name_trie, number_start);
  if (found != TRIE_NULL) {
    collect_children(t9_name_trie, found, stack, collected);
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
  Trie number_trie = {};
  Trie t9_name_trie = {};

  trie_init(&number_trie);
  trie_init(&t9_name_trie);

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
  trie_free(&number_trie);
  trie_free(&t9_name_trie);
  return 0;
}
