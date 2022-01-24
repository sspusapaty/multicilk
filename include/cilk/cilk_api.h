#include <sched.h>
#include <pthread.h>
#include <threads.h>
#ifndef _CILK_API_H
#define _CILK_API_H
#ifdef __cplusplus
extern "C" {
#endif

extern int __cilkrts_is_initialized(void);
extern int __cilkrts_atinit(void (*callback)(void));
extern int __cilkrts_atexit(void (*callback)(void));
extern unsigned __cilkrts_get_nworkers(void);
extern unsigned __cilkrts_get_worker_number(void) __attribute__((deprecated));
struct __cilkrts_worker *__cilkrts_get_tls_worker(void);


/** CILK THREADS API **/
typedef struct {
	int n_workers;
    cpu_set_t boss_affinity;
} cilk_config_t;

#include <pthread.h>
thrd_t thrd_current();
thrd_t worker_current();
void cilk_thrd_init(cilk_config_t config);
cilk_config_t cilk_cfg_from_env(const char* name);
void cilkmutex_init();
/** END CILK THREADS API **/


#if defined(__cilk_pedigrees__) || defined(ENABLE_CILKRTS_PEDIGREE)
#include <inttypes.h>
typedef struct __cilkrts_pedigree {
    uint64_t rank;
    struct __cilkrts_pedigree *parent;
} __cilkrts_pedigree;
extern __cilkrts_pedigree __cilkrts_get_pedigree(void);
extern void __cilkrts_bump_worker_rank(void);
extern uint64_t __cilkrts_get_dprand(void);
#endif // defined(__cilk_pedigrees__) || defined(ENABLE_CILKRTS_PEDIGREE)

#undef VISIBILITY

#ifdef __cplusplus
}
#endif

#endif /* _CILK_API_H */
