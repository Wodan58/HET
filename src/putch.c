/*
    module  : putch.c
    version : 1.1
    date    : 12/30/19
*/

/**
putch  :  N  ->
N : numeric, writes character whose ASCII is N.
*/
void do_putch(void)
{
    char *ptr;

    assert(vec_size(WS));
    ptr = (char *)(vec_back(WS) & ~BIT_IDENT);
    printf("%c", atoi(ptr));
    vec_pop_back(WS);
}
