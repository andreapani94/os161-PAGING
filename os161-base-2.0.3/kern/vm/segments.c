#include <segments.h>
#include <kern/errno.h>

int
segments_valid_address(struct segment* segments, vaddr_t vaddr)
{
    uint8_t i;
    struct segment* s;

    for (i = 0; i < NUM_SEGMENTS; i++) {
        s = &segments[i];
        if (vaddr >= s->vbase && vaddr <= s->vtop) {
            /* within boundaries = VALID ADDRESS */
            return 0;
        }
    }
    return EINVAL;
}

struct segment*
segments_find_segment(struct segment* segments, vaddr_t vaddr)
{
    uint8_t i;
    struct segment* s = NULL;

    for (i = 0; i < NUM_SEGMENTS; i++) {
        s = &segments[0];
        if (vaddr >= s->vbase && vaddr <= s->vtop) {
            /* this is the segment we are looking for */
            return s;
        }
    }
    return NULL;
}