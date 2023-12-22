#include <stdlib.h>
#include <stdio.h>

#define MAX_TLB 64
#define TLB_ENTRY_SIZE 64

int 
main(void)
{
    void* big_alloc;

    big_alloc = malloc(TLB_ENTRY_SIZE * (MAX_TLB + 1));
    if (big_alloc == NULL) {
       printf("malloc: Failed allocation\n");
       return -1;
    }
    return 0;
}