#ifndef SWAPFILE_H
#define SWAPFILE_H

#include <types.h>
#include <machine/vm.h>

#define SWAPFILE_SIZE 9 * 1024 * 1024   /* 9 MB max */
#define SWAPFILE_MAX_PAGES SWAPFILE_SIZE / PAGE_SIZE
#define SWAPFILE_PATH "lhd0raw:/SWAPFILE"

struct swapfile_entry {
    struct addrspace* as;
    vaddr_t vaddr;
};

int swapfile_init(void);
void swapfile_shutdown(void);
int swapfile_writepage(struct addrspace*, vaddr_t);
int swapfile_readpage(struct addrspace*, vaddr_t);

#endif