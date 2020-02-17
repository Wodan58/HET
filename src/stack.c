/*
    module  : stack.c
    version : 1.1
    date    : 02/17/20
*/

/**
stack  :  .. X Y Z  ->  .. X Y Z [Z Y X ..]
Pushes the stack as a list.
*/
void do_stack(void)
{
    Stack *list;

    vec_copy(list, WS);
    stk_push(WS, (intptr_t)list);
}
