#include <stdio.h>
#include "heap_manager.h"

void square(int val, int* ret){
    *ret = val*val;
}

int main_gc(void){
    int val = 11;
    square(val, &val);
    printf("val: %d\n", val);

    int arr_size = 25*1000;
    int* arr = heap_alloc(sizeof(int)*arr_size, VALUE);
    for(int i = 0; i < arr_size; i++){
        arr[i] = rand()%48183;
    }
    void* hptr = ((void*) arr) - ALLOC_OVERHEAD;
    printf("ptr addr: %p / %p, info val: %p\n", hptr, 
        arr, (void*)((alloc_chunk*) hptr)->info);
        
    /*for(int i = 0; i < arr_size; i++){
        printf("number: %d\n",arr[i]);
    }*/

    char* carr = heap_alloc(sizeof(char)*arr_size, VALUE);
    carr[arr_size % 233] = 'p';

    heap_dealloc((alloc_chunk*) hptr);
    heap_dealloc((alloc_chunk*) ((void*) carr - ALLOC_OVERHEAD));
   

    return 0;
}