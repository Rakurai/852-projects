#include <stdio.h>

int main() {
    // are the bytes reversed?  check first byte of multi-byte int
    printf((char)(int)1 == 1 ? "little endian\n" : "big endian\n");
}
