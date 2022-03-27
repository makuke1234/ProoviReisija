#ifndef PRIORITY_Q_H
#define PRIORITY_Q_H

#include <stdint.h>
#include <stdbool.h>

#if SIZE_MAX == UINT64_MAX
	#define DEGREE_BITS 62
#else
	#define DEGREE_BITS 30
#endif

#define VISITED 1
#define NOT_VISITED 0

#define MARKED 1
#define NOT_MARKED 0

#define MAX_DEGREE (DEGREE_BITS + 1)


typedef struct fibNode
{
	// Data
	float key;
	size_t idx;

	size_t marked:1;
	size_t visited:1;
	size_t degree:DEGREE_BITS;

	// Linked list behaviour
	struct fibNode * left, * right;

	struct fibNode * parent;
	struct fibNode * child;

} fibNode_t;

typedef struct fibHeap
{
	size_t n;
	fibNode_t * min;

	size_t n_lut;
	fibNode_t ** lut;

} fibHeap_t, pq_t;


void pq_init(fibHeap_t * q);

bool pq_empty(fibHeap_t * q);
bool pq_pushWithPriority(fibHeap_t * q, size_t idx, float distance);
size_t pq_extractMin(fibHeap_t * q);
void pq_decPriority(fibHeap_t * q, size_t idx, float distance);

void pq_destroy(fibHeap_t * q);

#endif
