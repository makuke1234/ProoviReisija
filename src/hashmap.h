#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>


typedef struct hashNode
{
	char * key;
	void * value;

	struct hashNode * next;
} hashNode_t;

typedef struct hashNodeConstKey
{
	const char * key;
	void * value;

	struct hashNodeConstKey * next;
} hashNodeConstKey_t, hashNodeCK_t;



typedef struct hashMap
{
	hashNode_t ** nodes;
	size_t numNodes, numItems;
} hashMap_t;

typedef struct hashMapConstKey
{
	hashNodeCK_t ** nodes;
	size_t numNodes, numItems;
} hashMapConstKey_t, hashMapCK_t;

/* *********** Funktsioonide deklaratsioonid *********** */


hashNode_t * hashNode_make(const char * key, void * value);
void hashNode_free(hashNode_t * restrict self);
void hashNode_recursiveFree(hashNode_t * restrict self);

hashNodeCK_t * hashNodeCK_make(const char * key, void * value);
void hashNodeCK_free(hashNodeCK_t * restrict self);
void hashNodeCK_recursiveFree(hashNodeCK_t * restrict self);



bool hashMap_init(hashMap_t * restrict self, size_t minSize);
hashMap_t * hashMap_make(size_t minSize);

bool hashMap_resize(hashMap_t * restrict self, size_t minSize);
size_t hashMap_hash(const char * key, size_t mapSize);

bool hashMap_insert(hashMap_t * restrict self, const char * key, void * value);
hashNode_t * hashMap_get(const hashMap_t * restrict self, const char * key);
void * hashMap_remove(hashMap_t * restrict self, const char * key);

void hashMap_destroy(hashMap_t * restrict self);
void hashMap_free(hashMap_t * restrict self);


bool hashMapCK_init(hashMapCK_t * restrict self, size_t minSize);
hashMapCK_t * hashMapCK_make(size_t minSize);

bool hashMapCK_resize(hashMapCK_t * restrict self, size_t minSize);

bool hashMapCK_insert(hashMapCK_t * restrict self, const char * key, void * value);
hashNodeCK_t * hashMapCK_get(const hashMapCK_t * restrict self, const char * key);
void * hashMapCK_remove(hashMapCK_t * restrict self, const char * key);

void hashMapCK_destroy(hashMapCK_t * restrict self);
void hashMapCK_free(hashMapCK_t * restrict self);


#endif
