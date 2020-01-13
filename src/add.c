/*
    module  : add.c
    version : 1.2
    date    : 01/13/20
*/

/**
add  :  M I  ->  N
Numeric N is the result of adding integer I to numeric M.
Also supports float.
*/
void do_add(void)
{
    char *first, *second, *result;

    assert(vec_size(WS) > 1);
    second = (char *)(vec_pop(WS) & ~BIT_IDENT);
    first = (char *)(vec_pop(WS) & ~BIT_IDENT);
    result = mem_malloc(strlen(first) + strlen(second) + 1);
    sprintf(result, "%d", atoi(first) + atoi(second));
    stk_push(WS, (intptr_t)result | BIT_IDENT);
}
