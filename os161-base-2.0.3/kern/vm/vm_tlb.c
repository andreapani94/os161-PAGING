#include <types.h>
#include <mips/tlb.h>
#include "vmtlb.h"
#include <lib.h>
#include <kern/errno.h>
#include "vmstats.h"

static 
int
tlb_getrrvictim()
{
    int victim = -1;

    static uint8_t next_victim = 0;
    victim = next_victim;
    next_victim = (next_victim + 1) % NUM_TLB;
    return victim;
} 

int
vmtlb_insert(vaddr_t vaddr, paddr_t paddr, bool iswritable)
{
    uint8_t i;
    uint32_t ehi, elo;
    int victim;
    

    /* try to insert the addresses into the TLB */
    for (i = 0; i < NUM_TLB; i++) {
        tlb_read(&ehi, &elo, i);
        if (elo & TLBLO_VALID) {
            /* not free */
            continue;
        }
        /* free entry is found */
        ehi = vaddr;
        if (iswritable) {
            elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
        } else {
            elo = paddr | TLBLO_VALID;
        }
        //kprintf("tlb %u: 0x%x -> 0x%x\n", i, ehi, elo);
        tlb_write(ehi, elo, i);
        /* TLB Faults with Free */
        vms.vms_tlbfaultsfree++;

        return 0;
    }

    /* if not */
    /* try and get a victim */
    victim = tlb_getrrvictim();
    if (victim == -1) {
        /* return memory error */
        kprintf("TLB ran out of entries\n");
        return EFAULT;
    }
    /* insert into the victim */
    ehi = vaddr;
    elo = paddr | TLBLO_VALID;
    if (iswritable) {
        elo = elo | TLBLO_DIRTY;
    }
    //kprintf("tlb %u: 0x%x -> 0x%x\n", (uint32_t) victim, ehi, elo);
    tlb_write(ehi, elo, (uint32_t) victim);
    /* TLB Faults with Replace */
    vms.vms_tlbfaultsreplace++;

    return 0;
}

void
vmtlb_reset()
{
    int i;

    /* make every TLB entry INVALID */
    for (i = 0; i < NUM_TLB; i++) {
        tlb_write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
    }

    /* TLB Invalidations */
    vms.vms_tlbinvalidations++;
}

