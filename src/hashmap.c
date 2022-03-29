#include "hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

hashNode_t * hashNode_make(const char * key, void * value)
{
	assert(key != NULL);
	
	hashNode_t * node = malloc(sizeof(hashNode_t));
	if (node == NULL)
	{
		return NULL;
	}
	node->key   = strdup(key);
	if (node->key == NULL)
	{
		free(node);
		return NULL;
	}
	node->value = value;
	node->next  = NULL;
	return node;
}
void hashNode_free(hashNode_t * restrict self)
{
	assert(self != NULL);

	if (self->key != NULL)
	{
		free(self->key);
	}
	free(self);
}
void hashNode_recursiveFree(hashNode_t * restrict self)
{
	while (self != NULL)
	{
		hashNode_t * restrict next = self->next;
		hashNode_free(self);
		self = next;
	}
}

hashNodeCK_t * hashNodeCK_make(const char * key, void * value)
{
	assert(key != NULL);
	
	hashNodeCK_t * node = malloc(sizeof(hashNodeCK_t));
	if (node == NULL)
	{
		return NULL;
	}
	node->key   = key;
	node->value = value;
	node->next  = NULL;
	return node;
}
void hashNodeCK_free(hashNodeCK_t * restrict self)
{
	assert(self != NULL);

	free(self);
}
void hashNodeCK_recursiveFree(hashNodeCK_t * restrict self)
{
	while (self != NULL)
	{
		hashNodeCK_t * restrict next = self->next;
		hashNodeCK_free(self);
		self = next;
	}
}



static inline bool s_hashMap_isPrime(size_t number)
{
	if (number <= 3)
	{
		return number > 1;
	}
	else
	{
		for (size_t i = 3, n = (size_t)sqrt((double)number) + 1; i < n; i += 2)
		{
			if ((number % i) == 0)
			{
				return false;
			}
		}
		return true;
	}
}
static inline size_t s_hashMap_findNextPrime(size_t lowerBound)
{
	if ((lowerBound > 2) && ((lowerBound % 2) == 0))
	{
		++lowerBound;
	}
	while (!s_hashMap_isPrime(lowerBound))
	{
		lowerBound += 2;
	}
	return lowerBound;
}


void hashMap_zero(hashMap_t * restrict self)
{
	assert(self != NULL);

	*self = (hashMap_t){
		.nodes    = NULL,
		.numNodes = 0,
		.numItems = 0
	};
}
bool hashMap_init(hashMap_t * restrict self, size_t minSize)
{
	assert(self != NULL);
	assert(minSize > 0);

	self->numNodes = s_hashMap_findNextPrime(minSize);
	self->numItems = 0;
	self->nodes    = malloc(self->numNodes * sizeof(hashNode_t *));
	if (self->nodes == NULL)
	{
		return false;
	}
	for (size_t i = 0; i < self->numNodes; ++i)
	{
		self->nodes[i] = NULL;
	}
	return true;
}
hashMap_t * hashMap_make(size_t minSize)
{
	assert(minSize > 0);

	hashMap_t * map = malloc(sizeof(hashMap_t));
	if (map == NULL)
	{
		return NULL;
	}

	if (hashMap_init(map, minSize) == false)
	{
		free(map);
		return NULL;
	}

	return map;
}

