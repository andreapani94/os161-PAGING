/* code for keeping track of free physical frames */

#include <types.h>
#include <lib.h>
#include <bitmap.h>
#include <vm.h>
#include <spinlock.h>
#include "coremap.h"

// bitmap initialization
static struct coremap_entry* coremap = NULL;
static uint32_t num_frames;

void
coremap_init()
{
    uint32_t i;
    paddr_t firstfree;

    /* Find number of frames to manage */
    num_frames = ram_getsize() / PAGE_SIZE;

    /* Initialize the coremap */
    coremap = kmalloc(sizeof(struct coremap_entry) * num_frames);
    if (coremap == NULL) {
        panic("Cannot initialize the coremap!");
    }
    KASSERT(coremap != NULL);

    /* get first free physical address */
    /* some frames are already occupied by the kernel */
    /* since vm_bootstrap is called after the bootstrap of the kernel */
    firstfree = ram_getfirstfree();

    /* Initialize frames status */
    for (i = 0; i < num_frames; i++) {
        if (i < COREMAP_INDEX(firstfree)) {
            /* frames occupied by the kernel */
            coremap[i].is_free = false;
        } else {
            /* free to use by user processes */
            coremap[i].is_free = true;
        }
    }
    return;
}


paddr_t
coremap_alloc()
{
    uint32_t i;
    paddr_t paddr;

    // synchronization? or in the calling function?

    /* search the coremap for a free frame */
    for (i = 0; i < num_frames; i++) {
        if (coremap[i].is_free) {
            paddr = i * PAGE_SIZE;
            coremap[i].is_free = false;
            return paddr;
        }
    }

    /* if not call the page replacement algorithm */

    return paddr;
}