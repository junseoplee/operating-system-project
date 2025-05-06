#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "compress.h"

#define MAX_INPUT_SIZE 100000
#define MAX_TOKENS 10000
#define MAX_THREADS 32

char* input;
int input_len;
int block_size;
int num_threads;

Token tokens[MAX_THREADS][MAX_TOKENS];
int token_counts[MAX_THREADS];

typedef struct {
    int thread_id;
    int start_idx;
    int end_idx;
} ThreadArg;

double get_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

void* thread_worker(void* arg) {
    ThreadArg* t = (ThreadArg*)arg;
    int tid = t->thread_id;
    token_counts[tid] = rle_compress_tokens(input, t->start_idx, t->end_idx, tokens[tid]);
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "사용법: %s <입력파일> <출력파일> <스레드 수>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];
    num_threads = atoi(argv[3]);

    FILE* in = fopen(input_file, "r");
    if (!in) {
        perror("입력 파일 열기 실패");
        return 1;
    }

    input = malloc(MAX_INPUT_SIZE);
    fread(input, sizeof(char), MAX_INPUT_SIZE, in);
    fclose(in);
    input_len = strlen(input);
    block_size = input_len / num_threads;

    pthread_t threads[MAX_THREADS];
    ThreadArg args[MAX_THREADS];

    struct timeval start_time, end_time;
    struct rusage usage;
    gettimeofday(&start_time, NULL);

    for (int i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].start_idx = i * block_size;
        args[i].end_idx = (i == num_threads - 1) ? input_len : (i + 1) * block_size;
        pthread_create(&threads[i], NULL, thread_worker, &args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    Token merged[MAX_TOKENS];
    int merged_len = 0;
    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < token_counts[i]; j++) {
            Token t = tokens[i][j];
            if (merged_len > 0 && merged[merged_len - 1].ch == t.ch) {
                merged[merged_len - 1].count += t.count;
            } else {
                merged[merged_len++] = t;
            }
        }
    }

    FILE* out = fopen(output_file, "w");
    if (!out) {
        perror("출력 파일 열기 실패");
        return 1;
    }

    for (int i = 0; i < merged_len; i++) {
        fprintf(out, "%c%d", merged[i].ch, merged[i].count);
    }
    fclose(out);

    gettimeofday(&end_time, NULL);
    getrusage(RUSAGE_SELF, &usage);
    printf("\n=== 성능 측정 결과 (스레드 기반) ===\n");
    printf("총 실행 시간: %.6f초\n", get_time_diff(start_time, end_time));
    printf("Voluntary Context Switches   : %ld\n", usage.ru_nvcsw);
    printf("Involuntary Context Switches : %ld\n", usage.ru_nivcsw);

    free(input);
    return 0;
}
