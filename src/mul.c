/*
    module  : mul.c
    version : 1.2
    date    : 01/13/20
*/

/**
mul  :  I J  ->  K
Integer K is the product of integers I and J.  Also supports float.
*/
void do_mul(void)
{
    char *first, *second, *result;

    assert(vec_size(WS) > 1);
    second = (char *)(vec_pop(WS) & ~BIT_IDENT);
    first = (char *)(vec_pop(WS) & ~BIT_IDENT);
    result = mem_malloc(strlen(first) + strlen(second) + 1);
    sprintf(result, "%d", atoi(first) * atoi(second));
    stk_push(WS, (intptr_t)result | BIT_IDENT);
}
