#ifndef _CILK_C11_H
#define  _CILK_C11_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <assert.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

/* wrapper struct to hold function and args to pass to
 * newly created pthread
 */
struct cilk_c11_thrd_args {
    int(*func)(void*);
    void *arg;
    cilk_config_t config;
};

/* Creates a new pthread with a new cilk runtime (new set of workers) with the configuration
 * set by `config`. The pthread will call the function `func()` with `arg` as the argument.
 * The return value follows the documentation of `pthread_create()`
 */
__attribute__((unused))
int cilk_thrd_create(cilk_config_t config, thrd_t* thread, int (*func)(void*), void *arg);

/* Terminates the calling thread with the return code res.
 * Cannot be called by an active cilk worker.
 * If called by a cilk worker the behavior is a no-op.
 */
__attribute__((unused))
void thrd_exit(int res);

__attribute__((unused))
int thrd_join(thrd_t thr, int *res);

__attribute__((unused))
void worker_exit(int res);

__attribute__((unused))
int worker_join(thrd_t thr, int *res);
#endif // CILK_C11_H
