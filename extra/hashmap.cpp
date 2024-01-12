
#include <cassert>
#include <cstdlib>
#include <cstring>

typedef struct {
  int x;
  int y;
  int value;
} Entry;

typedef struct {
  Entry *entries;
  int occupied;
  int len;
} HashMap;

// capacity must be power-of-two
void map_with_capacity(HashMap *map, int capacity) {
  assert(map->entries == NULL);
  Entry *ptr = (Entry *)malloc(capacity * sizeof(Entry));
  memset(ptr, -1, capacity * sizeof(Entry));
  map->entries = ptr;
  map->occupied = 0;
  map->len = capacity;
}

void map_insert(HashMap *map, int x, int y, int value);
void map_resize(HashMap *map, int new_capacity) {
  HashMap bigger = {};
  map_with_capacity(&bigger, new_capacity);
  for (int i = 0; i < map->len; i++) {
    Entry entry = map->entries[i];
    if (entry.x != -1) {
      map_insert(&bigger, entry.x, entry.y, entry.value);
    }
  }
  free(map->entries);
  *map = bigger;
}

Entry *map_find_entry(HashMap *map, int x, int y) {
  if (map->len == 0) {
    return NULL;
  }
  int hash = (x + y) * (x + y + 1) / 2 + x;
  int mask = map->len - 1;
  // modulo to entry count, len is always power of two so this works
  int bin = hash & mask;
  for (int i = 0; i < map->len; i++) {
    Entry *entry = &map->entries[(hash + i) & mask];
    if (entry->x == -1 || (entry->x == x && entry->y == y)) {
      return entry;
    }
  }
  return NULL;
}

void map_insert(HashMap *map, int x, int y, int value) {
  if (map->len == 0) {
    map_with_capacity(map, 8);
  } else if (map->occupied > map->len / 2) {
    map_resize(map, map->len * 2);
  }
  Entry *entry = map_find_entry(map, x, y);
  entry->x = x;
  entry->y = y;
  entry->value = value;
}

void map_remove(HashMap *map, int x, int y, int value) {
  Entry *entry = map_find_entry(map, x, y);
  if (entry) {
    entry->x = -1;
  }
}