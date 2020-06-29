/*
    module  : putch.c
    version : 1.3
    date    : 06/07/20
*/

/**
putch  :  N  ->
N : numeric, writes character whose ASCII is N.
*/
void do_putch(eval_env *ENV)
{
    char *ptr;

    assert(vec_size(ENV->WS));
    ptr = (char *)(vec_pop(ENV->WS) & ~BIT_IDENT);
    printf("%c", atoi(ptr));
}
