#include <stdio.h>
#include "compress.h"

void rle_compress_range(const char* input, int start, int end, char* output) {
    int out_index = 0;
    int count = 1;

    for (int i = start + 1; i < end; i++) {
        if (input[i] == input[i - 1]) {
            count++;
        } else {
            out_index += sprintf(&output[out_index], "%c%d", input[i - 1], count);
            count = 1;
        }
    }
    out_index += sprintf(&output[out_index], "%c%d", input[end - 1], count);
    output[out_index] = '\0';
}
