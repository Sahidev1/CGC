
#include "CGC.h"
#include "threadctrl.h"

#define STACK_SIZE (8*4096) // About 64kilobytes of stack space. 
#define STACK_NOTUSED (0x0)
#define SWITCH_INIT (0x0)

/**
 * This function should be called from main with address of the function that the worker thread should run.
*/
int runner(shared_args* wargs){
    pthread_t worker, gc;
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

    pthread_create(&worker, &worker_attr, &worker_fun, wargs);
    pthread_create(&gc, NULL, &gc_procedure, wargs);

    pthread_join(worker, NULL);
    pthread_join(gc, NULL);

    destroy_locks();

    free(worker_stackaddr);
    return 0;
}
