#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <cilk/cilk_api.h>
#include <sys/time.h>

#ifdef PRINT
#define THREAD_PRINT(...) \
    printf("Thread ID = %lu, Core = %d:\n", cilk_thrd_current(), sched_getcpu()); \
    printf(__VA_ARGS__);

#define SELF_PRINT(...) \
    printf("self ID = %lu, Core = %d:\n", pthread_self(), sched_getcpu()); \
    printf(__VA_ARGS__);

#define PRINT_CPUSET(cpuset) \
    int count = CPU_COUNT(&cpuset); \
    printf("[");\
    for (int i = 0; i < CPU_SETSIZE; ++i) { \
        if (CPU_ISSET(i, &cpuset)) {\
            printf("%d",i);\
            if (i < count-1) printf(",");\
        }\
    }\
    printf("]");

#define CFG_PRINT(cfg) \
    printf("nworkers = %d; cpuset = ",cfg.n_workers);\
    PRINT_CPUSET(cfg.boss_affinity);\
    printf("\n");
#else
#define THREAD_PRINT(...)
#define SELF_PRINT(...)
#define PRINT_CPUSET(cpuset)
#define CFG_PRINT(cfg)
#endif
