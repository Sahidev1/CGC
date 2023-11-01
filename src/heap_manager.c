#include "heap_manager.h"
#include "stdio.h" //this is for debugging only

#define SIZE_BITMASK (0xffff)
#define DATATYPE_BITMASK (~0x1)
#define REFSET_BITMASK (~0x2)
#define SIGMASK (~0xfffc)

#define GC_TRIGGER_SIZE (1<<16)

alloc_chunk* first;
alloc_chunk* tail;

pthread_mutex_t heap_lock;
pthread_mutex_t GC_lock;

static int marker = 0x1; 

typedef enum{STACK_LAYOUT_UNKNOWN, STACK_LAYOUT_KNOWN}stacklayout;

static stacklayout stklay = STACK_LAYOUT_UNKNOWN;
static void* given_stack_ptr; 
static void* given_stack_end;

static size_t alloc_buildup = 0;

static int flip_marker(int marker){
    return (~marker)&(0x1);
}

/**
 * We encode the size of the allocation on the info field as a 2^48 bit value.
*/
static int set_chunk_size (alloc_chunk* chnk, size_t size){
    size <<= 16;
    chnk->info = chnk->info & SIZE_BITMASK;
    chnk->info = chnk->info | (int64_t)size;

    return 0;
}

static size_t get_chunk_size (alloc_chunk* chnk){
    return (chnk->info&(~SIZE_BITMASK))>>16;
}

/**
 * We encode datatype of the allocation on the info field as a 1 bit flag.
*/
static int set_datatype (alloc_chunk* chnk, datatype type){
    chnk->info = chnk->info & DATATYPE_BITMASK;
    chnk->info = chnk->info | (type & ~DATATYPE_BITMASK);

    return 0;
}

/**
 * returns datatype of an alloc_chunk
*/
static datatype get_datatype(alloc_chunk* chnk){
    return chnk->info&(~DATATYPE_BITMASK);
}

/**
 * We use this to set reference state, second least significant bit is set to
 * 0 or 1. 
*/
static int set_refstate (alloc_chunk* chnk, refstate set_state){
    chnk->info = chnk->info&REFSET_BITMASK;
    chnk->info = chnk->info|(set_state<<1);

    return 0;
}

/**
 * This function returs the reference state of the heap chunk.
*/
static int get_refstate(alloc_chunk* chnk){
    return (chnk->info&(~REFSET_BITMASK))>>1;
}

static int set_signature(alloc_chunk* chnk){
    int64_t signature = SIGNATURE;
    chnk->info = chnk->info&SIGMASK;
    //printf("info: %p, SIGMASK %p, signature: %ld\n",(void*) chnk->info,(void*) SIGMASK, signature);
    chnk->info = chnk->info|(signature<<2);
    //printf("updated info: %p, sigshifted %p\n", (void*)chnk->info, (void*) (signature<<2));

    return 0;
}



/**
 * Give the address of an alloc_chank we remove it from the linkedlist
 * and free the memory space associated with it. 
 * 
 * It is extremely important that it is 100% certain that
 * chnk points to an allocated heap chunk on the linked list. 
*/
static int heap_dealloc(alloc_chunk* chnk){
    //printf("dealloc addr: %p\n", chnk);

    pthread_mutex_lock(&heap_lock);

    alloc_chunk* prev = chnk->prev;
    alloc_chunk* next = chnk->next;
    if(chnk == first){
        first = first->next;

        if(first != NULL)first->prev = NULL;
        else tail = NULL;
    }
    else if(next == NULL){
        prev->next = NULL;
        tail = prev;
    } else {
        prev->next = next;
        next->prev = prev;
    }
    free(chnk);
    pthread_mutex_unlock(&heap_lock);

    return 0;
}

/**
 * This function takes an address and sees if it points to any chunks in the alloc_chunk linked list, if it does the alloc chunk gets marked.
 * Then the function goes through the alloc chunks data_segment to see if there is any potential heap pointers there by recursively calling this function again.
*/
static int search_and_mark(void* pot_heapaddr){
    alloc_chunk* iter = first;
    datatype dtype = REFERENCE; 
    
    while(iter != NULL){
        if(((void*)iter) + ALLOC_OVERHEAD == pot_heapaddr){
            // If chunk is already marked then we leave. This should prevent being stuck in cyclical references.
            if (get_refstate(iter) == marker) return 0; 
            set_refstate(iter, marker);
            dtype = get_datatype(iter);
            break;
        }
        iter = iter->next;
    }


    if(iter != NULL && dtype == REFERENCE){
        void* heap_ptr = ((void*) iter) + ALLOC_OVERHEAD;
        void* end = heap_ptr + get_chunk_size(iter);
        void* derefed;
        int word_incr = 8;
        while (heap_ptr <= end){
            derefed = (void*) *((int64_t*) heap_ptr);
            if(derefed != NULL)search_and_mark(derefed);
            heap_ptr += word_incr;
        }
    }
    
    return 0;
}

/**
 * This function clears all alloc_chunks reference state to unmarked. 
*/
static int clear_marks(){
    alloc_chunk* iter = first;
    while(iter != NULL){
        set_refstate(iter,flip_marker(marker));
        iter = iter->next;
    }

    return 0;
}

