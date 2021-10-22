#define _GNU_SOURCE
#include <stdatomic.h>
#include <stdio.h>
#include <unwind.h>

#include "debug.h"

#include "cilk-internal.h"
#include "cilk2c.h"
#include "fiber.h"
#include "global.h"
#include "readydeque.h"
#include "scheduler.h"

// Needed only for threading.
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "init.h" // needed for threading.

// For parsing cilk threading environment variables.
#include <string.h>
#include <assert.h> // replace with cilk asserts.

extern void _Unwind_Resume(struct _Unwind_Exception *);
extern _Unwind_Reason_Code _Unwind_RaiseException(struct _Unwind_Exception *);

CHEETAH_INTERNAL unsigned cilkg_nproc = 0;

CHEETAH_INTERNAL struct cilkrts_callbacks cilkrts_callbacks = {
    0, 0, false, {NULL}, {NULL}};


/** Internal functions for cilk thread **/
pthread_t cilk_thrd_current() {
    __cilkrts_worker *w = __cilkrts_get_tls_worker();
    if (w == NULL) return pthread_self();
    // NOTE(TFK): Assert that w->g != NULL?
    return w->g->boss;
}

pthread_t cilk_thrd_get_worker_pthread(int i) {
    return my_cilkrts->threads[i];
}

int cilk_thrd_num_workers() {
    return my_cilkrts->nworkers;
}

void cilk_thrd_shutdown(void* n) {
    // printf("CILKRTS cilk_thrd_shutdown\n");
    // NOTE(TFK): Probably ought to pass the pointer via argument?
    __cilkrts_shutdown(my_cilkrts);
}

void cilk_thrd_init(cilk_config_t config) {
    if (__cilkrts_is_initialized()) {
        printf("Error CILKRTS is already initalized!\n");
    }
    //printf("Initializing cilkrts for new thread %llu\n", pthread_self());
    my_cilkrts = __cilkrts_startup(config.n_workers, NULL);

    for (unsigned i = 0; i < cilkrts_callbacks.last_init; ++i)
        cilkrts_callbacks.init[i]();

    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &(config.boss_affinity));

    pthread_key_t key;
    pthread_key_create(&key, cilk_thrd_shutdown);
    pthread_setspecific(key, my_cilkrts); // so that its not null and will have destructor called.
}

