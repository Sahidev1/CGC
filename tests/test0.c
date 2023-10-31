#include <stdio.h>

#include <unistd.h>
#include "CGC.h"


void square(int val, int* ret){
    *ret = val*val;
}

int main_gc(void){
    printf("10 seconds before test start\n");
    sleep(10);

    start_autoGC();

    int val = 11;
    square(val, &val);
    //printf("val: %d\n", val);
    printf("stack addr: %p\n", &val);

    int arr_size = 750*1000*1000;
    int* arr = palloc(sizeof(int)*arr_size, REFERENCE);

    //void* hptr = ((void*) arr) - ALLOC_OVERHEAD;
   /*printf("ptr addr: %p / %p, info val: %p\n", hptr, 
        arr, (void*)((alloc_chunk*) hptr)->info);*/
        
    /*for(int i = 0; i < arr_size; i++){
        printf("number: %d\n",arr[i]);
    }*/
  
    
    char* carr = palloc(sizeof(char)*arr_size, REFERENCE);
    carr[arr_size % 233] = 'p';
    long* vals = palloc(sizeof(long)*arr_size, REFERENCE);

    /*for(int i = 0; i < arr_size; i++){
        arr[i] = rand()%48183;
    }*/


    int** ptr = palloc(sizeof(int**), REFERENCE);
    *ptr = palloc(sizeof(int), REFERENCE);
    *(*ptr) = 124;

    sleep(2);
    ptr = NULL;
    carr = NULL;
    sleep(5);
    arr = NULL;
    sleep(5);
    vals = NULL;
    sleep(10);
    stop_autoGC();

     
    //heap_dealloc((alloc_chunk*) hptr);

   // printf("Dealloced\n");
    //sleep(5);
    //heap_dealloc((alloc_chunk*) ((void*) carr - ALLOC_OVERHEAD));
    //heap_dealloc((alloc_chunk*) ((void*) vals - ALLOC_OVERHEAD));
    return 0;
}

int main(void){
    shared_args warg = {
        .main_gc_ptr = &main_gc,
        .is_working=TRUE
    };
    shared_args *const wargs = &warg;
    runner(wargs);   

    return 0;
}