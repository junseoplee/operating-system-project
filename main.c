#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compress.h"

#define MAX_INPUT_SIZE 10000
#define MAX_OUTPUT_SIZE 20000

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <입력파일> <출력파일>\n", argv[0]);
        return 1;
    }

    FILE* in = fopen(argv[1], "r");
    if (!in) {
        perror("입력 파일 열기 실패");
        return 1;
    }

    char input[MAX_INPUT_SIZE];
    fread(input, sizeof(char), MAX_INPUT_SIZE, in);
    fclose(in);

    char output[MAX_OUTPUT_SIZE];
    memset(output, 0, sizeof(output));
    rle_compress(input, output);

    FILE* out = fopen(argv[2], "w");
    if (!out) {
        perror("출력 파일 열기 실패");
        return 1;
    }

    fwrite(output, sizeof(char), strlen(output), out);
    fclose(out);

    printf("압축 완료: %s → %s\n", argv[1], argv[2]);
    return 0;
}
