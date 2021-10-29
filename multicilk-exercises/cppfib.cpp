#include <cilk/cilk_cpp_threads.hpp>
#include "util.h"

const int fib_mem[] = {0,1,1,2,3,5,8,13};

int fib( int n ) {
    THREAD_PRINT("hello world from cilk!\n");
    if (n < 2) return fib_mem[n];
    
    int x = cilk_spawn fib(n-1);
    int y = fib(n-2);
    
    cilk_sync;

    return x+y;
}

void wrap_fib(int n, int* ret) {
    *ret = fib(n);
}

int main(int argc, char** argv) {
    THREAD_PRINT("hello world from main!\n");
    
    // get cilk runtime config
    cilk_config_t cfg = cilk_thrd_config_from_env("CILK_CONFIG1");
    CFG_PRINT(cfg);
    int n = 10;
    if (argc == 2) {
        n = atoi(argv[1]);
    }

    // create thread with new cilk runtime
    int ret = -1;
    std::thread thr = cilk_thread_create(cfg, wrap_fib, n, &ret);

    thr.join();

    printf("fib(%d)=%d\n", n, ret);
    
    return 0;
}
