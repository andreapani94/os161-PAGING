#include <types.h>
#include <lib.h>
#include <vm.h>
#include <kern/errno.h>
#include <pt.h>
#include <addrspace.h>
#include <coremap.h>
#include <vmstats.h>
#include "opt-paging.h"
 
int
inner_pt_create(struct addrspace* as, vaddr_t vaddr)
{
    KASSERT(as != NULL);

    as->page_table[OUTER_PT_INDEX(vaddr)] = kmalloc(sizeof(struct pt_entry) * INNER_PT_SIZE);
    if (as->page_table[OUTER_PT_INDEX(vaddr)] == NULL) {
        return EPERM;
    }
    KASSERT(as->page_table[OUTER_PT_INDEX(vaddr)] != NULL);
    bzero(as->page_table[OUTER_PT_INDEX(vaddr)], sizeof(struct pt_entry) * INNER_PT_SIZE);
    return 0;
}


int
pt_load(struct segment* s, vaddr_t vaddr, paddr_t paddr)
{
    uint32_t elf_offset;
    uint32_t page_size = PAGE_SIZE;
    int res;
    KASSERT(s != NULL);
    KASSERT(s->elf_file != NULL);

    /* set up the offset into the file */
    int page_index = vaddr - s->vbase;
    elf_offset = s->elf_segment_start + page_index;
    /* modify the paddr to take into account base page displacement */
    segments_before_load(s, vaddr, &paddr, &elf_offset, &page_size);
    res = load_page(s->elf_file, paddr, page_size, elf_offset, s->executable);
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

int
pt_insert(struct addrspace* as, vaddr_t vaddr, paddr_t paddr)
{
    (void) as;
    (void) vaddr;
    (void) paddr;
    return 0;
}

struct pt_entry*
pt_get(struct addrspace* as, vaddr_t vaddr)
{
    struct pt_entry* entry;

    KASSERT(as != NULL);
    KASSERT(as->page_table != NULL);

    entry = &as->page_table[OUTER_PT_INDEX(vaddr)][INNER_PT_INDEX(vaddr)];
    
    return entry;
}
