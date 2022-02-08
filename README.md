
# Installation
To build OpenCilk with Multicilk you **must** build it from src, and make sure to build with the `opencilk/v1.0` tagged versions of `opencilk-project` and `productivity-tools`.

If you already have a built version of Opencilk v1.0 (built with `opencilk-project` from src), then you can simply replace the cheetah directory there with this new cheetah directory (after renaming the directory to "cheetah") and recompile `opencilk-project` with `make`.

`opencilk-project` v1.0 can be found at https://github.com/OpenCilk/opencilk-project/releases/tag/opencilk%2Fv1.0.

# Using Multicilk

## C API
To use the C API, include this line at the top of your code
`#include  <cilk/cilk_c11_threads.h>`

In this API you can use the following functions to create cilks:
* `int cilk_thrd_create(cilk_config_t config, thrd_t* thread, int (*func)(void*), void *arg)`

You can also use every function from the C11 API safely (except the keyword `thread_local`).
* The API is here: https://en.cppreference.com/w/c/thread
* To use thread local storage related functions, see the `cilkls` branch. You will need to compile the modified glibc and link it with your Multicilk program.
* To use mutex/cnd var related functions, see the `mutexes_and_cnd_vars` branch. You will need to compile the modified glibc and link it with your Multicilk program.

TODO: Merge the different glibc's into one unified glibc to use.

We also provide the following extra functions for working with Cilk runtimes.

* `cilk_config_t cilk_cfg_from_env(const  char* name)`
This function takes in an evironment variable and outputs a cilk configuration based on the value of that environment variable. This value must be written in the form "nworkers=#;cpuset=#,#,#..." where '#' is an integer.

* `void cilk_thrd_init(cilk_config_t config)`
 This function takes a cilk configuration and creates a Cilk runtime which is stored in the cilk's local storage to be accessed during any cilk computation.
