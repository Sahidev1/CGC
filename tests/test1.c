#include <stdio.h>
#include <unistd.h>
#include "CGC.h"

//test passed if segfault.
int gc_main(void){
    sleep(1);
    // errounously allocating space for REFERENCES/pointers AS VALUE datatype
    int64_t** ptr_ptr = palloc(sizeof(int64_t*)*5, VALUE);

    // correctly allocating space
    int64_t** corr_ptr = palloc(sizeof(int64_t*)*5, REFERENCE);

    int arr_size = 1000*1000;
    printf("point 0\n");
    // We then allocate a few of the pointers of the alloced space to some arbitrary array.
    ptr_ptr[0] = palloc(sizeof(int64_t) * arr_size, VALUE);
    corr_ptr[0] = palloc(sizeof(int64_t) * arr_size, VALUE);
    printf("ptr_ptr: %p\n", ptr_ptr[0]);
    printf("corrptr: %p\n", corr_ptr[0]);
    printf("allocs succesfull\n");


    // Below loop will most likely cause a segfault since the GC will have freed the memory allocation associated with ptr_ptr[0] since
    // it is expected to be a VALUE so GC wont scan it and the heap chunk will remain unmarked.  
    for (int i = 0; i < arr_size-10; i++)
    {
        printf("addr: %p\n", (ptr_ptr[0]+i));
        ptr_ptr[0][i] = 11;
        //corr_ptr[0][i] = rand();
    }
    printf("passed loop\n");
    sleep(5);
    ptr_ptr[0] = NULL;
    printf("UNREFERENCED PTR\n");
    sleep(5);
    //corr_ptr[0] = NULL;
    printf("UNREFERENCED PTR\n");
    sleep(5);




    return 0;
}

int main(){
    shared_args args = {
        .main_gc_ptr = &gc_main,
        .is_working = TRUE
    };
    shared_args *const wargs = &args;
    runner(wargs);

    return 0;
}