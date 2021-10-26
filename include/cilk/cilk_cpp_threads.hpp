#ifndef _CILK_CPP_H
#define  _CILK_CPP_H

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <thread>

/* General wrapper function to create cilk runtime object and set it's
 * destructor before calling the thrd_args function member.
 */
struct cilk_wrapper_helper {
	cilk_config_t config;
	template <class F, class... Args>
	void operator()(F&& f, Args&&... args) const {
		cilk_thrd_init(config);
		f(std::forward<Args>(args)...);
	}
};

template <class F, class... Args>
std::thread cilk_thread_create(cilk_config_t config, F&& f, Args&&... args) {
    std::thread thr(cilk_wrapper_helper{config}, f, std::forward<Args>(args)...);
	return thr;
}

#endif
