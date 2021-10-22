include ../config.mk

SRCS = $(wildcard *.c)
HDRS = $(wildcard *.h)
OBJS = $(patsubst %.c,%.o,$(SRCS))

DEFINES = $(ABI_DEF)

ifdef PRINT
CFLAGS = -DPRINT
endif

OPTIONS = $(OPT) $(ARCH) $(DBG) -Wall $(DEFINES) -fopencilk #-fno-omit-frame-pointer
TIMING_COUNT := 1

.PHONY: all clean

TESTS = helloworld simplefib

all: $(TESTS)

$(TESTS): %: %.o 
	$(CC) $(GLIBC) $^ -o $@ -fopencilk #-ldl -lrt -lm -lc

%.o: %.c
	$(CC) -I../include/ -c $(OPTIONS) $(CFLAGS) -DTIMING_COUNT=$(TIMING_COUNT) -o $@ $<

%.o: %.cpp
	$(CXX) -std=c++11 -c $(OPTIONS) -DTIMING_COUNT=$(TIMING_COUNT) -o $@ $<

clean:
	rm -f *.o *~ $(TESTS) $(CPP_TESTS) core.*
