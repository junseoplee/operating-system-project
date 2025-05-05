#ifndef COMPRESS_H
#define COMPRESS_H

typedef struct {
    char ch;
    int count;
} Token;

int rle_compress_tokens(const char* input, int start, int end, Token* tokens);

#endif
