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

void list_push(ArrayList *list, const void *element, int size) {
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

  counter = 0;
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
  int string_start;
  int string_end;
  ArrayList leaf_data;
  NodeHandle alphabet[ALPHABET_SIZE];
} TrieNode;

typedef struct {
  ArrayList nodes;
} Trie;

void node_init(TrieNode *node) {
  node->leaf_data = {};
  for (int i = 0; i < ALPHABET_SIZE; i++) {
    node->alphabet[i] = -1;
  }
}

void trie_init(Trie *trie) {
  *trie = {};
  assert(trie->nodes.allocation == NULL);
  TrieNode empty = {};
  node_init(&empty);
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

NodeHandle trie_new_node(Trie *trie, int string_start, int string_end) {
  assert(string_start < string_end);
  NodeHandle offset = trie->nodes.size / sizeof(TrieNode);
  TrieNode empty = {};
  node_init(&empty);
  empty.string_start = string_start;
  empty.string_end = string_end;
  list_push(&trie->nodes, &empty, sizeof(TrieNode));
  return offset;
}

TrieNode *trie_get(Trie *trie, NodeHandle handle) {
  // assert(handle >= 0 && handle < (trie->nodes.size / (int)sizeof(TrieNode)));
  TrieNode *nodes = (TrieNode *)trie->nodes.allocation;
  return nodes + handle;
}

int string_push(ArrayList *buffer, const char *string, int string_len) {
  NodeHandle offset = buffer->size / sizeof(char);
  list_push(buffer, string, string_len * sizeof(char));
  return offset;
}

const char *string_get(ArrayList *buffer, int offset) {
  return (const char *)buffer->allocation + offset;
}

int get_alphabet_index(char c) {
  unsigned char index = ALPHABET_LUT[(int)c];
  return index;
}

NodeHandle find_child(Trie *trie, NodeHandle handle, char key) {
  assert(key != 0);
  TrieNode *node = trie_get(trie, handle);

  int index = get_alphabet_index(key);
  return node->alphabet[index];
}

NodeHandle node_insert(Trie *trie, ArrayList *string, const char *key,
                       int key_len) {
  assert(key_len > 0);
  int key_start = string_push(string, key, key_len);
  int key_end = key_start + key_len;
  const char *str = string_get(string, 0);

  NodeHandle current = TRIE_ROOT;
  while (key_start < key_end) {
    char c = str[key_start];
    int index = get_alphabet_index(c);

    TrieNode *current_ptr = trie_get(trie, current);
    NodeHandle child = current_ptr->alphabet[index];

    if (child == TRIE_NULL) {
      NodeHandle inserted = trie_new_node(trie, key_start, key_end);
      current_ptr = trie_get(trie, current);
      current_ptr->alphabet[index] = inserted;
      return inserted;
    } else {
      TrieNode *child_ptr = trie_get(trie, child);

      const char *inserted = str + key_start;
      const char *original = str + child_ptr->string_start;
      int inserted_len = key_end - key_start;
      int original_len = child_ptr->string_end - child_ptr->string_start;

      int min_len = (inserted_len > original_len) ? original_len : inserted_len;

      int same_len = 0;
      for (; same_len < min_len; same_len++) {
        if (inserted[same_len] != original[same_len]) {
          break;
        }
      }

      assert(same_len > 0);

      //  current         child
      //  |-----| -> |---------------|
      //  |-----| -> |----| -> |-----|
      //              inserted  child
      if (same_len == original_len) {
        current = child;
      } else {
        assert(same_len < original_len);
        NodeHandle inserted =
            trie_new_node(trie, key_start, key_start + same_len);
        TrieNode *inserted_ptr = trie_get(trie, inserted);
        current_ptr = trie_get(trie, current);
        child_ptr = trie_get(trie, child);

        int new_index = get_alphabet_index(original[same_len]);
        current_ptr->alphabet[index] = inserted;
        inserted_ptr->alphabet[new_index] = child;

        child_ptr->string_start += same_len;
        assert(child_ptr->string_start < child_ptr->string_end);

        current = inserted;
      }
      key_start += same_len;
    }
  }
  return current;
}

NodeHandle node_find_prefix(Trie *trie, ArrayList *string, const char *key,
                            int key_len) {
  const char *str = string_get(string, 0);
  int key_start = 0;
  int key_end = key_len;

  NodeHandle current = TRIE_ROOT;
  while (key_start < key_len) {
    char c = key[key_start];
    int index = get_alphabet_index(c);

    TrieNode *current_ptr = trie_get(trie, current);
    NodeHandle child = current_ptr->alphabet[index];
    if (child == TRIE_NULL) {
      return TRIE_NULL;
    }

    TrieNode *child_ptr = trie_get(trie, child);
    int remaining_key_len = key_end - key_start;
    int child_key_len = child_ptr->string_end - child_ptr->string_start;

    int min_len =
        (remaining_key_len > child_key_len) ? child_key_len : remaining_key_len;

    const char *inserted = key + key_start;
    const char *original = str + child_ptr->string_start;
    if (strncmp(inserted, original, min_len) != 0) {
      return TRIE_NULL;
    }

    current = child;
    // add the child_key_len instead of min_len on purpose here, if we get past
    // the end, we'll end the while loop anyway
    key_start += child_key_len;
  }
  return current;
}

void encode_t9(char *string) {
  for (int i = 0; string[i] != 0; i++) {
    string[i] = T9_LUT[(int)string[i]];
  }
}

void collect_leaf_data(TrieNode *node, ArrayList *collected) {
  if (node->leaf_data.size > 0) {
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

void add_number(char *line, ArrayList *contacts, ArrayList *string,
                Trie *number_trie, Trie *t9_name_trie) {
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
  node = node_insert(number_trie, string, number, number_size);
  if (leaf_add_data(number_trie, node, contacts, number, name, new_contact)) {
    return exists();
  }

  encode_t9(name_start);
  node = node_insert(t9_name_trie, string, name_start, name_size);
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
              ArrayList *collected, ArrayList *string, Trie *number_trie,
              Trie *t9_name_trie) {
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
  found = node_find_prefix(number_trie, string, number_start, number_size);
  if (found != TRIE_NULL) {
    collect_children(number_trie, found, stack, collected);
  }

  found = node_find_prefix(t9_name_trie, string, number_start, number_size);
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
  ArrayList string = {};
  Trie number_trie = {};
  Trie t9_name_trie = {};

  trie_init(&number_trie);
  trie_init(&t9_name_trie);

  char *line = NULL;
  size_t line_len = 0;

  while (getline(&line, &line_len, stdin) > 0) {
    switch (*line) {
    case '+':
      add_number(line, &contacts, &string, &number_trie, &t9_name_trie);
      break;
    case '?':
      do_query(line, &contacts, &stack, &collected, &string, &number_trie,
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
  list_free(&string);
  trie_free(&number_trie);
  trie_free(&t9_name_trie);
  return 0;
}
