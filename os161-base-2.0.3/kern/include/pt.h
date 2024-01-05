#ifndef PT_H
#define PT_H

#include <types.h>
#include <addrspace.h>

struct addrspace;

#define OUTER_PT_SIZE 1024 /* 10 leftmost bits of the virtual address */
#define INNER_PT_SIZE 1024
#define OUTER_PT_INDEX(vaddr) (vaddr >> 22)
#define INNER_PT_INDEX(vaddr) (vaddr >> 12) & 0x3FF

struct pt_entry_1 {
    struct pt_entry_2* inner_pt;
};

struct pt_entry_2 {
    paddr_t paddr;
    bool valid;
    bool dirty;
    bool swapped;
};

paddr_t pt_translate(struct addrspace* as, vaddr_t);
int pt_insert(struct addrspace*, vaddr_t, paddr_t);
struct pt_entry* pt_get(struct addrspace* as, vaddr_t);
void inner_pt_create(struct pt_entry_1* outer_pt, uint32_t outer_pt_index);
int pt_load(struct addrspace* as, vaddr_t vaddr, paddr_t paddr);

#endif