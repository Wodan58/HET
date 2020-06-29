/*
    module  : het.c
    version : 1.11
    date    : 06/22/20
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <inttypes.h>
#include "gc.h"

#define kmalloc(Z)		GC_malloc(Z)
#define kcalloc(N,Z)		GC_malloc((N)*(Z))
#define krealloc(N,Z)		GC_realloc((N),(Z))
#define kfree(Z)

#include "khash.h"
#include "kvec.h"

#define BIT_IDENT	1
#define MAX_IDENT	100

#define SPECIAL(i)	((i) >= '!' && (i) <= '?')
#define WORD(i)		(((i) & BIT_IDENT) != 0)
#define LIST(i)		(((i) & BIT_IDENT) == 0)

static char ident[MAX_IDENT + 1];	/* identifier */

typedef vector(intptr_t) Stack;

KHASH_MAP_INIT_STR(Symbol, intptr_t);

typedef void (*proc_t)();

KHASH_MAP_INIT_STR(Foreign, proc_t);

typedef struct name_val {
    char *name;
    intptr_t value;
} name_val;

typedef vector(name_val) Local;

typedef vector(int) Index;

KHASH_SET_INIT_STR(Strings);

typedef struct eval_env {
    intptr_t t, f, s, w, l;	/* standard identifiers */
    Stack *WS, *PS;		/* working stack, program stack */
    khash_t(Symbol) *MEM;	/* memory */
    khash_t(Foreign) *FFI;	/* foreign function interface */
    Local *LOC;			/* local definitions/variables */
    Index *IDX;			/* frame pointers */
    khash_t(Strings) *STR;	/* string table */
} eval_env;

int yylex(void);
void readfactor(eval_env *ENV, int ch);
void writefactor(intptr_t Value);

const char *my_strdup(khash_t(Strings) *STR, char *str)
{
    int rv;
    char *ptr;
    khiter_t key;

    if ((key = kh_get(Strings, STR, str)) == kh_end(STR)) {
	ptr = GC_malloc_atomic(strlen(str) + 1);
	strcpy(ptr, str);
	key = kh_put(Strings, STR, ptr, &rv);
    }
    return kh_key(STR, key);
}

#include "builtin.h"

/* -------------------------------------------------------------------------- */

intptr_t str2word(khash_t(Strings) *STR, char *str)
{
    return (intptr_t)my_strdup(STR, str) | BIT_IDENT;
}

intptr_t ch2word(khash_t(Strings) *STR, int ch)
{
    static char str[2];

    str[0] = ch;
    return (intptr_t)my_strdup(STR, str) | BIT_IDENT;
}

intptr_t lookup(eval_env *ENV, char *str)
{
    int i;
    name_val nv;
    khiter_t key;
    intptr_t value = 0;

    for (i = vec_size(ENV->LOC) - 1; i >= 0; i--) {
	nv = vec_at(ENV->LOC, i);
	if (!strcmp(str, nv.name))
	    return nv.value;
    }
    if ((key = kh_get(Symbol, ENV->MEM, str)) != kh_end(ENV->MEM))
	value = kh_value(ENV->MEM, key);
    return value;
}

void enter(eval_env *ENV, char *str, intptr_t value)
{
    int rv;
    khiter_t key;

    key = kh_put(Symbol, ENV->MEM, str, &rv);
    kh_value(ENV->MEM, key) = value;
}

void local(eval_env *ENV, char *str, intptr_t value)
{
    name_val nv;

    nv.name = str;
    nv.value = value;
    vec_push(ENV->LOC, nv);
}

/* -------------------------------------------------------------------------- */

proc_t foreign(eval_env *ENV, char *str)
{
    khiter_t key;
    proc_t value = 0;

    if ((key = kh_get(Foreign, ENV->FFI, str)) != kh_end(ENV->FFI))
	value = kh_value(ENV->FFI, key);
    return value;
}

void inter(eval_env *ENV, char *str, proc_t value)
{
    int rv;
    khiter_t key;

    key = kh_put(Foreign, ENV->FFI, str, &rv);
    kh_value(ENV->FFI, key) = value;
}

void init_ffi(eval_env *ENV)
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

void readlist(eval_env *ENV)
{
    int ch;
    Stack *list = 0;

    if ((ch = yylex()) != ')') {
	do {
	    readfactor(ENV, ch);
	    vec_push(list, vec_pop(ENV->PS));
	} while ((ch = yylex()) != ')');
	vec_push(list, 0);
	vec_reverse(list);
    }
    vec_push(ENV->PS, (intptr_t)list);
}

