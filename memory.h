/*
    module  : memory.h
    version : 1.2
    date    : 11/25/19
*/
void fatal(char *str);
void mem_report(void);
void mem_init(void);
void scan(void);
void mark(intptr_t ptr);
void remind(intptr_t ptr);
void forget(intptr_t ptr);
char *mem_strdup(char *str);
void *mem_malloc(size_t size);
void *mem_realloc(void *old, size_t size);
void mem_exit(void);
void *chk_malloc(size_t size);
void *chk_realloc(void *old, size_t size);
