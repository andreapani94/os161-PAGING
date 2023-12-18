#ifndef PT_H
#define PT_H

#include <types.h>
#include <addrspace.h>

struct addrspace;

#define PT_SIZE (MIPS_KSEG0 / PAGE_SIZE)
#define PT_INDEX(vaddr) (vaddr / PAGE_SIZE)

struct pt_entry {
    paddr_t paddr;
    uint8_t isvalid;
    uint8_t isreadable;
    uint8_t iswriteable;
    uint8_t isexecutable;
};

paddr_t pt_translate(struct addrspace* as, vaddr_t);
int pt_insert(struct addrspace*, vaddr_t, paddr_t);

#endif