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
#include <assert.h>
#include <stdbool.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <sys/time.h>
#include <time.h>

#include <inttypes.h>

#include "cilk_c11_threads.h"

#include "multicilk_tools.h"


struct args {
    pthread_t* main_thread;
};

bool test_multicilk_sizes() {
    printf("testing multicilk sizes\n");
    //pthread_t thread1, thread2, thread3;
    pthread_t thread1;

    struct args arg = {};


    cilk_config_t cfg = cilk_thrd_config_from_env("CILK_CONFIG_1"); 

    int res = cilk_thrd_create(cfg, &thread1, multicilk_test_dispatch, &arg);
    assert(res == cilk_thrd_success);
    int answer;
    sleep(4);
    debug_multicilk_stop();
    res = cilk_thrd_join(thread1, &answer);
    debug_report_multicilk_threads();
    return true;
}




int main(int argc, char** argv) {

    printf("Running thrd_create, thrd_join, thrd_detach tests...\n");
    test_multicilk_sizes();
    printf("Done\n");
    //run_test("one_thrd_create_join_success", one_thrd_create_join_success, 2);
    //run_test("one_thrd_create_join_success", one_thrd_create_join_success, 4);
    //run_test("three_thrd_create_join_success", three_thrd_create_join_success, 4);
    //run_test("one_thrd_create_detach_success", one_thrd_create_detach_success, 4);
    //run_test("three_thrd_create_detach_success", three_thrd_create_detach_success, 4);

    return 0;
}
