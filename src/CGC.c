
#include "CGC.h"
#include "thread_procedures.h"

#include <stdio.h> // for debbugging.

#define STACK_SIZE (256*4096) // About 1MB of stack space. 
#define STACK_NOTUSED (0x0)
#define SWITCH_INIT (0x0)

static shared_args* wargs;
static pthread_t gc;

/**
 * Creates the GC thread
*/
int start_autoGC(){
    wargs->run_GC = TRUE;
    int val = pthread_create(&gc, NULL, &gc_procedure, wargs);
    //printf("PHTREAD CREATE , VAL: %d \n",val);
    return val;
}

/**
 * Detaches the GC thread, making sure the thread gets cleaned up by the system
 * after it finishes
*/
int stop_autoGC(){
    wargs->run_GC = FALSE;
    return pthread_detach(gc);
}

/**
 * This function should be called from main with address of the function that the worker thread should run.
*/
int runner(shared_args* args){
    wargs = args;
    wargs->run_GC = FALSE;
    pthread_t worker;
    pthread_attr_t worker_attr;
    void* worker_stackaddr;

    /**
     * We allocate stack space for the worker thread
     * and set all values in the space to a value 
     * which is meant to indicate that the stack address has not
     * been used, this will help us estimate the bottom of the stack.
    */
    worker_stackaddr = malloc(STACK_SIZE);
    memset(worker_stackaddr, STACK_NOTUSED, STACK_SIZE);

    initialize_locks();

    /**
     * We initialize the worker thread attributes. Attributes includes
     * an address to the stack space and size of the stack. 
    */
    pthread_attr_init(&worker_attr);
    pthread_attr_setstack(&worker_attr, worker_stackaddr, STACK_SIZE);

    wargs->worker_stackaddr = worker_stackaddr;
    wargs->stack_size = STACK_SIZE;

    set_stack_layout(wargs->worker_stackaddr + wargs->stack_size, 
    wargs->worker_stackaddr);

    pthread_create(&worker, &worker_attr, &worker_fun, wargs);

    pthread_join(worker, NULL);

    destroy_locks();

    free(worker_stackaddr);
    return 0;
}
