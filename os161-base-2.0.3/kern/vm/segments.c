#include <segments.h>
#include <vm.h>
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
        s = &segments[i];
        if (vaddr >= s->vbase && vaddr <= s->vtop) {
            /* this is the segment we are looking for */
            return s;
        }
    }
    return NULL;
}

void
segments_before_load(struct segment* s, vaddr_t vaddr, paddr_t* paddr, uint32_t* elf_offset, uint32_t* page_size)
{
    uint32_t page_index;
    uint32_t first_psize;

    /* compute the correct page size and paddr */
    /* based on the location of the page in the segment */
    page_index = (vaddr - s->vbase) / PAGE_SIZE;
    if (page_index == 0) {
        /* first page in a segment */
        *page_size = PAGE_SIZE - s->elf_base_offset;
        *paddr += s->elf_base_offset;
        *elf_offset = s->elf_segment_start;
    } else if (page_index < s->npages) {
        first_psize = PAGE_SIZE - s->elf_base_offset;
        *page_size = PAGE_SIZE;
        *elf_offset = s->elf_segment_start + first_psize + ((page_index-1) * PAGE_SIZE);
    } else {
        
    }

    return;
}