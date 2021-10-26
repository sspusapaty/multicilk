#define _GNU_SOURCE
#include <assert.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/cilk_c11_threads.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

__attribute__((unused))
static __always_inline int
cilk_thrd_err_map (int err_code)
{
  switch (err_code)
  {
    case 0:
      return cilk_thrd_success;
    case ENOMEM:
      return cilk_thrd_nomem;
    case ETIMEDOUT:
      return cilk_thrd_timedout;
    case EBUSY:
      return cilk_thrd_busy;
    default:
      return cilk_thrd_error;
  }
}

/* General wrapper function to create cilk runtime object and set it's
 * destructor before calling the thrd_args function member.
 */
__attribute__((unused))
static void* cilk_thrd_c11_wrapper(void* args) {
    struct cilk_c11_thrd_args* c11_args = (struct cilk_c11_thrd_args*) args;
    cilk_thrd_init(c11_args->config);
    void* f_arg = c11_args->arg;
    int(*func)(void*) = c11_args->func;
    free(c11_args);
    return (void*) ((int64_t) func(f_arg));
}

/* Creates a new pthread with a new cilk runtime (new set of workers) with the configuration
 * set by `config`. The pthread will call the function `func()` with `arg` as the argument.
 * The return value follows the documentation of `pthread_create()`
 */
__attribute__((unused))
int cilk_thrd_create(cilk_config_t config, pthread_t* thread, int (*func)(void*), void *arg) {
    struct cilk_c11_thrd_args* c11_args = (struct cilk_c11_thrd_args*) malloc(sizeof(struct cilk_c11_thrd_args));
    c11_args->arg = arg;
    c11_args->func = func;
    c11_args->config = config;
    return pthread_create(thread, NULL, cilk_thrd_c11_wrapper, c11_args); 
}

/* Same as `cilk_thrd_create()` but can specify attributes of the newly created pthread.
 */
__attribute__((unused))
int cilk_thrd_create_with_attr(cilk_config_t config, pthread_t *thread, pthread_attr_t* attr, int (*func) (void*), void *arg) {
    struct cilk_c11_thrd_args* c11_args = (struct cilk_c11_thrd_args*) malloc(sizeof(struct cilk_c11_thrd_args));
    c11_args->arg = arg;
    c11_args->func = func;
    c11_args->config = config;
    return pthread_create(thread, attr, cilk_thrd_c11_wrapper, c11_args);
}

/* Terminates the calling thread with the return code res.
 * Cannot be called by an active cilk worker. If called by a worker,
 * the behavior is undefined.
 * Does not support automatically calling the appropriate destructor
 * on data stored in tss.
 * Does not support automatically popping cleanup handlers from the
 * thread's stack and running them.
 */
__attribute__((unused))
void cilk_thrd_exit(int res) {
    pthread_exit((void*)((int64_t)res)); 
}

__attribute__((unused))
int cilk_thrd_join (pthread_t thr, int *res)
{
  void *pthread_res;
  int err_code = pthread_join(thr, &pthread_res);
  /*
  	Note replacing the below code with:
  		if (res)
  		 *res = (int) ((int64_t) pthread_res);
  		return cilk_thrd_err_map (err_code);  

	Results in an uninitialized value. 
  */ 
  int coerced_res = (int)((int64_t)pthread_res);
  if (res)
   *res = coerced_res;
  return cilk_thrd_err_map (err_code);
}
