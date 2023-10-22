#include <stdio.h>
#include <stdlib.h>
#include <cstring>

void bad() __attribute__((noreturn));
void bad() {
  printf("Nespravny vstup.\n");
  exit(1);
}

int character_length(char c) {
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

ReadStatus read_message_part(int *length) {
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

// a modular equation in the form of
//  x === a (mod m)
struct Equation {
  int a;
  int m;
};

ReadStatus read_message(Equation *equation) {
  int p1 = 0;
  int p2 = 0;

  int status1 = read_message_part(&p1);
  int status2 = read_message_part(&p2);

  if (p1 == 0 && status1 == READ_STATUS_FILE_END) {
    return READ_STATUS_FILE_END;
  }

  if (status1 != READ_STATUS_PIPE || status2 != READ_STATUS_LINE_END) {
    bad();
  }

  equation->a = p1;
  equation->m = p1 + p2;

  return READ_STATUS_LINE_END;
}

// stolen from https://www.geeksforgeeks.org/c-program-for-basic-and-extended-euclidean-algorithms-2/
int egcd(int a, int b, int* x, int* y) {
  if (a == 0) {
    *x = 0;
    *y = 1;
    return b;
  }
  int x1, y1;
  int gcd = egcd(b % a, a, &x1, &y1);
  *x = y1 - (b / a) * x1;
  *y = x1;
 
  return gcd;
}

// https://en.wikipedia.org/wiki/Chinese_remainder_theorem#Generalization_to_non-coprime_moduli
bool intersect_equations(Equation eqa, Equation eqb, Equation *out) {
  int a = eqa.a;
  int b = eqb.a;
  
  int m = eqa.m;
  int n = eqb.m;

  int u, v = 0;
  int g = egcd(m, n, &u, &v);

  if (a % g != b % g) {
    return false;
  }
  
  out->a = (a*v*n + b*u*m) / g;
  out->m = (m * n) / g;
  return true;
}


int main() {
  printf("Zpravy:\n");

  Equation p, m = {};
  int i = 0;
  while (read_message(&m) != READ_STATUS_FILE_END) {
    i++;
    if (i == 1) {
      p = m;
      continue;
    }
    if (!intersect_equations(p, m, &p)) {
      printf("Nelze dosahnout.\n");
      return 0;
    }
  }

  if (i < 2) {
    bad();
  }

  int time = p.a % p.m;
  if (time < 0) {
    time += p.m;
  }
  printf("Synchronizace za: %d\n", time);
  return 0;
}