bool hashMap_initCopy(hashMap_t * restrict self, size_t minSize, const hashMap_t * restrict other)
{
	assert(self != NULL);
	assert(minSize > 0);
	assert(other != NULL);
	
	if (!hashMap_init(self, minSize))
	{
		return false;
	}

	for (size_t i = 0; i < other->numNodes; ++i)
	{
		hashNode_t * node = other->nodes[i];
		while (node != NULL)
		{
			if (!hashMap_insert(self, node->key, node->value))
			{
				hashMap_destroy(self);
				return false;
			}
			node = node->next;
		}
	}

	return true;
}
hashMap_t * hashMap_makeCopy(size_t minSize, const hashMap_t * restrict other)
{
	assert(minSize > 0);
	assert(other != NULL);

	hashMap_t * mem = malloc(sizeof(hashMap_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!hashMap_initCopy(mem, minSize, other))
	{
		free(mem);
		return NULL;
	}

	return mem;
}

bool hashMap_resize(hashMap_t * restrict self, size_t minSize)
{
	assert(self != NULL);
	assert(minSize > 0);

	hashMap_t newMap;
	if (!hashMap_initCopy(&newMap, minSize, self))
	{
		return false;
	}

	// Destroy old hashmap
	hashMap_destroy(self);
	*self = newMap;
	return true;
}
size_t hashMap_hash(const char * key, size_t mapSize)
{
	assert(key != NULL);
	assert(mapSize > 0);

	size_t hash = 0;
	for (; *key != '\0'; ++key)
	{
		hash = (size_t)(hash * (size_t)37) + (size_t)*key;
	}
	return hash % mapSize;
}

bool hashMap_insert(hashMap_t * restrict self, const char * key, void * value)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	// Kui räsitabel on "küllastunud", proovib mälu juurde allokeerida
	if (self->numItems >= self->numNodes)
	{
		// Tagastusväärtust ei kontrollita, kui see ebaõnnestub, siis säilub esialgne räsitabel, mis hakkab olema "üleküllastunud"
		hashMap_resize(self, (self->numItems + 1) * 2);
	}

	hashNode_t ** pnode = &self->nodes[hashMap_hash(key, self->numNodes)];

	while ((*pnode) != NULL)
	{
		if (strcmp((*pnode)->key, key) == 0)
		{
			return false;
		}
		pnode = &(*pnode)->next;
	}

	hashNode_t * node = hashNode_make(key, value);
	if (node == NULL)
	{
		return false;
	}

	*pnode = node;
	++self->numItems;
	return true;
}
hashNode_t * hashMap_get(const hashMap_t * restrict self, const char * key)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	if (!self->numItems)
	{
		return NULL;
	}

	hashNode_t * node = self->nodes[hashMap_hash(key, self->numNodes)];

	while (node != NULL)
	{
		if (strcmp(node->key, key) == 0)
		{
			return node;
		}
		node = node->next;
	}
	return NULL;
}
void * hashMap_remove(hashMap_t * restrict self, const char * key)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	if (!self->numItems)
	{
		return NULL;
	}

	hashNode_t ** pnode = &self->nodes[hashMap_hash(key, self->numNodes)];
	while ((*pnode) != NULL)
	{
		if (strcmp((*pnode)->key, key) == 0)
		{
			// remove node, return value
			hashNode_t * node = *pnode;
			void * value = node->value;
			*pnode = node->next;

			hashNode_free(node);
			--self->numItems;
			return value;
		}
		pnode = &(*pnode)->next;
	}
	return NULL;
}

void hashMap_destroy(hashMap_t * restrict self)
{
	assert(self != NULL);

	if (self->nodes == NULL)
	{
		return;
	}

	for (size_t i = 0; i < self->numNodes; ++i)
	{
		hashNode_recursiveFree(self->nodes[i]);
	}
	free(self->nodes);
}
void hashMap_free(hashMap_t * restrict self)
{
	assert(self != NULL);

	hashMap_destroy(self);
	free(self);
}


void hashMapCK_zero(hashMapCK_t * restrict self)
{
	assert(self != NULL);

	*self = (hashMapCK_t){
		.nodes    = NULL,
		.numNodes = 0,
		.numItems = 0
	};
}
bool hashMapCK_init(hashMapCK_t * restrict self, size_t minSize)
{
	assert(self != NULL);
	assert(minSize > 0);

	self->numNodes = s_hashMap_findNextPrime(minSize);
	self->numItems = 0;
	self->nodes    = malloc(self->numNodes * sizeof(hashNodeCK_t *));
	if (self->nodes == NULL)
	{
		return false;
	}
	for (size_t i = 0; i < self->numNodes; ++i)
	{
		self->nodes[i] = NULL;
	}
	return true;
}
hashMapCK_t * hashMapCK_make(size_t minSize)
{
	assert(minSize > 0);

	hashMapCK_t * map = malloc(sizeof(hashMapCK_t));
	if (map == NULL)
	{
		return NULL;
	}

	if (hashMapCK_init(map, minSize) == false)
	{
		free(map);
		return NULL;
	}

	return map;
}

