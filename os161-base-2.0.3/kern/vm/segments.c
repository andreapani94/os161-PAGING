#include <segments.h>
#include <errno.h>

int
segments_valid_address(struct segment* segments, vaddr_t vaddr)
{
    uint8_t i;
    struct segment* s;

    for (i = 0; i < NUM_SEGMENTS; i++) {
        s = &segments[0];
        if (vaddr >= s->vbase && vaddr <= s->vtop) {
            /* within boundaries = VALID ADDRESS */
            return 0;
        }
    }
    return EINVAL;
}