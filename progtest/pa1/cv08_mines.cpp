#include <cassert>
#include <cmath>
#include <cstring>
#include <stdio.h>

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

void list_free(ArrayList *list) {
  if (list->allocation) {
    free(list->allocation);
  }
}

int bad() {
  printf("Nespravny vstup.\n");
  return 1;
}

int main() {
  printf("Zadejte hraci plochu:\n");
  int width = 0;
  int current_width = 0;
  ArrayList input = {};

  int value = false;
  bool loop = true;
  while (loop) {
    switch (getchar()) {
    case '.':
      current_width += 1;
      value = false;
      list_push(&input, &value, 1);
      break;
    case '*':
      current_width += 1;
      value = true;
      list_push(&input, &value, 1);
      break;
    case '\n':
      if (width == 0) {
        width = current_width;
      } else if (current_width != width) {
        return bad();
      }
      current_width = 0;
      break;
    case EOF:
      loop = false;
      break;
    default:
      return bad();
    }
  }

  if (width == 0)
    return bad();

  printf("Vyplnena hraci plocha:\n");

  assert(input.size % width == 0);
  int lines = (input.size / width);
  for (int y = 0; y < lines; y++) {
    for (int x = 0; x < width; x++) {
      bool self_is_mine = ((bool *)input.allocation)[y * width + x];
      if (self_is_mine) {
        printf("*");
        continue;
      }

      int mines = 0;
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          int x_ = x + dx;
          int y_ = y + dy;
          if ((x_ == x && y_ == y) || x_ == -1 || y_ == -1 || x_ == width ||
              y_ == lines) {
            continue;
          }

          bool is_mine = ((bool *)input.allocation)[y_ * width + x_];
          if (is_mine) {
            mines += 1;
          }
        }
      }
      if (mines == 0) {
        printf(".");
      } else {
        printf("%d", mines);
      }
    }
    printf("\n");
  }

  list_free(&input);
  return 0;
}