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
#include <pthread.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include "util.h"
#include <sys/time.h>
#include <cilk/cilk_c11_threads.h>
#define GET_VARIABLE_NAME(Variable) (#Variable)

struct args {
    int val;
};

int dispatch(void *n) {
    struct args* x = (struct args*)n;
    int r = fib(x->val);
    cilk_thrd_exit(cilk_thrd_success);
    return r;
}

bool one_thrd_create_join_success(int num_workers) {
    pthread_t thread;
    struct args arg = {42};
    cilk_config_t cfg;
    cfg.n_workers = num_workers;
    int res = cilk_thrd_create(cfg, &thread, dispatch, (void*)&arg);
    assert(res == cilk_thrd_success);
    
    int answer;
    res = cilk_thrd_join(thread, &answer);
    assert(res == cilk_thrd_success);
    printf("answer %d\n", answer);   
    return answer == cilk_thrd_success;
}

int main(int argc, char** argv) {

    printf("Running thrd_exit tests...\n");
    run_test("one_thrd_create_join_success", one_thrd_create_join_success, 4);

    return 0;
}
