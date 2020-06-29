/*
    module  : get.c
    version : 1.3
    date    : 06/22/20
*/

/**
get  :  ->  F
Reads a factor from input and pushes it onto stack.
*/
void do_get(eval_env *ENV)
{
    readfactor(ENV, yylex());
}
/* get */
