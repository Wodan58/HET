/*
    module  : put.c
    version : 1.2
    date    : 01/13/20
*/

/**
put  :  X  ->
Writes X to output, pops X off stack.
*/
void do_put(void)
{
    assert(vec_size(WS));
    writefactor(vec_pop(WS));
}
