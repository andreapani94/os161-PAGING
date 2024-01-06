#ifndef COREMAP_H
#define COREMAP_H

#include "opt-paging.h"
#include <types.h>

#define COREMAP_INDEX(paddr) (paddr / PAGE_SIZE)

struct coremap_entry {
    bool is_free;
};

void coremap_init(void);
paddr_t coremap_alloc(void);
paddr_t coremap_kalloc(unsigned);
void coremap_free(paddr_t);
void coremap_kfree(paddr_t, unsigned);

#endif