#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <cilk/cilk_api.h>
#include <sys/time.h>

#ifdef PRINT
#define MACRO_PRINT(self,...) { \
    char buff[200]; \
    snprintf(buff, 200, "Thread ID = %lu, Core = %d:\n", self, sched_getcpu()); \
    char buff2[1024];\
    snprintf(buff2, 1024, __VA_ARGS__);\
    printf("%s%s",buff,buff2); \
    }


#define THREAD_PRINT(...) MACRO_PRINT(thrd_current(),__VA_ARGS__);

#define SELF_PRINT(...) MACRO_PRINT(pthread_self(),__VA_ARGS__);

#define PRINT_CPUSET(cpuset) \
    int count = CPU_COUNT(&cpuset); \
    printf("[");\
    for (int i = 0; i < CPU_SETSIZE; ++i) { \
        if (CPU_ISSET(i, &cpuset)) {\
            printf("%d",i);\
            if (--count) printf(",");\
        }\
    }\
    printf("]");

#define CFG_PRINT(cfg) {\
    printf("nworkers = %d; cpuset = ",cfg.n_workers);\
    PRINT_CPUSET(cfg.boss_affinity);\
    printf("\n");\
}
#else
#define THREAD_PRINT(...)
#define SELF_PRINT(...)
#define PRINT_CPUSET(cpuset)
#define CFG_PRINT(cfg)
#endif
