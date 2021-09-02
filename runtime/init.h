#ifndef _CILK_INIT_H
#define _CILK_INIT_H

#include "cilk-internal.h"

void invoke_cilkified_root(global_state *g, __cilkrts_stack_frame *sf);
void wait_until_cilk_done(global_state *g);
__attribute__((noreturn))
void exit_cilkified_root(global_state *g, __cilkrts_stack_frame *sf);

// Used by Cilksan to set nworkers to 1 and force reduction
void __cilkrts_internal_set_nworkers(unsigned int nworkers);
void __cilkrts_internal_set_force_reduce(unsigned int force_reduce);

global_state *__cilkrts_startup(int argc, char *argv[]);
void __cilkrts_shutdown(global_state *g);

#endif /* _CILK_INIT_H */
