/*
    module  : add.c
    version : 1.4
    date    : 06/22/20
*/

/**
add  :  M I  ->  N
Numeric N is the result of adding integer I to numeric M.
Also supports float.
*/
void do_add(eval_env *ENV)
{
    intptr_t temp;
    char *first, *second;

    assert(vec_size(ENV->WS) > 1);
    second = (char *)(vec_pop(ENV->WS) & ~BIT_IDENT);
    first = (char *)(vec_pop(ENV->WS) & ~BIT_IDENT);
    sprintf(ident, "%d", atoi(first) + atoi(second));
    temp = (intptr_t)my_strdup(ENV->STR, ident) | BIT_IDENT;
    vec_push(ENV->WS, temp);
}
