#ifndef THREADCTRL_H
#define THREADCTRL_H

#include <stdlib.h>
#include <time.h>
#include "heap_manager.h"

typedef enum {FALSE, TRUE}boolean;

typedef struct Shared_args{
    int(*const main_gc_ptr)(void);
    boolean is_working;
    void* worker_stackaddr;
    size_t stack_size;
}shared_args;


void* worker_fun(void* arg);
void* gc_procedure(void* arg);

#endif