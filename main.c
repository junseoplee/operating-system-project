#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include "compress.h"

#define MAX_INPUT_SIZE 10000
#define MAX_OUTPUT_SIZE 20000

typedef struct {
    char* input;
    char* output;
} CompressArgs;

void* compress_thread_func(void* arg) {
    CompressArgs* args = (CompressArgs*)arg;
    rle_compress(args->input, args->output);
    return NULL;
}

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

    char* input = malloc(MAX_INPUT_SIZE);
    char* output = malloc(MAX_OUTPUT_SIZE);
    fread(input, sizeof(char), MAX_INPUT_SIZE, in);
    fclose(in);

    pid_t pid = fork();
    if (pid == 0) { // child process
        pthread_t tid;
        CompressArgs args = { input, output };
        pthread_create(&tid, NULL, compress_thread_func, &args);
        pthread_join(tid, NULL);

        FILE* out = fopen(argv[2], "w");
        if (!out) {
            perror("출력 파일 열기 실패");
            return 1;
        }
        fwrite(output, sizeof(char), strlen(output), out);
        fclose(out);
        printf("[Child] 압축 완료\n");
        exit(0);
    } else if (pid > 0) {
        wait(NULL);
        printf("[Parent] 자식 프로세스 종료 대기 완료\n");
    } else {
        perror("fork 실패");
        return 1;
    }

    free(input);
    free(output);
    return 0;
}
