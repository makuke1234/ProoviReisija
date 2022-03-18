#include "priorityQ.h"

#include <math.h>

#define SQRT5_INV	0.44721359549995793928183473374626
#define FIB_A		1.6180339887498948482045868343656

size_t fibCalc(size_t idx)
{
	return (size_t)(pow(FIB_A, (double)idx) * SQRT5_INV + 0.5);
}



bool pQ_init(pQ_t * q)
{
	assert(q != NULL);
	q->roots = fibList_make();

	return q->roots != NULL;
}

bool pQ_addPriority(pQ_t * q, size_t idx, float distance)
{
	assert(q != NULL);
	assert(q->first != NULL);

}
size_t pQ_extractMin(pQ_t * q)
{
	assert(q != NULL);
	assert(q->first != NULL);
	
}
void pQ_decPriority(pQ_t * q, size_t idx, float distance)
{
	assert(q != NULL);
	assert(q->first != NULL);

}

static inline void fibTree_free(fibTree_t * restrict fib)
{
	if (fib != NULL)
	{
		if (fib->children != NULL)
		{
			for (size_t i = 0, num = fib->numChildren; i < num; ++i)
			{
				fibTree_t * restrict child = fib->children[i];
				fibTree_free(child);
			}
			free(fib->children);
		}
		free(fib);
	}
}

void pQ_destroy(pQ_t * q)
{
	assert(q != NULL);

	fibTree_free(q->root);
}
