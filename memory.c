/*
    module  : memory.c
    version : 1.6
    date    : 02/10/20
*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>
#include "rmalloc.h"
#include "khash.h"
#include "memory.h"

#define SPECIAL(i)	((i) >= '!' && (i) <= '?')

#if LONG_MAX == 2147483647
KHASH_MAP_INIT_INT(Backup, char);
#else
KHASH_MAP_INIT_INT64(Backup, char);
#endif

static khash_t(Backup) *MEM;	/* memory */

static intptr_t allocated, max_alloc;

/*
    Report a fatal error and abort execution.
*/
void mem_fatal(void)
{
    fprintf(stderr, "Out of memory\n");
    abort();
}

/*
    Report the maximum amount of memory allocated.
*/
void mem_report(void)
{
    fprintf(stderr, "max. allocated = %" PRIdPTR "\n", max_alloc);
}

/*
    Initialise gc memory.
*/
void mem_init(void)
{
    atexit(mem_report);
    MEM = kh_init(Backup);
}

/*
    Walk registered blocks and free those that have not been marked.
*/
void scan(void)
{
    khiter_t key;

    for (key = kh_begin(MEM); key != kh_end(MEM); key++)
	if (kh_exist(MEM, key)) {
	    if (kh_value(MEM, key))
		kh_value(MEM, key) = 0;
	    else {
		free((void *)kh_key(MEM, key));
		kh_del(Backup, MEM, key);
	    }
	}
    if (max_alloc < allocated)
	max_alloc = allocated;
    allocated = 0;
}

/*
    Mark a block as in use.
*/
void mark(intptr_t ptr)
{
    khiter_t key;

    if ((key = kh_get(Backup, MEM, ptr)) != kh_end(MEM))
	kh_value(MEM, key) = 1;
}

/*
    Register an allocated memory block.
*/
void remind(intptr_t ptr)
{
    int rv;
    khiter_t key;

    key = kh_put(Backup, MEM, ptr, &rv);
    kh_value(MEM, key) = 0;
}

/*
    Forget about a memory block.
*/
void forget(intptr_t ptr)
{
    khiter_t key;

    if ((key = kh_get(Backup, MEM, ptr)) != kh_end(MEM))
	kh_del(Backup, MEM, key);
}

/*
    Register an allocated string.
*/
char *mem_strdup(char *str)
{
    intptr_t temp;

    allocated += strlen(str) + 1;
    do
	if ((temp = (intptr_t)strdup(str)) == 0)
	    mem_fatal();
    while (SPECIAL(temp));
    remind(temp);
    return (char *)temp;
}

/*
    Register an allocated block.
*/
void *mem_malloc(size_t size)
{
    intptr_t temp;

    allocated += size;
    do
	if ((temp = (intptr_t)malloc(size)) == 0)
	    mem_fatal();
    while (SPECIAL(temp));
    remind(temp);
    return (void *)temp;
}

/*
    Deregister an old block and register a new one.
*/
void *mem_realloc(void *old, size_t size)
{
    intptr_t temp;

    allocated += size / 2;
    do
	if ((temp = (intptr_t)realloc(old, size)) == 0)
	    mem_fatal();
    while (SPECIAL(temp));
    if (temp != (intptr_t)old) {
	forget((intptr_t)old);
	remind(temp);
    }
    return (void *)temp;
}

/*
    Cleanup memory registration.
*/
void mem_exit(void)
{
    kh_destroy(Backup, MEM);
}

/*
    Check whether malloc returns 0.
*/
void *chk_malloc(size_t size)
{
    void *ptr;

    if ((ptr = malloc(size)) == 0)
	mem_fatal();
    return ptr;
}

/*
    Check whether realloc returns 0.
*/
void *chk_realloc(void *old, size_t size)
{
    void *ptr;

    if ((ptr = realloc(old, size)) == 0)
	mem_fatal();
    return ptr;
}

#if 0
int main()
{
    char *ptr;

    mem_init();
    ptr = mem_strdup("aap");
    ptr = mem_strdup("noot");
    ptr = mem_strdup("mies");
    mark((intptr_t)ptr);
    scan();
    mem_exit();
    free(ptr);
    return 0;
}
#endif
