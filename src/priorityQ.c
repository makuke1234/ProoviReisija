#include "priorityQ.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>

static inline void pq_merge_impl(fibHeap_t * q, fibNode_t * master, fibNode_t * slave);
static inline void pq_promote_impl(fibHeap_t * q, fibNode_t * n);
static inline void pq_free_impl(fibNode_t * n, fibNode_t * firstParent);

static inline void pq_merge_impl(fibHeap_t * q, fibNode_t * master, fibNode_t * slave)
{
	assert(q != NULL);

	// Remove slave from root
	slave->left->right = slave->right;
	slave->right->left = slave->left;

	slave->left = slave;
	slave->right = slave;

	// Attach slave to master's children
	if (master->child == NULL)
	{
		master->child = slave;
	}
	else
	{
		master->child->left->right = slave;
		slave->right = master->child;
		slave->left = master->child->left;
		master->child->left = slave;
	}

	slave->parent = master;

	// Increase master's degree by 1
	++(master->degree);

	// Decrease root degree by 1
	--(q->n);
}
static inline void pq_promote_impl(fibHeap_t * q, fibNode_t * n)
{
	n->parent = NULL;
	n->marked = NOT_MARKED;

	n->left = q->min->left;
	n->right = q->min;

	q->min->left->right = n;
	q->min->left = n;

	// Increase root degree
	++(q->n);

	if (n->key < q->min->key)
	{
		q->min = n;
	}
}
static inline void pq_free_impl(fibNode_t * n, fibNode_t * firstParent)
{
	if (n == NULL)
	{
		return;
	}

	pq_free_impl(n->left, firstParent);
	pq_free_impl(n->child, n);
	free(n);
}

void pq_init(fibHeap_t * q)
{
	assert(q != NULL);

	*q = (fibHeap_t){
		.n   = 0,
		.min = NULL,
		
		.n_lut = 0,
		.lut   = NULL
	};
}

bool pq_empty(fibHeap_t * q)
{
	return q->min == NULL;
}
bool pq_pushWithPriority(fibHeap_t * q, size_t idx, float distance)
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
	if (idx < q->n_lut && q->lut[idx] != NULL)
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
		if (n->key < q->min->key)
		{
			q->min = n;
		}
	}
	++(q->n);

	return true;
}
size_t pq_extractMin(fibHeap_t * q)
{
	assert(q != NULL);
	
	if (q->min == NULL)
	{
		return SIZE_MAX;
	}

	fibNode_t * min = q->min;
	fibNode_t * child = min->child;

	size_t idx = min->idx;
	q->n = q->n + min->degree - 1;
	min->left->right = min->right;
	min->right->left = min->left;
	q->min = min->right;
	
	free(min);

	if (child != NULL)
	{
		// Promote children to root
		q->min->left->right = child;
		fibNode_t * oldLeft = q->min->left;
		q->min->left = child->left;
		child->left->right = q->min;
		child->left = oldLeft;
	}

	// Merge all roots with equal degree
	fibNode_t * A[MAX_DEGREE];
	for (size_t i = 0; i < MAX_DEGREE; ++i)
	{
		A[i] = NULL;
	}

	fibNode_t * temp = q->min;
	do
	{
		size_t d = temp->degree;
		while (A[d] != NULL)
		{
			fibNode_t * t2 = A[d];
			
			if (temp->key < t2->key)
			{
				pq_merge_impl(q, temp, t2);
			}
			else
			{
				if (temp == q->min)
				{
					q->min = t2;
				}
				pq_merge_impl(q, t2, temp);
				temp = t2;
				if (t2->right == t2)
				{
					q->min = t2;
				}
			}
			A[d] = NULL;
			++d;
		}
		A[d] = temp;
		temp = temp->right;

	} while (temp != q->min);
	
	// Find new minimum
	temp = q->min;
	fibNode_t * oldmin = q->min;
	do
	{
		if (temp->key < q->min->key)
		{
			q->min = temp;
		}

		temp = temp->right;
	} while (temp != oldmin);

	return idx;

	/*fibNode_t * temp = q->min, * ptr = temp, * x = NULL;

	if (temp->child != NULL)
	{
		x = temp->child;
		do
		{
			ptr = x->right;
			q->min->left->right = x;
			x->right = q->min;
			x->left = q->min->left;
			q->min->left = x;
			if (x->key < q->min->key)
			{
				q->min = x;
			}
			x->parent = NULL;
			x = ptr;
		} while (ptr != temp->child);
	}

	temp->left->right = temp->right;
	temp->right->left = temp->left;

	if ((temp == temp->right) && (temp->child == NULL))
	{
		q->min = NULL;
	}
	else
	{
		q->min = temp->right;
		pq_consolidate_impl(q);
	}
	--(q->n);

	size_t idx = temp->idx;
	q->lut[idx] = NULL;

	free(temp);

	return idx;*/
}
void pq_decPriority(fibHeap_t * q, size_t idx, float distance)
{
	assert(q != NULL);

	if (idx >= q->n_lut || q->lut[idx] == NULL)
	{
		return;
	}
	fibNode_t * n = q->lut[idx];

	if (n->key < distance)
	{
		return;
	}
	else
	{
		n->key = distance;
		fibNode_t * parent = n->parent;
		if ((parent != NULL) && (distance < parent->key))
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
				--(n->parent->degree);
			}
			n->left->right = n->right;
			n->right->left = n->left;
			n->left = n;
			n->right = n;
			pq_promote_impl(q, n);

			while (parent != NULL)
			{
				if (parent->marked == NOT_MARKED)
				{
					parent->marked = MARKED;
					break;
				}
				else
				{
					// Promote to root list
					if ((parent->parent != NULL) && (parent->parent->child = parent))
					{
						if (parent->left == parent)
						{
							parent->parent->child = NULL;
						}
						else
						{
							parent->parent->child = parent->left;
						}
						--(parent->parent->degree);
					}
					parent->left->right = n->right;
					parent->right->left = n->left;
					parent->left = parent;
					parent->right = parent;
					
					fibNode_t * gp = parent->parent;
					pq_promote_impl(q, parent);

					parent = gp;
				}
			}
		}
	}
}

void pq_destroy(fibHeap_t * q)
{
	assert(q != NULL);

	pq_free_impl(q->min, q->min);
	free(q->lut);
}
