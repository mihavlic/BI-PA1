#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef __int128_t int_t; 

void bad() __attribute__((noreturn));
void bad() {
  printf("Nespravny vstup.\n");
  exit(1);
}

int_t character_length(char c) {
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

ReadStatus read_message_part(int_t *length) {
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
  int_t a;
  int_t m;
};

ReadStatus read_message(Equation *equation) {
  int_t p1 = 0;
  int_t p2 = 0;

  int_t status1 = read_message_part(&p1);
  int_t status2 = read_message_part(&p2);

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
int_t egcd(int_t a, int_t b, int_t* x, int_t* y) {
  if (a == 0) {
    *x = 0;
    *y = 1;
    return b;
  }
  int_t x1, y1;
  int_t gcd = egcd(b % a, a, &x1, &y1);
  *x = y1 - (b / a) * x1;
  *y = x1;
 
  return gcd;
}

// euclidian modulo, it always results in a positive output
int_t mod_euc(int_t lhs, int_t rhs) {
    int_t r = lhs % rhs;
    if (r < 0) {
        if (rhs > 0) {
          return r + rhs;
        } else {
          return r - rhs;
        }
    } else {
      return r;
    }
}

// https://en.wikipedia.org/wiki/Chinese_remainder_theorem#Generalization_to_non-coprime_moduli
bool intersect_equations(Equation eqa, Equation eqb, Equation *out) {
  int_t a = eqa.a;
  int_t b = eqb.a;
  
  int_t m = eqa.m;
  int_t n = eqb.m;

  int_t u, v = 0;
  int_t g = egcd(m, n, &u, &v);

  if (a % g != b % g) {
    return false;
  }

  int_t M = (m * n) / g;
  int_t c = ((a*v*n + b*u*m) / g);

  out->a = mod_euc(c, M);
  out->m = M;

  return true;
}


int main() {
  printf("Zpravy:\n");

  Equation p, m = {};
  int_t i = 0;
  while (read_message(&m) != READ_STATUS_FILE_END) {
    i++;
    if (m.m == 0) {
      bad();
    }
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

  printf("Synchronizace za: %lld\n", (long long)mod_euc(p.a, p.m));
  return 0;
}