#include <types.h>
#include <lib.h>

#include "vmstats.h"

struct vmstats vms;

void 
print_vm_stats(void)
{
    kprintf("----------------------VM STATS:-----------------------:\n");
    kprintf("\tTLB Faults: %u\n", vms.vms_tlbfaults);
    kprintf("\tTLB Faults with Free: %u\n", vms.vms_tlbfaultsfree);
    kprintf("\tTLB Faults with Replace: %u\n", vms.vms_tlbfaultsreplace);
    kprintf("\tTLB Invalidations: %u\n", vms.vms_tlbinvalidations);
    kprintf("\tTLB Reloads: %u\n", vms.vms_tlbreloads);
    kprintf("\tPage Faults (Zeroed): %u\n", vms.vms_pagefaultszeroed);
    kprintf("\tPage Faults (Disk): %u\n", vms.vms_pagefaultsdisk);
    kprintf("\tPage Faults from ELF: %u\n", vms.vms_pagefaultself);
    kprintf("\tPage Faults from SWAPFILE: %u\n", vms.vms_pagefaultsswapfile);
    kprintf("\tSWAPFILE Writes: %u\n", vms.vms_swapfilewrites);

    /* warnings */
    if (vms.vms_tlbfaultsfree + vms.vms_tlbfaultsreplace != vms.vms_tlbfaults) {
        kprintf("!!!Warning: TLB Faults should be equal to TLB Faults with Free + TLB Faults with Replace!!!\n");
    }
    if (vms.vms_tlbreloads + vms.vms_pagefaultsdisk + vms.vms_pagefaultszeroed != vms.vms_tlbfaults) {
        kprintf("!!!Warning: TLB Faults should be equal to TLB Reloads + Page Faults (Disk) + Page Faults (Zeroed)!!!\n");
    }
    if (vms.vms_pagefaultself + vms.vms_pagefaultsswapfile != vms.vms_pagefaultsdisk) {
        kprintf("!!!Warning: Page Faults from Disk should be equal to Page Fauls from ELF + Page Faults (Disk)!!!\n");
    }
}