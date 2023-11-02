#ifndef THREAD_PROCEDURES_H
#define THREAD_PROCEDURES_H

#include <stdlib.h>
#include <time.h>
#include "heap_manager.h"


void* worker_fun(void* arg);
void* gc_procedure(void* arg);

#endif