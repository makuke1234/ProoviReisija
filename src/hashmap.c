#include "hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

hashNode_t * hashNode_make(const char * key, void * value)
{
	assert(key != NULL);
	
	// Allokeeritakse mälu
	hashNode_t * node = malloc(sizeof(hashNode_t));
	if (node == NULL)
	{
		return NULL;
	}
	// Etteantud võti duplikeeritakse dünaamilisse mällu
	node->key   = strdup(key);
	// Kui mälu on otsas, siis vabastatakse mälu
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

	// Võimalusel vabastatakse ka võti
	if (self->key != NULL)
	{
		free(self->key);
	}
	free(self);
}
void hashNode_recursiveFree(hashNode_t * restrict self)
{
	// Harusid vabastatakse seni kuni jõutakse lingitud listi lõppu, mida tähistab mäluaadress NULL
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


/**
 * @brief Evaluates, if given number is prime
 * 
 * @param number Number to be evaluated
 * @return true Number is prime
 * @return false Isn't prime
 */
static inline bool s_hashMap_isPrime(size_t number)
{
	// Kui arv on väiksem-võrdne 3-st, siis sobivad ainult 2 ja 3 ehk 1-st rangelt suuremad arvud
	if (number <= 3)
	{
		return number > 1;
	}
	else
	{
		// Algarvude jaguvuse leidmisel on mõistlik tuletada meelde asjaolu, et jagajad ja võimalikud jagatised peavad hakkama korduma,
		// kui jagaja on suurem arvu ruutjuure väärtusest. Samuti on oluline teada, et ükski arv ei saa jaguda arvuga,
		// mis on suurem kui arv/2 välja arvatud arv ise.
		/*
		 * Näide arvuga 15, ruutjuur 15-st ülespoole ümardatuna on 4:
		 * 15/1 = 15
		 * 15/2 -> ei jagu
		 * 15/3 = 5
		 * 15/4 = ei jagu
		 * 15/5 = 3 -> juba olnud
		 * 15/6 -> ei jagu
		 * 15/7 -> ei jagu
		 * ... -> ei jagu
		 * 15/15 = 1 -> juba olnud
		 */
		// Kuna paaritud arvud saavad ainult paaritute arvudega jaguda, siis liigutakse edasi ainult üle paaritute arvude
		for (size_t i = 3, n = (size_t)sqrt((double)number) + 1; i < n; i += 2)
		{
			// Kui jagamise jääk on 0, siis järelikult jagub ning järelikult ei ole algarv
			if ((number % i) == 0)
			{
				return false;
			}
		}

		// Kui kood on siia jõudnud, siis peab olema tegu algarvuga
		return true;
	}
}
/**
 * @brief Finds the next closest larger or equal prime number in respect to the lower bound
 * 
 * @param lowerBound Lower bound from which to search prime numbers
 * @return size_t Found prime number
 */
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

	// Räsitabelitel on oluline, et "ämbrite/bucketite" arv oleks algarv, see vähendab oluliselt
	// konfliktide teket räsi arvutamisel (sest tuleb leida jagamise jääk ämbrite arvuga jagamisel)
	// ning suurendab seega jõudlust nii elementide lisamisel, leidmisel kui ka eemaldamisel.
	self->numNodes = s_hashMap_findNextPrime(minSize);
	self->numItems = 0;
	// Allokeeritakse vajalik mälu, kontrollitakse õnnestumist
	self->nodes    = malloc(self->numNodes * sizeof(hashNode_t *));
	if (self->nodes == NULL)
	{
		return false;
	}
	// Allokeeritud "ämbrid" nullitakse ära, on oluline märkida, et memset(self->nodes, 0, sizeof(hashNode_t *) * self->numNodes);
	// põhjustaks defineerimata käitumise, sest NULL ei ole garanteeritult iga kompilaatori/OS-ga konstant 0
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

	// Elemendid kopeeritakse ühest räsitabelist teise
	for (size_t i = 0; i < other->numNodes; ++i)
	{
		hashNode_t * node = other->nodes[i];
		while (node != NULL)
		{
			if (!hashMap_insert(self, node->key, node->value))
			{
				// Kui tabelist "other" koopia tegemine ebaõnnestub, siis on garanteeriutd, et see jääb puutumata
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
		// Kui koopia tegemine ebaõnnestus, siis vabastatakse allokeeritud mälu
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
		// Kui suuruse muutmine ebaõnnestub, siis on garantii, et andmed ei lähe kaotsi
		return false;
	}

	// Vana räsitabel hävitatakse
	hashMap_destroy(self);
	// Praegune räsitabel pannakse uuega võrduma
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
		// Räsi arvutamisel eelistan kasutatada jällegi algarve, sest see tagab väiksema
		// tõenäosuse konfliktide tekkeks räsitabelisse elementide paigutamisel
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
		// Mõne aja pärast hakkab see muidugi tõsiseid probleeme tekitama, kuid see, et süsteemil pole piisavalt mälu
		// tabeli suurendamiseks on juba tõsine probleem, seega pole mõtet jõudluse pärast muretseda
		hashMap_resize(self, (self->numItems + 1) * 2);
	}

	// Esmalt leiab räsi põhjal sobiva "bucketi" aadressi
	hashNode_t ** pnode = &self->nodes[hashMap_hash(key, self->numNodes)];

	// Kui sellele "bucketi" aadressile on lingitud loend juba tehtud, siis minnakse lingitud loendi lõppu
	// Kui selgub, et sellise "võtmega" element juba leidub räsitabelis, siis tagastakse viga näitav 'false'
	while ((*pnode) != NULL)
	{
		if (strcmp((*pnode)->key, key) == 0)
		{
			return false;
		}
		pnode = &(*pnode)->next;
	}

	// Tehakse uus liige lingitud loendisse
	hashNode_t * node = hashNode_make(key, value);
	if (node == NULL)
	{
		return false;
	}

	*pnode = node;

	// Suurendatakse informatiivset elementide arvu räsitabelis
	++self->numItems;
	return true;
}
hashNode_t * hashMap_get(const hashMap_t * restrict self, const char * key)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);
	
	// Kui räsitabelis elemente pole, siis pole mõtet otsidagi ning tagastatakse viga näitav NULL
	if (!self->numItems)
	{
		return NULL;
	}

	// Leitakse "võtme" räsi põhjal eeldatav "bucket"
	hashNode_t * node = self->nodes[hashMap_hash(key, self->numNodes)];

	// Otsitakse lingitud loendist sobiva "võtmega" elementi
	while (node != NULL)
	{
		// "Võti" sobib, s.o erinevus nende vahel on 0
		if (strcmp(node->key, key) == 0)
		{
			// Tagastakse dünaamiliselt allokeeritud element, s.o andmestruktuuri aadress
			return node;
		}
		node = node->next;
	}
	// Kui sellist ei leitud, siis järelikult pole sellist elementi räsitabelis
	return NULL;
}
void * hashMap_remove(hashMap_t * restrict self, const char * key)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	// Kui räsitabelis elemente ei ole, siis pole mõtet ka midagi proovida eemaldada
	if (!self->numItems)
	{
		return NULL;
	}

	// Leitakse etteantud "võtme" põhjal tema räsile vastav "bucketi" aadress
	hashNode_t ** pnode = &self->nodes[hashMap_hash(key, self->numNodes)];
	// Otsitakse üles vastav võti
	while ((*pnode) != NULL)
	{
		// Võti leiti
		if (strcmp((*pnode)->key, key) == 0)
		{
			hashNode_t * node = *pnode;
			// Eemaldatud elemendi väärtus talletatakse
			void * value = node->value;
			// Praeguse õige võtmega elementi tähistava muutuja aadressile kirjutatakse hoopis järgmise elemendi aadress
			// Sellega eemaldatakse hõlpsasti lingitud loendist element
			*pnode = node->next;

			// Eemaldatud elemendi dünaamiliselt allokeeritud mälu vabastakse
			hashNode_free(node);

			// Vähendatakse informatiivset elementide arvu räsitabelis
			--self->numItems;

			// Tagastakse väärtuse väljal olnud mäluaadressi-suurune väärtus
			// Olgu märgitud, et väärtuse väljal võib olla ka kasutaja poolt sisestatud NULL, sel juhul peab kasutaja
			// ise vahet tegema, kas tegu oli veaga või mitte
			return value;
		}
		// Vastasel juhul liigutakse järgmise elemendi aadressi juurde
		pnode = &(*pnode)->next;
	}

	// Vastavat võtit ei leitudki :( ning tagastatakse viga näitav NULL
	return NULL;
}

void hashMap_destroy(hashMap_t * restrict self)
{
	assert(self != NULL);

	// Kui "bucketid" on juba vabastatud, siis pole enam midagi vabastada :/
	if (self->nodes == NULL)
	{
		return;
	}

	// Kõik "bucketites" olevate lingitud lonedite mälu vabastatakse ükshaaval
	for (size_t i = 0; i < self->numNodes; ++i)
	{
		hashNode_recursiveFree(self->nodes[i]);
	}

	// Lõpuks vabastakse ka "bucketite" massiivi mälu
	free(self->nodes);
}
void hashMap_free(hashMap_t * restrict self)
{
	assert(self != NULL);

	hashMap_destroy(self);
	// Lisanduvalt vabastatakse ka räsitabeli andmestruktuuri enda dünaamiliselt allokeeritud mälu
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

