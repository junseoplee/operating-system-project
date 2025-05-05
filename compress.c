#include <stdio.h>
#include "compress.h"

int rle_compress_tokens(const char* input, int start, int end, Token* tokens) {
    if (start >= end) return 0;

    int token_idx = 0;
    char current = input[start];
    int count = 1;

    for (int i = start + 1; i < end; i++) {
        if (input[i] == current) {
            count++;
        } else {
            tokens[token_idx].ch = current;
            tokens[token_idx].count = count;
            token_idx++;
            current = input[i];
            count = 1;
        }
    }

    tokens[token_idx].ch = current;
    tokens[token_idx].count = count;
    token_idx++;

    return token_idx;
}
