#include "debug.h"
#include "cilk-internal.h"
#include "global.h"

#include <sched.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cilk/cilk_api.h>

// Maintain a private cpu_t data type that contains pointers (e.g., char* flags) to data
//  allocated by the runtime system.
typedef struct cilk_private_cpu_t {
    int processor;
    int core_id;
    int physical_id;
    char* flags;
} cilk_private_cpu_t;

// Public cilk_cpu_t type only contains value types, no pointers.
typedef struct cilk_cpu_t {
    int processor;
    int core_id;
    int physical_id;
} cilk_cpu_t;

static cilk_private_cpu_t* cilk_cpu_list;
static int cilk_cpu_list_len;

int cilk_cputopology_nprocs() {
    return cilk_cpu_list_len;    
}

bool cilk_cputopology_proc_hasflag(int processor, const char* flag) {
    assert(flag != NULL && processor >= 0 && processor < cilk_cpu_list_len &&
           cilk_cpu_list != NULL);
    assert(cilk_cpu_list[processor].flags != NULL);
    return strstr(cilk_cpu_list[processor].flags, flag);
}

// NOTE(TFK): need to verify that processor and 'idx' in cilk_cpu_list is equivalent.
cilk_cpu_t cilk_cputopology_getproc(int processor) {
    assert(cilk_cpu_list != NULL && processor < cilk_cpu_list_len &&
           processor >= 0);
    cilk_cpu_t info;
    info.processor = cilk_cpu_list[processor].processor;
    info.core_id = cilk_cpu_list[processor].core_id;
    info.physical_id = cilk_cpu_list[processor].physical_id;
    return info;
}

// Methods for parsing either the integer or string data from a line in /proc/cpuinfo
int parse_cpuinfo_integer(char* line) {
    assert(line != NULL);
    int len = strlen(line);
    assert(len > 0);

    char* buffer = (char*) malloc(sizeof(char) * len);
    int buffer_len = 0;

    for (int i = 0; i < len; i++) {
        if (line[i] >= 48 && line[i] <= 57) { // numeric 0-9
            buffer[buffer_len++] = line[i];
        }
    }

    assert(buffer_len < len && buffer_len > 0);
    buffer[buffer_len] = 0;

    int x = atoi(buffer);

    free(buffer);
    return x;
}

char* parse_cpuinfo_string(char* line) {
    char* ptr;
    char* token = strtok_r(line, ":", &ptr);
    if (token != NULL) {
        token = strtok_r(NULL, ":", &ptr);
        if (token != NULL) {
            int len = strlen(token);
            assert(len > 0);
            char* content = (char*) malloc(sizeof(char)*len);
            for (int i = 0; i < len; i++) {
                content[i] = token[i];
            }
            return content;
        }
    }
    return NULL;
}

// Populate fields of the provided cpuinfo by calling the appropriate parsing function.
void process_cpuinfo_line(char* line, cilk_private_cpu_t* cpuinfo) {
    if (strstr(line, "processor")) {
        cpuinfo->processor = parse_cpuinfo_integer(line);
    } else if (strstr(line, "core id")) {
        cpuinfo->core_id = parse_cpuinfo_integer(line);
    } else if (strstr(line, "physical id")) {
        cpuinfo->physical_id = parse_cpuinfo_integer(line);
    } else if (strstr(line, "flags")) {
        cpuinfo->flags = parse_cpuinfo_string(line);
    }
}

void print_cpu_list_item(cilk_private_cpu_t cpu) {
    printf("processor %d core id %d physical id %d\n flags:%s\n", cpu.processor, cpu.core_id, cpu.physical_id, cpu.flags);
}

// For non-linux systems.
void setup_fake_cpu_topology() {
    cpu_set_t mask;
    pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &mask);
    //int n_cores = CPU_COUNT(&mask);
    assert(false);
    int n_cores = 0;
    assert(n_cores > 0);
    cilk_cpu_list = (cilk_private_cpu_t*) malloc(sizeof(cilk_private_cpu_t) * n_cores);
    for (int i = 0; i < n_cores; i++) {
        cilk_cpu_list[i].processor = i;
        cilk_cpu_list[i].core_id = -1;
        cilk_cpu_list[i].physical_id = -1;
        cilk_cpu_list[i].flags = NULL;
    }
}

void setup_cpu_topology() {
    FILE* f = fopen("/proc/cpuinfo", "r");
    if (f == NULL) {
        setup_fake_cpu_topology();
        return;
    }
    char* line = NULL;
    size_t n = 0;

    cilk_private_cpu_t* cpu_list = malloc(sizeof(cilk_private_cpu_t)*4);
    int cpu_list_len = -1;
    int cpu_list_cap = 4;

    while (getline(&line, &n, f) > 0) {
        if (strstr(line, "processor")) cpu_list_len++;
        if (cpu_list_len >= cpu_list_cap) {
            cpu_list_cap *= 2;
            cpu_list = (cilk_private_cpu_t*) realloc(cpu_list, sizeof(cilk_private_cpu_t)*cpu_list_cap);
        }
        if (strstr(line, "processor") || strstr(line, "core id") || strstr(line, "physical id") || strstr(line, "flags")) {
            assert(cpu_list_len >= 0 && "Error cpu_list_len < 0\n");
            assert(cpu_list_len < cpu_list_cap && "Error cpu_list_len > cpu_list_cap\n");
            process_cpuinfo_line(line, &(cpu_list[cpu_list_len]));
        }
    }
    cpu_list_len++;

    cilk_cpu_list = cpu_list;
    cilk_cpu_list_len = cpu_list_len;
}

// Deallocate data
void cleanup_cpu_topology() {
    for (int i = 0; i < cilk_cpu_list_len; i++) {
        free(cilk_cpu_list[i].flags);
    }
    free(cilk_cpu_list);
}
