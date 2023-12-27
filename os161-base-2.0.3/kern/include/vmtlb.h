#ifndef VMTLB_H
#define VMTLB_H

#include "opt-paging.h"
#include <types.h>

int vmtlb_insert(vaddr_t, paddr_t, bool);
void vmtlb_reset(void);

#endif
