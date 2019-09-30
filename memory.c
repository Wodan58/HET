/*
    module  : memory.c
    version : 1.2
    date    : 09/30/19
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

#if LONG_MAX == 2147483647
KHASH_MAP_INIT_INT(Backup, char);
#else
KHASH_MAP_INIT_INT64(Backup, char);
#endif

static khash_t(Backup) *MEM;	/* memory */

static intptr_t allocated, max_alloc;

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
    char *ptr;

    allocated += strlen(str) + 1;
    if ((ptr = strdup(str)) != 0)
	remind((intptr_t)ptr);
    return ptr;
}

/*
    Register an allocated block.
*/
void *mem_malloc(size_t size)
{
    void *ptr;

    allocated += size;
    if ((ptr = malloc(size)) != 0)
	remind((intptr_t)ptr);
    return ptr;
}

/*
    Deregister an old block and register a new one.
*/
void *mem_realloc(void *old, size_t size)
{
    void *ptr;

    allocated += size / 2;
    if ((ptr = realloc(old, size)) != 0) {
	if (ptr != old) {
	    forget((intptr_t)old);
	    remind((intptr_t)ptr);
	}
	return ptr;
    }
    return old;
}

/*
    Cleanup memory registration.
*/
void mem_exit(void)
{
    kh_destroy(Backup, MEM);
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
