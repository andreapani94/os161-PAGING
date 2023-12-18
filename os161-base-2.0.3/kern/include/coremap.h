#ifndef COREMAP_H
#define COREMAP_H

#include "opt-paging.h"
#include <types.h>

#if OPT_PAGING

void coremap_init(void);
paddr_t coremap_alloc(void);

#endif

#endif