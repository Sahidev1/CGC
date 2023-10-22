#include "heap_manager.h"
#include "stdio.h" //this is for debugging only

#define SIZE_BITMASK (0xffff)
#define DATATYPE_BITMASK (~0x1)
#define REFSET_BITMASK (~0x2)
#define SIGMASK (~0xfffc)

alloc_chunk* first;

pthread_mutex_t heap_lock;
pthread_mutex_t GC_lock;


/**
 * We encode the size of the allocation on the info field as a 2^48 bit value.
*/
int set_chunk_size (alloc_chunk* chnk, size_t size){
    size <<= 16;
    chnk->info = chnk->info & SIZE_BITMASK;
    chnk->info = chnk->info | (int64_t)size;

    return 0;
}

size_t get_chunk_size (alloc_chunk* chnk){
    return (chnk->info&(~SIZE_BITMASK))>>16;
}

/**
 * We encode datatype of the allocation on the info field as a 1 bit flag.
*/
int set_datatype (alloc_chunk* chnk, datatype type){
    chnk->info = chnk->info & DATATYPE_BITMASK;
    chnk->info = chnk->info | (type & ~DATATYPE_BITMASK);

    return 0;
}

datatype get_datatype(alloc_chunk* chnk){
    return chnk->info&(~DATATYPE_BITMASK);
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

refstate get_refstate(alloc_chunk* chnk){
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
int heap_dealloc(alloc_chunk* chnk){
    printf("dealloc addr: %p\n", chnk);

    pthread_mutex_lock(&heap_lock);

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
    pthread_mutex_unlock(&heap_lock);

    return 0;
}

static int search_and_mark(void* pot_heapaddr){
    //sleep(2);
    alloc_chunk* iter = first;
    
    while(iter != NULL){
        if(((void*)iter) + ALLOC_OVERHEAD == pot_heapaddr){
            set_refstate(iter, MARKED);
            break;
        }
        iter = iter->next;
    }
    

    if(iter != NULL){
     //   printf ("datatype: %d\n", (int) get_datatype(iter));
       // printf ("get chunk size: %d\n", (int) get_chunk_size(iter));
        void* heap_ptr = ((void*) iter) + ALLOC_OVERHEAD;
        void* end = heap_ptr + get_chunk_size(iter);
        void* derefed;
        int word_incr = 8;
        while (heap_ptr <= end){
            derefed = (void*) *((int64_t*) heap_ptr);
            //printf("heap_ptr: %p, derefed: %p\n", heap_ptr, derefed);
            if(derefed != NULL)search_and_mark(derefed);
            heap_ptr += word_incr;
        }
    }
    
    return 0;
}

static int clear_marks(){
    alloc_chunk* iter = first;
    while(iter != NULL){
        set_refstate(iter,UNMARKED);
        iter = iter->next;
    }

    return 0;
}

static int mark(void* stack_ptr, void* stack_end){
    clear_marks();
    int void_decr = 8;

    void* pot_ptr;
    while(stack_ptr > stack_end){
        pot_ptr = (void*) *((int64_t*) stack_ptr);
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
    while(iter != NULL){
        printf("chunck addr: %p\n", iter);
        printf("data type: %d, get refstate: %d, get size: %d\n", (int) get_datatype(iter),
        (int) get_refstate(iter),(int) get_chunk_size(iter));
        iter = iter->next;
    }
} 

static int sweep(){
    alloc_chunk* iter = first;
    alloc_chunk* next;
    while(iter != NULL){
        //printf("refstate: %d\n", (int) get_refstate(iter));
        next = iter->next;
        if(get_refstate(iter) == UNMARKED){
            heap_dealloc(iter);
        }
        iter = next;
    }
    return 0;
}

int GC(void* stack_ptr, void* stack_end){
    pthread_mutex_lock(&GC_lock);
    mark(stack_ptr, stack_end);
    chunk_iterator();
    sweep();
    pthread_mutex_unlock(&GC_lock);
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
    pthread_mutex_lock(&GC_lock);
    pthread_mutex_lock(&heap_lock);

    int padding = 8 - (size%8);
    padding = padding == 8?0:padding;
    void* heap_chunk = malloc(size + ALLOC_OVERHEAD + padding);
    alloc_chunk* chnk = (alloc_chunk*) heap_chunk;
    set_chunk_size(chnk, size);
    set_datatype(chnk, type);
    set_refstate(chnk, UNMARKED);
    set_signature(chnk);
    chnk->next = NULL;

    //printf("chunk size: %ld, get chunk size: %ld\n",size, get_chunk_size(chnk));

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

    pthread_mutex_unlock(&heap_lock);
    pthread_mutex_unlock(&GC_lock);

    return ((void*) chnk) + ALLOC_OVERHEAD; 
}



void initialize_locks(){
    pthread_mutex_init(&heap_lock, NULL);
    pthread_mutex_init(&GC_lock, NULL);
}