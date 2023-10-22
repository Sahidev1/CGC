#ifndef HEAP_MANAGER_H
#define HEAP_MANAGER_H

#include <stdint.h>
#include <stdlib.h>

#define ALLOC_OVERHEAD sizeof(alloc_chunk)

typedef enum{REFERENCE,VALUE}datatype;
typedef enum{UNMARKED, MARKED}refstate;

typedef struct Alloc_chunk{
    int64_t info;
    struct Alloc_chunk* next;
    struct Alloc_chunk* prev;
}alloc_chunk;

int set_chunk_size (alloc_chunk* chnk, size_t size);
int set_datatype (alloc_chunk* chnk, datatype type);
int set_refstate (alloc_chunk* chnk, refstate set_state);

void* heap_alloc(size_t size, datatype type);
int heap_dealloc(alloc_chunk* chnk);


#endif