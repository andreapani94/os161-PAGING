#ifndef VMSTATS_H
#define VMSTATS_H

#include "opt-paging.h"


/* struct needed by print_vm_stats in order to display statistics about vm */
struct vmstats {
    unsigned int vms_tlbfaults; /* The number of TLB misses */
    unsigned int vms_tlbfaultsfree; /* The number of TLB misses for which there was free space in the TLB */
    unsigned int vms_tlbfaultsreplace; /* The number of TLB misses for which replacement occured */
    unsigned int vms_tlbinvalidations;  /* The number of times the TLB was invalidated */
    unsigned int vms_tlbreloads; /* The number of TLB misses for pages that were already in memory */                   

};

extern struct vmstats vms;

/* Prints a bunch of statistics related to the virtual memory subsystem */
void print_vm_stats(void);

#endif /* _VM_STATS_H */