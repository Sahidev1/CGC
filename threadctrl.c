#include "threadctrl.h"
#include <stdlib.h>

void* worker_fun(void* arg){
    shared_args *const wargs = (shared_args*) arg;

    wargs->main_gc_ptr();
    return NULL;
}

void* gc_procedure(void* arg){

    return NULL;
}