cilk_config_t cilk_thrd_config_from_env(const char* cilk_env_name) {
    char* value = getenv(cilk_env_name);
    if (value == NULL) {
        printf("WARNING cilk config environment variable %s is not defined. Using default cilk config \n", cilk_env_name);
        cilk_config_t cfg;
        cfg.n_workers = 0;
        cpu_set_t mask;
        // get the mask from the parent thread (master thread)
        pthread_getaffinity_np(pthread_self(), sizeof(mask), &mask);
        // Get the number of available cores (copied from os-unix.c)
        cfg.n_workers = CPU_COUNT(&mask);
        cfg.boss_affinity = mask;
        return cfg;
    }
    int value_len = strlen(value);

    // Filter characters.
    char* value_filtered = malloc(sizeof(char) * (value_len+1));
    int value_filtered_len = 0;
    for (int i = 0; i < value_len; i++) {
        if ((value[i] >= 48 && value[i] <= 57) || // 0-9
            (value[i] >= 65 && value[i] <= 90) || //A-Z
            (value[i] >= 97 && value[i] <= 122) || // a-z
            (value[i] == 61) || // =
            (value[i] == 44) || // ,
            (value[i] == 59) // ;
            ) {
            
            if (value[i] >= 65 && value[i] <= 90) {
                // change upper case to lower case so that names are not 
                //   case sensitive.
                value_filtered[value_filtered_len++] = value[i] + (97 - 65); 
            } else {
                value_filtered[value_filtered_len++] = value[i];
            }
            continue;
        }
        printf("WARNING: Invalid character '%c' found in cilk thread config string with name '%s' and value '%s'." \
                "This character will be ignored.\n", value[i], cilk_env_name, value);
    }
    assert(value_filtered_len <= value_len);
    value_filtered[value_filtered_len++] = 0; // add null terminator.

    int nworkers = 0;
    cpu_set_t cilk_mask;
    CPU_ZERO(&cilk_mask);

    char* ptr;
    char* token = NULL;
    token = strtok_r(value_filtered, ";", &ptr);
    if (token == NULL) printf("INVALID\n");
    do {
        char* ptr1 = NULL;
        char* name = strtok_r(token, "=", &ptr1);
        if (name == NULL) continue;
        char* data = strtok_r(NULL, "=", &ptr1);
        if (data == NULL) continue;
        if (strcmp(name, "nworkers") == 0) {
            int data_len = strlen(data);
            if (data_len == 0) {
                printf("WARNING: Cilk environment variable %s specified the key %s with no value\n", 
                cilk_env_name, name);
                continue;
            }
            
            char* buffer = (char*) malloc(sizeof(char) * (data_len+1));
            int buffer_len = 0;
            for (int i = 0; i < data_len; i++) {
                if (data[i] >= 48 && data[i] <= 57) { // 0-9
                    buffer[buffer_len++] = data[i];
                    continue; 
                }
                if (data[i] == 0) continue; // terminator character.
                printf("WARNING: Cilk environment variable %s specified the key %s with a value that contained an invalid character '%c' This character will be ignored.\n",
                       cilk_env_name, name, data[i]);
            }
            assert(buffer_len <= data_len);
            buffer[buffer_len++] = 0; // add null terminator.
            nworkers = atoi(buffer);
            free(buffer);                        
        }
        if (strcmp(name, "cpuset") == 0) {
            int data_len = strlen(data);

            // Sanitize data: ',' and 0-9
            char* data_clean = (char*) malloc(sizeof(char)* (data_len+1));
            int data_clean_len = 0;
            for (int i = 0; i < data_len; i++) {
                if (data[i] >= 48 && data[i] <= 57) { // 0-9
                    data_clean[data_clean_len++] = data[i];
                    continue;
                }
                if (data[i] == 44) continue; // ','
                if (data[i] == 0) continue; // null terminator.
                printf("WARNING: Cilk environment variable %s specified the key %s with a value that contained an invalid character '%c' This character will be ignored.\n",
                       cilk_env_name, name, data[i]);
            }
            assert(data_clean_len <= data_len);
            data_clean[data_clean_len++] = 0; // add null terminator.
            
            if (data_clean_len == 0) {
                printf("WARNING: Cilk environment variable %s specified the key %s with no value\n", 
                cilk_env_name, name);
                continue;
            }

            CPU_ZERO(&cilk_mask);
            char* ptr2;
            char* cpu_str = strtok_r(data, ",", &ptr2);
            while (cpu_str != NULL) {
                int cpu_idx = atoi(cpu_str);
                //printf("setting cpuid %d\n", cpu_idx);
                CPU_SET(cpu_idx, &cilk_mask);
                cpu_str = strtok_r(NULL, ",", &ptr2);
            }
            free(data_clean);
        }
    } while ((token = strtok_r(NULL, ";", &ptr)));

    cilk_config_t cfg;
    cfg.n_workers = nworkers;
    cfg.boss_affinity = cilk_mask;
    return cfg;
}


/** end internal functions for cilk thread **/

// Internal method to get the Cilk worker ID.  Intended for debugging purposes.
//
// TODO: Figure out how we want to support worker-local storage.
unsigned __cilkrts_get_worker_number(void) {
    __cilkrts_worker *w = __cilkrts_get_tls_worker();
    if (w)
        return w->self;
    // Use the last exiting worker from default_cilkrts instead
    return my_cilkrts->exiting_worker;
}

// Test if the Cilk runtime has been initialized.  This method is intended to
// help initialization of libraries that depend on the OpenCilk runtime.
int __cilkrts_is_initialized(void) { return NULL != my_cilkrts; }

// These callback-registration methods can run before the runtime system has
// started.
//
// Init callbacks are called in order of registration.  Exit callbacks are
// called in reverse order of registration.

// Register a callback to run at Cilk-runtime initialization.  Returns 0 on
// successful registration, nonzero otherwise.
int __cilkrts_atinit(void (*callback)(void)) {
    if (cilkrts_callbacks.last_init >= MAX_CALLBACKS /*||
        cilkrts_callbacks.after_init*/)
        return -1;

    cilkrts_callbacks.init[cilkrts_callbacks.last_init++] = callback;
    return 0;
}

