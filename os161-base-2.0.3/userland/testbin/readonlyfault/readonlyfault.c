
int 
main(void)
{
    char* bad_address = (char*) 0x00400000;
    *bad_address = 'A';
    return 0;
}