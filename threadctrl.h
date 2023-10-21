#ifndef THREADCTRL_H
#define THREADCTRL_H

typedef enum {FALSE, TRUE}boolean;

typedef struct Shared_args{
    int(*const main_gc_ptr)(void);
    boolean is_working;
}shared_args;


void* worker_fun(void* arg);

#endif