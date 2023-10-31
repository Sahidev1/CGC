
#include <string.h>
//#include <stdlib.h>
#include "structures.h"

#define READ_BUFFER_SIZE 32
#define MEM_SIZE (1<<16)

char* PHYSICAL_MEMORY;
char page_buffer[FRAME_SIZE + 1];

int debug_counter = 0;
char *read_buffer;

int TBL_hit_count, page_fault_count;

void print_debug(char* msg){
    printf("debug(Nr: %d, Msg: %s\n )\n", debug_counter++, msg);
}

void print_data (int virtual_address, int physical_address, int stored_value){
        printf("Virtual address: %d, Physical address: %d, Value stored: %d\n", virtual_address, physical_address, stored_value);
}

int maingc (void){
    //start_autoGC();

    TBL_hit_count = 0;
    page_fault_count = 0;
    FILE *fptr;
    disc_reader disc; 
    decoded_virt decoder;
    fifo *list;
    page_table pt;
    TBL *tbl_cache;

    PHYSICAL_MEMORY = alloc(MEM_SIZE*sizeof(char));
    list = alloc(sizeof(fifo));
    tbl_cache = alloc(sizeof(TBL));

    print_debug("allocs done\n");

    fptr = fopen("/home/ali/Documents/projects/CGC/samples/LAB3/data/addresses.txt", READ_MODE);
    if (fptr == NULL) {
        printf ("failed to open file, make sure path is correct!\n");
        return -1;
    }

        print_debug("file read done\n");

    init_page_table(&pt);
    init_freelist(list);
    open_disk(&disc);
    init_TBL(tbl_cache);

    print_debug("inits called");

    read_buffer = alloc(sizeof(char) * READ_BUFFER_SIZE);
    print_debug("read buffer alloc done");
    size_t size = (size_t) READ_BUFFER_SIZE;
    int virtual_address = -1;
    int base_address;
    int physical_address;
    char stored_value; 
    runGC();
    while (getline(&read_buffer, &size, fptr) != -1){
        virtual_address = atoi(read_buffer);
        translate_virt_addr(&decoder, virtual_address);
        if(TBL_peek(tbl_cache, decoder.page_nr, &base_address) == 0){
            TBL_hit_count++;
            physical_address = base_address + decoder.offset;
            stored_value = PHYSICAL_MEMORY[physical_address];
            print_data(virtual_address, physical_address, stored_value);
            print_debug("pong");
            continue;
        }

        if (is_page_free(&pt, decoder.page_nr)){
            base_address = get_free_frame_baddr(list);
            page_fault_count++;
            print_debug("dong");
            if (base_address == -1) {
                printf("Error: Out of physical memory!\n");
                return -1;
            }
            set_page_base_addr(&pt, decoder.page_nr, base_address);
            read_disk(&disc, page_buffer, decoder.page_nr);
            write_frame(page_buffer, PHYSICAL_MEMORY ,base_address);
        }

        get_page_base_addr(&pt, decoder.page_nr, &base_address);
        TBL_enqueue (tbl_cache, decoder.page_nr, base_address);
        print_debug("dongalong");
        physical_address = base_address + decoder.offset;
        stored_value = PHYSICAL_MEMORY[physical_address];
        print_data(virtual_address, physical_address, stored_value);
        print_debug("song");
    }
    print_debug("out of while loop");

    printf("Page fault count: %d, TBL Hitrate: %d\n", page_fault_count, TBL_hit_count);
    

    list = NULL;
    tbl_cache= NULL;
    PHYSICAL_MEMORY = NULL;
    read_buffer=NULL;

    //stop_autoGC();
    //runGC();
    return 0;
}

int main(){
    shared_args arg = {.main_gc_ptr =&maingc,.is_working=TRUE};
    runner(&arg);
    return 0;
}