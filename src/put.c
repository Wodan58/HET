/*
    module  : put.c
    version : 1.1
    date    : 12/30/19
*/

/**
put  :  X  ->
Writes X to output, pops X off stack.
*/
void do_put(void)
{
    assert(vec_size(WS));
    writefactor(vec_back(WS));
    vec_pop_back(WS);
}
