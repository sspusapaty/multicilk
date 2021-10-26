#define _GNU_SOURCE
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

/* cilk thread error codes
 */
enum
{
  cilk_thrd_success  = 0,
  cilk_thrd_busy     = 1,
  cilk_thrd_error    = 2,
  cilk_thrd_nomem    = 3,
  cilk_thrd_timedout = 4
};

/* Creates a new pthread with a new cilk runtime (new set of workers) with the configuration
 * set by `config`. The pthread will call the function `func()` with `arg` as the argument.
 * The return value follows the documentation of `pthread_create()`
 */
__attribute__((unused))
int cilk_thrd_create(cilk_config_t config, pthread_t* thread, int (*func)(void*), void *arg);

/* Same as `cilk_thrd_create()` but can specify attributes of the newly created pthread.
 */
__attribute__((unused))
int cilk_thrd_create_with_attr(cilk_config_t config, pthread_t *thread, pthread_attr_t* attr, int (*func) (void*), void *arg);

/* Terminates the calling thread with the return code res.
 * Can only be called when 1 active worker exists. If called when
 * multiple workers exist, the behavior is undefined.
 * Does not support automatically calling the appropriate destructor
 * on data stored in tss.
 * Does not support automatically popping cleanup handlers from the
 * thread's stack and running them.
 */
__attribute__((unused))
void cilk_thrd_exit(int res);

__attribute__((unused))
int cilk_thrd_join (pthread_t thr, int *res);
