#include "priorityQ.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Merges 2 trees, inserts slave as master's child
 * 
 * @param q Pointer to Fibonacci heap
 * @param master Pointer to master node
 * @param slave Pointer to slave node
 */
static inline void pq_merge_impl(fibHeap_t * restrict q, fibNode_t * restrict master, fibNode_t * restrict slave);
/**
 * @brief Cuts tree, promotes x to root
 * 
 * @param q Pointer to Fibonacci heap
 * @param x Pointer to node to be cut
 * @param y Node's parent node
 */
static inline void pq_cut_impl(fibHeap_t * restrict q, fibNode_t * x, fibNode_t * y);
/**
 * @brief Cascadingly cuts tree
 * 
 * @param q Pointer to Fibonacci heap
 * @param n Pointer to node cut
 */
static inline void pq_cascading_cut_impl(fibHeap_t * restrict q, fibNode_t * n);
/**
 * @brief Promotes node to root node of the heap
 * 
 * @param q Pointer ot Fibonacci heap
 * @param n Pointer to node to promote to root
 */
static inline void pq_promote_impl(fibHeap_t * restrict q, fibNode_t * restrict n);
/**
 * @brief Recursive function that frees the entire heap starting from node n
 * 
 * @param n First node
 * @param firstParent First node in a row of nodes, is used to know when to stop :)
 */
static inline void pq_free_impl(fibNode_t * n, fibNode_t * firstParent);
/**
 * @brief Prints the entire Fibonacci heap
 * 
 * @param n First node to print
 * @param firstParent First node in a row of nodes, is used to know when to stop :)
 * @param fp Output file
 */
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

	// Suurendab look-up tabelit kui vaja
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
	// Vähendab prioriteetsust kui eksisteerib kuhjas, tagastab 'true'
	else if ((q->lut[idx] != NULL) && (distance <= q->lut[idx]->key))
	{
		if (distance < q->lut[idx]->key)
		{
			pq_decPriority(q, idx, distance);
		}
		return true;
	}
	
	// Teeb uue kuhjaelemendi
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

	// Vaatab, kas sellise prioriteetsusega element juba eksisteerib, kui jah, siis eemaldab selle kuhjast
	if (q->lut[idx] != NULL)
	{
		pq_decPriority(q, idx, -INFINITY);
		pq_extractMin(q);
	}
	// Lisab uue elemendi look-up tabelisse
	q->lut[idx] = n;

	// Lisab elemendi Fibonacci kuhja
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
		// Eemaldab "lapse" "vanemad"
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
			// Annab "lastele" "ametikõrgendust", s.o lisab need peajuurde
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

	// Kõik samasuguse kraadiga (sama paljude järglastega) juured liidetakse, kuhi korrastatakse
	// woodfrog repositooriumist https://github.com/woodfrog/FibonacciHeap oli abi kuhja korrastamise implementeerimisel
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
	
	// Leiab uue miinimumi
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

	// Leiab soovitud elemendi look-up tabelist üles
	fibNode_t * n = q->lut[idx];
	// Uus prioriteetsus peab olema kindlasti väiksem või võrdne praegusega
	assert(distance <= n->key);
	// Kui on võrdne, siis ei ole vaja midagi teha
	if (distance == n->key)
	{
		return;
	}

	// Seatakse uus prioriteetsus
	n->key = distance;
	fibNode_t * parent = n->parent;
	// Kui uus prioriteetsus on väiksem "vanema" omast, siis tuleb praegune haru "lõigata" ning lisada peajuurde
	if ((parent != NULL) && (distance < parent->key))
	{
		pq_cut_impl(q, n, parent);
		pq_cascading_cut_impl(q, parent);
	}
	// Kui pärast lõikamist on prioriteetsus väiksem kui praegusel miinimumil, siis tuleb miinimumi uuendada
	if (distance < q->min->key)
	{
		q->min = n;
	}
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
