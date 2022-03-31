#include "test.h"
#include "../src/priorityQ.h"

#include <stdbool.h>
#include <math.h>
#include <stdarg.h>

bool isclose(float a, float b, float epsilon)
{
	return fabsf(a - b) < epsilon;
}

void testidx(size_t idx, size_t expectedIdx)
{
	test(idx == expectedIdx, "Expected index %zu, got %zu", expectedIdx, idx);
}
void testidx_or(size_t idx, size_t numExp, ...)
{
	va_list ap;
	va_start(ap, numExp);
	bool isExp = false;
	for (size_t i = 0; i < numExp; ++i)
	{
		isExp |= idx == va_arg(ap, size_t);
	}
	va_end(ap);

	test(isExp, "Got index %zu", idx);
	fprintf(stderr, "Got index %zu\n", idx);
}

int main(void)
{
	setlib("Fibonacci heap");
	
	pq_t q;
	pq_init(&q);

	// Insert 4, 3, 0, 2

	test(pq_pushWithPriority(&q, 0, 4.0f), "Pushing failed!");
	test(pq_pushWithPriority(&q, 1, 3.0f), "Pushing failed!");
	test(pq_pushWithPriority(&q, 2, 0.0f), "Pushing failed!");
	test(pq_pushWithPriority(&q, 3, 2.0f), "Pushing failed!");
	test(pq_pushWithPriority(&q, 4, 10.0f), "Pushing failed!");
	test(pq_pushWithPriority(&q, 5, 8.0f), "Pushing failed!");
	test(pq_pushWithPriority(&q, 6, 8.0f), "Pushing failed!");
	test(pq_pushWithPriority(&q, 7, 8.0f), "Pushing failed!");

	testidx(pq_extractMin(&q), 2);
	testidx(pq_extractMin(&q), 3);
	testidx(pq_extractMin(&q), 1);
	testidx(pq_extractMin(&q), 0);

	testidx_or(pq_extractMin(&q), 3, 5, 6, 7);
	testidx_or(pq_extractMin(&q), 3, 5, 6, 7);
	testidx_or(pq_extractMin(&q), 3, 5, 6, 7);
	testidx(pq_extractMin(&q), 4);
	
	testidx(pq_extractMin(&q), SIZE_MAX);

	pq_destroy(&q);

	return 0;
}
