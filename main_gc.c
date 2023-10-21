#include <stdio.h>
#include "heap_manager.h"

void square(int val, int* ret){
    *ret = val*val;
}

int main_gc(void){
    int val = 11;
    square(val, &val);
    printf("val: %d\n", val);

    int arr_size = 500*1000*1000;
    int* arr = heap_alloc(sizeof(int)*arr_size, REFERENCE);
    for(int i = 0; i < arr_size; i++){
        arr[i] = rand()%48183;
    }
    void* hptr = ((void*) arr) - ALLOC_OVERHEAD;
    printf("ptr addr: %p / %p, info val: %ld\n", hptr, 
        arr, ((alloc_chunk*) hptr)->info);
        
    /*for(int i = 0; i < arr_size; i++){
        printf("number: %d\n",arr[i]);
    }*/

    heap_dealloc((alloc_chunk*) hptr);
   

    return 0;
}