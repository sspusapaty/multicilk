#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/cilk_c11_threads.h>
#include <errno.h>
#include <threads.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

__attribute__((unused))
static __always_inline bool
is_cilk_worker() {
    return worker_current() != thrd_current();
}

/* General wrapper function to create cilk runtime object and set it's
 * destructor before calling the thrd_args function member.
 */
__attribute__((unused))
static int cilk_thrd_c11_wrapper(void* args) {
    struct cilk_c11_thrd_args* c11_args = (struct cilk_c11_thrd_args*) args;
    cilk_thrd_init(c11_args->config);
    void* f_arg = c11_args->arg;
    thrd_start_t func = c11_args->func;
    free(c11_args);
    func(f_arg);
    return thrd_success;
}

/* Creates a new pthread with a new cilk runtime (new set of workers) with the configuration
 * set by `config`. The pthread will call the function `func()` with `arg` as the argument.
 * The return value follows the documentation of `pthread_create()`
 */
static int (*real_thrd_create)(thrd_t*, thrd_start_t, void*) = NULL;
__attribute__((unused))
int thrd_create(thrd_t* thr, thrd_start_t f, void* args)
{
    if (!real_thrd_create)
        real_thrd_create = dlsym(RTLD_NEXT, "thrd_create");
    int res = real_thrd_create(thr, f, args);
    if (res == thrd_success & is_cilk_worker()) {
        cpu_set_t cpuset;
        pthread_getaffinity_np(thrd_current(), sizeof(cpu_set_t), &cpuset);
        pthread_setaffinity_np(*thr, sizeof(cpu_set_t), &cpuset);
    }
    return res;
}

__attribute__((unused))
int cilk_thrd_create(cilk_config_t config, thrd_t* thread, thrd_start_t func, void *arg) {
    struct cilk_c11_thrd_args* c11_args = (struct cilk_c11_thrd_args*) malloc(sizeof(struct cilk_c11_thrd_args));
    c11_args->arg = arg;
    c11_args->func = func;
    c11_args->config = config;
    return thrd_create(thread, cilk_thrd_c11_wrapper, c11_args); 
}


/* Terminates the calling thread with the return code res.
 * Cannot be called by an active cilk worker. If called by a worker,
 * the behavior is undefined.
 * Does not support automatically calling the appropriate destructor
 * on data stored in tss.
 * Does not support automatically popping cleanup handlers from the
 * thread's stack and running them.
 */
static void (*real_thrd_exit)(int) = NULL;
__attribute__((unused))
void thrd_exit(int res)
{
    if (!real_thrd_exit)
        real_thrd_exit = dlsym(RTLD_NEXT, "thrd_exit");
    if (!is_cilk_worker())
        real_thrd_exit(res);
    real_thrd_exit(res); // should be modified later once semantics are more clear
}

static int (*real_thrd_join)(thrd_t, int*) = NULL;
__attribute__((unused))
int thrd_join (thrd_t thr, int *res)
{
    if (!real_thrd_join)
        real_thrd_join = dlsym(RTLD_NEXT, "thrd_join");
    if (is_cilk_worker() && thr == thrd_current())
        return thrd_error;
    return real_thrd_join(thr, res);
}

__attribute__((unused))
int worker_join (thrd_t thr, int *res)
{
    if (!real_thrd_join)
        real_thrd_join = dlsym(RTLD_NEXT, "thrd_join");
    return real_thrd_join(thr, res);
}

__attribute__((unused))
void worker_exit (int res)
{
    if (!real_thrd_exit)
        real_thrd_exit = dlsym(RTLD_NEXT, "thrd_exit");
    return thrd_exit(res);
}
