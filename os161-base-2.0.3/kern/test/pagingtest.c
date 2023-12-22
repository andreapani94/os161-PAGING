#include <types.h>
#include <test.h>
#include <lib.h>
#include <mips/tlb.h>
#include <vmtlb.h>
#include <vm.h>

int
mydumbtest(int nargs, char** args)
{
    (void)nargs;
	(void)args;

    kprintf("This is some dumb test!...\n");
    return 0;
}

int
tlbtest(int nargs, char** args)
{
    (void)nargs;
	(void)args;
    uint8_t i;
    uint32_t ehi, elo;
    vaddr_t vaddr = 0x40000;
    paddr_t paddr = 0x0;

    kprintf("Starting TLB test...\n");
    kprintf("Filling all TLB entries...\n");
    /* fill the TLB with VALID entries */
    for (i = 0; i < NUM_TLB; i++) {
        vaddr = (vaddr + (i * PAGE_SIZE));
        ehi = vaddr;
        paddr = (paddr + (i * PAGE_SIZE)) | TLBLO_DIRTY | TLBLO_VALID;
        elo = paddr;
        DEBUG(DB_VM, "tlb: 0x%x -> 0x%x\n", ehi, elo);
        tlb_write(ehi, elo, i);
    }
    kprintf("Trying to insert one more entry...\n");
    i++;
    vaddr = (vaddr + (i * PAGE_SIZE));
    paddr = (paddr + (i * PAGE_SIZE)) | TLBLO_DIRTY | TLBLO_VALID;
    int res = vmtlb_insert(vaddr, paddr);
    if (res != 0) {
        kprintf("Cannot insert a new entry, TLB completely filled...\n");
        kprintf("TLB test failed...\n");
    } else {
        kprintf("TLB test completed!\n");
    }
    return 0;
}
