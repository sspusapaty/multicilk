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

volatile bool stop = false;

struct args {
    pthread_t* main_thread;
};

typedef struct {
    uint64_t* thread_ids;
    pthread_t boss_thread;
    int* thread_id_affinities_len;
    int** thread_id_affinities;
    int nthreads;
} cilk_debug_thread_info;


void print_cpuset_info (pthread_t thrd, cilk_debug_thread_info* info) {
    cpu_set_t mask;
    CPU_ZERO(&mask);

    int thread_index = __sync_fetch_and_add(&(info->nthreads), 1);

    info->thread_ids[thread_index] = pthread_self();


    //pthread_getaffinity_np(thrd, CPU_SETSIZE, &mask);
    pthread_getaffinity_np(thrd, sizeof(cpu_set_t), &mask);
    //printf("CPUSET for %llu:", thrd);
    int count = 0;
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &mask)) {
            count++;
            //printf("%d,", i);
        }
    }

    info->thread_id_affinities_len[thread_index] = count; 

    void* q = malloc(count*sizeof(int));

    info->thread_id_affinities[thread_index] = q;//malloc(count*sizeof(int));
    count = 0;
    for (int i = 0; i < CPU_SETSIZE; i++) {
        if (CPU_ISSET(i, &mask)) {
            info->thread_id_affinities[thread_index][count++] = i;
            //printf("%d,", i);
        }
    }



    //printf("\n");
}


void busy_loop(cilk_debug_thread_info* info) {
    int iteration = 0;
    if (!stop) {
      //printf("Thread name is %llu\n", pthread_self()); 
      print_cpuset_info(pthread_self(), info);
    }
   int a = 3;
   int b = 7;
   int sum = 1;
   while (!stop) {
        iteration++;
	sum = sum*a*b;
	//usleep(1000);
    }
    if (!stop) {
      printf("sum is %d\n", sum);
    }
    //printf("n iterations is %d\n", iteration);
}

cilk_debug_thread_info debug_info_arr[128];
int debug_info_arr_len = 0;

int multicilk_test_dispatch(void *n) {
    debug_info_arr_len = 0;
    stop = false;
    cilk_debug_thread_info* info = &(debug_info_arr[__sync_fetch_and_add(&debug_info_arr_len,1)]);
    info->thread_ids = (uint64_t*) calloc(1024, sizeof(uint64_t));
    info->thread_id_affinities_len = (int*) calloc(1024, sizeof(int));
    info->thread_id_affinities = (int**) calloc(1024, sizeof(int*));
    printf("thread_id_affinities %p\n", info->thread_id_affinities);
    printf("info %p\n", info);
    info->nthreads = 0;
    info->boss_thread = cilk_thrd_current();

    printf("run dispatch\n");
    for (int i = 0; i < 10000; i++) {
        cilk_spawn busy_loop(info);
    }
    cilk_sync;

    return 0;
}

int multicilk_test_dispatch_cpp(int n) {
	return multicilk_test_dispatch(NULL);
}
void debug_multicilk_stop() {
	stop = true;
}

void debug_report_cilk_threads(cilk_debug_thread_info* info) {
    printf("===Boss Thread ID    %lu===\n", info->boss_thread);
    printf("    Num Worker Threads: %d\n", info->nthreads);
    for (int i = 0; i < info->nthreads; i++) {
        printf("Worker: %d   thread_id:%lu\n", i, info->thread_ids[i]);
    }
    printf("Worker thread affinities\n");
    for (int i = 0; i < info->nthreads; i++) {
        printf("Worker: %d  affinities:[", i);
        for (int j = 0; j + 1 < info->thread_id_affinities_len[i]; j++) {
            printf("%d,", info->thread_id_affinities[i][j]);        
        }
        printf("%d]\n", info->thread_id_affinities[i][info->thread_id_affinities_len[i]-1]);
    }
}

void debug_report_multicilk_threads() {
    for (int i = 0; i < debug_info_arr_len; i++) {
        debug_report_cilk_threads(&(debug_info_arr[i]));
    }
}

