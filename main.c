#define GNU_SOURCE
#include "threadctrl.h"
#include <pthread.h>
#include <string.h>
#include "heap_manager.h"

#define STACK_SIZE (8*4000)
#define STACK_NOTUSED (0x0)
#define SWITCH_INIT (0x0)

int main_gc(void);

shared_args warg = {
    .main_gc_ptr = &main_gc,
    .is_working=TRUE
};
shared_args *const wargs = &warg;

int main(void){
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

    return 0;
}