void readfactor(eval_env *ENV, int ch)
{
    if (ch == '(')
	readlist(ENV);
    else if (SPECIAL(ch))
	vec_push(ENV->PS, ch);
    else
	vec_push(ENV->PS, (intptr_t)my_strdup(ENV->STR, ident) | BIT_IDENT);
}

void readterm(eval_env *ENV)
{
    int ch;

    do
	readfactor(ENV, ch = yylex());
    while (ch != '.' && ch != ';');
    vec_push(ENV->PS, 0);
    vec_reverse(ENV->PS);
}

/* -------------------------------------------------------------------------- */

intptr_t next(eval_env *ENV)
{
    return vec_size(ENV->PS) ? vec_pop(ENV->PS) : 0;
}

void eval(eval_env *ENV)
{
    int i, j;
    proc_t proc;
    Stack *list;
    char *ptr, *str;
    intptr_t temp, value;

    temp = next(ENV);
again:
    switch (temp) {
    case '!'  : assert(vec_size(ENV->WS));
		temp = vec_pop(ENV->WS);
		if (SPECIAL(temp))
		    vec_push(ENV->PS, temp);
		else if (WORD(temp))
		    vec_push(ENV->WS, temp);
		else {
		    list = (Stack *)temp;
		    vec_push(ENV->PS, ')');
		    for (i = 0, j = vec_size(list); i < j; i++)
			vec_push(ENV->PS, vec_at(list, i));
		    vec_push(ENV->PS, '(');
		}
		break;
    case '$'  : assert(vec_size(ENV->WS));
		temp = vec_pop(ENV->WS);
		assert(!SPECIAL(temp) && WORD(temp));
		proc = foreign(ENV, (char *)(temp & ~BIT_IDENT));
		assert(proc);
		(*proc)(ENV);
		break;
    case '%'  : assert(vec_size(ENV->WS) && vec_size(ENV->IDX));
		temp = vec_pop(ENV->WS);
		assert(!SPECIAL(temp) && WORD(temp));
		ptr = (char *)(temp & ~BIT_IDENT);	
		temp = vec_size(ENV->WS) ? vec_back(ENV->WS) : 0;
		if (!temp || SPECIAL(temp) || WORD(temp))
		    local(ENV, ptr, temp);
		else {
		    vec_copy(list, (Stack *)temp);
		    local(ENV, ptr, (intptr_t)list);
		}
		break;
    case '('  : vec_push(ENV->IDX, vec_size(ENV->LOC));
		break;
    case ')'  : do
		    vec_setsize(ENV->LOC, vec_pop(ENV->IDX));
		while ((temp = next(ENV)) == ')');
		goto again;
		break;
    case '*'  : assert(vec_size(ENV->WS));
		temp = vec_pop(ENV->WS);
		assert(!SPECIAL(temp) && WORD(temp));
		vec_push(ENV->WS, lookup(ENV, (char *)(temp & ~BIT_IDENT)));
		break;
    case '+'  : assert(vec_size(ENV->WS));
		value = vec_pop(ENV->WS);
		temp = vec_size(ENV->WS) ? vec_pop(ENV->WS) : 0;
		assert(!SPECIAL(temp) && LIST(temp));
		list = (Stack *)temp;
		vec_push(list, value);
		vec_push(ENV->WS, (intptr_t)list);
		break;
    case '.'  : if (vec_size(ENV->WS)) {
		    writefactor(vec_pop(ENV->WS));
		    putchar('\n');
		}
		assert(!vec_size(ENV->LOC) && !vec_size(ENV->IDX));
		break;
    case '/'  : assert(vec_size(ENV->WS));
		temp = vec_pop(ENV->WS);
		assert(!SPECIAL(temp) && LIST(temp));
		list = (Stack *)temp;
		assert(vec_size(list));
		temp = vec_pop(list);
		vec_push(ENV->WS, (intptr_t)list);
		vec_push(ENV->WS, temp);
		break;
    case ':'  : assert(vec_size(ENV->WS));
		temp = vec_pop(ENV->WS);
		assert(!SPECIAL(temp) && WORD(temp));
		ptr = (char *)(temp & ~BIT_IDENT);
		temp = vec_size(ENV->WS) ? vec_back(ENV->WS) : 0;
		if (!temp || SPECIAL(temp) || WORD(temp))
		    enter(ENV, ptr, temp);
		else {
		    vec_copy(list, (Stack *)temp);
		    enter(ENV, ptr, (intptr_t)list);
		}
		break;
    case ';'  : if (vec_size(ENV->WS))
		    vec_pop(ENV->WS);
		break;
    case '<'  : assert(vec_size(ENV->WS));
		temp = vec_pop(ENV->WS);
		assert(temp & BIT_IDENT);
		ptr = (char *)(temp & ~BIT_IDENT);
		for (list = 0, i = strlen(ptr) - 1; i >= 0; i--)
		    vec_push(list, ch2word(ENV->STR, ptr[i]));
		vec_push(ENV->WS, (intptr_t)list);
		break;
    case '='  : assert(vec_size(ENV->WS));
		value = vec_pop(ENV->WS);
		temp = vec_size(ENV->WS) ? vec_pop(ENV->WS) : 0;
	        if (SPECIAL(value)) {
		    if (SPECIAL(temp))
			temp = temp == value ? ENV->t : ENV->f;
		    else
			temp = ENV->f;
		} else if (WORD(value)) {
		    if (SPECIAL(temp))
			temp = ENV->f;
		    else if (WORD(temp))
			temp = strcmp((char *)(value & ~BIT_IDENT),
				(char *)(temp & ~BIT_IDENT)) ? ENV->f : ENV->t;
		    else
			temp = ENV->f;
		} else if (!vec_size((Stack *)value) &&
			   !vec_size((Stack *)temp))
		    temp = ENV->t;
		else
		    temp = ENV->f;
		vec_push(ENV->WS, temp);
		break;
    case '>'  : assert(vec_size(ENV->WS));
		temp = vec_pop(ENV->WS);
		assert(!SPECIAL(temp) && LIST(temp));
		list = (Stack *)temp;
		ptr = ident;
		for (j = 0, i = vec_size(list) - 1; i >= 0; i--) {
		    value = vec_at(list, i);
		    assert(!SPECIAL(value) && WORD(value));
		    str = (char *)(value & ~BIT_IDENT);
		    if (j < MAX_IDENT)
			ptr[j++] = *str;
		}
		ptr[j] = 0;
		temp = (intptr_t)my_strdup(ENV->STR, ptr) | BIT_IDENT;
		vec_push(ENV->WS, temp);
		break;
    case '?'  : assert(vec_size(ENV->WS));
		temp = vec_pop(ENV->WS);
		if (SPECIAL(temp))
		    temp = ENV->s;
		else if (WORD(temp))
		    temp = ENV->w;
		else
		    temp = ENV->l;
		vec_push(ENV->WS, temp);
		break;
    default   : vec_push(ENV->WS, temp);
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

void print(eval_env *ENV)
{
    int i, j, k;

    k = vec_size(ENV->PS);
    j = vec_size(ENV->WS);
    for (i = 0; i < j; i++) {
	writefactor(vec_at(ENV->WS, i));
	putchar(' ');
    }
    if (j || k) {
	putchar('-');
	putchar('-');
    }
    for (i = k - 1; i >= 0; i--) {
	putchar(' ');
	writefactor(vec_at(ENV->PS, i));
	break;
    }
    if (j || k)
	putchar('\n');
}

/* -------------------------------------------------------------------------- */

void init(eval_env *ENV)
{
    ENV->STR = kh_init(Strings);
    ENV->t = str2word(ENV->STR, "t");
    ENV->f = str2word(ENV->STR, "f");
    ENV->s = str2word(ENV->STR, "s");
    ENV->w = str2word(ENV->STR, "w");
    ENV->l = str2word(ENV->STR, "l");
    vec_init(ENV->WS);
    vec_init(ENV->PS);
    vec_init(ENV->LOC);
    vec_init(ENV->IDX);
    ENV->MEM = kh_init(Symbol);
    ENV->FFI = kh_init(Foreign);
    init_ffi(ENV);
}

int start_main(int argc, char *argv[])
{
    char *ptr;
    eval_env ENV;
    int debugging = 0;

    fprintf(stderr, "HET  -  compiled at %s on %s\n", __TIME__, __DATE__);
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
    init(&ENV);
    for (;;) {
	if (!vec_size(ENV.PS))
	    if (readterm(&ENV), debugging)
		print(&ENV);
	if (eval(&ENV), debugging)
	    print(&ENV);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int (* volatile m)(int, char **) = start_main;

    GC_init(&argc);
    return (*m)(argc, argv);
}
