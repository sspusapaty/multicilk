/*
 * Copyright (c) 1994-2003 Massachusetts Institute of Technology
 * Copyright (c) 2003 Bradley C. Kuszmaul
 * Copyright (c) 2013 I-Ting Angelina Lee and Tao B. Schardl 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
//#include "../cilk_threads.h"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include "util.h"
#include <sys/time.h>

#include "cilk_c11_threads.h"

struct args {
    pthread_t* main_thread;
};

__attribute__((weak)) void parallel_subroutine(void* n) {
  pthread_t* main_thread = ((struct args*)n)->main_thread;
  cilk_spawn dummy(n);
  dummy(n);
  cilk_sync;
  // We are inside a cilkified region. The cilk_thrd_current is always the same, but
  // we are no longer running on the boss thread. So pthread_self should not equal the main thread.
  int main_main = pthread_equal(cilk_thrd_current(), *main_thread);
  int curr_main = pthread_equal(pthread_self(), *main_thread);
  assert(main_main != 0);
  assert(curr_main == 0);
}

int dispatch(void *n) {
    pthread_t* main_thread = ((struct args*)n)->main_thread;
    int r = 0;

    parallel_subroutine(n);

    // After exiting the parallel subroutine, uncilkify should be called. 
    // We are, therefore, now executing on the boss thread. So pthread_self and the main_thread
    // should be equal.
    int main_main = pthread_equal(cilk_thrd_current(), *main_thread);
    int curr_main = pthread_equal(pthread_self(), *main_thread);
    assert(main_main != 0);
    assert(curr_main != 0);

    parallel_subroutine(n);

    return r;
}

bool one_thrd_current_equals_success(int num_workers) {
    pthread_t thread;

    struct args arg = {&thread};
    cilk_config_t cfg;
    cfg.n_workers = num_workers;
    int res = cilk_thrd_create(cfg, &thread, dispatch, (void*)&arg);
    assert(res == cilk_thrd_success);
    
    int answer;
    res = cilk_thrd_join(thread, &answer);
    assert(res == cilk_thrd_success);
   
    return true;
}

int main(int argc, char** argv) {

    printf("Running thrd_current and thrd_equals tests...\n");
    run_test("one_thrd_current_equals_success", one_thrd_current_equals_success, 2);
    run_test("one_thrd_current_equals_success", one_thrd_current_equals_success, 4);

    return 0;
}
