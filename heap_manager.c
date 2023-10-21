#include "heap_manager.h"
#include "threadctrl.h"

#define SIZE_BITMASK (0xff)
#define DATATYPE_BITMASK (~0x1)

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


void* heap_alloc(size_t size){
    void* heap_chunk = malloc(size + ALLOC_OVERHEAD);
    alloc_chunk* chnk = (alloc_chunk*) heap_chunk;
    set_chunk_size(chnk, size);

}