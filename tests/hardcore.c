#include <unistd.h>
#include <stdio.h>
#include "CGC.h"
#include <stdlib.h>
#include <time.h>

time_t seed;

static int print_nr = 0;

void prnt(char* str){
    printf("print: %d, msg:\n%s\n",++print_nr,str);
}

int randmax(int max){
    return rand() %(max + 1);
}

void nestedptr_test(){
    prnt("nested ptr test starting");
    


    int nr_allocs = 5;
    int max_size = 40;
    int max_sz_nsted = 50;
    int64_t*** stackallocs[nr_allocs];
    printf("stackadd: %p\n", &max_size);

    for (int i = 0; i < nr_allocs; i++)
    {
        stackallocs[i] = alloc(sizeof(int64_t***)*max_size);
    }
    sleep(3);
    prnt ("stack allocs init");



    for (int j = 0; j < nr_allocs; j++)
    {
        for (int i = 0; i < max_size; i++)
        {
            *(stackallocs[j] + i) =  alloc(sizeof(int64_t**)*max_sz_nsted);
        }
        
    }
    prnt("second level nested allocs done");
    sleep(3);

    for (int i = 0; i < nr_allocs; i++)
    {
        for (int j = 0; j < max_size; j++)
        {
            
                for (int k = 0; k < max_sz_nsted; k++)
                {
                    *(stackallocs[i][j] + k) = palloc(sizeof(int64_t*)*100000, VALUE);
                }
                
            
        }
        
    }
    
    prnt("third level nest allocs done");
    sleep(3);
    
    for (int i = 0; i < nr_allocs; i++)
    {
        stackallocs[i] = NULL;
    }
    prnt("all stack allocks nullified");
    sleep(10);
    
    
}

void mullallocs(){

    int max_allocs = 50;
    int64_t* allocptrs[max_allocs];
    printf("Test seed number: %ld\n", seed);

    int nr_allocs = randmax(max_allocs);
    printf("nr allocs: %d\n", nr_allocs);
    int max_heap_alloc_size = 10000000;
    for (int i = 0; i < nr_allocs; i++)
    {
        allocptrs[i] = alloc(sizeof(int64_t)*randmax(max_heap_alloc_size)); 
    }
    sleep(3);
    prnt("all allocated");

    for (int i = 0; i < nr_allocs; i++)
    {
        if(randmax(2)==2) {
            prnt("alloc nulled");
            allocptrs[i] = NULL;
        }
        sleep(1);
    }
    sleep(5);
    for (int i = 0; i < nr_allocs; i++)
    {   
        allocptrs[i] = NULL;
    }
    prnt("all nulled");
    sleep(5);
    
}

int gcmain(){
    sleep(12);
    start_autoGC();
    seed = time(NULL);
    srand(seed);
    mullallocs();
    nestedptr_test();
    sleep(1);
    int* ptr = palloc(sizeof(int)*1000*10, VALUE);
    //test with nested pointers

    stop_autoGC();
    return 0;
}

int main(){
    shared_args shargs = {.main_gc_ptr = &gcmain, .is_working=TRUE};
    runner(&shargs);
    return 0;
}