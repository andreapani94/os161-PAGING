//#include <stdio.h>

int a = 5;

int
main(void)
{
    //int b = 6;
    //b += a;
    //printf("pagingtest: %d\n", b);
    int array[100];
    int i;
    for (i = 0; i < 100; i++) {
        a++;
        a += array[i];
    }
    return 0;
}