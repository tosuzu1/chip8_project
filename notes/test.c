#include <stdio.h>
#include <stdint.h>

int main() {
    uint8_t a = 7;
    double b = 0;
    b = (double)a/60;
    printf("██uble = %lf\n", b);
    perror("This is a ERror\n");
    printf("did something\n");
}