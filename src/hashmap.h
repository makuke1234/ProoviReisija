#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/**
 * @brief Data structure for handling hashtable "nodes"
 * 
 */
typedef struct hashNode
{
	char * key;
	void * value;

	struct hashNode * next;
} hashNode_t;

/**
 * @brief Data structure for handling hashtable "nodes", is optimized to used
 * constant pre-allocted memory for "key"s
 * 
 */
typedef struct hashNodeConstKey
{
	const char * key;
	void * value;

	struct hashNodeConstKey * next;
} hashNodeConstKey_t, hashNodeCK_t;



/**
 * @brief Data structure that holds an array of hashtable's nodes
 * 
 */
typedef struct hashMap
{
	hashNode_t ** nodes;
	size_t numNodes, numItems;
} hashMap_t;

/**
 * @brief Data structure that holds and array of hashtable's nodes, uses the constant-key version
 * 
 */
typedef struct hashMapConstKey
{
	hashNodeCK_t ** nodes;
	size_t numNodes, numItems;
} hashMapConstKey_t, hashMapCK_t;

/* *********** Funktsioonide deklaratsioonid *********** */


/**
 * @brief Creates hashtable's node using "key" and value, allocates memory for key and
 * the object itself
 * 
 * @param key Pointer to null-terminated character array that holds "key"
 * @param value Pointer, value, will be copied to data structure
 * @return hashNode_t* Pointer to newly allocated hashNode structure, NULL on failure
 */
hashNode_t * hashNode_make(const char * key, void * value);
/**
 * @brief Frees memory held by hashtable's node, frees memory held by "key"
 * 
 * @param self Pointer to heap-allocated hashNode structure
 */
void hashNode_free(hashNode_t * restrict self);
/**
 * @brief Frees memory held by hashtable's node recursively, first node of nodes' linked list must be given
 * 
 * @param self Pointer to heap-allocated hashNode structure, first node in a hashtable's "slot"
 */
void hashNode_recursiveFree(hashNode_t * restrict self);

/**
 * @brief Creates hashtable's node using "key" and value, allocates memory only for the object itself,
 * key's pointer is just copied
 * 
 * @param key Pointer to null-terminated character array that holds "key", must
 * be "alive" for the entire lifetime of the object with this "key"
 * @param value Pointer, value, will be copied to data structure
 * @return hashNodeCK_t* Pointer to newly allocated hashNodeCK structure, NULL on failure
 */
hashNodeCK_t * hashNodeCK_make(const char * key, void * value);
/**
 * @brief Frees memory held by hashtable's node
 * 
 * @param self Pointer to heap-allocated hashNodeCK structure
 */
void hashNodeCK_free(hashNodeCK_t * restrict self);
/**
 * @brief Frees memory held by hashtable's node recursively, first node of nodes' linked list must be given
 * 
 * @param self Pointer to heap-allocated hashNode structure, first node in a hashtable's "slot"
 */
void hashNodeCK_recursiveFree(hashNodeCK_t * restrict self);



/**
 * @brief Zeros the memory of the hashMap structure
 * 
 * @param self Pointer to the hashMap structure
 */
void hashMap_zero(hashMap_t * restrict self);
/**
 * @brief Initialises hashMap structure with pre-determined hashtable size
 * 
 * @param self Pointer to the hashMap structure
 * @param minSize Minimum desired size of the hashtable in number of slots, this will
 * be calculated to be the next closest prime number
 * @return true Success
 * @return false Failure
 */
bool hashMap_init(hashMap_t * restrict self, size_t minSize);
/**
 * @brief Allocates memory for the hashMap structure on the heap and initialises
 * hashMap structure with pre-determined hashtable size
 * 
 * @param minSize Minimum desired size of the hashtable in number of slots, this will be
 * calculated to be the next closest prime number
 * @return hashMap_t* Pointer to the newly allocated hashMap, NULL on failure
 */
hashMap_t * hashMap_make(size_t minSize);

/**
 * @brief Resizes said hashMap structure's slot-array to have a new minimum desired size
 * Reserves the original hashMap structure in case of failure
 * 
 * @param self Pointer to hashMap structure to be resized
 * @param minSize New minimum desired size, closest larger prime number of it is calculated
 * @return true Success resizing
 * @return false Failure
 */
bool hashMap_resize(hashMap_t * restrict self, size_t minSize);
/**
 * @brief Hashmap's default hash function, is used to calculated the slot index of
 * given key with given hashmap size
 * 
 * @param key Pointer to null-terminated character array of "key"
 * @param mapSize Hashmap's size/number of slots
 * @return size_t Calculated/hashed hashmap slot index in the range 0 ... (mapSize - 1) inclusive
 */
