#ifndef PRIORITY_Q_H
#define PRIORITY_Q_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#if SIZE_MAX == UINT64_MAX
	#define DEGREE_BITS 63
#else
	#define DEGREE_BITS 31
#endif

#define MARKED 1
#define NOT_MARKED 0
#define MAX_DEGREE 64

// Reciprocal value of logarithm in base 2 of the golden ratio
#define LOG2_GR_REC 1.4404829720657015340797013126062

/**
 * @brief Data structure that holds doubly linked list of tree nodes'
 * siblings and also a pointer to it's parent and child node.
 * Contains member 'key' which denotes the priority of the node, and 'idx'
 * which denotes the identifier of the node.
 * Also holds a boolean value whether the node is 'marked' or not + node's degree
 * which is it's number of children.
 * 
 */
typedef struct fibNode
{
	// Data
	float key;
	size_t idx;

	size_t marked:1;
	size_t degree:DEGREE_BITS;

	// Linked list behaviour
	struct fibNode * left, * right;

	struct fibNode * parent;
	struct fibNode * child;

} fibNode_t;

/**
 * @brief Data structure that holds the number of nodes in Fibonacci heap
 * root tree and a pointer to the minimum root node.
 * 
 * Also holds number of items in look-up-table of all nodes, and the look-up-table
 * itself, which holds pointers to said nodes with a particular 'idx'.
 * 
 */
typedef struct fibHeap
{
	size_t n;
	fibNode_t * min;

	size_t n_lut;
	fibNode_t ** lut;

} fibHeap_t, pq_t;


/**
 * @brief Initializes Fibonacci heap data structure.
 * Complexity: O(1).
 * 
 * @param q Pointer to fibHeap_t structure
 */
void pq_init(fibHeap_t * restrict q);

/**
 * @brief Checks whether the heap is empty.
 * Complexity: O(1).
 * 
 * @param q Pointer to fibHeap_t structure
 * @return true Heap is empty
 * @return false Heap contains items
 */
bool pq_empty(fibHeap_t * restrict q);
/**
 * @brief Pushes a new item to the heap with a particular identifier 'idx' &
 * priority of 'distance'. 'Updates' item's priority if it already exists.
 * Complexity: O(1) if item doesn't exist prior or new distance is smaller than or equal to previous.
 * Complexity: O(log n) if item exist prior and distance is bigger than previous.
 * 
 * @param q Pointer to fibHeap_t structure
 * @param idx Identifier of pushable node
 * @param distance Priority/key of the new node
 * @return true Success pushing
 * @return false Failure
 */
bool pq_pushWithPriority(fibHeap_t * restrict q, size_t idx, float distance);
/**
 * @brief Extracts first item's (minimum item's) identifier 'idx' from heap and also
 * removes it.
 * Complexity: O(log n).
 * 
 * @param q Pointer to fibHeap_t structure
 * @return size_t Identifier of minimum item in the heap
 */
size_t pq_extractMin(fibHeap_t * restrict q);
/**
 * @brief Decreases the priority of an existing node with identifier 'idx'.
 * New priority must be smaller or equal to previous!
 * Complexity: O(1).
 * 
 * @param q Pointer to fibHeap_t structure
 * @param idx Identifier of decreaseable node
 * @param distance New decreased priority of the node
 */
void pq_decPriority(fibHeap_t * restrict q, size_t idx, float distance);
/**
 * @brief Prints the entire Fibonacci heap structure.
 * Complexity: O(n).
 * 
 * @param q Pointer to fibHeap_t structure
 * @param fp File to print to
 */
void pq_print(fibHeap_t * restrict q, FILE * restrict fp);

/**
 * @brief Destroys the Fibonacci heap.
 * Complexity: O(n).
 * 
 * @param q Pointer to fibHeap_t structure
 */
void pq_destroy(fibHeap_t * restrict q);

#endif
