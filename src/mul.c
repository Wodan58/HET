/*
    module  : mul.c
    version : 1.1
    date    : 10/26/19
*/

/**
mul  :  I J  ->  K
Integer K is the product of integers I and J.  Also supports float.
*/
void do_mul(void)
{
    char *first, *second, *result;

    assert(vec_size(WS) > 1);
    second = (char *)(vec_back(WS) & ~BIT_IDENT);
    vec_pop_back(WS);
    first = (char *)(vec_back(WS) & ~BIT_IDENT);
    result = mem_malloc(strlen(first) + strlen(second) + 1);
    sprintf(result, "%d", atoi(first) * atoi(second));
    vec_back(WS) = (intptr_t)result | BIT_IDENT;
}
