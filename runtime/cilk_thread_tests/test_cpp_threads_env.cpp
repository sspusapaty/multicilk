#include <iostream>
#include <thread>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <functional>
//#include <numa.h>
#include "multicilk_tools.h"
using namespace std;
//bool stop = false;
//void print_cpuset_info (pthread_t thrd) {
//    cpu_set_t mask;
//    CPU_ZERO(&mask);
//    //pthread_getaffinity_np(thrd, CPU_SETSIZE, &mask);
//    pthread_getaffinity_np(thrd, sizeof(cpu_set_t), &mask);
//    printf("CPUSET for %llu:", thrd);
//    for (int i = 0; i < CPU_SETSIZE; i++) {
//        if (CPU_ISSET(i, &mask)) {
//            printf("%d,", i);
//        }
//    }
//    printf("\n");
//}
//
//void busy_loop() {
//    int iteration = 0;
//    if (!stop) {
//    printf("Thread name is %llu\n", pthread_self()); 
//    print_cpuset_info(pthread_self());
//    }
//   while (!stop) {
//        iteration++;
//	usleep(1000);
//    }
//    //printf("n iterations is %d\n", iteration);
//}
//
//void cilk_thread (int a) {
//	cilk_spawn printf("hello %d\n", a);
//	printf("there %d\n", a);
//	cilk_sync;
//	for (int i = 0; i < 1000; i++) {
//		cilk_spawn busy_loop();
//	}
//	cilk_sync;
//}

struct cilk_wrapper_helper {
	cilk_config_t config;
	template <class F, class... Args>
	void operator()(F&& f, Args&&... args) const {
		cilk_thrd_init(config);
		f(std::forward<Args>(args)...);
		//cilk_thread_wrapper(std::forward<F>(f), std::forward<Args>(args)...);
	}
};

template <class F, class... Args>
std::thread cilk_thread_create(cilk_config_t config, F&& f, Args&&... args) {
	thread thr(cilk_wrapper_helper{config}, f, std::forward<Args>(args)...);
	return thr;
}


int main() {
    //cout << "Numa available " << numa_available() << endl;
    	cilk_config_t config1 = cilk_thrd_config_from_env("CILK_CONFIG_1");
    	cilk_config_t config2 = cilk_thrd_config_from_env("CILK_CONFIG_2");

	//config1.n_workers = 4;
	//config2.n_workers = 8;
	cout << "Starting cpp threading test " << endl;


	// Interface possibility (1)
	thread thr(cilk_wrapper_helper{config1}, multicilk_test_dispatch_cpp, 3);
	sleep(3);
	debug_multicilk_stop();
	thr.join();
        debug_report_multicilk_threads();

	// Interface possibility (2)
	thread thr2 = cilk_thread_create(config2, multicilk_test_dispatch_cpp, 4);//(cilk_wrapper_helper{config2}, cilk_thread, 4);
	sleep(3);
	debug_multicilk_stop();
	thr2.join();
        debug_report_multicilk_threads();
	return 0;
}
