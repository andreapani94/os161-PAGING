#include <types.h>
#include <lib.h>

#include "vmstats.h"

struct vmstats vms;

void 
print_vm_stats(void)
{
    kprintf("-----------VM STATS:------------:\n");
    kprintf("\tTLB faults: %u\n", vms.vms_tlbfaults);
    kprintf("\tTLB faults with free space available: %u\n", vms.vms_tlbfaultsfree);
}