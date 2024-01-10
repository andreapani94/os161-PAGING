#ifndef SWAPFILE_H
#define SWAPFILE_H

#include <types.h>
#include <machine/vm.h>

#define SWAPFILE_SIZE 9 * 1024 * 1024   /* 9 MB max */
#define SWAPFILE_MAX_PAGES SWAPFILE_SIZE / PAGE_SIZE
#define SWAPFILE_PATH "emu0:/SWAPFILE"

struct swapfile_entry {
    struct addrspace* as;
    paddr_t paddr;
};

int swapfile_init(void);
void swapfile_shutdown(void);
int swapfile_writepage(paddr_t, uint32_t*);
int swapfile_readpage(paddr_t, uint32_t);

#endif