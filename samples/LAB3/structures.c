#include "structures.h"
#include <stdio.h>

/* page table functions */

void init_page_table(page_table *pt){
    for(int i = 0; i < PAGE_TABLE_SIZE; i++){
        pt->page_array[i] = 0;
    }
}

int get_page_base_addr (page_table *pt, int pgnr, int *base_addr_ptr){
    if (pgnr >= PAGE_TABLE_SIZE) return -1;
    *base_addr_ptr = pt->page_array[pgnr]&(~(SET_USED));
    return 0;
}

/* checks if page slot is free(holds no valid frame address), page slots that have never
    been written to have free bit set to 0, free bit is
    MSB.
*/
int is_page_free(page_table *pt, int pgnr){
    int addr = pt->page_array[pgnr];
    return (addr & SET_USED) == 0x0;
}

int set_page_base_addr (page_table *pt, int pgnr, int base_addr){
    if (pgnr >= PAGE_TABLE_SIZE) return -1;
    if (pt->page_array[pgnr] == (base_addr | SET_USED)) return -2;
    pt->page_array[pgnr] = base_addr | SET_USED;
    return 0;
}

int find_base_addr (page_table *pt, int base_addr){
    for (int i = 0; i < PAGE_TABLE_SIZE; i++){
        if (pt->page_array[i] == (base_addr|SET_USED)){
            return i;
        }
    }
    return -1;
}

/* Free frame list functions*/

int init_freelist (fifo *freelist){
    freelist->head = alloc (sizeof(fnode));
    fnode *iterator = freelist->head;
    int i = 0;
    while(i < FRAME_SIZE){
        iterator->frame_address = 256*i;
        iterator->next = alloc (sizeof(fnode));
        iterator = iterator->next;
        i++;
    }
    return 0;
}

int get_free_frame_baddr (fifo *freelist){
    int addr = -1;
    deque_free_frame(freelist, &addr);
    return addr;
}

int deque_free_frame (fifo *freelist, int *addr){
    if (freelist->head == NULL) return -1;
    fnode *head = freelist->head;
    *addr = head->frame_address;
    freelist->head = freelist->head->next;
    return 0;
}



/* virtual address decoded functions */

void translate_virt_addr (decoded_virt *pi, int logaddr){
    logaddr &= RIGHT_16BIT_MASK;
    pi->offset = logaddr & PAGE_OFFSET_MASK; 
    logaddr >>= 8;
    pi->page_nr = logaddr & PAGE_OFFSET_MASK;
}

void print_page_info (decoded_virt *pi){
    printf("Page Nr: %d, Offset: %d \n", pi->page_nr, pi->offset);
}

/* Disc functions*/

void open_disk (disc_reader *disc){
    disc->fstream = fopen(DISK_PATH, READ_MODE);
}

int read_disk (disc_reader *disc, char* buffer ,int pgnr){
    int offset = pgnr*256;
    fseek(disc->fstream, offset, SEEK_SET);
    fread(buffer, FRAME_SIZE + 1, 1, disc->fstream);
    rewind(disc->fstream);
    return 0;
}

/* TBL functions */

void init_TBL(TBL *tbl_cache){
    tbl_cache->max_entries = TBL_ENTRIES;
    tbl_cache->curr_entries = 0;
    tbl_cache->head = NULL;
    tbl_cache->tail = NULL;
}

int TBL_enqueue(TBL *tbl_cache, int pgnr, int frame_addr){
    if (tbl_cache->head == NULL){
        tbl_cache->head = alloc (sizeof(tnode));
        printf ("made it here\n");
        tbl_cache->head->frame_address = frame_addr;
        tbl_cache->head->page_nummer = pgnr;
        tbl_cache->tail = tbl_cache->head;
        tbl_cache->head->next = NULL;
        tbl_cache->curr_entries++;
        return 0;
    }
    tnode *enqueue;
    while (tbl_cache->curr_entries >= tbl_cache->max_entries){
        TBL_dequeue(tbl_cache, NULL, NULL);
    }
    if (tbl_cache->curr_entries < tbl_cache->max_entries){
        enqueue = alloc (sizeof(tnode));
        enqueue->frame_address = frame_addr;
        enqueue->page_nummer = pgnr;
        enqueue->next = tbl_cache->head;
        tbl_cache->head->prev = enqueue;
        enqueue->prev = NULL;
        tbl_cache->head = enqueue;
        tbl_cache->curr_entries++;
        return 0;
    }
    return -1;
}

int TBL_dequeue(TBL *tbl_cache, int* page_nr, int* frame_addr){
    if (tbl_cache->head == NULL || tbl_cache->tail == NULL) return -1;
    if (page_nr != NULL && frame_addr != NULL){
        *page_nr = tbl_cache->tail->page_nummer;
        *frame_addr = tbl_cache->tail->frame_address;
    }
    tnode *temp = tbl_cache->tail;
    tbl_cache->tail = tbl_cache->tail->prev;
    if (tbl_cache->tail == NULL){
        tbl_cache->head == NULL;
    }
    else {
        tbl_cache->tail->next = NULL;
    }
    tbl_cache->curr_entries--;
    return 0;
}

int TBL_peek (TBL *tbl_cache, int page_nr, int *frame_addr){
    tnode *head = tbl_cache->head;
    while (head != NULL){   
        if(head->page_nummer == page_nr){
            *frame_addr = head->frame_address;
            return 0;
        }
        head = head->next;
    }
    return -1;
}

/* Other functions*/

int write_frame (char *page_buffer, char *PHYS_MEM, int base_addr){
    int lastIndex = base_addr + FRAME_SIZE - 1;
    int i = base_addr;
    int j = 0;
    while (i <= lastIndex){
        PHYS_MEM[i++] = page_buffer[j++];
    }
}