size_t hashMap_hash(const char * key, size_t mapSize);

/**
 * @brief Inserts a new "key" with a pointer-sized value to the hashmap
 * 
 * @param self Pointer to the hashMap structure
 * @param key Pointer to null-terminated character array of data's "key"
 * @param value Pointer-sized "value"
 * @return true Success inserting
 * @return false Failure
 */
bool hashMap_insert(hashMap_t * restrict self, const char * key, void * value);
/**
 * @brief Finds corresponding hashNode object with desired "key" from hashmap
 * 
 * @param self Pointer to hashMap structure
 * @param key Pointer to null-terminated character array of desired "key"
 * @return hashNode_t* Pointer to hashNode object in the hashmap, NULL if an object with that "key" was not found
 */
hashNode_t * hashMap_get(const hashMap_t * restrict self, const char * key);
/**
 * @brief Removes object from hashmap with desired "key"
 * 
 * @param self Pointer to hashMap structure
 * @param key Pointer to null-terminated character array of desired "key"
 * @return void* "value" property previously held by hashNode object in the hashmap.
 * NULL on failure while removing
 */
void * hashMap_remove(hashMap_t * restrict self, const char * key);

/**
 * @brief Destroys the hashMap object
 * 
 * @param self Pointer to hashMap structure
 */
void hashMap_destroy(hashMap_t * restrict self);
/**
 * @brief Destroys the hashMap object, frees memory
 * 
 * @param self Pointer to heap-allocated hashMap structure
 */
void hashMap_free(hashMap_t * restrict self);


/**
 * @brief Zeros the memory of the hashMapCK structure
 * 
 * @param self Pointer to the hashMapCK structure
 */
void hashMapCK_zero(hashMap_t * restrict self);
/**
 * @brief Initialises hashMapCK structure with pre-determined hashtable size
 * 
 * @param self Pointer to the hashMapCK structure
 * @param minSize Minimum desired size of the hashtable in number of slots, this will
 * be calculated to be the next closest prime number
 * @return true Success
 * @return false Failure
 */
bool hashMapCK_init(hashMapCK_t * restrict self, size_t minSize);
/**
 * @brief Allocates memory for the hashMapCK structure on the heap and initialises
 * hashMapCK structure with pre-determined hashtable size
 * 
 * @param minSize Minimum desired size of the hashtable in number of slots, this will be
 * calculated to be the next closest prime number
 * @return hashMapCK_t* Pointer to the newly allocated hashMapCK, NULL on failure
 */
hashMapCK_t * hashMapCK_make(size_t minSize);

/**
 * @brief Resizes said hashMapCK structure's slot-array to have a new minimum desired size
 * Reserves the original hashMapCK structure in case of failure
 * 
 * @param self Pointer to hashMapCK structure to be resized
 * @param minSize New minimum desired size, closest larger prime number of it is calculated
 * @return true Success resizing
 * @return false Failure
 */
bool hashMapCK_resize(hashMapCK_t * restrict self, size_t minSize);

/**
 * @brief Inserts a new "key" with a pointer-sized value to the hashmap
 * 
 * @param self Pointer to the hashMapCK structure
 * @param key Pointer to null-terminated character array of data's "key", key is copied, thus it must be left
 * "alive" for the entire lifetime of this newly inserted object
 * @param value Pointer-sized "value"
 * @return true Success inserting
 * @return false Failure
 */
bool hashMapCK_insert(hashMapCK_t * restrict self, const char * key, void * value);
/**
 * @brief Finds corresponding hashNode object with desired "key" from hashmap
 * 
 * @param self Pointer to hashMapCK structure
 * @param key Pointer to null-terminated character array of desired "key"
 * @return hashNodeCK_t* Pointer to hashNodeCK object in the hashmap, NULL if an object with that "key" was not found
 */
hashNodeCK_t * hashMapCK_get(const hashMapCK_t * restrict self, const char * key);
/**
 * @brief Removes object from hashmap with desired "key"
 * 
 * @param self Pointer to hashMapCK structure
 * @param key Pointer to null-terminated character array of desired "key"
 * @return void* "value" property previously held by hashNodeCK object in the hashmap.
 * NULL on failure while removing
 */
void * hashMapCK_remove(hashMapCK_t * restrict self, const char * key);

/**
 * @brief Destroys the hashMapCK object
 * 
 * @param self Pointer to hashMapCK structure
 */
void hashMapCK_destroy(hashMapCK_t * restrict self);
/**
 * @brief Destroys the hashMapCK object, frees memory
 * 
 * @param self Pointer to heap-allocated hashMapCK structure
 */
void hashMapCK_free(hashMapCK_t * restrict self);


#endif
