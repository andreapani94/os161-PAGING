#include <types.h>
#include <lib.h>
#include <vm.h>
#include "pt.h"
#include <addrspace.h>
#include "opt-paging.h"


paddr_t 
pt_translate(struct addrspace* as, vaddr_t vaddr)
{
    uint32_t pt_index = vaddr / PAGE_SIZE;
    paddr_t paddr = as->page_table[pt_index].paddr;
    return paddr;
}

int
pt_insert(struct addrspace* as, vaddr_t vaddr, paddr_t paddr)
{
    uint32_t pt_index = vaddr / PAGE_SIZE;
    as->page_table[pt_index].paddr = paddr;
    return 0;
}

struct pt_entry*
pt_get(struct addrspace* as, vaddr_t vaddr)
{
    struct pt_entry* entry = NULL;

    entry = &as->page_table[PT_INDEX(vaddr)];
    KASSERT(entry != NULL);

    return entry;
}
