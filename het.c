/*
    module  : het.c
    version : 1.9
    date    : 01/20/20
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <inttypes.h>
#include "rmalloc.h"
#include "khash.h"
#include "kvec.h"
#include "memory.h"

#define BIT_IDENT	1
#define MAX_IDENT	100

#define SPECIAL(i)	((i) >= '!' && (i) <= '?')
#define WORD(i)		(((i) & BIT_IDENT) != 0)
#define LIST(i)		(((i) & BIT_IDENT) == 0)

				/* identifier */
static char ident[MAX_IDENT + 1];

static intptr_t t, f, s, w, l;	/* standard identifiers */

typedef vector(intptr_t) Stack;

static Stack *WS, *PS;		/* working stack, program stack */

KHASH_MAP_INIT_STR(Symbol, intptr_t);

static khash_t(Symbol) *MEM;	/* memory */

typedef void (*proc_t)(void);

KHASH_MAP_INIT_STR(Foreign, proc_t);

static khash_t(Foreign) *FFI;	/* foreign function interface */

typedef struct name_val {
    char *name;
    intptr_t value;
} name_val;

typedef vector(name_val) Local;

static Local *LOC;		/* local definitions/variables */

typedef vector(int) Index;

static Index *IDX;		/* frame pointers */

void gc(void);
int yylex(void);
void readfactor(int ch);
void markfactor(intptr_t Value);
void writefactor(intptr_t Value);

#include "builtin.h"

/* -------------------------------------------------------------------------- */

void marklist(Stack *List)
{
    int i;

    for (i = vec_size(List) - 1; i >= 0; i--)
	markfactor(vec_at(List, i));
}

void markfactor(intptr_t Value)
{
    if (!Value || SPECIAL(Value))
	;
    else if (WORD(Value))
	mark(Value & ~BIT_IDENT);
    else {
	mark(Value);
	mark((intptr_t)&vec_at((Stack *)Value, 0));
	marklist((Stack *)Value);
    }
}

void gc(void)
{
    int i;
    name_val nv;
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
    for (i = vec_size(LOC) - 1; i >= 0; i--) {
	nv = vec_at(LOC, i);
	mark((intptr_t)nv.name);
	markfactor(nv.value);
    }
    scan();
}

void free_rest(void)
{
    scan();
    vec_destroy(WS);
    vec_destroy(PS);
    vec_destroy(LOC);
    vec_destroy(IDX);
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
    static char str[2];

    str[0] = ch;
    return (intptr_t)mem_strdup(str) | BIT_IDENT;
}

