#include <types.h>
#include <vm.h>
#include <spinlock.h>
#include <kern/errno.h>
#include <current.h>
#include <mips/tlb.h>
#include "pt.h"

#if OPT_DUMBVM
#else

static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;

static
paddr_t
getppages(unsigned long npages)
{
	paddr_t addr;

    /* before vm_bootstrap() or no freed frames available */
    spinlock_acquire(&stealmem_lock);
    addr = ram_stealmem(npages);
    spinlock_release(&stealmem_lock);
	

	return addr;
}

void
vm_bootstrap()
{

}

void 
vm_shutdown()
{

}

int 
vm_fault(int faulttype, vaddr_t faultaddress)
{
	#if OPT_DUMBVM
	#else
    struct addrspace* as;
    paddr_t paddr;
    int i;
	uint32_t ehi, elo;
    int spl;
    /* extract the page number using a bit mask */
    faultaddress &= PAGE_FRAME;

    switch (faulttype) {
	    case VM_FAULT_READONLY:
		/* Handle the read-only case by terminating the process */
	    case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
		    break;
	    default:
		    return EINVAL;
	}

    if (curproc == NULL) {
		/*
		 * No process. This is probably a kernel fault early
		 * in boot. Return EFAULT so as to panic instead of
		 * getting into an infinite faulting loop.
		 */
		return EFAULT;
	}

    as = proc_getas();
	if (as == NULL) {
		/*
		 * No address space set up. This is probably also a
		 * kernel fault early in boot.
		 */
		return EFAULT;
	}

    /* for now assume that all pages have been loaded in the */
    /* page table (NO DEMAND PAGING) */
    paddr = pt_translate(as, faultaddress);

    /* make sure it's page-aligned */
	KASSERT((paddr & PAGE_FRAME) == paddr);

    /* Disable interrupts on this CPU while frobbing the TLB. */
	spl = splhigh();

	for (i=0; i<NUM_TLB; i++) {
		tlb_read(&ehi, &elo, i);
		if (elo & TLBLO_VALID) {
			continue;
		}
		ehi = faultaddress;
		elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
		tlb_write(ehi, elo, i);
		splx(spl);
		return 0;
	}

	kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
	splx(spl);
	return EFAULT;

	#endif
    (void) faulttype;
    (void) faultaddress;
    return 0;
}

vaddr_t 
alloc_kpages(unsigned npages)
{
    paddr_t pa;

	pa = getppages(npages);
	if (pa==0) {
		return 0;
	}
	return PADDR_TO_KVADDR(pa);
}

void 
free_kpages(vaddr_t addr)
{
    (void) addr;
}

void 
vm_tlbshootdown(const struct tlbshootdown *tlbshootdown)
{
    (void) tlbshootdown;
}

#endif

