/*
    module  : put.c
    version : 1.3
    date    : 06/07/20
*/

/**
put  :  X  ->
Writes X to output, pops X off stack.
*/
void do_put(eval_env *ENV)
{
    assert(vec_size(ENV->WS));
    writefactor(vec_pop(ENV->WS));
}