// Register a callback to run at Cilk-runtime exit.  Returns 0 on successful
// registration, nonzero otherwise.
int __cilkrts_atexit(void (*callback)(void)) {
    if (cilkrts_callbacks.last_exit >= MAX_CALLBACKS)
        return -1;

    cilkrts_callbacks.exit[cilkrts_callbacks.last_exit++] = callback;
    return 0;
}

// Called after a normal cilk_sync (i.e. not the cilk_sync called in the
// personality function.) Checks if there is an exception that needs to be
// propagated. This is called from the frame that will handle whatever exception
// was thrown.
void __cilkrts_check_exception_raise(__cilkrts_stack_frame *sf) {

    __cilkrts_worker *w = sf->worker;
    deque_lock_self(w);
    Closure *t = deque_peek_bottom(w, w->self);
    Closure_lock(w, t);
    char *exn = t->user_exn.exn;

    // zero exception storage, so we don't unintentionally try to
    // handle/propagate this exception again
    clear_closure_exception(&(t->user_exn));
    sf->flags &= ~CILK_FRAME_EXCEPTION_PENDING;

    Closure_unlock(w, t);
    deque_unlock_self(w);
    if (exn != NULL) {
        _Unwind_RaiseException((struct _Unwind_Exception *)exn); // noreturn
    }

    return;
}


// Called after a cilk_sync in the personality function.  Checks if
// there is an exception that needs to be propagated, and if so,
// resumes unwinding with that exception.
void __cilkrts_check_exception_resume(__cilkrts_stack_frame *sf) {

    __cilkrts_worker *w = sf->worker;
    deque_lock_self(w);
    Closure *t = deque_peek_bottom(w, w->self);
    Closure_lock(w, t);
    char *exn = t->user_exn.exn;

    // zero exception storage, so we don't unintentionally try to
    // handle/propagate this exception again
    clear_closure_exception(&(t->user_exn));
    sf->flags &= ~CILK_FRAME_EXCEPTION_PENDING;

    Closure_unlock(w, t);
    deque_unlock_self(w);
    if (exn != NULL) {
        _Unwind_Resume((struct _Unwind_Exception *)exn); // noreturn
    }

    return;
}

// Called by generated exception-handling code, specifically, at the beginning
// of each landingpad in a spawning function.  Ensures that the stack pointer
// points at the fiber and call-stack frame containing sf before any catch
// handlers in that frame execute.
void __cilkrts_cleanup_fiber(__cilkrts_stack_frame *sf, int32_t sel) {

    if (sel == 0)
        // Don't do anything during cleanups.
        return;

    __cilkrts_worker *w = sf->worker;
    deque_lock_self(w);
    Closure *t = deque_peek_bottom(w, w->self);

    // If t->parent_rsp is non-null, then the Cilk personality function executed
    // __cilkrts_sync(sf), which implies that sf is at the top of the deque.
    // Because we're executing a non-cleanup landingpad, execution is continuing
    // within this function frame, rather than unwinding further to a parent
    // frame, which would belong to a distinct closure.  Hence, if we reach this
    // point, set the stack pointer in sf to t->parent_rsp if t->parent_rsp is
    // non-null.

    if (NULL == t->parent_rsp) {
        deque_unlock_self(w);
        return;
    }

    SP(sf) = (void *)t->parent_rsp;
    t->parent_rsp = NULL;

    if (t->saved_throwing_fiber) {
        cilk_fiber_deallocate_to_pool(w, t->saved_throwing_fiber);
        t->saved_throwing_fiber = NULL;
    }

    deque_unlock_self(w);
    __builtin_longjmp(sf->ctx, 1); // Does not return
    return;
}

