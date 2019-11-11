/*
    module  : less.c
    version : 1.1
    date    : 11/04/19
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
    second = (char *)(vec_back(WS) & ~BIT_IDENT);
    vec_pop_back(WS);
    first = (char *)(vec_back(WS) & ~BIT_IDENT);
    if (isdigit(*first) && isdigit(*second)) {
	leng1 = strlen(first);
	leng2 = strlen(second);
	if (leng1 != leng2) {
	    if (leng1 < leng2)
		vec_back(WS) = t;
	    else
		vec_back(WS) = f;
	    return;
	}
    }
    if (strcmp(first, second) < 0)
	vec_back(WS) = t;
    else
	vec_back(WS) = f;
}
