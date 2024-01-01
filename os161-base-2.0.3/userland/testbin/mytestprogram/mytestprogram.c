#include <stdio.h>

int
main(void)
{
    int a = 5;
    int* p = &a;
    printf("Hello World!\n");
    printf("%p\n", p);
    return 0;
}