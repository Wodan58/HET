/*
    module  : unstack.c
    version : 1.1
    date    : 03/23/20
*/

/**
unstack  :  [X Y ..]  ->  ..Y X
The list [X Y ..] becomes the new stack.
*/
void do_unstack(void)
{
    int i, j;
    Stack *list;

    list = (Stack *)vec_pop(WS);
    vec_setsize(WS, 0);
    for (i = 0, j = vec_size(list); i < j; i++)
	stk_push(WS, vec_at(list, i));
}
