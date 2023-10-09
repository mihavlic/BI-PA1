#include <stdio.h>

int main() {
    int times[8] = {};
    for (int i = 0; i < 2; i++) {
        printf("Zadejte cas t%d:\n", i + 1);

        int n = 0;
        int *time = times + 4 * i;
        scanf(" %d :%d :%d ,%d%n ", time, time + 1, time + 2, time + 3, &n);
        
        if (
            n == 0
            || 0 > time[0] || time[0] > 23
            || 0 > time[1] || time[1] > 59
            || 0 > time[2] || time[2] > 59
            || 0 > time[3] || time[3] > 999
        ) {
            printf("Nespravny vstup.\n");
            return 1;
        }
    }

    int ms1 = (((times[0] * 60) + times[1]) * 60 + times[2]) * 1000 + times[3];
    int ms2 = (((times[4] * 60) + times[5]) * 60 + times[6]) * 1000 + times[7];
    int millis = ms2 - ms1;

    if (millis < 0) {
        printf("Nespravny vstup.\n");
        return 1;
    }

    int secs = millis / 1000;
    millis %= 1000;

    int mins = secs / 60;
    secs %= 60;

    int hours = mins / 60;
    mins %= 60;

    printf("Doba: %2d:%02d:%02d,%03d\n", hours, mins, secs, millis);
    return 0;
}