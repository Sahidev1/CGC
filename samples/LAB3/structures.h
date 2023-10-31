#include <stdio.h>

#ifndef STRUCTURES_H_
#define STRUCTURES_H

#include "CGC.h"

#define PAGE_TABLE_SIZE 256
#define FRAME_SIZE 256
#define SET_USED 0x80000000
#define DISK_PATH "samples/LAB3/data/BACKING_STORE.bin"
#define RIGHT_16BIT_MASK 0x0000ffff
#define PAGE_OFFSET_MASK 0x000000ff
#define READ_MODE "r"
#define TBL_ENTRIES 16

typedef struct decoded_virt {
    int page_nr;
    int offset;
} decoded_virt;

typedef struct fnode {
    int frame_address;
    struct fnode *next;
}fnode;

typedef struct fifo{
    fnode *head;
}fifo;

typedef struct disc_reader{
    FILE *fstream;
}disc_reader;

typedef struct page_table {
    int page_array[PAGE_TABLE_SIZE];
}page_table;

typedef struct tnode {
    struct tnode *next;
    struct tnode *prev;
    int page_nummer;
    int frame_address;
}tnode;

typedef struct TBL {
    int max_entries;
    int curr_entries;
    tnode *head;
    tnode *tail;
}TBL;

int get_page_base_addr (page_table *pt, int pgnr, int *base_addr_ptr);

int set_page_base_addr (page_table *pt, int pgnr, int base_addr);

int find_base_addr (page_table *pt, int base_addr);

void init_page_table(page_table *pt);

int is_page_free(page_table *pt, int pgnr);

int init_freelist (fifo *freelist);

int get_free_frame_baddr (fifo *freelist);

int deque_free_frame (fifo *freelist, int *addr);

void translate_virt_addr (decoded_virt *pi, int logaddr);

void print_page_info (decoded_virt *pi);

void open_disk (disc_reader *disc);

int read_disk (disc_reader *disc, char* buffer ,int pgnr);

int write_frame (char *page_buffer, char *PHYS_MEM, int base_addr);

// TBL FUNCTIONS BELOW

void init_TBL(TBL *tbl_cache);

int TBL_enqueue(TBL *tbl_cache, int pgnr, int frame_addr);

int TBL_dequeue(TBL *tbl_cache, int* page_nr, int* frame_addr);

int TBL_peek (TBL *tbl_cache, int page_nr, int *frame_addr);


#endif
