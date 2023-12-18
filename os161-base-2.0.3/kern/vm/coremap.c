/* code for keeping track of free physical frames */

#include <types.h>
#include <lib.h>
#include <bitmap.h>
#include <vm.h>
#include <spinlock.h>
#include "coremap.h"

#if OPT_DUMBVM
#else 

//static struct bitmap* free_frames = NULL;

void
coremap_init()
{
    // /* get size of RAM */
    // size_t ramsize = ram_getsize();

    // /* number of frames */
    // uint32_t frames_num = ramsize / PAGE_SIZE;

    // /* initialize the bitmap */
    // free_frames = bitmap_create(frames_num);
    // if (free_frames == NULL) {
    //     panic("Unable to initialize free frames list!");
    // }
    // KASSERT(free_frames != NULL);
    return;
}


paddr_t
coremap_alloc()
{
    paddr_t paddr;

    paddr = getppages(1);

    // /* unsigned int frame_index = 0;


    // /* there is a free frame in the list */
    // if (bitmap_alloc(free_frames, &frame_index) == 0) {
    //     KASSERT(bitmap_isset(free_frames, frame_index));

    // } else {
    //     /* no free frame left (ENOSPC)*/
    // } */
    
    return paddr;
}

#endif