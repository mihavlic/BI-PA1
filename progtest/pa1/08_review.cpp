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

void list_reserve(ArrayList *list, int new_size) {
  if (new_size > list->capacity) {
    // linear growth because we§re hitting the memory limit
    int new_capacity = list->capacity + 128;
    if (new_capacity == 0) {
      new_capacity = 8;
    }
    if (new_capacity < new_size) {
      new_capacity = new_size;
    }
    list->capacity = new_capacity;
    list->allocation = realloc(list->allocation, new_capacity);
  }
  // note size is not updated
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

// binary min/max heap
// https://www.digitalocean.com/community/tutorials/min-heap-binary-tree
// https://www.researchgate.net/publication/316321198_Optimizing_Binary_Heaps
//
// This is a balanced binary tree, with the property that a parent is smaller
// any of its children. Because it is a binary tree, we can map integers to
// parents/children:
//  - parent(i) = (i - 1) / 2
//  - left_child(i) = 2i + 1
//  - right_child(i) = 2i + 2
// and because it's balanced, the integers will get consecutively used. This
// means that when adding a node, we will just select the next integer, and then
// lookup its parent, swapping it up the tree until the heap property holds.

typedef struct {
  ArrayList data;
  bool is_min_heap;
} BinaryHeap;

int make_parent(int i) { return (i - 1) / 2; }
int left_child(int i) { return 2 * i + 1; }
int right_child(int i) { return 2 * i + 2; }

int heap_get_value(BinaryHeap *heap, int index) {
  int offset = index * sizeof(int);
  assert(offset < heap->data.size);
  int value = ((int *)(heap->data.allocation))[index];
  if (!heap->is_min_heap) {
    value = -value;
  }
  return value;
}

void heap_set_value(BinaryHeap *heap, int index, int value) {
  int offset = index * sizeof(int);
  assert(offset < heap->data.size);
  if (!heap->is_min_heap) {
    value = -value;
  }
  ((int *)(heap->data.allocation))[index] = value;
}

int heap_get_size(BinaryHeap *heap) {
  int bytes = heap->data.size;
  assert(bytes % sizeof(int) == 0);
  return bytes / sizeof(int);
}

int heap_get_root(BinaryHeap *heap) { return heap_get_value(heap, 0); }

void heap_init(BinaryHeap *heap, bool is_min_heap) {
  assert(heap->data.allocation == NULL);
  heap->data = {};
  heap->is_min_heap = is_min_heap;
}

void heap_free(BinaryHeap *heap) { list_free(&heap->data); }

// move an element up to uphold invariants
void sift_up(BinaryHeap *heap, int index) {
  int size = heap_get_size(heap);
  if (size <= 1) {
    return;
  }

  int *a = (int *)heap->data.allocation;
  int j = index;
  int x = a[j];

  while (j != 0) {
    int parent = make_parent(j);
    if (x >= a[parent])
      break;
    a[j] = a[parent];
    j = parent;
  }

  a[j] = x;
}

// uphold the heap property by swapping down until we reach the bottom
void sift_down(BinaryHeap *heap, int index) {
  int size = heap_get_size(heap);
  if (size <= 1)
    return;

  int *a = (int *)heap->data.allocation;
  int i = index;
  int x = a[i];

  while (left_child(i) < size) {
    int left = left_child(i);
    int right = right_child(i);

    if (right < size) {
      if (a[right] < a[left])
        left = right;
    }
    if (a[left] >= x)
      break;

    a[i] = a[left];
    i = left;
  }

  a[i] = x;
}

void heap_push(BinaryHeap *heap, int value) {
  int next_index = heap_get_size(heap);
  if (!heap->is_min_heap) {
    value = -value;
  }
  // add the value to a bottom node
  list_push(&heap->data, &value, sizeof(int));
  // update the heap
  sift_up(heap, next_index);
}

int heap_pop(BinaryHeap *heap) {
  int size = heap_get_size(heap);
  if (size == 0) {
    assert(false);
  }

  int root_value = heap_get_root(heap);
  int last_value = heap_get_value(heap, size - 1);
  heap_set_value(heap, 0, last_value);
  heap->data.size -= sizeof(int);
  // update the heap
  sift_down(heap, 0);

  return root_value;
}

typedef struct {
  BinaryHeap min;
  BinaryHeap max;
} MedianSoup;

void soup_init(MedianSoup *soup) {
  heap_init(&soup->min, true);
  heap_init(&soup->max, false);
}

void soup_clear(MedianSoup *soup) {
  list_reset(&soup->min.data);
  list_reset(&soup->max.data);
}

void soup_free(MedianSoup *soup) {
  heap_free(&soup->min);
  heap_free(&soup->max);
}

void soup_rebalance(MedianSoup *soup) {
  int min_size = heap_get_size(&soup->min);
  int max_size = heap_get_size(&soup->max);
  if (abs(min_size - max_size) > 1) {
    if (max_size > min_size) {
      heap_push(&soup->min, heap_pop(&soup->max));
    } else {
      heap_push(&soup->max, heap_pop(&soup->min));
    }
  }
}

bool soup_is_empty(MedianSoup *soup) {
  return heap_get_size(&soup->min) == 0 && heap_get_size(&soup->max) == 0;
}

int soup_median(MedianSoup *soup) {
  assert(!soup_is_empty(soup));

  int min_size = heap_get_size(&soup->min);
  int max_size = heap_get_size(&soup->max);

  assert(abs(min_size - max_size) <= 1);

  if (max_size >= min_size) {
    return heap_get_root(&soup->max);
  } else {
    return heap_get_root(&soup->min);
  }
}

void soup_insert(MedianSoup *soup, int n) {
  BinaryHeap *heap = NULL;
  // initialize if empty
  if (soup_is_empty(soup)) {
    heap = &soup->min;
  } else {
    int median = soup_median(soup);
    if (n < median) {
      heap = &soup->max;
    } else {
      heap = &soup->min;
    }
  }
  heap_push(heap, n);
  soup_rebalance(soup);
}

typedef struct {
  // year  0-3000 16 bits
  // month 0-12   8 bits
  // day   0-31   8 bits
  unsigned short year;
  unsigned char month;
  unsigned char day;

  unsigned score;
  char *message;
} Review;

bool is_leap(int y) { return (y % 100 == 0) ? (y % 400 == 0) : (y % 4 == 0); }

int days_in_moth(int y, int m) {
  const int moth_density[13] = {0,  31, 28, 31, 30, 31, 30,
                                31, 31, 30, 31, 30, 31};
  if (m == 2 && is_leap(y)) {
    return 29;
  } else {
    return moth_density[m];
  }
}

uint32_t get_entry_timestamp(Review *entry) {
  return (uint32_t)entry->year << 16 | (uint32_t)entry->month << 8 |
         (uint32_t)entry->day;
}

int read_review(Review *entry) {
  // + 2023-11-16 98 Fake_review
  // ^ already handled
  int year = 0;
  int month = 0;
  int day = 0;
  int score = 0;
  char message_buf[4097] = {};

  int matched =
      scanf("%d-%d-%d %d %4096s", &year, &month, &day, &score, message_buf);

  if (matched != 5 || year < 1 || month < 1 || month > 12 || day < 1 ||
      day > days_in_moth(year, month) || score < 1) {
    DEBUGF("+ review scanf failed: year %d, month %d, day %d, score %d, "
           "message_buf %s, match %d\n",
           year, month, day, score, message_buf, matched);
    return 1;
  }

  int message_len = strlen(message_buf);
  char *message = (char *)malloc(message_len + 1);
  memcpy(message, message_buf, message_len + 1);

  entry->year = (unsigned short)year;
  entry->month = (unsigned char)month;
  entry->day = (unsigned char)day;
  entry->score = (unsigned)score;
  entry->message = message;

  return 0;
}

int last_in_day(int i, Review *reviews, int review_count) {
  if (i >= review_count) {
    return i;
  }

  Review *start = &reviews[i];
  uint32_t start_timestamp = get_entry_timestamp(start);

  int j = i + 1;
  for (; j < review_count; j++) {
    if (start_timestamp != get_entry_timestamp(reviews + j)) {
      break;
    }
  }
  return j - 1;
}

typedef struct {
  int start;
  int end;
  unsigned median;
  double average;
} ResultWindow;

void search_reviews(ArrayList *reviews, MedianSoup *window, int window_size,
                    ResultWindow *result) {
  int review_count = reviews->size / sizeof(Review);

  assert(window_size > 0);
  assert(window_size <= review_count);

  *result = {};
  double winning_difference = 0.0;
  uint32_t winning_end_timestamp = 0;
  int winning_count = 0;

  Review *reviews_ptr = (Review *)reviews->allocation;
  for (int start = 0; start < review_count;) {
    soup_clear(window);

    uint64_t sum = 0;
    int end = last_in_day(start + window_size - 1, reviews_ptr, review_count);
    int prev = start;
    while (end < review_count) {
      for (int i = prev; i <= end; i++) {
        unsigned score = reviews_ptr[i].score;
        sum += score;
        soup_insert(window, score);
      }

      int count = end - start + 1;
      unsigned median = soup_median(window);
      assert(median > 0);

      double average = double(sum) / double(count);
      assert(average > 0);
      double difference = fabs(average - (double)median);

      bool won = difference > winning_difference;
      if (difference == winning_difference) {
        uint32_t end_timestamp = get_entry_timestamp(reviews_ptr + end);
        if (end_timestamp > winning_end_timestamp ||
            ((end_timestamp == winning_end_timestamp) &&
             count > winning_count)) {
          winning_end_timestamp = end_timestamp;
          winning_count = count;
          won = true;
        }
      }

      if (won) {
        *result = ResultWindow{start, end, median, average};
        winning_difference = difference;
      }

      prev = end + 1;
      end = last_in_day(end + 1, reviews_ptr, review_count);
    }

    start = last_in_day(start, reviews_ptr, review_count) + 1;
  }
}

int user_search_reviews(char command, ArrayList *reviews, MedianSoup *window) {
  int window_count = 0;
  int matched = scanf("%d", &window_count);

  if (matched != 1 || window_count < 1) {
    DEBUGF("search scanf failed: commad '%c', count %d, matched %d\n", command,
           window_count, matched);
    return 1;
  }

  int review_count = reviews->size / sizeof(Review);
  if (window_count > review_count) {
    printf("Neexistuje.\n");
    return 0;
  }

  ResultWindow result = {};
  search_reviews(reviews, window, window_count, &result);

  Review *reviews_ptr = (Review *)reviews->allocation;
  Review *start = (reviews_ptr) + result.start;
  Review *end = (reviews_ptr) + result.end;

  printf("%d-%02d-%02d - %d-%02d-%02d: %.6f %u\n", start->year, start->month,
         start->day, end->year, end->month, end->day, result.average,
         result.median);

  if (command == '?') {
    for (int i = result.start; i <= result.end; i++) {
      Review *review = reviews_ptr + i;
      printf("  %d: %s\n", review->score, review->message);
    }
  }

  return 0;
}

void bad() { printf("Nespravny vstup.\n"); }

int main() {
  printf("Recenze:\n");

  ArrayList reviews = {};
  MedianSoup window = {};
  soup_init(&window);

  uint32_t prev_timestamp = 0;
  uint32_t current_timestamp = 0;

  bool loop = true;
  while (loop) {
    char command = getchar();
    switch (command) {
    case ' ':
    case '\n':
      continue;
    case '+': {
      Review entry = {};
      if (read_review(&entry)) {
        bad();
        loop = false;
      } else {
        list_push(&reviews, &entry, sizeof(Review));
        current_timestamp = get_entry_timestamp(&entry);

        if (prev_timestamp > current_timestamp) {
          DEBUG("Bad order\n");
          bad();
          loop = false;
        }
        prev_timestamp = current_timestamp;
      }
      break;
    }
    case '?':
    case '#':
      DEBUGF("%c\n", command);
      // a query when no reviews have been added is an error
      if (reviews.size == 0 ||
          user_search_reviews(command, &reviews, &window)) {
        bad();
        loop = false;
      }
      break;
    case EOF:
      loop = false;
      DEBUG("EOF\n");
      break;
    default:
      DEBUGF("unexpected character '%c'\n", command);
      bad();
      loop = false;
      break;
    }
  }

  int review_count = reviews.size / sizeof(Review);
  for (int i = 0; i < review_count; i++) {
    Review *review = ((Review *)reviews.allocation) + i;
    free(review->message);
  }

  list_free(&reviews);
  soup_free(&window);
  return 0;
}
