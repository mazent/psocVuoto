#include "utili/includimi.h"


void * soc_malloc(size_t dim)
{
    return malloc(dim) ;
}

void * soc_calloc(size_t num, size_t size)
{
	return calloc(num, size) ;
}

void soc_free(void * v)
{
	if (v)
		free(v) ;
}

