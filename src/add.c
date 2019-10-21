/*
    module  : add.c
    version : 1.1
    date    : 10/18/19
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
    second = (char *)(vec_back(WS) & ~BIT_IDENT);
    vec_pop_back(WS);
    first = (char *)(vec_back(WS) & ~BIT_IDENT);
    result = mem_malloc(strlen(first) + strlen(second) + 1);
    sprintf(result, "%d", atoi(first) + atoi(second));
    vec_back(WS) = (intptr_t)result | BIT_IDENT;
}
