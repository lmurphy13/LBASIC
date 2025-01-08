#include "lbasic_print.h"

#include <stdio.h>

void print(char *str) {
    printf("%s", str);
}

void println(char *str) {
    printf("%s\n", str);
}

void printint(int num) {
    printf("%d", num);
}

void printfloat(float num) {
    printf("%f", num);
}

// int main(void) {
//     int a = 45 + 5;
//     printint(a);
// }