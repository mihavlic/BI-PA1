#include <stdio.h>
#include <cmath>

struct Vec2 {
  long double x;
  long double y;
};

struct Vec2 sub(Vec2 a, Vec2 b) {
  return Vec2 {
    b.x - a.x,
    b.y - a.y
  };
}

long double dot(Vec2 a, Vec2 b) {
  long double c = a.y * b.y;
  return fma(a.x, b.x, c);
}

long double len(Vec2 a) {
  return sqrt(dot(a, a));
}

struct Vec2 normalize(Vec2 a) {
  long double l = len(a);
  return Vec2 {
    a.x / l,
    a.y / l
  };
}

int bad() {
  printf("Nespravny vstup.\n");
  return 1;
}

int main() {
  Vec2 points[4] = {};
  for (int i = 0; i < 4; i++) {
    printf("Bod #%d:\n", i + 1);

    int n = 0;
    scanf(" ( %Lf , %Lf )%n", &points[i].x, &points[i].y, &n);
    
    if (
      n == 0
      || std::isnan(points[i].x)
      || std::isnan(points[i].y)
      || std::isinf(points[i].x)
      || std::isinf(points[i].y)
    ) {
      return bad();
    }
  }

  char rem[2] = {};
  scanf(" %1s", rem);
  if (rem[0] != 0) {
      return bad();
  }

  for (int i = 0; i < 4; i++) {
    Vec2 a = points[i];
    Vec2 b = points[(i + 1) % 4];
    Vec2 c = points[(i + 2) % 4];

    Vec2 v1 = normalize(sub(a, b));
    Vec2 v2 = normalize(sub(b, c));
    long double product = dot(v1, v2);

    if (fabs(product) > 0.01) {
      printf("Body netvori obdelnik.\n");
      return 0;
    }
  }

  printf("Body tvori obdelnik.\n");
  return 0;
}
