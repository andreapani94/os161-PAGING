#include <types.h>
#include <vm.h>
#include <spinlock.h>
#include <kern/errno.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <spl.h>
#include <proc.h>
#include "pt.h"
#include "vmtlb.h"
#include "coremap.h"
#include "vmstats.h"

static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;
static bool vm_initialized = false;

paddr_t
getppages(unsigned npages)
{
	paddr_t addr;

	if (vm_initialized) {
		/* VM manager has taken over */
		// assert that the user is not allocating more than one page
		if (npages == 1) {
			// user program
			addr = coremap_alloc();
		} else {
			// kernel
			addr = coremap_kalloc(npages);
		}
	} else {
		/* early initialization (kernel) */
		spinlock_acquire(&stealmem_lock);
		addr = ram_stealmem(npages);
		spinlock_release(&stealmem_lock);
	}
	
	return addr;
}


void
vm_bootstrap()
{
	/* initialize the coremap */
	coremap_init();
	vm_initialized = true;
}

void 
vm_shutdown()
{
	print_vm_stats();
}

int 
vm_fault(int faulttype, vaddr_t faultaddress)
{
    struct addrspace* as;
    paddr_t paddr;
    int res, spl;
	struct pt_entry* entry;

	/* a page fault occurred */
	vms.vms_tlbfaults++;

    /* extract the page number using a bit mask */
    faultaddress &= PAGE_FRAME;

    switch (faulttype) {
	    case VM_FAULT_READONLY:
		/* Handle the read-only case by terminating the process */
			break;
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
    //paddr = pt_translate(as, faultaddress);
	entry = pt_get(as, faultaddress);
	if (entry == NULL) {
		return EFAULT;
	}
	paddr = entry->paddr;
	if (paddr == 0) {
		// allocate a frame
		paddr = getppages(1);
		pt_insert(as, faultaddress, paddr);
		KASSERT(entry->paddr == paddr);
	} else {
		/* page already in memory */
		/* TLB reload */
		vms.vms_tlbreloads++;
	}

    /* make sure it's page-aligned */
	KASSERT((paddr & PAGE_FRAME) == paddr);

	spl = splhigh();

	res = vmtlb_insert(faultaddress, paddr, true);

	splx(spl);

    (void) faulttype;
    (void) faultaddress;
    return res;
}

vaddr_t 
alloc_kpages(unsigned npages)
{
    paddr_t paddr;

	paddr = getppages(npages);
	if (paddr == 0) {
		return 0;
	}
	
	return PADDR_TO_KVADDR(paddr);
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


