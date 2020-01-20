/*
    module  : get.c
    version : 1.1
    date    : 01/20/20
*/

/**
get  :  ->  F
Reads a factor from input and pushes it onto stack.
*/
void do_get(void)
{
    readfactor(yylex());
}
/* get */
