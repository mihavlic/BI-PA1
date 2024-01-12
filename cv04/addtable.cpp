#include <stdio.h>

int bad() {
    printf("Nespravny vstup.\n");
    return 1;
}

int digits(int i) {
    if (i == 0) {
        return 1;
    }

    int count = 0;
    while (i != 0) {
        i /= 10;
        count++;
    }
    return count;
}

int main() {
    int n = 0;
    char c = 0;

    printf("Rozsah:\n");
    if (scanf("%d %c", &n, &c) != 1 || n <= 0) {
        return bad();
    }

    int padd = digits(n * n);

    printf("%*s|", padd, "");
    for (int i = n; i > 0; i--) {
        printf(" %*d", padd, i);
    }

    printf("\n");
    for (int j = 0; j < padd; j++) {
        putchar('-');
    }
    printf("+");
    for (int i = n; i > 0; i--) {
        for (int j = 0; j <= padd; j++) {
            putchar('-');
        }
    }
    
    printf("\n");

    for (int i = 1; i <= n; i++) {
        printf("%*d|", padd, i);
        for (int j = n; j >= i; j--) {
            printf(" %*d", padd, i * j);
        }
        printf("\n");
    }
    
    return 0;
}