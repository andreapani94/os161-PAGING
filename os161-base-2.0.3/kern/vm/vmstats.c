#include <types.h>
#include <lib.h>

#include "vmstats.h"

struct vmstats vms;

void 
print_vm_stats(void)
{
    kprintf("-----------VM STATS:------------:\n");
    kprintf("\tTLB faults: %u\n", vms.vms_tlbfaults);
    kprintf("\tTLB faults with entry added in free space: %u\n", vms.vms_tlbfaultsfree);
    kprintf("\tTLB faults with entry added with replacement: %u\n", vms.vms_tlbfaultsreplace);
    kprintf("\tTLB invalidations: %u\n", vms.vms_tlbinvalidations);
    kprintf("\tTLB reloads: %u\n", vms.vms_tlbreloads);
}