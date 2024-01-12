#include <stdio.h>

int bad() {
    printf("Nespravny vstup.\n");
    return 1;
}

int main() {
    int n, s = 0;

    printf("Zadejte pocet poli:\n");
    if (scanf("%d", &n) != 1 || n <= 0) {
        return bad();
    }

    printf("Zadejte velikost pole:\n");
    if (scanf("%d", &s) != 1 || s <= 0) {
        return bad();
    }
    
    char c[2] = {};
    scanf(" %1s", c);
    if (c[0] != 0) {
        return bad();
    }

    int w = 2 + n * s;
    int w1 = w - 1;
    for (int y = 0; y < w; y++) {
        bool yside = y == 0 || y == w1;
        for (int x = 0; x < w; x++) {
            bool xside = x == 0 || x == w1;

            char c = 0;
            if (xside && yside) {
                c = '+';
            } else if (xside) {
                c = '|';
            } else if (yside) {
                c = '-';
            } else {
                if ((((x - 1) / s) + ((y - 1) / s)) % 2 == 1) {
                    c = 'X';
                } else {
                    c = ' ';
                }
            }
            putchar(c);
        }
        printf("\n");
    }
}