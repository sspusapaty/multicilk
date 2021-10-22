#include <cilk/cilk_c11_threads.h>
#include "util.h"
// TODO test
typedef struct {
    int n;
} args;

const int fib_mem[] = {0,1,1,2,3,5,8,13};

int fib( int n ) {
    THREAD_PRINT("hello world from cilk!\n");
    
    if (n < 2) return fib_mem[n];
    
    int x = cilk_spawn fib(n-1);
    int y = fib(n-2);
    
    cilk_sync;

    return x+y;
}

typedef struct {
    int n;
    int fib;
} entry;

entry buffer[2];
pthread_mutex_t queueMutex;
pthread_cond_t queueCond;

int produce( void* arg ) {
    int max = ((args*)arg)->n;
    
    for (int n = 1; n <= max; ++n) {
        int data = fib(n);
        entry e;
        e.n = n;
        e.fib = data;
        
        int i = 0;
        while (1) {
            if (buffer[i] == -1) {
                pthread_mutex_lock(&queueMutex);
                buffer[i] = e;
                pthread_mutex_unlock(&queueMutex);
                break;
            }
            i = (i+1)%2;
        }

    }

    return 0;
}

int consume( void* arg ) {
    while (1) {
        int i = 0;
        entry data;
        while (1) {
            if (buffer[i] != -1) {
                pthread_mutex_lock(&queueMutex);
                data = buffer[i];
                buffer[i] = -1;
                pthread_mutex_unlock(&queueMutex);
                break;
            }
            i = (i+1)%2;
        }

        printf("Consumer decided fib(%d) is %d!\n",data.n,fib(data.n) == data.fib);
        if (data.n == 40) break;
    }
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
