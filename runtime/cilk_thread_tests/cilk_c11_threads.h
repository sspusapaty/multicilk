#define _GNU_SOURCE
#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
//#include "util.h"
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <cilk/cilk_api.h>

//typedef int cilk_place_t;

struct cilk_c11_thrd_args {
    int(*func)(void*);
    void *arg;
    cilk_config_t config;
};

typedef struct cilk_thrd_args cilk_thrd_args;

enum
{
  cilk_thrd_success  = 0,
  cilk_thrd_busy     = 1,
  cilk_thrd_error    = 2,
  cilk_thrd_nomem    = 3,
  cilk_thrd_timedout = 4
};

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

__attribute__((unused))
static void* cilk_thrd_c11_wrapper(void* args) {
    struct cilk_c11_thrd_args* c11_args = (struct cilk_c11_thrd_args*) args;
    cilk_thrd_init(c11_args->config);
    void* f_arg = c11_args->arg;
    int(*func)(void*) = c11_args->func;
    free(c11_args);
    return (void*) ((int64_t) func(f_arg));
}

__attribute__((unused))
static int cilk_thrd_create(cilk_config_t config, pthread_t* thread, int (*func)(void*), void *arg) {
    struct cilk_c11_thrd_args* c11_args = (struct cilk_c11_thrd_args*) malloc(sizeof(struct cilk_c11_thrd_args));
    c11_args->arg = arg;
    c11_args->func = func;
    c11_args->config = config;
    return pthread_create(thread, NULL, cilk_thrd_c11_wrapper, c11_args); 
}


__attribute__((unused))
static int cilk_thrd_create_with_attr(cilk_config_t config, pthread_t *thread, pthread_attr_t* attr, int (*func) (void*), void *arg) {
    struct cilk_c11_thrd_args* c11_args = (struct cilk_c11_thrd_args*) malloc(sizeof(struct cilk_c11_thrd_args));
    c11_args->arg = arg;
    c11_args->func = func;
    c11_args->config = config;
    return pthread_create(thread, attr, cilk_thrd_c11_wrapper, c11_args);
}

// Can only be called when 1 active worker exists
__attribute__((unused))
static void cilk_thrd_exit(int res) {
    pthread_exit((void*)((int64_t)res)); 
}

__attribute__((unused))
static int cilk_thrd_join (pthread_t thr, int *res)
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
