#include <stdio.h>
#include <stdlib.h>

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

unsigned lcm(unsigned m, unsigned n) {
  return (m * n) / gcd(m, n);
}

struct Message {
  unsigned offset;
  unsigned period;
};

void bad() __attribute__ ((noreturn));
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
      case '|': return READ_STATUS_PIPE;
      case '\n': return READ_STATUS_LINE_END;
      case EOF: return READ_STATUS_FILE_END;
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

bool merge_sequences(Message a, Message b, Message *out) {
  // fprintf(stderr, "A: %u / %u\nB: %u / %u\n", a.offset, a.period, b.offset, b.period);

  if (a.period > b.period) {
    Message tmp = a;
    a = b;
    b = tmp;
  }

  unsigned period = lcm(a.period, b.period);

  if (a.offset == 0 && b.offset == 0) {
    out->offset = 0;
    out->period = period;
    return true;
  }

  for (unsigned i = b.period - b.offset; i < period; i += b.period) {
    if ((i + a.offset) % a.period == 0) {
      out->offset = i;
      out->period = period;
      // fprintf(stderr, "> HUH %u / %u\n", out->offset, out->period);
      return true;
    }
  }
  return false;
}

int main() {
  printf("Zpravy:\n");

  Message p = {};
  int i = 0;
  for (;; i++) {
    Message m = {};
    
    if (read_message(&m) == READ_STATUS_FILE_END) {
      break;
    }

    if (i == 0) {
      p = m;
      continue;
    }

    if (!merge_sequences(p, m, &p)) {
      printf("Nelze dosahnout.\n");
      return 0;
    }
  }

  if (i < 2) {
    bad();
  }
  
  printf("Synchronizace za: %u\n", p.offset);
  return 0;
}