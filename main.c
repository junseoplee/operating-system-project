#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "compress.h"

#define MAX_INPUT_SIZE 100000
#define MAX_TOKENS 10000

double get_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "사용법: %s <입력파일> <출력파일> <프로세스 수>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];
    int num_processes = atoi(argv[3]);

    FILE* in = fopen(input_file, "r");
    if (!in) {
        perror("입력 파일 열기 실패");
        return 1;
    }

    char* input = malloc(MAX_INPUT_SIZE);
    fread(input, sizeof(char), MAX_INPUT_SIZE, in);
    fclose(in);
    int input_len = strlen(input);
    int block_size = input_len / num_processes;

    Token** token_buffers = mmap(NULL, sizeof(Token*) * num_processes, PROT_READ | PROT_WRITE,
                                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* token_counts = mmap(NULL, sizeof(int) * num_processes, PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    for (int i = 0; i < num_processes; i++) {
        token_buffers[i] = mmap(NULL, sizeof(Token) * MAX_TOKENS, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        token_counts[i] = 0;
    }

    struct timeval start_time, end_time;
    struct rusage usage;
    gettimeofday(&start_time, NULL);

    for (int i = 0; i < num_processes; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            int start_idx = i * block_size;
            int end_idx = (i == num_processes - 1) ? input_len : (i + 1) * block_size;
            token_counts[i] = rle_compress_tokens(input, start_idx, end_idx, token_buffers[i]);
            exit(0);
        }
    }

    for (int i = 0; i < num_processes; i++) wait(NULL);

    Token merged[MAX_TOKENS];
    int merged_len = 0;
    for (int i = 0; i < num_processes; i++) {
        for (int j = 0; j < token_counts[i]; j++) {
            Token t = token_buffers[i][j];
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
    printf("\n=== 성능 측정 결과 ===\n");
    printf("총 실행 시간: %.6f초\n", get_time_diff(start_time, end_time));
    printf("Voluntary Context Switches   : %ld\n", usage.ru_nvcsw);
    printf("Involuntary Context Switches : %ld\n", usage.ru_nivcsw);

    free(input);
    for (int i = 0; i < num_processes; i++) {
        munmap(token_buffers[i], sizeof(Token) * MAX_TOKENS);
    }
    munmap(token_buffers, sizeof(Token*) * num_processes);
    munmap(token_counts, sizeof(int) * num_processes);
    return 0;
}
