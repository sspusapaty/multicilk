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
#include "cilk_c11_threads.h"
#include <sys/time.h>

struct args {
    int val;
};

struct args global_args = {42};

int dispatch(void *n) {
    struct args* x = (struct args*)n;
    int r = fib(x->val);
    return r;
}

bool one_thrd_create_join_success(int num_workers) {
    pthread_t thread;
    struct args arg = {42};
    cilk_config_t cfg;
    cfg.n_workers = num_workers;

    int res = cilk_thrd_create(cfg, &thread, dispatch, (void*)&arg);
    assert(res == cilk_thrd_success);
    
    int answer = 0;
    
    res = cilk_thrd_join(thread, &answer);
    assert(res == cilk_thrd_success);
    printf("answer is %d\n", answer); 
    return answer == 267914296;
}


// Note tests with detach need to store the arguments either on heap or in globals.
bool one_thrd_create_detach_success(int num_workers) {
    pthread_t thread;
    cilk_config_t cfg;
    cfg.n_workers = num_workers;
    int res = cilk_thrd_create(cfg, &thread, dispatch, (void*)&global_args);
    assert(res == cilk_thrd_success);
    res = pthread_detach(thread);
    assert(res == cilk_thrd_success);
   
    return true;
}

bool three_thrd_create_join_success(int num_workers) {
    pthread_t thread1, thread2, thread3;
    struct args arg = {42};
    cilk_config_t cfg;
    cfg.n_workers = num_workers;
    int res1 = cilk_thrd_create(cfg, &thread1, dispatch, (void*)&arg);
    int res2 = cilk_thrd_create(cfg, &thread2, dispatch, (void*)&arg);
    int res3 = cilk_thrd_create(cfg, &thread3, dispatch, (void*)&arg);
    assert(res1 == cilk_thrd_success &&
           res2 == cilk_thrd_success &&
           res3 == cilk_thrd_success);
    
    int answer1, answer2, answer3;
    res1 = cilk_thrd_join(thread1, &answer1);
    res2 = cilk_thrd_join(thread2, &answer2);
    res3 = cilk_thrd_join(thread3, &answer3);
    assert(res1 == cilk_thrd_success &&
           res2 == cilk_thrd_success &&
           res3 == cilk_thrd_success);
   
    return (answer1 == 267914296) && (answer2 == 267914296) && (answer3 == 267914296);
}

bool three_thrd_create_detach_success(int num_workers) {
    pthread_t thread1, thread2, thread3;
    cilk_config_t cfg;
    cfg.n_workers = num_workers;
    int res1 = cilk_thrd_create(cfg, &thread1, dispatch, (void*)&global_args);
    int res2 = cilk_thrd_create(cfg, &thread2, dispatch, (void*)&global_args);
    int res3 = cilk_thrd_create(cfg, &thread3, dispatch, (void*)&global_args);
    assert(res1 == cilk_thrd_success &&
           res2 == cilk_thrd_success &&
           res3 == cilk_thrd_success);
    
    res1 = pthread_detach(thread1);
    res2 = pthread_detach(thread2);
    res3 = pthread_detach(thread3);
    assert(res1 == cilk_thrd_success &&
           res2 == cilk_thrd_success &&
           res3 == cilk_thrd_success);
   
    return true;
}

int main(int argc, char** argv) {
    printf("Running thrd_create, thrd_join, thrd_detach tests...\n");
    run_test("one_thrd_create_join_success", one_thrd_create_join_success, 4);
    run_test("one_thrd_create_join_success", one_thrd_create_join_success, 100);
    run_test("three_thrd_create_join_success", three_thrd_create_join_success, 4);
    run_test("one_thrd_create_detach_success", one_thrd_create_detach_success, 4);
    run_test("three_thrd_create_detach_success", three_thrd_create_detach_success, 4);
    //// Sleep to catch bad behavior with detached threads.
    sleep(10);
    return 0;
}
