/*
    module  : gc.c
    version : 1.3
    date    : 12/13/24
*/

/**
gc  :  ->
Initiates garbage collection.
*/
void do_gc(eval_env *ENV)
{
    GC_gcollect();
}
/* gc */
