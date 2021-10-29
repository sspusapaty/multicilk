
# Installation
To build OpenCilk with Multicilk you **must** build it from src, and make sure to build with the `opencilk/v1.0` tagged versions of `opencilk-project` and `productivity-tools`.

If you already have a built version of Opencilk v1.0 (built with `opencilk-project` from src), then you can simply replace the cheetah directory there with this new cheetah directory (after renaming the directory to "cheetah") and recompile `opencilk-project` with `make`.

`opencilk-project` v1.0 can be found at https://github.com/OpenCilk/opencilk-project/releases/tag/opencilk%2Fv1.0.

# Using Multicilk

## C API
To use the C API, include this line at the top of your code
`#include  <cilk/cilk_c11_threads.h>`

In this API you can use the following functions to identify, create, and join cilks:
* `pthread_t cilk_thrd_current()`
* `int cilk_thrd_create(cilk_config_t config, pthread_t* thread, int (*func)(void*), void *arg)`
* `int cilk_thrd_create_with_attr(cilk_config_t config, pthread_t *thread, pthread_attr_t* attr, int (*func) (void*), void *arg)`
* `int cilk_thrd_join (pthread_t thr, int *res)`

We also provide the following extra functions for working with Cilk runtimes.

* `cilk_config_t cilk_thrd_config_from_env(const  char* name)`
This function takes in an evironment variable and outputs a cilk configuration based on the value of that environment variable. This value must be written in the form "nworkers=#;cpuset=#,#,#..." where '#' is an integer.

* `void cilk_thrd_init(cilk_config_t config)`
 This function takes a cilk configuration and creates a Cilk runtime which is stored in the cilk's local storage to be accessed during any cilk computation.

## C++ API
To use the C++ API, include this line at the top of your code
`#include  <cilk/cilk_cpp_threads.hpp>`

The API can use all of the functions provided by the C API and has the following additional function:
``
template <class F, class... Args>
std::thread cilk_thread_create(cilk_config_t config, F&& f, Args&&... args)
``

To create cilks based off of C++ std::threads.
# Running Multicilk Exercises
The following command will take you to some exercises that use the Multicilk API (both C and C++)
`cd cheetah-multicilk/multicilk-exercises`

Run `make` to build the exercises.
Use `PRINT=1` when compiling if you would like to see thread id and cpu id information printed out to console (macros used are `THREAD_PRINT()` in the exercise code).

The exercises are relatively basic but show how to use the functions provided in the API.
