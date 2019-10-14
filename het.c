/*
    module  : het.c
    version : 1.2
    date    : 10/11/19
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <inttypes.h>
#include "rmalloc.h"
#include "khash.h"
#include "vector.h"
#include "memory.h"

#define BIT_IDENT	1
#define MAX_IDENT	100

#define SPECIAL(i)	((i) >= '!' && (i) <= '?')
#define WORD(i)		(((i) & BIT_IDENT) != 0)
#define LIST(i)		(((i) & BIT_IDENT) == 0)

char ident[MAX_IDENT + 1];	/* identifier */

intptr_t t, f, s, w, l;		/* standard identifiers */

typedef vector(intptr_t) Stack;

Stack *WS, *PS;			/* working stack, program stack */

KHASH_MAP_INIT_STR(Symbol, intptr_t);

static khash_t(Symbol) *MEM;	/* memory */

typedef void (*proc_t)(void);

KHASH_MAP_INIT_STR(Foreign, proc_t);

static khash_t(Foreign) *FFI;	/* foreign function interface */

void do_gc(void);
void readlist(void);
void marklist(Stack *List);
void writefactor(intptr_t Value);

void reverse(Stack *list)
{
    int i, j;
    intptr_t *PValue1, *PValue2, Temp;

    for (i = 0, j = vec_size(list) - 1; i < j; i++, j--) {
	PValue2 = vec_index(list, j);
	PValue1 = vec_index(list, i);
	Temp = *PValue1;
	*PValue1 = *PValue2;
	*PValue2 = Temp;
    }
}

/* -------------------------------------------------------------------------- */

void markfactor(intptr_t Value)
{
    if (!Value || SPECIAL(Value))
	;
    else if (WORD(Value))
	mark(Value & ~BIT_IDENT);
    else {
	mark(Value);
	mark((intptr_t)vec_data((Stack *)Value));
	marklist((Stack *)Value);
    }
}

void marklist(Stack *List)
{
    int i;

    for (i = vec_size(List) - 1; i >= 0; i--)
	markfactor(vec_at(List, i));
}

void do_gc(void)
{
    khiter_t key;

    mark(t & ~BIT_IDENT);
    mark(f & ~BIT_IDENT);
    mark(s & ~BIT_IDENT);
    mark(w & ~BIT_IDENT);
    mark(l & ~BIT_IDENT);
    marklist(WS);
    marklist(PS);
    for (key = kh_begin(MEM); key != kh_end(MEM); key++)
	if (kh_exist(MEM, key)) {
	    mark((intptr_t)kh_key(MEM, key));
	    markfactor((intptr_t)kh_value(MEM, key));
	}
    scan();
}

void free_rest(void)
{
    scan();
    stk_free(WS);
    stk_free(PS);
    kh_destroy(Symbol, MEM);
    kh_destroy(Foreign, FFI);
    mem_exit();
}

/* -------------------------------------------------------------------------- */

intptr_t str2word(char *str)
{
    return (intptr_t)mem_strdup(str) | BIT_IDENT;
}

intptr_t ch2word(int ch)
{
    char str[2];

    str[0] = ch;
    str[1] = 0;
    return (intptr_t)mem_strdup(str) | BIT_IDENT;
}

intptr_t lookup(char *str)
{
    khiter_t key;
    intptr_t value = 0;

    if ((key = kh_get(Symbol, MEM, str)) != kh_end(MEM))
	value = kh_value(MEM, key);
    return value;
}

void enter(char *str, intptr_t value)
{
    int rv;
    khiter_t key;

    key = kh_put(Symbol, MEM, str, &rv);
    kh_value(MEM, key) = value;
}

/* -------------------------------------------------------------------------- */

proc_t foreign(char *str)
{
    khiter_t key;
    proc_t value = 0;

    if ((key = kh_get(Foreign, FFI, str)) != kh_end(FFI))
	value = kh_value(FFI, key);
    return value;
}

void inter(char *str, proc_t value)
{
    int rv;
    khiter_t key;

    key = kh_put(Foreign, FFI, str, &rv);
    kh_value(FFI, key) = value;
}

void init_ffi(void)
{
    inter("gc", do_gc);
}

/* -------------------------------------------------------------------------- */

int getch(void)
{
    int ch;

    if ((ch = getchar()) == EOF)
	exit(0);
    return ch;
}

