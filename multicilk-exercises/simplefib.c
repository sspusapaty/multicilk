#include <cilk/cilk_c11_threads.h>
#include "util.h"

typedef struct {
    int n;
    int tag;
} fib_args;

const int fib_mem[] = {0,1,1,2,3,5,8,13};

int fib( void* arg ) {
    int n = ((fib_args*)arg)->n;
    int tag = ((fib_args*)arg)->tag;
    // will print thread id, cpu id when compiled with PRINT=1; 
    THREAD_PRINT("hello world from cilk# %d!\n", tag);
    
    if (n < 2) return fib_mem[n];
    
    fib_args fx = {.n = n-1, .tag = tag };
    int x = cilk_spawn fib(&fx);
    fib_args fy = {.n = n-2, .tag = tag };
    int y = fib(&fy);
    
    cilk_sync;

    return x+y;
}

int main(int argc, char** argv) {
    THREAD_PRINT("hello world from main!\n");
    
    // get cilk runtime config
    cilk_config_t cfgx = cilk_thrd_config_from_env("CILK_CONFIG1");
    cilk_config_t cfgy = cilk_thrd_config_from_env("CILK_CONFIG2");
    CFG_PRINT(cfgx);
    CFG_PRINT(cfgy);
    
    // get input from cmd line if exists, otherwise use 10
    pthread_t thr1, thr2;
    int n = 10;
    if (argc == 2) {
        n = atoi(argv[1]);
    }

    fib_args x = {.n = n, .tag = 1};
    fib_args y = {.n = n, .tag = 2};
    
    // create thread with new cilk runtime
    int err = cilk_thrd_create(cfgx, &thr1, fib, &x);
    int err2 = cilk_thrd_create(cfgy, &thr2, fib, &y);
    if (err || err2) {
        printf("thrd creation not successful!\n");
        return -1;
    }
    
    // join threads to get result;
    int res;
    cilk_thrd_join(thr1, &res);
    cilk_thrd_join(thr2, NULL);

    printf("fib(%d) = %d\n", n, res);
    
    return 0;
}
