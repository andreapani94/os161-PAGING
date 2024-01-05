#include <types.h>
#include <lib.h>
#include <vm.h>
#include <errno.h>
#include "pt.h"
#include <addrspace.h>
#include "opt-paging.h"

static 
void
inner_pt_create(struct pt_entry_1* outer_pt, uint32_t outer_pt_index)
{
    uint32_t outer_pt_index;

    KASSERT(pt != NULL);

    outer_pt[outer_pt_index].inner_pt = kmalloc(sizeof(struct pt_entry_2) * INNER_PT_SIZE);
    if (as->page_table[outer_pt_index] == NULL) {
        panic("Cannot create an inner page table!");
    } 
}

static 
int
load_page(struct addrspace* as, vaddr_t vaddr)
{
    KASSERT(as != NULL);

    
}


paddr_t 
pt_translate(struct addrspace* as, vaddr_t vaddr)
{
    struct pt_entry_1* inner_pt;
    paddr_t paddr;

    KASSERT(as != NULL);
    KASSERT(as->page_table != NULL);

    
    inner_pt = as->page_table[outer_pt_index];
    if (inner_pt == NULL) {
        /* first time access to a page */
        /* no inner page table has been created */
        /* create the page table*/
        inner_pt_create(as->page_table, OUTER_PT_INDEX(vaddr));
        /* allocate a frame for the new page */
        paddr = coremap_alloc();
        /* bring the page in from the ELF file */


    } else {
        /* simply translate */
        /* swapped case */

    }
    (void) as;
    (void) vaddr;
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
    (void) as;
    (void) vaddr;
    return NULL;
}
