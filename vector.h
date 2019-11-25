/*
    module  : vector.h
    version : 1.3
    date    : 11/18/19
*/
#define vector(type)	struct { unsigned n, m; type *a; }
#define vec_init(v)	do { (v) = mem_malloc(sizeof(*(v))); (v)->n = 0; \
			(v)->m = 1; (v)->a = mem_malloc(sizeof(*(v)->a)); \
			} while (0)
#define vec_top(v)	((v)->a + (v)->n - 1)
#define vec_push(v)	(((v)->n++ == (v)->m) ? ((v)->a = mem_realloc((v)->a, \
			sizeof(*(v)->a) * ((v)->m <<= 1))), vec_top(v) : \
			vec_top(v))
#define vec_add(v, w)	(*vec_push(v) = (w))
#define vec_size(v)	((v) ? (v)->n : 0)
#define vec_at(v, i)	((v)->a[i])
#define vec_back(v)	((v)->a[(v)->n - 1])
#define vec_pop_back(v)	(--(v)->n)

#define stk_init(v)	do { (v) = malloc(sizeof(*(v))); (v)->n = 0; \
			(v)->m = 1; (v)->a = malloc(sizeof(*(v)->a)); \
			} while (0)
#define stk_push(v)	(((v)->n++ == (v)->m) ? ((v)->a = realloc((v)->a, \
			sizeof(*(v)->a) * ((v)->m <<= 1))), vec_top(v) : \
			vec_top(v))
#define stk_add(v, w)	(*stk_push(v) = (w))
#define stk_free(v)	do { free((v)->a); free(v); } while (0)
