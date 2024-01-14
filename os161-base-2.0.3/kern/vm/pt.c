#include <types.h>
#include <lib.h>
#include <vm.h>
#include <kern/errno.h>
#include <pt.h>
#include <addrspace.h>
#include <coremap.h>
#include <vmstats.h>
#include "opt-paging.h"
 
void
inner_pt_create(struct pt_entry_1* outer_pt, uint32_t outer_pt_index)
{
    KASSERT(outer_pt != NULL);

    outer_pt[outer_pt_index].inner_pt = kmalloc(sizeof(struct pt_entry_2) * INNER_PT_SIZE);
    if (outer_pt[outer_pt_index].inner_pt == NULL) {
        panic("Cannot create an inner page table!");
    } 
    bzero(outer_pt[outer_pt_index].inner_pt, sizeof(struct pt_entry_2) * INNER_PT_SIZE);
}


int
pt_load(struct segment* s, vaddr_t vaddr, paddr_t paddr)
{
    uint32_t file_offset;
    uint32_t page_index;
    int res;
    KASSERT(s != NULL);
    KASSERT(s->elf_file != NULL);

    /* set up the offset into the file */
    page_index = vaddr - s->vbase;
    file_offset = s->elf_segment_start + page_index;
    res = load_page(s->elf_file, paddr, file_offset, s->executable);
    if (res) {
        return res;
    }
    /* Page Faults from ELF */
    vms.vms_pagefaultself++;
    /* Page Faults (Disk) */
    vms.vms_pagefaultsdisk++;
    
    (void) paddr;
    return 0;
}


paddr_t 
pt_translate(struct addrspace* as, vaddr_t vaddr)
{
    struct pt_entry_2* inner_pt;
    struct pt_entry_2 pt_entry;
    paddr_t paddr = 0;

    KASSERT(as != NULL);
    KASSERT(as->page_table != NULL);

    
    inner_pt = as->page_table[OUTER_PT_INDEX(vaddr)].inner_pt;
    if (inner_pt == NULL) {
        /* first time access to a page */
        /* no inner page table has been created */
        /* create the page table*/
        inner_pt_create(as->page_table, OUTER_PT_INDEX(vaddr));
        /* allocate a frame for the new page */
        paddr = coremap_kalloc(1);
        /* update the page_table */
        inner_pt = as->page_table[OUTER_PT_INDEX(vaddr)].inner_pt;
        KASSERT(inner_pt != NULL);
        inner_pt[INNER_PT_INDEX(vaddr)].paddr = paddr;
        inner_pt[INNER_PT_INDEX(vaddr)].dirty = false;
        inner_pt[INNER_PT_INDEX(vaddr)].swapped = false;
        KASSERT(paddr == inner_pt[INNER_PT_INDEX(vaddr)].paddr);
        /* bring the page in from the ELF file */
        /* as now vm_fault will use the pt for translation */
        

    } else {
       pt_entry = inner_pt[INNER_PT_INDEX(vaddr)];
       if (pt_entry.swapped) {
            /* bring the page in from the SWAPFILE */
            /* TODO */
       } else {
            /* standard translation */
            paddr = pt_entry.paddr;
       }
    }
    return paddr;
}

int
pt_insert(struct addrspace* as, vaddr_t vaddr, paddr_t paddr)
{
    (void) as;
    (void) vaddr;
    (void) paddr;
    return 0;
}

struct pt_entry_2*
pt_get(struct addrspace* as, vaddr_t vaddr)
{
    struct pt_entry_2 *inner_pt, *entry;

    KASSERT(as != NULL);
    KASSERT(as->page_table != NULL);

    inner_pt = as->page_table[OUTER_PT_INDEX(vaddr)].inner_pt;
    KASSERT(inner_pt != NULL);
    entry = &inner_pt[INNER_PT_INDEX(vaddr)];
    
    return entry;
}
