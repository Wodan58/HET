/*
    module  : unstack.c
    version : 1.4
    date    : 06/22/20
*/

/**
unstack  :  [X Y ..]  ->  ..Y X
The list [X Y ..] becomes the new stack.
*/
void do_unstack(eval_env *ENV)
{
    int i, j;
    Stack *list;

    list = (Stack *)vec_pop(ENV->WS);
    vec_setsize(ENV->WS, 0);
    for (i = 0, j = vec_size(list); i < j; i++)
	vec_push(ENV->WS, vec_at(list, i));
}
