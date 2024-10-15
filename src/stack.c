/*
    module  : stack.c
    version : 1.5
    date    : 09/25/24
*/

/**
stack  :  .. X Y Z  ->  .. X Y Z [Z Y X ..]
Pushes the stack as a list.
*/
void do_stack(eval_env *ENV)
{
    Stack *list;

    vec_copy_all(list, ENV->WS);
    vec_push(ENV->WS, (intptr_t)list);
}