bool hashMapCK_initCopy(hashMapCK_t * restrict self, size_t minSize, const hashMapCK_t * restrict other)
{
	assert(self != NULL);
	assert(minSize > 0);
	assert(other != NULL);
	
	if (!hashMapCK_init(self, minSize))
	{
		return false;
	}

	for (size_t i = 0; i < other->numNodes; ++i)
	{
		hashNodeCK_t * node = other->nodes[i];
		while (node != NULL)
		{
			if (!hashMapCK_insert(self, node->key, node->value))
			{
				hashMapCK_destroy(self);
				return false;
			}
			node = node->next;
		}
	}

	return true;
}
hashMapCK_t * hashMapCK_makeCopy(size_t minSize, const hashMapCK_t * restrict other)
{
	assert(minSize > 0);
	assert(other != NULL);

	hashMapCK_t * mem = malloc(sizeof(hashMapCK_t));
	if (mem == NULL)
	{
		return NULL;
	}
	else if (!hashMapCK_initCopy(mem, minSize, other))
	{
		free(mem);
		return NULL;
	}

	return mem;
}

bool hashMapCK_resize(hashMapCK_t * restrict self, size_t minSize)
{
	assert(self != NULL);
	assert(minSize > 0);

	hashMapCK_t newMap;
	if (!hashMapCK_initCopy(&newMap, minSize, self))
	{
		return false;
	}

	// Destroy old hashmap
	hashMapCK_destroy(self);
	*self = newMap;
	return true;
}

bool hashMapCK_insert(hashMapCK_t * restrict self, const char * key, void * value)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	// Kui räsitabel on "küllastunud", proovib mälu juurde allokeerida
	if (self->numItems >= self->numNodes)
	{
		// Tagastusväärtust ei kontrollita, kui see ebaõnnestub, siis säilub esialgne räsitabel, mis hakkab olema "üleküllastunud"
		hashMapCK_resize(self, (self->numItems + 1) * 2);
	}

	hashNodeCK_t ** pnode = &self->nodes[hashMap_hash(key, self->numNodes)];

	while ((*pnode) != NULL)
	{
		if (strcmp((*pnode)->key, key) == 0)
		{
			return false;
		}
		pnode = &(*pnode)->next;
	}

	hashNodeCK_t * node = hashNodeCK_make(key, value);
	if (node == NULL)
	{
		return false;
	}

	*pnode = node;
	++self->numItems;

	// Prindib sisu
	printf("hashmap contents:\n");
	for (size_t i = 0; i < self->numNodes; ++i)
	{
		hashNodeCK_t * node = self->nodes[i];
		while (node != NULL)
		{
			printf("node %s: %p\n", node->key, node->value);

			node = node->next;
		}
	}
	printf("\n");

	return true;
}
hashNodeCK_t * hashMapCK_get(const hashMapCK_t * restrict self, const char * key)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	if (!self->numItems)
	{
		return NULL;
	}

	hashNodeCK_t * node = self->nodes[hashMap_hash(key, self->numNodes)];

	while (node != NULL)
	{
		if (strcmp(node->key, key) == 0)
		{
			return node;
		}
		node = node->next;
	}
	return NULL;
}
void * hashMapCK_remove(hashMapCK_t * restrict self, const char * key)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	if (!self->numItems)
	{
		return NULL;
	}

	hashNodeCK_t ** pnode = &self->nodes[hashMap_hash(key, self->numNodes)];
	while ((*pnode) != NULL)
	{
		if (strcmp((*pnode)->key, key) == 0)
		{
			// remove node, return value
			hashNodeCK_t * node = *pnode;
			void * value = node->value;
			*pnode = node->next;

			hashNodeCK_free(node);
			--self->numItems;
			return value;
		}
		pnode = &(*pnode)->next;
	}
	return NULL;
}

void hashMapCK_destroy(hashMapCK_t * restrict self)
{
	assert(self != NULL);

	if (self->nodes == NULL)
	{
		return;
	}

	for (size_t i = 0; i < self->numNodes; ++i)
	{
		hashNodeCK_recursiveFree(self->nodes[i]);
	}
	free(self->nodes);
}
void hashMapCK_free(hashMapCK_t * restrict self)
{
	assert(self != NULL);

	hashMapCK_destroy(self);
	free(self);
}

