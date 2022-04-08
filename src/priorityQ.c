#include "priorityQ.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

static inline void pq_merge_impl(fibHeap_t * restrict q, fibNode_t * restrict master, fibNode_t * restrict slave);
static inline void pq_cut_impl(fibHeap_t * restrict q, fibNode_t * x, fibNode_t * y);
static inline void pq_cascading_cut_impl(fibHeap_t * restrict q, fibNode_t * n);
static inline void pq_promote_impl(fibHeap_t * restrict q, fibNode_t * restrict n);
static inline void pq_free_impl(fibNode_t * n, fibNode_t * firstParent);
static inline void pq_print_impl(fibNode_t * n, fibNode_t * firstParent, FILE * restrict fp);

static inline void pq_merge_impl(fibHeap_t * restrict q, fibNode_t * restrict master, fibNode_t * restrict slave)
{
	assert(q != NULL);
	assert(master != NULL);
	assert(slave != NULL);
	assert(master != slave);

	// Remove slave from root
	slave->left->right = slave->right;
	slave->right->left = slave->left;

	slave->left  = slave;
	slave->right = slave;

	if (q->min == slave)
	{
		q->min = master;
	}

	// Attach slave to master's children
	if (master->child == NULL)
	{
		master->child = slave;
	}
	else
	{
		slave->right = master->child->right;
		slave->left = master->child;
		master->child->right->left = slave;
		master->child->right = slave;
	}

	slave->parent = master;

	// Increase master's degree by 1
	++(master->degree);
}
static inline void pq_cut_impl(fibHeap_t * restrict q, fibNode_t * x, fibNode_t * y)
{
	assert(q != NULL);
	assert(x != NULL);
	assert(y != NULL);

	if (x->right == x)
	{
		y->child = NULL;
	}
	else
	{
		y->child = x->right;

		x->left->right = x->right;
		x->right->left = x->left;
	}

	--(y->degree);
	pq_promote_impl(q, x);
}
static inline void pq_cascading_cut_impl(fibHeap_t * restrict q, fibNode_t * n)
{
	assert(q != NULL);
	assert(n != NULL);

	fibNode_t * parent = n->parent;
	if (parent != NULL)
	{
		if (n->marked == NOT_MARKED)
		{
			n->marked = MARKED;
		}
		else
		{
			pq_cut_impl(q, n, parent);
			pq_cascading_cut_impl(q, parent);
		}
	}
}
static inline void pq_promote_impl(fibHeap_t * restrict q, fibNode_t * restrict n)
{
	n->parent = NULL;
	n->marked = NOT_MARKED;

	if (q->min == NULL)
	{
		n->left = n;
		n->right = n;
		q->min = n;
	}
	else
	{
		n->left = q->min->left;
		n->right = q->min;


		q->min->left->right = n;
		q->min->left = n;
	
		if (n->key < q->min->key)
		{
			q->min = n;
		}
	}

	// Increase root degree
	++(q->n);
}
static inline void pq_free_impl(fibNode_t * n, fibNode_t * firstParent)
{
	if (n == NULL)
	{
		return;
	}

	if (n->left != firstParent)
	{
		pq_free_impl(n->left, firstParent);
	}
	pq_free_impl(n->child, n->child);
	free(n);

}
static inline void pq_print_impl(fibNode_t * n, fibNode_t * firstParent, FILE * restrict fp)
{
	if (n == NULL)
	{
		return;
	}
	fprintf(fp, "%f id: %zu\n", (double)n->key, n->idx);
	
	if (n->left != firstParent)
	{
		pq_print_impl(n->left, firstParent, fp);
	}
	fprintf(fp, "%zu Children\n", n->idx);
	pq_print_impl(n->child, n->child, fp);
}

void pq_init(fibHeap_t * restrict q)
{
	assert(q != NULL);

	*q = (fibHeap_t){
		.n   = 0,
		.min = NULL,
		
		.n_lut = 0,
		.lut   = NULL
	};
}

