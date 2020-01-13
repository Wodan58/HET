/*
    module  : less.c
    version : 1.2
    date    : 01/13/20
*/

/**
less  :  X Y  ->  B
Either both X and Y are numeric or both are strings or symbols.
Tests whether X less than Y.  Also supports float.
*/
void do_less(void)
{
    size_t leng1, leng2;
    char *first, *second;

    assert(vec_size(WS) > 1);
    second = (char *)(vec_pop(WS) & ~BIT_IDENT);
    first = (char *)(vec_pop(WS) & ~BIT_IDENT);
    if (isdigit(*first) && isdigit(*second)) {
	leng1 = strlen(first);
	leng2 = strlen(second);
	if (leng1 != leng2) {
	    stk_push(WS, leng1 < leng2 ? t : f);
	    return;
	}
    }
    stk_push(WS, strcmp(first, second) < 0 ? t : f);
}
