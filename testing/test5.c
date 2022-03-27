#include "test.h"
#include "../src/priorityQ.h"

#include <stdbool.h>
#include <math.h>

bool isclose(float a, float b, float epsilon)
{
	return fabsf(a - b) < epsilon;
}

void testidx(size_t idx, size_t expectedIdx)
{
	test(idx == expectedIdx, "Expected index %zu, got %zu", expectedIdx, idx);
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

	pq_print(&q);

	testidx(pq_extractMin(&q), 2);
	pq_print(&q);
	
	testidx(pq_extractMin(&q), 3);
	pq_print(&q);
	
	testidx(pq_extractMin(&q), 1);
	pq_print(&q);
	
	testidx(pq_extractMin(&q), 0);
	pq_print(&q);
	
	testidx(pq_extractMin(&q), SIZE_MAX);
	pq_print(&q);


	pq_destroy(&q);

	return 0;
}
