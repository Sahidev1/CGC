#include "heap_manager.h"
#include "stdio.h" //this is for debugging only

#define SIZE_BITMASK (0xff)
#define DATATYPE_BITMASK (~0x1)
#define REFSET_BITMASK (~0x2)

alloc_chunk* first;

int set_chunk_size (alloc_chunk* chnk, size_t size){
    size <<= 8;
    chnk->info = chnk->info & SIZE_BITMASK;
    chnk->info = chnk->info | (int64_t)size;

    return 0;
}

int set_datatype (alloc_chunk* chnk, datatype type){
    chnk->info = chnk->info & DATATYPE_BITMASK;
    chnk->info = chnk->info | (type & ~DATATYPE_BITMASK);

    return 0;
}

int set_refstate (alloc_chunk* chnk){
    chnk->info = chnk->info&REFSET_BITMASK;

    return 0;
}


void* heap_alloc(size_t size, datatype type){
    void* heap_chunk = malloc(size + ALLOC_OVERHEAD);
    alloc_chunk* chnk = (alloc_chunk*) heap_chunk;
    set_chunk_size(chnk, size);
    set_datatype(chnk, type);
    set_refstate(chnk);
    chnk->next = NULL;

    //printf ("alloced addr: %p\n", chnk);
    //printf("val hex: %#lx\n", (uint64_t)chnk->info);

    if (first == NULL){
        first = chnk;
    } else {
        alloc_chunk* iter = first; 
        while(iter->next != NULL) iter = iter->next;
        iter->next = chnk;
    }

    return ((void*) chnk) + ALLOC_OVERHEAD; 
}

int heap_dealloc(alloc_chunk* chnk){
    //printf("dealloc addr: %p\n", chnk);

    alloc_chunk* iter = first;
    alloc_chunk* prev = NULL;
    while (chnk != iter && iter != NULL){
        prev = iter;
        iter = iter->next;
    }

    if (prev == NULL){
        first = iter->next;
        free(iter);
        return 0;
    }

    prev->next = iter->next;
    free(iter);
    return 0;
}