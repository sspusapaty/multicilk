include config.mk

SRCS = $(wildcard *.c)
HDRS = $(wildcard *.h)
OBJS = $(patsubst %.c,%.o,$(SRCS))

DEFINES = $(ABI_DEF)

#TESTS = cilk_thrd_create cilk_thrd_current_equals cilk_thrd_exit multicilk_report #cilk_thrd_sleep_yield #thrd_tests/cilk_mtx thrd_tests/cilk_cnd
TESTS = cilk_thrd_create cilk_thrd_current_equals cilk_thrd_exit multicilk_report multicilk_report_env #cilk_thrd_sleep_yield #thrd_tests/cilk_mtx thrd_tests/cilk_cnd
OPTIONS = $(OPT) $(ARCH) $(DBG) -Wall $(DEFINES) -fopencilk #-fno-omit-frame-pointer
TIMING_COUNT := 1

CPP_TESTS = test_cpp_threads test_cpp_threads_env

.PHONY: all clean


TOOLS = multicilk_tools.o

all: $(TESTS) $(CPP_TESTS)

$(TESTS): %: %.o 
	$(CC) $(GLIBC) $^ $(TOOLS) -o $@ -fopencilk #-ldl -lrt -lm -lc

$(CPP_TESTS): %: %.o 
	$(CXX) $(GLIBC) $^ $(TOOLS) -o $@ -fopencilk #-ldl -lrt -lm -lc

multicilk_tools.o: multicilk_tools.c multicilk_tools.h
	$(CC) -c $(OPTIONS) -c -o $@ $<

test: all
	./cilk_thrd_create
	./cilk_thrd_current_equals
	./cilk_thrd_exit
	CILK_CONFIG_1="nworkers=1;cpuset=0" CILK_CONFIG_2="nworkers=1;cpuset=1" ./test_cpp_threads_env
	#./test_cpp_threads_env
	#CILK_CONFIG_1="nworkers=4;cpuset=0,1,2,3" CILK_CONFIG_2="nworkers=4;cpuset=4,5,6,7"./test_cpp_threads_env
	#CILK_CONFIG_1="nworkers=2;cpuset=0,1" CILK_CONFIG_2="nworkers=2; cpuset=2,3" ./test_cpp_threads_env



%.o: %.c multicilk_tools.o
	$(CC) -c $(OPTIONS) -DTIMING_COUNT=$(TIMING_COUNT) -o $@ $<

%.o: %.cpp multicilk_tools.o
	$(CXX) -std=c++11 -c $(OPTIONS) -DTIMING_COUNT=$(TIMING_COUNT) -o $@ $<

clean:
	rm -f *.o *~ $(TESTS) $(CPP_TESTS) core.*
