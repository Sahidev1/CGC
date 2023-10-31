#ifndef SHARED_STRUCT_H
#define SHARED_STRUCT_H

#include <stdlib.h>

typedef enum {FALSE, TRUE}boolean;

typedef struct Shared_args{
    int(*const main_gc_ptr)(void);
    boolean is_working;
    boolean run_GC;
    void* worker_stackaddr;
    size_t stack_size;
}shared_args;

#endif