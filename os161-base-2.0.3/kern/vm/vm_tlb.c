#include <types.h>
#include <mips/tlb.h>
#include "vmtlb.h"
#include <lib.h>
#include <kern/errno.h>

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
vmtlb_insert(vaddr_t vaddr, paddr_t paddr)
{
    uint8_t i;
    uint32_t ehi, elo;
    int victim;

    /* try to insert the addresses into the TLB */
    for (i = 0; i < NUM_TLB; i++) {
        tlb_read(&ehi, &elo, i);
        if (elo & TLBLO_VALID) {
            continue;
        }
        DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", vaddr, paddr);
        ehi = vaddr;
        elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
        tlb_write(ehi, elo, i);
        return 0;
    }
    /* if not */
    /* try and get a victim */
    victim = tlb_getrrvictim();
    if (victim == -1) {
        /* return memory error */
        return EFAULT;
    }
    /* insert into the victim */
    ehi = vaddr;
    elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
    tlb_write(ehi, elo, (uint32_t) victim);
    return 0;
}

