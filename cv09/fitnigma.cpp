#ifndef __PROGTEST__
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif /* __PROGTEST__ */

typedef struct {
  void *allocation;
  int size;
  int capacity;
} ArrayList;

void list_push(ArrayList *list, void *element, int size) {
  int new_size = list->size + size;
  if (new_size > list->capacity) {
    int new_capacity = list->capacity * 2;
    if (new_capacity == 0) {
      new_capacity = 8;
    }
    if (new_capacity < new_size) {
      new_capacity = new_size;
    }
    list->capacity = new_capacity;
    list->allocation = realloc(list->allocation, new_capacity);
  }
  memcpy((char *)list->allocation + list->size, element, size);
  list->size = new_size;
}

void push_word(ArrayList *list_list, ArrayList *word) {
  char null = 0;
  list_push(word, &null, sizeof(char));
  char *ptr = (char *)word->allocation;
  list_push(list_list, &ptr, sizeof(char *));
  // clear the word
  *word = {};
}

bool message_str_valid(const char *key, const char *str, int key_len) {
  int key_index = 0;
  int str_index = 0;
  bool prev_space = false;
  while (true) {
    char key_char = key[key_index];
    char str_char = str[str_index];

    char decrypted = key_char ^ str_char;

    if (isspace(decrypted)) {
      if (prev_space) {
        return false;
      }
      prev_space = true;
    } else {
      prev_space = false;
    }

    if (decrypted == 0) {
      break;
    }

    key_index += 1;
    str_index += 1;
    if (key_index == key_len) {
      key_index = 0;
    }
  }
  return true;
}

char **decrypt(const char *key, const char *str) {
  if (!key || !str || *key == 0) {
    return nullptr;
  }

  int key_len = strlen(key);
  if (!message_str_valid(key, str, key_len)) {
    return NULL;
  }

  ArrayList list_list = {};
  ArrayList last_word = {};

  int key_index = 0;
  int str_index = 0;
  while (true) {
    char key_char = key[key_index];
    char str_char = str[str_index];

    char decrypted = key_char ^ str_char;

    if (decrypted == 0 || isspace(decrypted)) {
      if (last_word.size > 0) {
        push_word(&list_list, &last_word);
      }
      if (decrypted == 0) {
        break;
      }
    } else {
      list_push(&last_word, &decrypted, sizeof(char));
    }

    key_index += 1;
    str_index += 1;
    if (key_index == key_len) {
      key_index = 0;
    }
  }

  void *null = nullptr;
  list_push(&list_list, &null, sizeof(void *));
  assert(last_word.allocation == nullptr);
  return (char **)list_list.allocation;
}

#ifndef __PROGTEST__
void destroyLogs(char **memblock) {
  if (!memblock)
    return;
  for (size_t i = 0; memblock[i] != NULL; ++i)
    free(memblock[i]);
  free(memblock);
}

int main(int argc, char *argv[]) {
  char **result;
  const char str0[] = {0x6c, 0x72, 0x41, 0x52, 0x1d, 0x12,
                       0x18, 0x00, 0x09, 0x0e, 0x3c};
  result = decrypt("<3progtest", str0);
  assert(!strcmp(result[0], "PA1"));
  assert(!strcmp(result[1], "rulezz"));
  assert(result[2] == NULL);
  destroyLogs(result);

  const char str1[] = {0x1b, 0x45, 0x0d, 0x03, 0x7f, 0x08, 0x41,
                       0x3e, 0x61, 0x52, 0x26, 0x44, 0x09, 0x65,
                       0x1f, 0x0b, 0x00, 0x13, 0x41};
  result = decrypt("I love PA1", str1);
  assert(!strcmp(result[0], "Real"));
  assert(!strcmp(result[1], "man"));
  assert(!strcmp(result[2], "code"));
  assert(!strcmp(result[3], "in"));
  assert(!strcmp(result[4], "C"));
  assert(result[5] == NULL);
  destroyLogs(result);

  const char str2[] = {0x19, 0x06, 0x1d, 0x7f, 0x1b, 0x55, 0x24, 0x75,
                       0x15, 0x65, 0x09, 0x02, 0x28, 0x1c, 0x06, 0x54,
                       0x0e, 0x4f, 0x25, 0x17, 0x10, 0x46, 0x6f};
  result = decrypt("Monty Python", str2);
  assert(!strcmp(result[0], "Tis"));
  assert(!strcmp(result[1], "but"));
  assert(!strcmp(result[2], "a"));
  assert(!strcmp(result[3], "flesh"));
  assert(!strcmp(result[4], "wound."));
  assert(result[5] == NULL);
  destroyLogs(result);

  const char str3[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c,
                       0x53, 0x49, 0x11, 0x10, 0x4e, 0x10, 0x4f,
                       0x09, 0x1a, 0x00, 0x55, 0x73};
  result = decrypt(" Romanes eunt domus ", str3);
  assert(!strcmp(result[0], "Romani"));
  assert(!strcmp(result[1], "ite"));
  assert(!strcmp(result[2], "domum"));
  assert(result[3] == NULL);
  destroyLogs(result);

  const char str4[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c,
                       0x53, 0x00, 0x0c, 0x01, 0x0b, 0x54, 0x44,
                       0x0b, 0x02, 0x18, 0x18, 0x53, 0x20};
  result = decrypt(" Romanes eunt domus ", str4);
  assert(result == NULL);
  destroyLogs(result);

  const char str5[] = {0x70, 0x59, 0x39, 0x07, 0x01, 0x1a, 0x29, 0x79};
  result = decrypt("Python", str5);
  assert(result == NULL);
  destroyLogs(result);

  assert(decrypt(NULL, "1234") == NULL);
  return 0;
}
#endif /* __PROGTEST__ */
