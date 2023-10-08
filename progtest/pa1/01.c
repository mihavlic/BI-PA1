#include <stdio.h>

int main() {
  printf("ml' nob:\n");

  int i = 0;
  char c[2] = {};
  if (scanf("%d %1s", &i, c) < 1) {
    printf("Neh mi'\n");
    return 1;
  }

  if (c[0] != '\0') {
    printf("bIjatlh 'e' yImev\n");
    return 1;
  }

  if (i < 0 || i > 8) {
    printf("Qih mi' %d\n", i);
    return 1;
  }

  printf("Qapla'\n");

  const char *responses[] = {
      "noH QapmeH wo' Qaw'lu'chugh yay chavbe'lu' 'ej wo' choqmeH may' DoHlu'chugh lujbe'lu'.",
      "bortaS bIr jablu'DI' reH QaQqu' nay'.",
      "Qu' buSHa'chugh SuvwI', batlhHa' vangchugh, qoj matlhHa'chugh, pagh ghaH SuvwI''e'.",
      "bISeH'eghlaH'be'chugh latlh Dara'laH'be'.",
      "qaStaHvIS wa' ram loS SaD Hugh SIjlaH qetbogh loD.",
      "Suvlu'taHvIS yapbe' HoS neH.",
      "Ha'DIbaH DaSop 'e' DaHechbe'chugh yIHoHQo'.",
      "Heghlu'meH QaQ jajvam.",
      "leghlaHchu'be'chugh mIn lo'laHbe' taj jej.",
  };

  printf("%s\n", responses[i]);
  return 0;
}