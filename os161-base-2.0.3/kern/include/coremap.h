#ifndef COREMAP_H
#define COREMAP_H

#include "opt-paging.h"
#include <types.h>

#define COREMAP_INDEX(paddr) (paddr / PAGE_SIZE)

struct coremap_entry {
    struct addrspace* as;
    vaddr_t vaddr;
    paddr_t paddr;
    bool is_free;
};

void coremap_init(void);
paddr_t coremap_alloc(vaddr_t);
paddr_t coremap_kalloc(unsigned);
void coremap_free(paddr_t);
void coremap_kfree(paddr_t, unsigned);
paddr_t coremap_replace(void);

#endif