int yylex(void)
{
    int ch, i = 0;

retry:
    while ((ch = getch()) <= ' ')
	;
    if (ch == '-' || isalnum(ch)) {
	do
	    if (i < MAX_IDENT)
		ident[i++] = ch;
	while ((ch = getch()) == '-' || isalnum(ch));
	ungetc(ch, stdin);
	ident[i] = 0;
	return BIT_IDENT;
    }
    if (ch == '#')
	while (getch() != '\n')
	    ;
    else if (SPECIAL(ch))
	return ch;
    goto retry;
    return 0;
}

void readfactor(int ch)
{
    if (ch == '(')
	readlist();
    else if (SPECIAL(ch))
	stk_add(PS, ch);
    else
	stk_add(PS, (intptr_t)mem_strdup(ident) | BIT_IDENT);
}

void readlist(void)
{
    int ch;
    Stack *list = 0;

    if ((ch = yylex()) != ')') {
	vec_init(list);
	do {
	    readfactor(ch);
	    vec_add(list, vec_back(PS));
	    vec_pop_back(PS);
	} while ((ch = yylex()) != ')');
	reverse(list);
    }
    stk_add(PS, (intptr_t)list);
}

void read(void)
{
    int ch;

    do
	readfactor(ch = yylex());
    while (ch != '.' && ch != ';');
    reverse(PS);
}

/* -------------------------------------------------------------------------- */

intptr_t next(void)
{
    intptr_t value = 0;

    if (!vec_empty(PS)) {
	value = vec_back(PS);
	vec_pop_back(PS);
    }
    return value;
}

void eval(void)
{
    int i, j;
    proc_t proc;
    char *ptr, *str;
    Stack *list, *quot;
    intptr_t temp, value;

    switch (temp = next()) {
    case '!'  : assert(vec_size(WS) > 0);
		temp = vec_back(WS);
		if (SPECIAL(temp) || WORD(temp))
		    stk_add(PS, temp);
		else {
		    list = (Stack *)temp;
		    for (j = vec_size(list), i = 0; i < j; i++)
			stk_add(PS, vec_at(list, i));
		}
		vec_pop_back(WS);
		break;
    case '$'  : assert(vec_size(WS) > 0);
		temp = vec_back(WS);
		vec_pop_back(WS);
		assert(!SPECIAL(temp) && WORD(temp));
		proc = foreign((char *)(temp & ~BIT_IDENT));
		assert(proc);
		(*proc)();
		break;
    case '*'  : assert(vec_size(WS) > 0);
		temp = vec_back(WS);
		assert(!SPECIAL(temp) && WORD(temp));
		vec_back(WS) = lookup((char *)(temp & ~BIT_IDENT));
		break;
    case '+'  : assert(vec_size(WS) > 0);
		value = vec_back(WS);
		vec_pop_back(WS);
		temp = vec_size(WS) ? vec_back(WS) : 0;
		assert(!SPECIAL(temp) && LIST(temp));
		if ((list = (Stack *)temp) == 0)
		    vec_init(list);
		vec_add(list, value);
		temp = (intptr_t)list;
		if (vec_size(WS))
		    vec_back(WS) = temp;
		else
		    stk_add(WS, temp);
		break;
    case '.'  : if (vec_size(WS)) {
		    writefactor(vec_back(WS));
		    vec_pop_back(WS);
		    putchar('\n');
		}
		break;
    case '/'  :	assert(vec_size(WS) > 0);
		temp = vec_back(WS);
		assert(!SPECIAL(temp) && LIST(temp));
		list = (Stack *)temp;
		assert(vec_size(list) > 0);
		temp = vec_back(list);
		vec_pop_back(list);
		if (vec_empty(list))
		    list = 0;
		vec_back(WS) = (intptr_t)list;
		stk_add(WS, temp);
		break;
    case ':'  : assert(vec_size(WS) > 0);
		temp = vec_back(WS);
		assert(!SPECIAL(temp) && WORD(temp));
		ptr = (char *)(temp & ~BIT_IDENT);
		vec_pop_back(WS);
		temp = vec_size(WS) ? vec_back(WS) : 0;
		if (!temp || SPECIAL(temp) || WORD(temp))
		    enter(ptr, (intptr_t)temp);
		else {
		    quot = (Stack *)temp;
		    if (vec_empty(quot))
			list = 0;
		    else
			vec_copy(quot, list);
		    enter(ptr, (intptr_t)list);
		}
		break;
    case ';'  : if (vec_size(WS))
		    vec_pop_back(WS);
		break;
    case '<'  : assert(vec_size(WS) > 0);
		temp = vec_back(WS);
		assert(temp & BIT_IDENT);
		vec_init(list);
		ptr = (char *)(temp & ~BIT_IDENT);
		for (i = strlen(ptr) - 1; i >= 0; i--)
		   vec_add(list, ch2word(ptr[i]));
		vec_back(WS) = (intptr_t)list;
		break;
    case '='  : assert(vec_size(WS) > 0);
		value = vec_back(WS);
		vec_pop_back(WS);
		temp = vec_size(WS) ? vec_back(WS) : 0;
		if (!temp || SPECIAL(value) || !WORD(value))
		    temp = temp == value ? t : f;
		else {
		    assert(!SPECIAL(temp) && WORD(temp) &&
			   !SPECIAL(value) && WORD(value));
		    temp = strcmp((char *)(temp & ~BIT_IDENT),
				  (char *)(value & ~BIT_IDENT)) ? f : t;
		}
		if (vec_size(WS))
		    vec_back(WS) = temp;
		else
		    stk_add(WS, temp);
		break;
    case '>'  : assert(vec_size(WS) > 0);
		temp = vec_back(WS);
		assert(!SPECIAL(temp) && LIST(temp));
		list = (Stack *)temp;
		ptr = mem_malloc(vec_size(list) + 1);
		for (j = 0, i = vec_size(list) - 1; i >= 0; i--) {
		    value = vec_at(list, i);
		    assert(!SPECIAL(value) && WORD(value));
		    str = (char *)(value & ~BIT_IDENT);
		    ptr[j++] = *str;
		}
		ptr[j] = 0;
		vec_back(WS) = (intptr_t)ptr | BIT_IDENT;
		break;
    case '?'  : assert(vec_size(WS) > 0);
		temp = vec_back(WS);
		if (SPECIAL(temp))
		    vec_back(WS) = s;
		else if (WORD(temp))
		    vec_back(WS) = w;
		else
		    vec_back(WS) = l;
		break;
    default   :	stk_add(WS, temp);
		break;
    }
}

