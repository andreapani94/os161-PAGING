#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <types.h>

#define NUM_SEGMENTS 3      /* heap not implemented ! */

struct segment {
    vaddr_t vbase;
    vaddr_t vtop;
    bool readable;
    bool writable;
    bool executable;
    /* ELF file */
    uint32_t elf_segment_start;
};

int segments_valid_address(struct segment*, vaddr_t);

#endif