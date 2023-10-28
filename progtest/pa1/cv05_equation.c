#include <stdio.h>
#include <cmath>
#include <cfloat>

int bad() {
    printf("Nespravny vstup.\n");
    return 1;
}

bool feq(double a, double b) {
    double epsilon = fabs(a * b);
    if (epsilon == 0.0) {
        epsilon = 1.0;
    }
    return fabs(a - b) < (DBL_EPSILON * (1000 * epsilon));
}

int main() {
    double a, b, o = 0.0;
    char op = 0;
    char c = 0;
    int n = 0;

    printf("Zadejte rovnici:\n");
    scanf("%lf %c %lf = %lf %n %c", &a, &op, &b, &o, &n, &c);

    if (n == 0 || c != 0) {
        return bad();
    }

    double out = 0.0;
    switch (op) {
    case '+':
        out = a + b;
        break;
    case '-':
        out = a - b;
        break;
    case '*':
        out = a * b;
        break;
    case '/':
        out = std::trunc(a / b);
        break;
    default:
        return bad();
    }

    if (std::isnan(out) || std::isinf(out)) {
        return bad();
    }

    if (feq(out, o)) {
        printf("Rovnice je spravne.\n");
    } else {
        printf("%g != %g\n", out, o);
    }

    return 0;
}