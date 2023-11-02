# CGC
A C mark and sweep garbage collector for linux using pthreads. 

A worker thread is used to run a program, while the garbage collection threads job is to periodically scan the workers threads stack and find references from the stack to the heap space.

Work in progress. 
