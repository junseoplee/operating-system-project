#include <stdio.h>
#include <string.h>
#include "compress.h"

void rle_compress(const char* input, char* output) {
    int count = 1;
    int j = 0;

    for (int i = 1; input[i - 1] != '\0'; i++) {
        if (input[i] == input[i - 1]) {
            count++;
        } else {
            j += sprintf(&output[j], "%c%d", input[i - 1], count);
            count = 1;
        }
    }
}
