#include <swapfile.h>
#include <vnode.h>
#include <vfs.h>
#include <kern/fcntl.h>
#include <types.h>
#include <spinlock.h>
#include <uio.h>
#include <kern/errno.h>
#include <vmstats.h>
#include <bitmap.h>

static struct vnode* swapfile = NULL;
static struct spinlock swapfile_lock = SPINLOCK_INITIALIZER;
static struct bitmap* swapfile_freeentries = NULL;

int
swapfile_init()
{
    int result;
    char swapfile_path[] = SWAPFILE_PATH;

    /* Try to open the SWAPFILE */
    result = vfs_open(swapfile_path, O_RDWR | O_CREAT, 0, &swapfile);
    if (result) {
        kprintf("Cannot open the SWAPFILE!\n");
        return result;
    }
    KASSERT(swapfile != NULL);

    swapfile_freeentries = bitmap_create(SWAPFILE_MAX_PAGES);
    if (swapfile_freeentries == NULL) {
        panic("Cannot initialize swapfile free bitmap...\n");
        return ENOMEM;
    }
    KASSERT(swapfile_freeentries != NULL);

    return 0;
}

int 
swapfile_writepage(paddr_t pageaddr, uint32_t* swap_index)
{
    struct iovec iov;
    struct uio ku;
    int result;
    uint32_t index;

    /* find a free entry in the SWAPFILE */
    spinlock_acquire(&swapfile_lock);
    result = bitmap_alloc(swapfile_freeentries, &index);
    if (result) {
        panic("Out of swap space\n");
        return result;
    }
    spinlock_release(&swapfile_lock);

    /* write the content of the page into the SWAPFILE */
    uio_kinit(&iov, &ku, (void*) PADDR_TO_KVADDR(pageaddr), PAGE_SIZE, index*PAGE_SIZE, UIO_WRITE);
    result = VOP_WRITE(swapfile, &ku);
    if (result) {
        return result;
    }
    if (ku.uio_resid != 0) {

    }

    /* Swapfile Writes */
    vms.vms_swapfilewrites++;
    *swap_index = index;

    return 0;
}

int
swapfile_readpage(paddr_t frame, uint32_t swap_index)
{
    struct iovec iov;
    struct uio ku;
    int result;


    KASSERT(swap_index < SWAPFILE_MAX_PAGES);

    /* read the content of the page from the swapfile */
    uio_kinit(&iov, &ku, (void*) PADDR_TO_KVADDR(frame), PAGE_SIZE, swap_index*PAGE_SIZE, UIO_READ);
    result = VOP_READ(swapfile, &ku);
    if (result) {
        return result;
    }
    if (ku.uio_resid != 0) {

    }

    /* mark the entry in the SWAPFILE as free */
    spinlock_acquire(&swapfile_lock);
    bitmap_unmark(swapfile_freeentries, swap_index);
    spinlock_release(&swapfile_lock);

    /* Page Faults from Swapfile */
    vms.vms_pagefaultsswapfile++;
    /* Page Faults from Disk */
    vms.vms_pagefaultsdisk++;

    return 0;
}

void
swapfile_shutdown()
{
    bitmap_destroy(swapfile_freeentries);
    /* close the SWAPFILE */
    vfs_close(swapfile);

    return;
}
