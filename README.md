# CGC
A C mark and sweep garbage collector for linux using pthreads. 

It's still work in progress, and many improvements are needed.

A worker thread is used to run a program and a garbage collector thread is used to garbage collect the worker threads unreferenced memory segments. The garbage collection threads job is to periodically scan the workers thread stack and find pointers to the heap space. The garbage collector has access to a linked list of allocated heap chunks. Each heap chunk pointer found in the stack will cause the heap chunk to be marked, then the heap chunks data segment is scanned for other heap chunk pointers, this process is repeated recursively untill we run out of heapchunk pointers or run out of unmarked heapchunks. Afther the mark phase is done all reachable heap chunks should be marked. In the sweep phase we travel through the linked list of heap chunks and free all unmarked heap chunks. 

## Usage

Example program:

```
#include "CGC.h" // The include header for the garbage collector functions

// This should be treated like how normal main function is used in C programs, put program code here.
int worker_main(void){
  start_autoGC(); //This starts running a garbage collector thread

  int arr_size = 100000;

//Below is the memory allocator which allocates heap space which is garbage collected.
  int* integer_arr = alloc(sizeof(int) * arr_size);


  // your program code that will be garabge collected

  stop_autoGC(); // this stops and cleans up the garbage collector thread, this should be called to avoid zombie threads

  return 0;
}

int main(){
  // Initializing struct of data shared between both threads
  shared_args args = {.main_gc_ptr = &worker_main, .is_working=TRUE};
  // setting up and running the worker thread.
  runner(&args);
  
  return 0;
}
```

How to compile your program from the root of CGC directory:
```
gcc -lpthread -Isrc myprogram.c src/heap_manager.c src/CGC.c src/thread_procedures.c

```




