#include <types.h>
#include <test.h>
#include <lib.h>

int
mydumbtest(int nargs, char** args)
{
    (void)nargs;
	(void)args;

    kprintf("This is some dumb test!...\n");
    return 0;
}