intptr_t lookup(char *str)
{
    int i;
    name_val nv;
    khiter_t key;
    intptr_t value = 0;

    for (i = vec_size(LOC) - 1; i >= 0; i--) {
	nv = vec_at(LOC, i);
	if (!strcmp(str, nv.name))
	    return nv.value;
    }
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

void local(char *str, intptr_t value)
{
    name_val nv;

    nv.name = str;
    nv.value = value;
    stk_push(LOC, nv);
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
#include "builtin.c"
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
    if (ch == '_' || isalnum(ch)) {
	do
	    if (i < MAX_IDENT)
		ident[i++] = ch;
	while ((ch = getch()) == '_' || isalnum(ch));
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

void readlist(void)
{
    int ch;
    Stack *list = 0;

    if ((ch = yylex()) != ')') {
	do {
	    readfactor(ch);
	    vec_push(list, vec_pop(PS));
	} while ((ch = yylex()) != ')');
	vec_push(list, 0);
	vec_reverse(list);
    }
    stk_push(PS, (intptr_t)list);
}

void readfactor(int ch)
{
    if (ch == '(')
	readlist();
    else if (SPECIAL(ch))
	stk_push(PS, ch);
    else
	stk_push(PS, (intptr_t)mem_strdup(ident) | BIT_IDENT);
}

void read(void)
{
    int ch;

    do
	readfactor(ch = yylex());
    while (ch != '.' && ch != ';');
    stk_push(PS, 0);
    vec_reverse(PS);
}

/* -------------------------------------------------------------------------- */

intptr_t next(void)
{
    return vec_size(PS) ? vec_pop(PS) : 0;
}

void eval(void)
{
    int i, j;
    proc_t proc;
    Stack *list;
    char *ptr, *str;
    intptr_t temp, value;

    temp = next();
again:
    switch (temp) {
    case '!'  : assert(vec_size(WS));
		temp = vec_pop(WS);
		if (SPECIAL(temp))
		    stk_push(PS, temp);
		else if (WORD(temp))
		    stk_push(WS, temp);
		else {
		    list = (Stack *)temp;
		    stk_push(PS, ')');
		    for (i = 0, j = vec_size(list); i < j; i++)
			stk_push(PS, vec_at(list, i));
		    stk_push(PS, '(');
		}
		break;
    case '$'  : assert(vec_size(WS));
		temp = vec_pop(WS);
		assert(!SPECIAL(temp) && WORD(temp));
		proc = foreign((char *)(temp & ~BIT_IDENT));
		assert(proc);
		(*proc)();
		break;
    case '%'  : assert(vec_size(WS) && vec_size(IDX));
		temp = vec_pop(WS);
		assert(!SPECIAL(temp) && WORD(temp));
		ptr = (char *)(temp & ~BIT_IDENT);	
		temp = vec_size(WS) ? vec_back(WS) : 0;
		if (!temp || SPECIAL(temp) || WORD(temp))
		    local(ptr, temp);
		else {
		    vec_copy(list, (Stack *)temp);
		    local(ptr, (intptr_t)list);
		}
		break;
    case '('  : stk_push(IDX, vec_size(LOC));
		break;
    case ')'  : do
		    vec_setsize(LOC, vec_pop(IDX));
		while ((temp = next()) == ')');
		goto again;
		break;
    case '*'  : assert(vec_size(WS));
		temp = vec_pop(WS);
		assert(!SPECIAL(temp) && WORD(temp));
		stk_push(WS, lookup((char *)(temp & ~BIT_IDENT)));
		break;
    case '+'  : assert(vec_size(WS));
		value = vec_pop(WS);
		temp = vec_size(WS) ? vec_pop(WS) : 0;
		assert(!SPECIAL(temp) && LIST(temp));
		list = (Stack *)temp;
		vec_push(list, value);
		stk_push(WS, (intptr_t)list);
		break;
    case '.'  : if (vec_size(WS)) {
		    writefactor(vec_pop(WS));
		    putchar('\n');
		}
		assert(!vec_size(LOC) && !vec_size(IDX));
		break;
    case '/'  : assert(vec_size(WS));
		temp = vec_pop(WS);
		assert(!SPECIAL(temp) && LIST(temp));
		list = (Stack *)temp;
		assert(vec_size(list));
		temp = vec_pop(list);
		stk_push(WS, (intptr_t)list);
		stk_push(WS, temp);
		break;
    case ':'  : assert(vec_size(WS));
		temp = vec_pop(WS);
		assert(!SPECIAL(temp) && WORD(temp));
		ptr = (char *)(temp & ~BIT_IDENT);
		temp = vec_size(WS) ? vec_back(WS) : 0;
		if (!temp || SPECIAL(temp) || WORD(temp))
		    enter(ptr, temp);
		else {
		    vec_copy(list, (Stack *)temp);
		    enter(ptr, (intptr_t)list);
		}
		break;
    case ';'  : if (vec_size(WS))
		    vec_pop(WS);
		break;
    case '<'  : assert(vec_size(WS));
		temp = vec_pop(WS);
		assert(temp & BIT_IDENT);
		ptr = (char *)(temp & ~BIT_IDENT);
		for (list = 0, i = strlen(ptr) - 1; i >= 0; i--)
		    vec_push(list, ch2word(ptr[i]));
		stk_push(WS, (intptr_t)list);
		break;
    case '='  : assert(vec_size(WS));
		value = vec_pop(WS);
		temp = vec_size(WS) ? vec_pop(WS) : 0;
	        if (SPECIAL(value)) {
		    if (SPECIAL(temp))
			temp = temp == value ? t : f;
		    else
			temp = f;
		} else if (WORD(value)) {
		    if (SPECIAL(temp))
			temp = f;
		    else if (WORD(temp))
			temp = strcmp((char *)(value & ~BIT_IDENT),
				      (char *)(temp & ~BIT_IDENT)) ? f : t;
		    else
			temp = f;
		} else if (!vec_size((Stack *)value) &&
			   !vec_size((Stack *)temp))
		    temp = t;
		else
		    temp = f;
		stk_push(WS, temp);
		break;
    case '>'  : assert(vec_size(WS));
		temp = vec_pop(WS);
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
		stk_push(WS, (intptr_t)ptr | BIT_IDENT);
		break;
    case '?'  : assert(vec_size(WS));
		temp = vec_pop(WS);
		if (SPECIAL(temp))
		    temp = s;
		else if (WORD(temp))
		    temp = w;
		else
		    temp = l;
		stk_push(WS, temp);
		break;
    default   : stk_push(WS, temp);
		break;
    }
}

/* -------------------------------------------------------------------------- */

void writelist(Stack *List)
{
    int i;

    printf("(");
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
    if (j || k) {
	putchar('-');
	putchar('-');
    }
    for (i = k - 1; i >= 0; i--) {
	putchar(' ');
	writefactor(vec_at(PS, i));
	break;
    }
    if (j || k)
	putchar('\n');
}

/* -------------------------------------------------------------------------- */

void init(void)
{
    mem_init();
    t = str2word("t");
    f = str2word("f");
    s = str2word("s");
    w = str2word("w");
    l = str2word("l");
    stk_init(WS);
    stk_init(PS);
    stk_init(LOC);
    stk_init(IDX);
    MEM = kh_init(Symbol);
    FFI = kh_init(Foreign);
    init_ffi();
}

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
    init();
    atexit(free_rest);
    for (;;) {
	if (!vec_size(PS))
	    if (read(), debugging)
		print();
	if (eval(), debugging)
	    print();
    }
    return 0;
}
