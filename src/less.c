/*
    module  : less.c
    version : 1.4
    date    : 06/22/20
*/

/**
less  :  X Y  ->  B
Either both X and Y are numeric or both are strings or symbols.
Tests whether X less than Y.  Also supports float.
*/
void do_less(eval_env *ENV)
{
    size_t leng1, leng2;
    char *first, *second;

    assert(vec_size(ENV->WS) > 1);
    second = (char *)(vec_pop(ENV->WS) & ~BIT_IDENT);
    first = (char *)(vec_pop(ENV->WS) & ~BIT_IDENT);
    if (isdigit(*first) && isdigit(*second)) {
	leng1 = strlen(first);
	leng2 = strlen(second);
	if (leng1 != leng2) {
	    vec_push(ENV->WS, leng1 < leng2 ? ENV->t : ENV->f);
	    return;
	}
    }
    vec_push(ENV->WS, strcmp(first, second) < 0 ? ENV->t : ENV->f);
}
