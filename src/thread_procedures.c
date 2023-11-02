#include "thread_procedures.h"
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
 * 
 * this is a bit dodgy
*/
void* gc_procedure(void* arg){
    long sleep_time_nanos = 1000 * 100; // 100 ns
    struct timespec req={.tv_sec=0, .tv_nsec=sleep_time_nanos};
    volatile size_t prev_heapsz = 0;
    volatile size_t curr_heapsz = 0;

    shared_args *const wargs = (shared_args*) arg;
    void* stack_ptr = wargs->worker_stackaddr + wargs->stack_size;
    int iter = 0;
    while(wargs->is_working && wargs->run_GC){
        curr_heapsz = get_heap_size();
        if (curr_heapsz != prev_heapsz){
            //printf("ALLOCED HEAP SIZE: %ld\n", curr_heapsz);
            prev_heapsz = curr_heapsz;
        }
        if(curr_heapsz <= 0){
            sleep_nanos(req);
            continue;
        }
        GC(stack_ptr, wargs->worker_stackaddr);
        sleep_nanos(req);        
    }
    return NULL;
}