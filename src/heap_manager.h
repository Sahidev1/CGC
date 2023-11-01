#ifndef HEAP_MANAGER_H
#define HEAP_MANAGER_H

#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define ALLOC_OVERHEAD sizeof(alloc_chunk)
#define SIGNATURE 13271 // arbitrary signature between 0 - 2^14

typedef enum{REFERENCE,VALUE}datatype;
typedef enum{UNMARKED, MARKED}refstate;

typedef struct Alloc_chunk{
    int64_t info;
    struct Alloc_chunk* next;
    struct Alloc_chunk* prev;
}alloc_chunk;

void* alloc (size_t size);
void* palloc (size_t size, datatype type);
void* GC_palloc (size_t size, datatype type);
void* GC_alloc (size_t size);


void initialize_locks();
void destroy_locks();

int GC(void* stack_ptr, void* stack_end);
int runGC ();

void chunk_iterator ();
int stack_iterator ();

int set_stack_layout (void* stackptr, void* stackend);

#endif