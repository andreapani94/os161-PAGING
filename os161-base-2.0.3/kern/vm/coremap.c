/* code for keeping track of free physical frames */

#include <types.h>
#include <lib.h>
#include <bitmap.h>
#include <vm.h>
#include "coremap.h"

#if OPT_PAGING

static struct bitmap* free_frames = NULL;

void
free_frames_init()
{
    /* get size of RAM */
    size_t ramsize = ram_getsize();

    /* number of frames */
    uint32_t frames_num = ramsize / PAGE_SIZE;

    /* initialize the bitmap */
    free_frames = bitmap_create(frames_num);
    if (free_frames == NULL) {
        panic("Unable to initialize free frames list!");
    }
    KASSERT(free_frames != NULL);
    return;
}


int
free_frames_alloc()
{
    unsigned int frame_index = 0;


    /* there is a free frame in the list */
    if (bitmap_alloc(free_frames, &frame_index) == 0) {
        KASSERT(bitmap_isset(free_frames, frame_index));

    } else {
        /* no free frame left (ENOSPC)*/
    }
    
    return frame_index;
}

#endif