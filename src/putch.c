/*
    module  : putch.c
    version : 1.2
    date    : 01/13/20
*/

/**
putch  :  N  ->
N : numeric, writes character whose ASCII is N.
*/
void do_putch(void)
{
    char *ptr;

    assert(vec_size(WS));
    ptr = (char *)(vec_pop(WS) & ~BIT_IDENT);
    printf("%c", atoi(ptr));
}
