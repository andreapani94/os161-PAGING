/* code for keeping track of free physical frames */

#include <types.h>
#include <lib.h>
#include <bitmap.h>
#include <vm.h>
#include <spinlock.h>
#include <coremap.h>
#include <swapfile.h>
#include <kern/errno.h>
#include <current.h>
#include <proc.h>
#include <pt.h>
#include <synch.h>

struct queue_node {
    struct coremap_entry* entry;
    struct queue_node* next;
};

static struct coremap_entry* coremap = NULL;
static uint32_t num_frames;
static struct queue_node* replacement_queue = NULL;
static struct lock* coremap_lock = NULL;

static
int
queue_push(struct coremap_entry* entry)
{
    struct queue_node* head = replacement_queue;

    /* allocate a new queue node */
    struct queue_node* node = kmalloc(sizeof(struct queue_node));
    if (node == NULL) {
        return ENOMEM;
    }
    node->entry = entry;
    node->next = NULL;
    /* insert the node into the queue */
    if (head == NULL) {
        replacement_queue = node;
    } else {
        /* go to the end of the queue */
        while (head->next != NULL) {
            head = head->next;
        }
        head->next = node;
    }
    return 0;
}

static
struct coremap_entry*
queue_pop()
{
    struct queue_node* node = NULL;
    struct coremap_entry* entry = NULL;

    if (replacement_queue == NULL) {
        return NULL;
    }
    node = replacement_queue;
    replacement_queue = replacement_queue->next;
    entry = node->entry;
    kfree(node);

    return entry;
}

void
coremap_init()
{
    uint32_t i;
    paddr_t firstfree;

    /* Find number of frames to manage */
    num_frames = ram_getsize() / PAGE_SIZE;

    /* Initialize the coremap */
    coremap = kmalloc(sizeof(struct coremap_entry) * num_frames);
    if (coremap == NULL) {
        panic("Cannot initialize the coremap!");
    }
    KASSERT(coremap != NULL);

    /* get first free physical address */
    /* some frames are already occupied by the kernel */
    /* since vm_bootstrap is called after the bootstrap of the kernel */
    firstfree = ram_getfirstfree();

    /* Initialize frames status */
    for (i = 0; i < num_frames; i++) {
        if ((i < COREMAP_INDEX(firstfree))) {
            /* frames occupied by the kernel */
            coremap[i].is_free = false;
        } else {
            /* free to use by kernel and user processes */
            coremap[i].is_free = true;
        }
    }
    /* initialize coremap lock */
    coremap_lock = lock_create("coremap lock");
    KASSERT(coremap_lock != NULL);

    return;
}


/* 
 * This function is used to find and allocate a frame for
 * user programs. Only 1 frame at a time can be allocated 
 * 
*/
paddr_t
coremap_alloc(vaddr_t vaddr)
{
    uint32_t i;
    paddr_t paddr = 0;
    struct proc* p = curproc;
    bool found = false;

    lock_acquire(coremap_lock);
    do {
        /* search the coremap for a free frame */
        for (i = 0; i < num_frames; i++) {
            if (coremap[i].is_free) {
                paddr = i * PAGE_SIZE;
                coremap[i].paddr = paddr;
                coremap[i].is_free = false;
                coremap[i].as = p->p_addrspace;
                coremap[i].vaddr = vaddr;
                /* insert it into the replacement queue */
                queue_push(&coremap[i]);
                found = true;
                break;
            }
        }
        /* if not call the page replacement algorithm */
        if (!found) {
            coremap_replace();
        }
    } while (!found);
    lock_release(coremap_lock);

    return paddr;
}

void
coremap_free(paddr_t paddr)
{
    KASSERT(COREMAP_INDEX(paddr) < num_frames);
    coremap[COREMAP_INDEX(paddr)].is_free = true;
    KASSERT(coremap[COREMAP_INDEX(paddr)].is_free);
    return;
}

void
coremap_kfree(paddr_t paddr)
{
    uint32_t i, nframes;

    KASSERT(COREMAP_INDEX(paddr) < num_frames);
    nframes = coremap[COREMAP_INDEX(paddr)].alloc_size;
    for (i = 0; i < nframes; i++) {
        coremap[COREMAP_INDEX(paddr) + i].is_free = true;
    }
    return;
}
/*
 * This function is used to find and allocate a number of 
 * contiguos frames for the kernel, since the kernel doesn't
 * implement paging but the coremap still allocates by frame
*/

paddr_t
coremap_kalloc(unsigned npages)
{
    uint32_t i, first = 0, last = 0;
    paddr_t paddr = 0;  // the starting physical address
    bool found = false;

    do {
        /* search for a sequence of free frames */
        for (i = 0; i < num_frames; i++) {
            if (coremap[i].is_free) {
                if ((i == 0) | !coremap[i-1].is_free) {
                    first = i;
                }
                if ((i - first)+1 >= npages) {
                    last = i;
                    paddr = first * PAGE_SIZE;
                    found = true;
                    break;
                }
            }  
        }

        if (found) {
            /* mark frames as allocated */
            for (i = 0; i < (last - first)+1; i++) {
                if (i == 0) {
                    coremap[first+i].alloc_size = npages;
                }
                coremap[first+i].paddr = (first+i) * PAGE_SIZE;
                coremap[first+i].is_free = false;
            }

        } else {
            coremap_replace();
        }
    } while (!found);

    return paddr;
}

paddr_t
coremap_replace()
{
    struct coremap_entry* victim;
    paddr_t paddr;
    int res;
    struct pt_entry* entry;

    KASSERT(replacement_queue != NULL);
    victim = queue_pop();
    /* the victim could have been already freed */
    while (victim->is_free) {
        victim = queue_pop();
    }
    KASSERT(victim != NULL);
    KASSERT(!victim->is_free);
    paddr = victim->paddr;
    /* write the page to SWAPFILE */
    entry = pt_get(victim->as, victim->vaddr);
    res = swapfile_writepage(victim->paddr, &entry->paddr);
    if (res) {
        panic("coremap_replace: Page swapout failure\n");
    }
    /* zero out the page */
    //bzero((void*) PADDR_TO_KVADDR(paddr), PAGE_SIZE);
    /* free up the frame */
    victim->is_free = true;
    /* update the address space of the process */
    KASSERT(entry != NULL);
    entry->swapped = true;

    return paddr;
}

