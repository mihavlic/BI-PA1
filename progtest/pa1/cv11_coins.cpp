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

void list_push(ArrayList *list, void *element, int size) {
  int new_size = list->size + size;
  list_reserve(list, new_size);
  memcpy((char *)list->allocation + list->size, element, size);
  list->size = new_size;
}

void list_free(ArrayList *list) { free(list->allocation); }

int find_count(int *numbers, int number_count, int sum) {
  int min = -1;
  for (int i = 0; i < number_count; i++) {
    int number = numbers[i];
    if (number >= sum) {
      if (number == sum) {
        return 1;
      }
      continue;
    }

    int count = find_count(numbers, number_count, sum - number);
    if (count != -1 && (count < min || min == -1)) {
      min = count;
    }
  }
  if (min == -1) {
    return -1;
  } else {
    return min + 1;
  }
}

void bad() { printf("Nespravny vstup.\n"); }

void run(ArrayList *numbers) {
  printf("Mince:\n");
  while (true) {
    int num = 0;
    int count = scanf("%d", &num);
    if (count != 1 || num < 0) {
      return bad();
    }

    if (num == 0) {
      break;
    }
    list_push(numbers, &num, sizeof(int));
  }

  printf("Castky:\n");
  while (true) {
    int num = 0;
    int count = scanf("%d", &num);
    if (feof(stdin)) {
      return;
    }
    if (count != 1 || num < 1) {
      return bad();
    }
    int min = find_count((int *)numbers->allocation,
                         numbers->size / sizeof(int), num);
    if (min > 0) {
      printf("= %d\n", min);
    } else {
      printf("= nema reseni\n");
    }
  }
}

int main() {
  ArrayList numbers = {};
  run(&numbers);
  list_free(&numbers);
  return 0;
}