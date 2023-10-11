#include <stdio.h>

int bad() {
    printf("Nespravny vstup.\n");
    return 1;
}

int main() {
    int times[8] = {};
    for (int i = 0; i < 2; i++) {
        printf("Zadejte cas t%d:\n", i + 1);

        int *time = times + 4 * i;
        char frac[5] = {};
        int count = scanf("%d :%d :%d , %4[0-9]", time, time + 1, time + 2, frac);
        // bruh
        time[3] =
            (frac[0] ? (frac[0] - '0') : 0) * 100
          + (frac[1] ? (frac[1] - '0') : 0) * 10
          + (frac[2] ? (frac[2] - '0') : 0);

        if (
            count < 4
            || frac[3] != 0
            || 0 > time[0] || time[0] > 23
            || 0 > time[1] || time[1] > 59
            || 0 > time[2] || time[2] > 59
            || 0 > time[3] || time[3] > 999
        ) {
            return bad();
        }
    }

    char rem[2] = {};
    scanf(" %1s", rem);
    if (rem[0] != 0) {
        return bad();
    }

    int ms1 = (((times[0] * 60) + times[1]) * 60 + times[2]) * 1000 + times[3];
    int ms2 = (((times[4] * 60) + times[5]) * 60 + times[6]) * 1000 + times[7];
    int millis = ms2 - ms1;

    if (millis < 0) {
        return bad();
    }

    int secs = millis / 1000;
    int mins = secs / 60;
    int hours = mins / 60;
    
    millis %= 1000;
    secs %= 60;
    mins %= 60;

    printf("Doba: %2d:%02d:%02d,%03d\n", hours, mins, secs, millis);
    return 0;
}