/**
 * The mark phase starting from the stack, we travel through the stack space in 8 byte increments, and for each non NULL 
 * 8 byte value we call the function search_and_mark
*/
static int mark(void* stack_ptr, void* stack_end){
    clear_marks();
    int void_decr = 8;

    void* pot_ptr;
    while(stack_ptr > stack_end){
        pot_ptr = (void*) *((int64_t*) stack_ptr); // extracting 8 byte value at stack address and casting it to a void pointer.
        if (pot_ptr != (void*) 0){
            search_and_mark(pot_ptr);
        }

        stack_ptr -= void_decr;
    }

    return 0;
}

//for debugging
void chunk_iterator (){
    alloc_chunk* iter = first;
    int chunk_count=0;
    while(iter != NULL){
        printf("chunck addr: %p, ", iter);
        printf("data type: %d, get refstate: %d, get size: %d, ", (int) get_datatype(iter),
        (int) get_refstate(iter),(int) get_chunk_size(iter));
        printf("chunk_count: %d\n", ++chunk_count);
        iter = iter->next;
    }
} 

//for debugging
int stack_iterator (){
    if(STACK_LAYOUT_UNKNOWN) return 1;
    void* stackptr = given_stack_ptr;
    void* end = given_stack_end;
    void* word;
    int consecutive_nulls = 0;
    while (stackptr > end){
        word = (void*) (*((int64_t*)stackptr));
        if (word == NULL) consecutive_nulls++;
        else {
            consecutive_nulls = 0;
            printf("stack addr: %p , word: %p\n", stackptr, word);
        }
        stackptr -= 8;
    }
}

/**
 * This function sweeps clean all unmarked(unreachable from the stack) chunks, freeing memory.
*/
static int sweep(){
    alloc_chunk* iter = first;
    alloc_chunk* next;
    while(iter != NULL){
        //printf("refstate: %d\n", (int) get_refstate(iter));
        next = iter->next;
        if(get_refstate(iter) != marker){
            heap_dealloc(iter);
        }
        iter = next;
    }
    return 0;
}

int gc_runs = 0; // remove this for debugging only!

/**
 * Garbage collection function that performs a mark and sweep
*/
int GC(void* stack_ptr, void* stack_end){
    pthread_mutex_lock(&GC_lock);
    mark(stack_ptr, stack_end);
    sweep();
    marker = flip_marker(marker);
    pthread_mutex_unlock(&GC_lock);
    return 0;
}

int set_stack_layout (void* stackptr, void* stackend){
    stklay = STACK_LAYOUT_KNOWN;
    given_stack_ptr = stackptr;
    given_stack_end =  stackend;

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
static void* heap_alloc(size_t size, datatype type){
    pthread_mutex_lock(&GC_lock);
    pthread_mutex_lock(&heap_lock);

    int padding = 8 - (size%8);
    padding = padding == 8?0:padding;
    void* heap_chunk = calloc(1,size + ALLOC_OVERHEAD + padding);
    alloc_chunk* chnk = (alloc_chunk*) heap_chunk;
    set_chunk_size(chnk, size);
    set_datatype(chnk, type);
    set_refstate(chnk, flip_marker(marker));
    set_signature(chnk);
    chnk->next = NULL;

    //printf("chunk size: %ld, get chunk size: %ld\n",size, get_chunk_size(chnk));

    //printf ("alloced addr: %p\n", chnk);
    //printf("val hex: %#lx\n", (uint64_t)chnk->info);

    if (first == NULL){
        first = chnk;
        chnk->prev = NULL;
        tail = first;
    } else {
        tail->next = chnk;
        chnk->prev = tail;
        tail = tail->next;
    }

    pthread_mutex_unlock(&heap_lock);
    pthread_mutex_unlock(&GC_lock);

    return ((void*) chnk) + ALLOC_OVERHEAD; 
}

static int inner_GC_call(){
    if (stklay == STACK_LAYOUT_UNKNOWN) return 1;

    if(alloc_buildup > (size_t) GC_TRIGGER_SIZE){
        GC(given_stack_ptr, given_stack_end);
        alloc_buildup = 0;
    }
    return 0;
}

int runGC (){
    if (stklay == STACK_LAYOUT_UNKNOWN) return 1;
    GC(given_stack_ptr, given_stack_end);
    alloc_buildup = 0;
    return 0;
}

/**
 * The function users should call to allocate conservatively
*/
void* alloc (size_t size){
    alloc_buildup += size;
    return heap_alloc(size, REFERENCE);
}

/**
 * The function users should call to allocate precisely.
*/
void* palloc (size_t size, datatype type){
    alloc_buildup += size;
    return heap_alloc(size, type);
}

void* GC_palloc (size_t size, datatype type){
    alloc_buildup += size;
    inner_GC_call();
    return heap_alloc(size, type);
}

void* GC_alloc (size_t size){
    alloc_buildup += size;
    inner_GC_call();
    return heap_alloc(size, REFERENCE);
}

/**
 * Function that initializes the mutex locks
*/
void initialize_locks(){
    pthread_mutex_init(&heap_lock, NULL);
    pthread_mutex_init(&GC_lock, NULL);
}

/**
 * destroy mutex locks
*/
void destroy_locks(){
    pthread_mutex_destroy(&heap_lock);
    pthread_mutex_destroy(&GC_lock);
}
