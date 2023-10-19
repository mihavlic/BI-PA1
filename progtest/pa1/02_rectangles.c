#include <stdio.h>
#include <cmath>
#include <cfloat>

struct Vec2 {
  double x;
  double y;
};

struct Vec2 sub(Vec2 a, Vec2 b) {
  return Vec2 {
    b.x - a.x,
    b.y - a.y
  };
}

double squared_len(Vec2 a) {
  return a.x * a.x + a.y * a.y;
}

bool feq(double a, double b) {
  return fabs(a - b) < (DBL_EPSILON * 1000.0 * a * b);
}

bool is_right_triangle(Vec2 p1, Vec2 p2, Vec2 p3) {
  Vec2 a = sub(p1, p2);
  Vec2 b = sub(p2, p3);
  Vec2 c = sub(p1, p3);

  double a2 = squared_len(a);
  double b2 = squared_len(b);
  double c2 = squared_len(c);

  // c^2 == a^2 + b^2
  return feq(c2, a2 + b2);
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
    scanf(" ( %lf , %lf )%n", &points[i].x, &points[i].y, &n);
    
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

  char c = 0;
  scanf(" %c", &c);
  if (c != 0) {
    return bad();
  }

  for (int i = 0; i < 4; i++) {
    Vec2 a = points[i];
    Vec2 b = points[(i + 1) % 4];
    Vec2 c = points[(i + 2) % 4];

    if (!is_right_triangle(a, b, c)) {
      printf("Body netvori obdelnik.\n");
      return 0;
    }
  }

  printf("Body tvori obdelnik.\n");
  return 0;
}