bool pq_empty(fibHeap_t * restrict q)
{
	return q->min == NULL;
}
bool pq_pushWithPriority(fibHeap_t * restrict q, size_t idx, float distance)
{
	assert(q != NULL);

	// Resize lut if necessary
	if (idx >= q->n_lut)
	{
		size_t newcap = (idx + 1) * 2;
		fibNode_t ** nmem = realloc(q->lut, sizeof(fibNode_t *) * newcap);
		if (nmem == NULL)
		{
			return false;
		}

		for (size_t i = q->n_lut; i < newcap; ++i)
		{
			nmem[i] = NULL;
		}

		q->lut = nmem;
		q->n_lut = newcap;
	}
	else if ((q->lut[idx] != NULL) && (distance <= q->lut[idx]->key))
	{
		if (distance < q->lut[idx]->key)
		{
			pq_decPriority(q, idx, distance);
		}
		return true;
	}
	
	fibNode_t * n = malloc(sizeof(fibNode_t));
	if (n == NULL)
	{
		return false;
	}
	*n = (fibNode_t){
		.key = distance,
		.idx = idx,
		.marked = NOT_MARKED,
		.degree = 0,
		.left = n,
		.right = n,
		.parent = NULL,
		.child = NULL
	};

	// Search if node with that idx already exists
	if (q->lut[idx] != NULL)
	{
		pq_decPriority(q, idx, -INFINITY);
		pq_extractMin(q);
	}
	// Add q to lut
	q->lut[idx] = n;

	// Add node to the Fibonacci heap
	if (q->min == NULL)
	{
		q->min = n;
	}
	else
	{
		n->right = q->min;
		n->left  = q->min->left;
		q->min->left->right = n;
		q->min->left = n;
		if (distance < q->min->key)
		{
			q->min = n;
		}
	}
	++(q->n);

	return true;
}
size_t pq_extractMin(fibHeap_t * restrict q)
{
	assert(q != NULL);
	
	if (q->min == NULL)
	{
		return SIZE_MAX;
	}

	fibNode_t * min = q->min;
	fibNode_t * child = min->child;

	size_t idx = min->idx;
	q->lut[idx] = NULL;

	q->n = q->n + min->degree - 1;
	q->min = min->right;

	min->left->right = q->min;
	q->min->left = min->left;
	
	if (min == min->right)
	{
		q->min = NULL;
	}
	free(min);

	if (child != NULL)
	{
		// Remove children parents
		fibNode_t * temp = child;
		do
		{
			temp->parent = NULL;
			temp->marked = NOT_MARKED;
			temp = temp->right;
		} while (temp != child);

		if (q->min == NULL)
		{
			q->min = child;
		}
		else
		{
			// Promote children to root
			q->min->left->right = child;
			fibNode_t * oldLeft = q->min->left;
			q->min->left = child->left;
			child->left->right = q->min;
			child->left = oldLeft;
		}
	}
	else if (q->min == NULL)
	{
		return idx;
	}

	// Merge all roots with equal degree, consolidate heap
	// Special thanks to woodfrog's repository https://github.com/woodfrog/FibonacciHeap helping to implement heap consolidation
	const size_t degree = MAX_DEGREE;
	fibNode_t * A[MAX_DEGREE];
	for (size_t i = 0; i < degree; ++i)
	{
		A[i] = NULL;
	}

	fibNode_t * x = q->min, * x2 = x;
	while (1)
	{
		size_t d = x->degree;
		if ((d < degree) && (A[d] != NULL))
		{
			fibNode_t * y = A[d];
			if (x == y)
			{
				break;
			}
			A[d] = NULL;
			if (x->key > y->key)
			{
				fibNode_t * t = x;
				x = y;
				y = t;
			}

			pq_merge_impl(q, x, y);
			--(q->n);
			x2 = x;
			continue;
		}
		else if (d < degree)
		{
			A[d] = x;
		}
		x = x->right;
		if (x2 == x)
		{
			break;
		}
	}
	q->min = x;
	
	// Find new minimum
	fibNode_t * oldmin = x;
	do
	{
		if (x->key < q->min->key)
		{
			q->min = x;
		}

		x = x->right;
	} while (x != oldmin);

	return idx;
}
void pq_decPriority(fibHeap_t * restrict q, size_t idx, float distance)
{
	assert(q != NULL);
	assert(idx < q->n_lut);

	fibNode_t * n = q->lut[idx];
	assert(distance <= n->key);
	if (distance == n->key)
	{
		return;
	}

	n->key = distance;
	fibNode_t * parent = n->parent;
	if ((parent != NULL) && (distance < parent->key))
	{
		pq_cut_impl(q, n, parent);
		pq_cascading_cut_impl(q, parent);
	}
	if (distance < q->min->key)
	{
		q->min = n;
	}
	/*if ((parent != NULL) && (distance < parent->key))
	{
		// Promote to root list
		if (n->parent->child == n)
		{
			if (n->left == n)
			{
				n->parent->child = NULL;
			}
			else
			{
				n->parent->child = n->left;
			}
		}
		--(n->parent->degree);
		n->left->right = n->right;
		n->right->left = n->left;
		n->left = n;
		n->right = n;
		pq_promote_impl(q, n);

		while (parent != NULL)
		{
			if ((parent->parent != NULL) && (parent->marked == NOT_MARKED))
			{
				parent->marked = MARKED;
				break;
			}
			else
			{
				// Promote to root list
				if (parent->parent != NULL)
				{
					if (parent->parent->child == parent)
					{
						if (parent->left == parent)
						{
							parent->parent->child = NULL;
						}
						else
						{
							parent->parent->child = parent->left;
						}
					}
					--(parent->parent->degree);
				}
				parent->left->right = parent->right;
				parent->right->left = parent->left;
				parent->left = parent;
				parent->right = parent;
				
				fibNode_t * gp = parent->parent;
				pq_promote_impl(q, parent);

				parent = gp;
			}
		}
	}
	else if ((parent == NULL) && (distance < q->min->key))
	{
		q->min = n;
	}*/
}
void pq_print(fibHeap_t * restrict q, FILE * restrict fp)
{
	pq_print_impl(q->min, q->min, fp);
}

void pq_destroy(fibHeap_t * restrict q)
{
	assert(q != NULL);

	pq_free_impl(q->min, q->min);
	free(q->lut);
}
