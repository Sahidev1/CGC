#include <stdio.h>
#include "heap_manager.h"
#include <unistd.h>

void square(int val, int* ret){
    *ret = val*val;
}

int main_gc(void){
    int val = 11;
    square(val, &val);
    //printf("val: %d\n", val);
    printf("stack addr: %p\n", &val);

    int arr_size = 750*1000*1000;
    int* arr = heap_alloc(sizeof(int)*arr_size, VALUE);

    void* hptr = ((void*) arr) - ALLOC_OVERHEAD;
   /*printf("ptr addr: %p / %p, info val: %p\n", hptr, 
        arr, (void*)((alloc_chunk*) hptr)->info);*/
        
    /*for(int i = 0; i < arr_size; i++){
        printf("number: %d\n",arr[i]);
    }*/
  
    
    char* carr = heap_alloc(sizeof(char)*arr_size, VALUE);
    carr[arr_size % 233] = 'p';
    long* vals = heap_alloc(sizeof(long)*arr_size, REFERENCE);

    /*for(int i = 0; i < arr_size; i++){
        arr[i] = rand()%48183;
    }*/




    int** ptr = heap_alloc(sizeof(int**), REFERENCE);
    *ptr = heap_alloc(sizeof(int), VALUE);
    *(*ptr) = 124;

    sleep(2);
    ptr = NULL;
    carr = NULL;
    sleep(5);
    arr = NULL;
    sleep(5);
    vals = NULL;
    sleep(5);

     
    //heap_dealloc((alloc_chunk*) hptr);

   // printf("Dealloced\n");
    //sleep(5);
    //heap_dealloc((alloc_chunk*) ((void*) carr - ALLOC_OVERHEAD));
    //heap_dealloc((alloc_chunk*) ((void*) vals - ALLOC_OVERHEAD));
   

    return 0;
}