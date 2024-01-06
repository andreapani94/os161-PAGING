/* code for keeping track of free physical frames */

#include <types.h>
#include <lib.h>
#include <bitmap.h>
#include <vm.h>
#include <spinlock.h>
#include <coremap.h>

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
            /* free to use by kernel and user processes */
            coremap[i].is_free = true;
        }
    }
    return;
}


/* 
 * This function is used to find and allocate a frame for
 * user programs. Only 1 frame at a time can be allocated 
 * according to paging
*/
paddr_t
coremap_alloc()
{
    uint32_t i;
    paddr_t paddr = 0;

    // synchronization? or in the calling function?

    /* search the coremap for a free frame */
    for (i = 0; i < num_frames; i++) {
        if (coremap[i].is_free) {
            paddr = i * PAGE_SIZE;
            coremap[i].is_free = false;
            break;
        }
    }

    /* if not call the page replacement algorithm */

    return paddr;
}

void
coremap_free(paddr_t frame_num)
{
    KASSERT(frame_num < num_frames);
    KASSERT(!coremap[frame_num].is_free);
    coremap[frame_num].is_free = true;
    KASSERT(coremap[frame_num].is_free);
    return;
}

/*
 * This function is used to find and allocate a number of 
 * contiguos frames for the kernel, since the kernel doesn't
 * implement paging but the coremap still allocates by frame
*/

paddr_t
coremap_kalloc(unsigned npages)
{
    uint32_t i, first = 0, last = 0;
    paddr_t paddr = 0;  // the starting physical address

    /* search for a sequence of free frames */
    for (i = 0; i < num_frames; i++) {
        if (coremap[i].is_free) {
            if ((i == 0) | !coremap[i-1].is_free) {
                first = i;
            }
            if ((i - first)+1 >= npages) {
                last = i;
                paddr = first * PAGE_SIZE;
                break;
            }
        }
        
    }

    /* mark frames as allocated */
    for (i = 0; i < (last - first)+1; i++) {
        coremap[first+i].is_free = false;
    }   

    return paddr;
}