void __cilkrts_sync(__cilkrts_stack_frame *sf) {

    __cilkrts_worker *w = sf->worker;

    CILK_ASSERT(w, sf->worker == __cilkrts_get_tls_worker());
    CILK_ASSERT(w, CHECK_CILK_FRAME_MAGIC(w->g, sf));
    CILK_ASSERT(w, sf == w->current_stack_frame);

    if (Cilk_sync(w, sf) == SYNC_READY) {
        // The Cilk_sync restores the original rsp stored in sf->ctx
        // if this frame is ready to sync.
        sysdep_longjmp_to_sf(sf);
    } else {
        longjmp_to_runtime(w);
    }
}

void __cilkrts_pause_frame(__cilkrts_stack_frame *sf, char *exn) {

    __cilkrts_worker *w = sf->worker;
    cilkrts_alert(CFRAME, w, "__cilkrts_pause_frame %p", (void *)sf);

    CILK_ASSERT(w, CHECK_CILK_FRAME_MAGIC(w->g, sf));
    CILK_ASSERT(w, sf->worker == __cilkrts_get_tls_worker());

    CILK_ASSERT(w, sf->flags & CILK_FRAME_DETACHED);
    __cilkrts_stack_frame **tail =
        atomic_load_explicit(&w->tail, memory_order_relaxed);
    --tail;
    /* The store of tail must precede the load of exc in global order.
       See comment in do_dekker_on. */
    atomic_store_explicit(&w->tail, tail, memory_order_seq_cst);
    __cilkrts_stack_frame **exc =
        atomic_load_explicit(&w->exc, memory_order_seq_cst);
    /* Currently no other modifications of flags are atomic so this
       one isn't either.  If the thief wins it may run in parallel
       with the clear of DETACHED.  Does it modify flags too? */
    sf->flags &= ~CILK_FRAME_DETACHED;
    if (__builtin_expect(exc > tail, 0)) {
        Cilk_exception_handler(exn);
        // If Cilk_exception_handler returns this thread won
        // the race and can return to the parent function.
    }
    // CILK_ASSERT(w, *(w->tail) == w->current_stack_frame);
}

void __cilkrts_leave_frame(__cilkrts_stack_frame *sf) {
    __cilkrts_worker *w = sf->worker;
    cilkrts_alert(CFRAME, w, "__cilkrts_leave_frame %p", (void *)sf);

    CILK_ASSERT(w, CHECK_CILK_FRAME_MAGIC(w->g, sf));
    CILK_ASSERT(w, sf->worker == __cilkrts_get_tls_worker());
    // WHEN_CILK_DEBUG(sf->magic = ~CILK_STACKFRAME_MAGIC);

    if (sf->flags & CILK_FRAME_DETACHED) { // if this frame is detached
        __cilkrts_stack_frame **tail =
            atomic_load_explicit(&w->tail, memory_order_relaxed);
        --tail;
        /* The store of tail must precede the load of exc in global order.
           See comment in do_dekker_on. */
        atomic_store_explicit(&w->tail, tail, memory_order_seq_cst);
        __cilkrts_stack_frame **exc =
            atomic_load_explicit(&w->exc, memory_order_seq_cst);
        /* Currently no other modifications of flags are atomic so this
           one isn't either.  If the thief wins it may run in parallel
           with the clear of DETACHED.  Does it modify flags too? */
        sf->flags &= ~CILK_FRAME_DETACHED;
        if (__builtin_expect(exc > tail, 0)) {
            Cilk_exception_handler(NULL);
            // If Cilk_exception_handler returns this thread won
            // the race and can return to the parent function.
        }
        // CILK_ASSERT(w, *(w->tail) == w->current_stack_frame);
    } else {
        // A detached frame would never need to call Cilk_set_return, which
        // performs the return protocol of a full frame back to its parent
        // when the full frame is called (not spawned).  A spawned full
        // frame returning is done via a different protocol, which is
        // triggered in Cilk_exception_handler.
        if (sf->flags & CILK_FRAME_STOLEN) { // if this frame has a full frame
            cilkrts_alert(RETURN, w,
                          "__cilkrts_leave_frame parent is call_parent!");
            // leaving a full frame; need to get the full frame of its call
            // parent back onto the deque
            Cilk_set_return(w);
            CILK_ASSERT(w, CHECK_CILK_FRAME_MAGIC(w->g, sf));
        }
    }
}

unsigned __cilkrts_get_nworkers(void) { return cilkg_nproc; }
