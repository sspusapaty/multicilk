#include <cilk/cilk_c11_threads.h>
#include "util.h"

typedef struct {
    int n;
} fib_args;

const int fib_mem[] = {0,1,1,2,3,5,8,13};

int fib( void* arg ) {
    THREAD_PRINT("hello world from cilk!\n");
    int n = ((fib_args*)arg)->n;
    
    if (n < 2) return fib_mem[n];
    
    fib_args fx = {.n = n-1};
    int x = cilk_spawn fib(&fx);
    fib_args fy = {.n = n-2};
    int y = fib(&fy);
    
    cilk_sync;

    return x+y;
}

int main(int argc, char** argv) {
    THREAD_PRINT("hello world from main!\n");
    
    // get cilk runtime config
    cilk_config_t cfg = cilk_thrd_config_from_env("CILK_CONFIG1");
    CFG_PRINT(cfg);
    pthread_t thr1;
    int n = 10;
    if (argc == 2) {
        n = atoi(argv[1]);
    }

    fib_args x = {.n = n};
    // create thread with new cilk runtime
    int err = cilk_thrd_create(cfg, &thr1, fib, &x);
    if (err) {
        printf("thrd creation not successful!\n");
        return -1;
    }

    int res;
    cilk_thrd_join(thr1, &res);

    printf("fib(%d) = %d\n", n, res);
    
    return 0;
}
