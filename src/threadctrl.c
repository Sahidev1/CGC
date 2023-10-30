#include "threadctrl.h"
#include "shared_struct.h"

#include <stdio.h> // for debugging

/**
 * makes thread sleep for certain nanoseconds
*/
static int sleep_nanos(struct timespec req){
    return nanosleep(&req, NULL);
}

/**
 * This is the worker function for the worker thread
*/
void* worker_fun(void* arg){
    shared_args *const wargs = (shared_args*) arg;

    printf("assigned stack addr: %p\n", wargs->worker_stackaddr);
    wargs->main_gc_ptr();
    wargs->is_working = FALSE;
    return NULL;
}

/**
 * This the garbage procedure for the garbage collector thread
*/
void* gc_procedure(void* arg){
    long sleep = 1000;
    struct timespec req={.tv_sec=0, .tv_nsec=sleep};

    shared_args *const wargs = (shared_args*) arg;
    void* stack_ptr = wargs->worker_stackaddr + wargs->stack_size;
    int iter = 0;
    while(wargs->is_working){
        GC(stack_ptr, wargs->worker_stackaddr);
        sleep_nanos(req);        
    }
    GC(stack_ptr, wargs->worker_stackaddr);
    printf("GC Done\n");
    return NULL;
}