#include <stdio.h>
#include <stdlib.h>
#include <cstring>

// stolen (and modified a bit) from some nerds
// https://android.googlesource.com/kernel/lk/+/qcom-dima-8x74-fixes/lib/libc/gcd_lcm.c
// binary gcd is a thing but is more LOC
// https://en.algorithmica.org/hpc/algorithms/gcd/

unsigned gcd(unsigned m, unsigned n) {
  unsigned x;
  if (m < n) {
    x = m;
    m = n;
    n = x;
  }

  while (n != 0) {
    x = m % n;
    m = n;
    n = x;
  }
  return m;
}

unsigned lcm(unsigned m, unsigned n) { return (m * n) / gcd(m, n); }

struct Message {
  unsigned offset;
  unsigned period;
};

void bad() __attribute__((noreturn));
void bad() {
  printf("Nespravny vstup.\n");
  exit(1);
}

unsigned character_length(char c) {
  if ('a' <= c && c <= 'z') {
    return 1 << (c - 'a');
  } else {
    bad();
  }
}

enum ReadStatus {
  READ_STATUS_PIPE,
  READ_STATUS_LINE_END,
  READ_STATUS_FILE_END,
};

ReadStatus read_message_part(unsigned *length) {
  while (true) {
    char c = getchar();
    switch (c) {
    case '|':
      return READ_STATUS_PIPE;
    case '\n':
      return READ_STATUS_LINE_END;
    case EOF:
      return READ_STATUS_FILE_END;
    }
    *length += character_length(c);
  }
}

ReadStatus read_message(Message *message) {
  unsigned p1 = 0;
  unsigned p2 = 0;

  unsigned status1 = read_message_part(&p1);
  unsigned status2 = read_message_part(&p2);

  if (p1 == 0 && status1 == READ_STATUS_FILE_END) {
    return READ_STATUS_FILE_END;
  }

  // assert that only one '|' character was in this line
  if ((status1 == READ_STATUS_PIPE) == (status2 == READ_STATUS_PIPE)) {
    bad();
  }

  message->offset = p2;
  message->period = p1 + p2;

  return READ_STATUS_LINE_END;
}

bool sequences_end(unsigned t, Message *messages, int len) {
  for (int i = 0; i < len; i++) {
    Message next = messages[i];
    if ((t + next.offset) % next.period != 0) {
      return false;
    }
  }
  return true;
}

int merge_sequences(Message *messages, int len) {
  Message max = {};
  unsigned period = 1;
  for (int i = 0; i < len; i++) {
    Message next = messages[i];
    period = lcm(period, next.period);
    if (next.period > max.period) {
      max = next;
    }
  }

  if (sequences_end(0, messages, len)) {
    return 0;
  }

  for (unsigned t = max.period - max.offset; t < period; t += max.period) {
    if (sequences_end(t, messages, len)) {
      return (int)t;
    }
  }

  return -1;
}

typedef struct {
  char *cursor;
  char *allocation;
  char *end;
} ArrayList;

void list_free(ArrayList *list) {
  if (list->allocation) {
    free(list->allocation);
  }
}

void list_reserve(ArrayList *list, int reserve) {
  char *cursor = list->cursor;
  char *allocation = list->allocation;
  int capacity = list->end - list->allocation;
  int len = cursor - allocation;

  if (cursor + reserve >= list->end) {
    // the allocated block is not big enough for the upcoming insertion
    // double the size, copy the contents, and free the old block
    int new_cap = capacity * 2;
    if (new_cap < reserve) {
      new_cap = reserve;
    }

    char *resized = (char*)(malloc(new_cap));
    memcpy(resized, allocation, len);
    free(allocation);

    list->cursor = resized + len;
    list->allocation = resized;
    list->end = resized + new_cap;
  }
}

void list_push(ArrayList *list, void *thing, int size) {
  list_reserve(list, size);
  memcpy(list->cursor, thing, size);
  list->cursor += size;
}

int list_len(ArrayList *list, int size) {
  int len = list->cursor - list->allocation;
  return len / size;
}

int main() {
  printf("Zpravy:\n");

  ArrayList messages = {};

  Message m = {};
  while (read_message(&m) != READ_STATUS_FILE_END) {
    list_push(&messages, &m, sizeof(m));
  }

  int len = list_len(&messages, sizeof(m));
  if (len < 2) {
    bad();
  }

  int t = merge_sequences((Message*)messages.allocation, len);
  if (t == -1) {
    printf("Nelze dosahnout.\n");
  } else {
    printf("Synchronizace za: %d\n", t);
  }
  return 0;
}