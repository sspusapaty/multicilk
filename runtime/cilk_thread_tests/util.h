#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
//#include "../cilk_threads.h"
#define ZERO 0

#ifndef CILK_THREAD_TEST_UTIL_H
#define CILK_THREAD_TEST_UTIL_H

unsigned long long todval (struct timeval *tp) {
    return tp->tv_sec * 1000 * 1000 + tp->tv_usec;
}

void run_test(char* name, bool (*func)(int), int arg) {
    struct timeval t1, t2;
    gettimeofday(&t1,0);
    bool res = func(arg);
    gettimeofday(&t2,0);
    unsigned long long runtime_ms = (todval(&t2)-todval(&t1))/1000;
    
    char* pass = "PASS";
    if (!res) pass = "FAIL";
    
    printf("TEST: %s(%d) = %s -- %f\n", name, arg, pass, runtime_ms/1000.0);
}

void __attribute__((weak)) dummy(void *p) { return; }


int fib(int n) {
    int x = 0;
    int y = 0;

    if(n < 2)
        return n;

    x = cilk_spawn fib(n-1);
    y = fib(n-2);
    //dummy(alloca(ZERO));
    cilk_sync;
    return x+y;
}
#endif // header guard
