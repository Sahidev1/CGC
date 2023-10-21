#ifndef HEAP_MANAGER_H
#define HEAP_MANAGER_H

#include <stdint.h>
#include <stdlib.h>

#define ALLOC_OVERHEAD sizeof(alloc_chunk)

typedef enum{REFERENCE,VALUE}datatype;

typedef struct Alloc_chunk{
    int64_t info;
    struct Alloc_chunk* next;
}alloc_chunk;

void* heap_alloc(size_t size);


#endif