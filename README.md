# CGC
A C mark and sweep garbage collector for linux using pthreads. 

A worker thread is used to run a program and a garbage collector thread is used to garbage collect the worker threads unreferenced memory segments. The garbage collection threads job is to periodically scan the workers thread stack and find pointers to the heap space. The garbage collector has access to a linked list of allocated heap chunks. Each heap chunk pointer found in the stack will cause the heap chunk to be marked, then the heap chunks data segment is scanned for other heap chunk pointers, this process is repeated recursively untill we run out of heapchunk pointers or run out of unmarked heapchunks. Afther the mark phase is done all reachable heap chunks should be marked. In the sweep phase we travel through the linked list of heap chunks and free all unmarked heap chunks. 


Work in progress. 
