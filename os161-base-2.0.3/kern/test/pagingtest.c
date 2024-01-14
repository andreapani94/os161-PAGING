#include <types.h>
#include <test.h>
#include <lib.h>
#include <mips/tlb.h>
#include <vmtlb.h>
#include <vm.h>
#include <swapfile.h>
#include <addrspace.h>
#include <copyinout.h>
#include <coremap.h>

int
mydumbtest(int nargs, char** args)
{
    (void)nargs;
	(void)args;

    kprintf("This is some dumb test!...\n");
    return 0;
}

int
tlbtest(int nargs, char** args)
{
    (void)nargs;
	(void)args;
    uint8_t i;
    uint32_t ehi, elo;
    uint32_t ehi2, elo2;
    vaddr_t vaddr = 0x40000;
    paddr_t paddr = 0x0;

    kprintf("Starting TLB test...\n");
    kprintf("Filling all TLB entries...\n");
    /* fill the TLB with VALID entries */
    for (i = 0; i < NUM_TLB; i++) {
        vaddr = (vaddr + (i * PAGE_SIZE));
        ehi = vaddr;
        paddr = (paddr + (i * PAGE_SIZE)) | TLBLO_DIRTY | TLBLO_VALID;
        elo = paddr;
        tlb_write(ehi, elo, i);
        tlb_read(&ehi2, &elo2, i);
        KASSERT(ehi2 == ehi);
        KASSERT(elo2 == elo);
    }
    kprintf("Trying to insert one more entry...\n");
    i++;
    vaddr = (vaddr + (i * PAGE_SIZE));
    paddr = (paddr + (i * PAGE_SIZE)) | TLBLO_DIRTY | TLBLO_VALID;
    int res = vmtlb_insert(vaddr, paddr, true);
    if (res != 0) {
        kprintf("Cannot insert a new entry, TLB completely filled...\n");
        kprintf("TLB test failed...\n");
    } else {
        kprintf("TLB test completed!\n");
    }
    return 0;
}

int 
swapfiletest(int nargs, char** args)
{
    (void)nargs;
	(void)args;
    int result;
    uint32_t swap_index;
    int i, sum1 = 0, sum2 = 0;
    //struct addrspace* as;

    kprintf("Starting swapfile test...\n");
    kprintf("Trying to open the swapfile and initialize associated structures...\n");
    result = swapfile_init();
    if (result) {
        kprintf("Swapfile test failed...\n");
        return 1;
    }
    kprintf("Initialization successful!\n");
    /* simulate page envinronment */
    //as = as_create();
    uint8_t* page_content = kmalloc(PAGE_SIZE);
    for (i = 0; i < PAGE_SIZE; i++) {
        page_content[i] = i;
        sum1 += page_content[i];
    }
    kprintf("The sum of all numbers contained in the page is %d\n", sum1);
    kprintf("Trying to swap out a page...\n");
    result = swapfile_writepage((paddr_t) page_content - MIPS_KSEG0, &swap_index);
    if (result) {
        kprintf("Swapfile test failed...\n");
        return 1;
    }
    kprintf("Swap out successful (index = %d)!\n", swap_index);
    kprintf("Zeroing out the memory previously occupied by the page...\n");
    bzero(page_content, PAGE_SIZE);
    for (i = 0; i < PAGE_SIZE; i++) {
        sum2 += page_content[i];
    }
    KASSERT(sum2 == 0);
    kprintf("The sum of all numbers contained in the page is %d\n", sum2);
    kprintf("Trying to swap in the same page...\n");
    result = swapfile_readpage((paddr_t) page_content - MIPS_KSEG0, swap_index);
    if (result) {
        kprintf("Swapfile test failed...\n");
        return 1;
    }
    kprintf("Swap out successful!\n");
    for (i = 0; i < PAGE_SIZE; i++) {
        sum2 += page_content[i];
    }
    kprintf("The sum of all numbers contained in the page is %d\n", sum2);
    KASSERT(sum1 == sum2);
    kprintf("Swapfile test completed successfully!\n");
    return 0;
}

int 
coremaptest(int nargs, char** args)
{
    (void)nargs;
	(void)args;
    paddr_t frame1, frame2, kstartframe;

    kprintf("Starting coremap test...\n");
    kprintf("User allocation:\n");
    kprintf("Trying to get two free frames...\n");
    frame1 = coremap_kalloc(1) / PAGE_SIZE;
    kprintf("Obtained frame %d\n", frame1);
    frame2 = coremap_kalloc(1) / PAGE_SIZE;
    KASSERT(frame1 == frame2-1);    // assuming single thread
    kprintf("Obtained frame %d\n", frame2);
    kprintf("Freeing both frames...\n");
    coremap_free(frame1);
    coremap_free(frame2);
    kprintf("Kernel allocation:\n");
    kprintf("Trying to get two consecutives frames...\n");
    kstartframe = coremap_kalloc(2) / PAGE_SIZE;
    kprintf("Obtained frame %d\n", kstartframe);
    KASSERT(kstartframe == frame1);
    kprintf("Another one...\n");
    frame2 = coremap_kalloc(1) / PAGE_SIZE;
    kprintf("Obtained frame %d\n", frame2);
    KASSERT(frame2 == kstartframe+2);
    kprintf("Coremap test completed!\n");
    return 0;
}
