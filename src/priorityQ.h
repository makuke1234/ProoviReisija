#ifndef PRIORITY_Q_H
#define PRIORITY_Q_H

#include <stdint.h>
#include <stdbool.h>

#if SIZE_MAX == UINT64_MAX
	#define IDX_BITS 63
#else
	#define IDX_BITS 31
#endif


struct fibTree;

typedef struct fibDLLNode
{
	struct fibTree * node;

	struct fibDLLNode * prev, * next;

} fibDLLNode_t;

typedef struct fibDLList
{
	fibDLLNode_t * first;
	size_t numNodes;

} fibDLList_t, pQ_t;

typedef struct fibTree
{
	// Data
	float distKey;

	size_t marked:1;
	size_t idx:IDX_BITS;


	// Linking
	struct fibTree * parent;

	fibDLList_t children;

} fibTree_t;



size_t fibCalc(size_t idx);

bool pQ_init(pQ_t * q);

bool pQ_addPriority(pQ_t * q, size_t idx, float distance);
size_t pQ_extractMin(pQ_t * q);
void pQ_decPriority(pQ_t * q, size_t idx, float distance);

void pQ_destroy(pQ_t * q);

#endif
