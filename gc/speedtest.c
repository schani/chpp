/* -*- c -*- */

#include <stdlib.h>

#include "gc.h"

#ifdef GC
#define MALLOC(s)      GC_malloc_atomic(s)
#define FREE(p)        GC_free(p)
#else
#define MALLOC(s)      malloc(s)
#define FREE(p)        free(p)
#endif

int
main (void)
{
    int i;

    for (i = 0; i < 1000000; ++i)
	FREE(MALLOC(512));

    return 0;
}
