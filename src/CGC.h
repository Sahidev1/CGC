#ifndef CGC_H
#define CGC_H

#define GNU_SOURCE
#include <string.h>
#include "heap_manager.h"
#include "shared_struct.h"


int runner(shared_args* args);
int start_autoGC();
int stop_autoGC();

#endif