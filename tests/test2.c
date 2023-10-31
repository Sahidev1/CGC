#include "CGC.h"
#include "stdio.h"
#include "unistd.h"

static int print_nr = 0;

void prnt(char* str){
    printf("print: %d\nmsg: %s\n",++print_nr,str);
}

//testing wheter a new alloc call triggers garbage collection
//track memory usage while it is running
int gcmain(void){
    sleep(10);
    int alloc_size = 1000*1000*400;

    int* iptr0 = palloc(sizeof(int)*alloc_size, VALUE);
    for(int i = 0; i < alloc_size; i++){
        iptr0[i] = 11;
    }
    sleep(2);
    iptr0 = NULL;
    prnt("iptr0 derefed");
    sleep(5);
    int* iptr1 = palloc(sizeof(int)*alloc_size/100, VALUE);
    for(int i = 0; i < alloc_size/100; i++){
        iptr1[i] = 11;
    }
    prnt("new allloc");
    sleep(5);

    return 0;
}

int main(){
    shared_args args = {.main_gc_ptr = &gcmain, .is_working = TRUE};
    runner(&args);

    return 0;
}