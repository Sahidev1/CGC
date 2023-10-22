#include "heap_manager.h"
#include "stdio.h" //this is for debugging only

#define SIZE_BITMASK (0xff)
#define DATATYPE_BITMASK (~0x1)
#define REFSET_BITMASK (~0x2)

alloc_chunk* first;

/**
 * We encode the size of the allocation on the info field as a 2^56 bit value.
*/
int set_chunk_size (alloc_chunk* chnk, size_t size){
    size <<= 8;
    chnk->info = chnk->info & SIZE_BITMASK;
    chnk->info = chnk->info | (int64_t)size;

    return 0;
}

/**
 * We encode datatype of the allocation on the info field as a 1 bit flag.
*/
int set_datatype (alloc_chunk* chnk, datatype type){
    chnk->info = chnk->info & DATATYPE_BITMASK;
    chnk->info = chnk->info | (type & ~DATATYPE_BITMASK);

    return 0;
}

/**
 * We use this to set reference state, second least significant bit is set to
 * 0 or 1. 
*/
int set_refstate (alloc_chunk* chnk, refstate set_state){
    chnk->info = chnk->info&REFSET_BITMASK;
    chnk->info = chnk->info|(set_state<<1);

    return 0;
}

/**
 * This function allocates a heap chunk on the heap. 
 * datatype informs whether the allocated space will hold
 * references(pointers) or non-reference data.
 * 
 * Then we add the allocated chunk to the linked list
 * of allocated chunks.
*/
void* heap_alloc(size_t size, datatype type){
    void* heap_chunk = malloc(size + ALLOC_OVERHEAD);
    alloc_chunk* chnk = (alloc_chunk*) heap_chunk;
    set_chunk_size(chnk, size);
    set_datatype(chnk, type);
    set_refstate(chnk, UNMARKED);
    chnk->next = NULL;

    //printf ("alloced addr: %p\n", chnk);
    //printf("val hex: %#lx\n", (uint64_t)chnk->info);

    if (first == NULL){
        first = chnk;
        chnk->prev = NULL;
    } else {
        alloc_chunk* iter = first; 
        while(iter->next != NULL) iter = iter->next;
        chnk->prev = iter;
        iter->next = chnk;
    }

    return ((void*) chnk) + ALLOC_OVERHEAD; 
}

/**
 * Give the address of an alloc_chank we remove it from the linkedlist
 * and free the memory space associated with it. 
 * 
 * It is extremely important that it is 100% certain that
 * chnk points to an allocated heap chunk on the linked list. 
*/
int heap_dealloc(alloc_chunk* chnk){
    //printf("dealloc addr: %p\n", chnk);

    alloc_chunk* prev = chnk->prev;
    alloc_chunk* next = chnk->next;
    if(chnk == first){
        first = first->next;
        if(first != NULL)first->prev = NULL;
    }
    else if(next == NULL){
        prev->next = NULL;
    } else {
        prev->next = next;
        next->prev = prev;
    }

    free(chnk);
    return 0;
}