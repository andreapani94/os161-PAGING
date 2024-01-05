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
static struct swapfile_entry* swapfile_map = NULL;
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

    /* Initialize the swapfile map */
    /* for page retreival in the file */
    swapfile_map = kmalloc(sizeof(struct swapfile_entry) * SWAPFILE_MAX_PAGES);
    if (swapfile_map == NULL) {
        kprintf("Cannot initialize swapfile map...\n");
        return ENOMEM;
    }
    KASSERT(swapfile_map != NULL);
    bzero(swapfile_map, sizeof(struct swapfile_entry) * SWAPFILE_MAX_PAGES);
    /* Initialize the bitmap of free entries */
    swapfile_freeentries = bitmap_create(SWAPFILE_MAX_PAGES);
    if (swapfile_freeentries == NULL) {
        panic("Cannot initialize swapfile free bitmap...\n");
        return ENOMEM;
    }
    KASSERT(swapfile_freeentries != NULL);

    return 0;
}

int 
swapfile_writepage(struct addrspace* as, paddr_t pageaddr)
{
    struct iovec iov;
    struct uio ku;
    int result;
    uint32_t index;

    KASSERT(as != NULL);

    //spinlock_acquire(&swapfile_lock);
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
    /* update the swapfile map */
    swapfile_map[index].as = as;
    swapfile_map[index].paddr = pageaddr;

    //spinlock_release(&swapfile_lock);

    return 0;
}

int
swapfile_readpage(struct addrspace* as, vaddr_t pageaddr)
{
    struct iovec iov;
    struct uio ku;
    int result;
    uint32_t i;
    int index = -1;


    KASSERT(as != NULL);

    //spinlock_acquire(&swapfile_lock);

    /* find the offset in which the page is located in the SWAPFILE */
    for (i = 0; i < SWAPFILE_MAX_PAGES; i++) {
        if (swapfile_map[i].as == as && swapfile_map[i].paddr == pageaddr) {
            index = i;
            swapfile_map[i].as = 0;
            swapfile_map[i].paddr = 0;
            break;
        }
    }
    if (index < 0) {
        panic("No swapped page was found!");
    }
    KASSERT(index >= 0);

    /* read the content of the page from the swapfile */
    uio_kinit(&iov, &ku, (void*) PADDR_TO_KVADDR(pageaddr), PAGE_SIZE, index*PAGE_SIZE, UIO_READ);
    result = VOP_READ(swapfile, &ku);
    if (result) {
        return result;
    }
    if (ku.uio_resid != 0) {

    }

    /* mark the entry in the SWAPFILE as free */
    spinlock_acquire(&swapfile_lock);
    bitmap_unmark(swapfile_freeentries, index);
    spinlock_release(&swapfile_lock);

    //spinlock_release(&swapfile_lock);

    return 0;
}

void
swapfile_shutdown()
{
    bitmap_destroy(swapfile_freeentries);
    kfree(swapfile_map);
    /* close the SWAPFILE */
    vfs_close(swapfile);

    return;
}
