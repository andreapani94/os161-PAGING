#ifndef VMSTATS_H
#define VMSTATS_H

#include "opt-paging.h"


/* struct needed by print_vm_stats in order to display statistics about vm */
struct vmstats {
    unsigned int vms_tlbfaults;         /* The number of TLB misses */
    unsigned int vms_tlbfaultsfree;     /* The number of TLB misses for which there was free space in the TLB */
    unsigned int vms_tlbfaultsreplace;  /* The number of TLB misses for which replacement occured */
    unsigned int vms_tlbinvalidations;  /* The number of times the TLB was invalidated */
    unsigned int vms_tlbreloads;        /* The number of TLB misses for pages that were already in memory */  
    unsigned int vms_pagefaultszeroed;  /* The number of TLB misses that required a page to be zero filled */
    unsigned int vms_pagefaultsdisk;    /* The number of TLB misses that required a page to be loaded from disk */
    unsigned int vms_pagefaultself;     /* The number of page faults that required getting a page from the ELF file */
    unsigned int vms_pagefaultsswapfile; /* The number of page faults that required getting a page from the SWAPFILE */                 
    unsigned int vms_swapfilewrites;    /* The number of page faults that required writing a page from the SWAPFILE */
};

extern struct vmstats vms;

/* Prints a bunch of statistics related to the virtual memory subsystem */
void print_vm_stats(void);

#endif /* _VM_STATS_H */