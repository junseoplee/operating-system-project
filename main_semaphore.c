#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <semaphore.h>
#include "compress.h"

#define MAX_INPUT_SIZE 2000000
#define MAX_TOKENS 100000
#define MAX_THREADS 64
#define MAX_PROCESSES 32

char* input;
int input_len;
int total_blocks;

typedef struct {
    int thread_id;
    int start_idx;
    int end_idx;
    int* count;
    sem_t* sem;
    Token* token_array;
    int* token_count;
} ThreadArg;

double get_time_diff(struct timeval start, struct timeval end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
}

void* thread_worker(void* arg) {
    ThreadArg* t = (ThreadArg*)arg;
    int tid = t->thread_id;

    t->token_count[tid] = rle_compress_tokens(input, t->start_idx, t->end_idx, &t->token_array[tid * MAX_TOKENS]);

    sem_wait(t->sem);
    (*t->count)++;
    sem_post(t->sem);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        fprintf(stderr, "사용법: %s <입력파일> <출력파일> <프로세스 수> <스레드 수>\n", argv[0]);
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];
    int num_procs = atoi(argv[3]);
    int num_threads = atoi(argv[4]);
    total_blocks = num_procs * num_threads;

    FILE* in = fopen(input_file, "r");
    if (!in) {
        perror("입력 파일 열기 실패");
        return 1;
    }

    input = malloc(MAX_INPUT_SIZE);
    fread(input, sizeof(char), MAX_INPUT_SIZE, in);
    fclose(in);
    input_len = strlen(input);
    int block_size = input_len / total_blocks;

    int* count = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *count = 0;

    sem_t* sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(sem, 1, 1); // 공유 세마포어

    Token* shared_tokens = mmap(NULL, sizeof(Token) * MAX_TOKENS * total_blocks, PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int* shared_token_counts = mmap(NULL, sizeof(int) * total_blocks, PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    struct timeval start_time, end_time;
    struct rusage usage;
    gettimeofday(&start_time, NULL);

    for (int p = 0; p < num_procs; p++) {
        pid_t pid = fork();
        if (pid == 0) {
            pthread_t threads[MAX_THREADS];
            ThreadArg args[MAX_THREADS];
            for (int t = 0; t < num_threads; t++) {
                int block_id = p * num_threads + t;
                args[t].thread_id = block_id;
                args[t].start_idx = block_id * block_size;
                args[t].end_idx = (block_id == total_blocks - 1) ? input_len : (block_id + 1) * block_size;
                args[t].count = count;
                args[t].sem = sem;
                args[t].token_array = shared_tokens;
                args[t].token_count = shared_token_counts;
                pthread_create(&threads[t], NULL, thread_worker, &args[t]);
            }
            for (int t = 0; t < num_threads; t++) {
                pthread_join(threads[t], NULL);
            }
            exit(0);
        }
    }

    for (int p = 0; p < num_procs; p++) {
        wait(NULL);
    }

    FILE* out = fopen(output_file, "w");
    if (!out) {
        perror("출력 파일 열기 실패");
        return 1;
    }

    Token prev = {'\0', 0};
    for (int i = 0; i < total_blocks; i++) {
        for (int j = 0; j < shared_token_counts[i]; j++) {
            Token current = shared_tokens[i * MAX_TOKENS + j];
            if (prev.ch == current.ch) {
                prev.count += current.count;
            } else {
                if (prev.ch != '\0') {
                    fprintf(out, "%c%d", prev.ch, prev.count);
                }
                prev = current;
            }
        }
    }
    if (prev.ch != '\0') {
        fprintf(out, "%c%d", prev.ch, prev.count);
    }
    fclose(out);

    gettimeofday(&end_time, NULL);
    getrusage(RUSAGE_SELF, &usage);

    printf("\n=== 성능 측정 결과 (semaphore 적용) ===\n");
    printf("총 실행 시간: %.6f초\n", get_time_diff(start_time, end_time));
    printf("Voluntary Context Switches   : %ld\n", usage.ru_nvcsw);
    printf("Involuntary Context Switches : %ld\n", usage.ru_nivcsw);
    printf("압축 성공 블록 수 (count)   : %d / %d\n", *count, total_blocks);

    if (*count == total_blocks) {
        printf("동기화 성공\n");
    } else {
        printf("동기화 실패 또는 코드 오류\n");
    }

    free(input);
    sem_destroy(sem);
    munmap(sem, sizeof(sem_t));
    munmap(count, sizeof(int));
    munmap(shared_tokens, sizeof(Token) * MAX_TOKENS * total_blocks);
    munmap(shared_token_counts, sizeof(int) * total_blocks);
    return 0;
}
