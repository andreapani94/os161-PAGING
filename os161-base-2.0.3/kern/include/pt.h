#ifndef PT_H
#define PT_H

#include <types.h>

#define OUTER_PT_SIZE 1024 /* 10 leftmost bits of the virtual address */
#define INNER_PT_SIZE 1024
#define OUTER_PT_INDEX(vaddr) (vaddr >> 22)
#define INNER_PT_INDEX(vaddr) (vaddr >> 12) & 0x3FF

struct addrspace;
struct segment;

/*
struct pt_entry_1 {
    struct pt_entry_2* inner_pt;
};

struct pt_entry_2 {
    paddr_t paddr;
    bool valid;
    bool dirty;
    bool swapped;
}; */


struct pt_entry {
    paddr_t paddr;  // this will hold the SWAPFILE index in case of swapped pages 
    bool valid;
    bool swapped;
};


int pt_insert(struct addrspace*, vaddr_t, paddr_t);
struct pt_entry* pt_get(struct addrspace* as, vaddr_t);
int inner_pt_create(struct addrspace* as, vaddr_t);
int pt_load(struct segment* s, vaddr_t vaddr, paddr_t paddr);

#endif