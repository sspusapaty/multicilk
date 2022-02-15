#define _GNU_SOURCE
#include <cilk/cilk_c11_threads.h>
#include "util.h"

int helloworld( void* arg ) {
    THREAD_PRINT("hello world from cilk!\n");
    return 0;
}

int main() {
    THREAD_PRINT("hello world from main!\n");
    // get cilk runtime config
    cilk_config_t cfg = cilk_cfg_from_env("");
    pthread_t thr1;
    
    // create thread with new cilk runtime
    int err = cilk_thrd_create(cfg, &thr1, helloworld, NULL);
    if (err) {
        printf("thrd creation not successful!\n");
        return -1;
    }

    thrd_join(thr1, NULL);

    return 0;
}
