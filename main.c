#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
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

double get_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <입력파일> <출력파일>\n", argv[0]);
        return 1;
    }

    struct timeval start_time, end_time;
    struct rusage usage;

    gettimeofday(&start_time, NULL);  // 시작 시간 측정

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
    if (pid == 0) { // 자식 프로세스
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
        exit(0);
    } else if (pid > 0) {
        wait(NULL);
    } else {
        perror("fork 실패");
        return 1;
    }

    gettimeofday(&end_time, NULL); // 종료 시간 측정
    getrusage(RUSAGE_SELF, &usage); // context switch 측정

    printf("\n=== 내부 성능 측정 결과 ===\n");
    printf("총 실행 시간: %.6f초\n", get_time_diff(start_time, end_time));
    printf("Voluntary Context Switches   : %ld\n", usage.ru_nvcsw);
    printf("Involuntary Context Switches : %ld\n", usage.ru_nivcsw);

    free(input);
    free(output);
    return 0;
}
