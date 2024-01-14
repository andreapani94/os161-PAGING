#include <types.h>
#include <vm.h>
#include <spinlock.h>
#include <kern/errno.h>
#include <current.h>
#include <mips/tlb.h>
#include <addrspace.h>
#include <spl.h>
#include <proc.h>
#include <pt.h>
#include <vmtlb.h>
#include <coremap.h>
#include <vmstats.h>
#include <swapfile.h>

#include "opt-paging.h"

static struct spinlock stealmem_lock = SPINLOCK_INITIALIZER;
static struct spinlock tlb_lock = SPINLOCK_INITIALIZER;
static bool vm_initialized = false;

paddr_t
getppages(unsigned npages, vaddr_t vaddr)
{
	paddr_t addr;

	if (vm_initialized) {
		/* VM manager has taken over */
		// assert that the user is not allocating more than one page
		if (npages == 1) {
			// user program
			addr = coremap_alloc(vaddr);
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
freeppages(paddr_t paddr, unsigned npages)
{
	(void) npages;
	if (vm_initialized) {
		if (npages == 1) {
			// user program
			coremap_free(paddr);
		} else {
			// kernel
			//coremap_kfree(npages);
		}
	}
}


void
vm_bootstrap()
{
	/* initialize the coremap */
	swapfile_init();	/* before coremap_init as ram_stealmem needs to be called */
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
	struct pt_entry_2* inner_pt;
	struct segment* s;
	bool iswritable;

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


	/* check if the faulting address is valid (within segments boundaries) */
	s = segments_find_segment(as->segments, faultaddress);
	if (s == NULL) {
		return EFAULT;
	}
	iswritable = s->writable;
	//faultaddress -= s->elf_page_displ;

	/* TLB Faults */
	vms.vms_tlbfaults++;

	/* PAGE FAULT HANDLING */
	inner_pt = as->page_table[OUTER_PT_INDEX(faultaddress)].inner_pt;
	if (inner_pt == NULL) {
        /* first time access to a page */
        /* no inner page table has been created */
        /* create the page table */
        inner_pt_create(as->page_table, OUTER_PT_INDEX(faultaddress));
		/* allocate a frame for the new page */
        paddr = coremap_alloc(faultaddress);
        /* update the page_table */
        inner_pt = as->page_table[OUTER_PT_INDEX(faultaddress)].inner_pt;
        KASSERT(inner_pt != NULL);
        inner_pt[INNER_PT_INDEX(faultaddress)].paddr = paddr;
		inner_pt[INNER_PT_INDEX(faultaddress)].valid = true;
        inner_pt[INNER_PT_INDEX(faultaddress)].dirty = false;
        inner_pt[INNER_PT_INDEX(faultaddress)].swapped = false;
        KASSERT(paddr == inner_pt[INNER_PT_INDEX(faultaddress)].paddr);
		if (s->elf_file != NULL) {
			/* bring the page in from the ELF file */
			res = pt_load(s, faultaddress, paddr);
			if (res) {
				panic("vm_fault: pt_load has returned an error");
			}
		} else {
			/* stack segment (or heap, when it's actually implemented)*/
			bzero((void*) PADDR_TO_KVADDR(paddr), PAGE_SIZE);
			/* Page Faults (Zeroed)*/
			vms.vms_pagefaultszeroed++;
		}
	} else {
		if (inner_pt[INNER_PT_INDEX(faultaddress)].swapped) {
			/* allocate a frame for the page to be brought in */
			paddr = coremap_alloc(faultaddress);
			uint32_t swap_index = inner_pt[INNER_PT_INDEX(faultaddress)].paddr;
            swapfile_readpage(paddr, swap_index);
			inner_pt[INNER_PT_INDEX(faultaddress)].swapped = false;
			inner_pt[INNER_PT_INDEX(faultaddress)].paddr = paddr;
		} else if (!inner_pt[INNER_PT_INDEX(faultaddress)].valid) {
			/* first time access to a page */
			/* inner page table already created */
			/* allocate a frame for the new page */
			paddr = coremap_alloc(faultaddress);
			/* update the page_table */
			inner_pt = as->page_table[OUTER_PT_INDEX(faultaddress)].inner_pt;
			KASSERT(inner_pt != NULL);
			inner_pt[INNER_PT_INDEX(faultaddress)].paddr = paddr;
			inner_pt[INNER_PT_INDEX(faultaddress)].valid = true;
			inner_pt[INNER_PT_INDEX(faultaddress)].dirty = false;
			inner_pt[INNER_PT_INDEX(faultaddress)].swapped = false;
			KASSERT(paddr == inner_pt[INNER_PT_INDEX(faultaddress)].paddr);
			if (s->elf_file != NULL) {
				/* bring the page in from the ELF file */
				res = pt_load(s, faultaddress, paddr);
				if (res) {
					panic("vm_fault: pt_load has returned an error");
				}
			} else {
				/* stack segment (or heap, when it's actually implemented)*/
				bzero((void*) PADDR_TO_KVADDR(paddr), PAGE_SIZE);
				/* Page Faults (Zeroed)*/
				vms.vms_pagefaultszeroed++;
			}
		} 
		else {
			/* TLB Reloads */
			vms.vms_tlbreloads++;
			paddr = inner_pt[INNER_PT_INDEX(faultaddress)].paddr;
		}		
	}
	

    /* make sure it's page-aligned */
	KASSERT((paddr & PAGE_FRAME) == paddr);

	spinlock_acquire(&tlb_lock);
	spl = splhigh();

	res = vmtlb_insert(faultaddress, paddr, iswritable);

	splx(spl);

	spinlock_release(&tlb_lock);

    (void) faulttype;
    (void) faultaddress;
    return res;
}

vaddr_t 
alloc_kpages(unsigned npages)
{
    paddr_t paddr;

	if (vm_initialized) {
		paddr = coremap_kalloc(npages);
	} else {
		spinlock_acquire(&stealmem_lock);
		paddr = ram_stealmem(npages);
		spinlock_release(&stealmem_lock);
		if (paddr == 0) {
			return 0;
		}
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