/* -------------------------------------------------------------------------- */

void writelist(Stack *List)
{
    int i;

    printf("(");
    if (List)
	for (i = vec_size(List) - 1; i >= 0; i--) {
	    writefactor(vec_at(List, i));
	    if (i)
		putchar(' ');
	}
    printf(")");
}

void writefactor(intptr_t Value)
{
    if (SPECIAL(Value))
	printf("%c", (int)Value);
    else if (WORD(Value))
	printf("%s", (char *)(Value & ~BIT_IDENT));
    else
	writelist((Stack *)Value);
}

void print(void)
{
    int i, j, k;

    k = vec_size(PS);
    j = vec_size(WS);
    for (i = 0; i < j; i++) {
	writefactor(vec_at(WS, i));
	putchar(' ');
    }
    if (j || k)
	putchar('_');
    for (i = k - 1; i >= 0; i--) {
	putchar(' ');
	writefactor(vec_at(PS, i));
	break;
    }
    if (j || k)
	putchar('\n');
}

/* -------------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
    char *ptr;
    int debugging = 0;

    if (argc > 1) {
	ptr = argv[1];
	if (*ptr == '-') {
	    while (*++ptr)
		if (*ptr == 'd')
		    debugging = 1;
	    ptr = argv[2];
	}
        if (ptr && !freopen(ptr, "r", stdin)) {
	    fprintf(stderr, "failed to open the file '%s'.\n", ptr);
	    return 0;
	}
    }
    setbuf(stdout, 0);
    mem_init();
    atexit(free_rest);
    t = str2word("t");
    f = str2word("f");
    s = str2word("s");
    w = str2word("w");
    l = str2word("l");
    stk_init(WS);
    stk_init(PS);
    MEM = kh_init(Symbol);
    FFI = kh_init(Foreign);
    init_ffi();
    for (;;) {
	if (vec_empty(PS))
	    if (read(), debugging)
		print();
	if (eval(), debugging)
	    print();
    }
    return 0;
}
