#include "pathFinding.h"
#include "mathHelper.h"
#include "logger.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

void pf_bSet(uint8_t * restrict bArray, size_t idx, bool value)
{
	assert(bArray != NULL);
	const uint8_t sop = idx % 8, shift1 = 0x01 << sop, shift2 = value << sop;
	const size_t idx8 = idx / 8;
	bArray[idx8] = (uint8_t)((bArray[idx8] & ~shift1) | shift2);
}
bool pf_bGet(const uint8_t * restrict bArray, size_t idx)
{
	assert(bArray != NULL);
	return (bArray[idx / 8] & (0x01 << (idx % 8))) != 0;
}
size_t pf_bArrBytes(size_t numItems)
{
	return (numItems + 7) / 8;
}
size_t pf_calcIdx(size_t row, size_t col, size_t numCols)
{
	return row * numCols + col;
}

bool pf_createRelations(
	uint8_t ** restrict prelations,
	size_t * restrict numRelations,
	const point_t *** restrict ppoints,
	const line_t * const restrict * restrict teed,
	size_t numTeed
)
{
	assert(prelations != NULL);
	assert(numRelations != NULL);
	assert(ppoints != NULL);
	assert(teed != NULL);
	assert(numTeed > 0);

	// Leiab ristmike koguarvu
	*numRelations = 0;
	for (size_t i = 0; i < numTeed; ++i)
	{
		const line_t * tee = teed[i];
		if (tee != NULL)
		{
			size_t newRel = mh_zmax(tee->src->idx, tee->dst->idx) + 1;
			*numRelations = mh_zmax(*numRelations, newRel);
		}
	}
	assert(*numRelations >= 2);

	// Teeb 1D-massiivi, mis hoiab 2D-maatriksit relatsioonidest, täidab nullidega
	size_t memSize = pf_bArrBytes((*numRelations) * (*numRelations));
	uint8_t * relmem = calloc(memSize, 1);
	if (relmem == NULL)
	{
		return false;
	}

	// Teeb ristmike pointerite massiivi
	const point_t ** points = malloc(sizeof(const point_t *) * (*numRelations));
	if (points == NULL)
	{
		free(relmem);
		return false;
	}

	// Täidab maatriksi ja ristmike pointerite massiivi
	for (size_t i = 0; i < numTeed; ++i)
	{
		const line_t * tee = teed[i];
		if (tee != NULL)
		{
			const size_t i1 = tee->src->idx, i2 = tee->dst->idx;
			pf_bSet(relmem, pf_calcIdx(i1, i2, *numRelations), true);
			pf_bSet(relmem, pf_calcIdx(i2, i1, *numRelations), true);
			
			points[i1] = tee->src;
			points[i2] = tee->dst;
		}
	}

	*prelations = relmem;
	*ppoints = points;
	return true;
}


bool pf_dijkstraSearch(
	prevDist_t * restrict * restrict pprevdist,
	const point_t ** restrict points,
	const uint8_t * restrict relations,
	size_t numRelations,
	const point_t * restrict start
)
{
	assert(pprevdist != NULL);
	assert(points != NULL);
	assert(relations != NULL);
	assert(numRelations >= 2);
	assert(start != NULL);

	// Kui kasutaja ei andust prevDist massiivi, siis allokeerib selle jaoks mälu
	prevDist_t * prevdist = (*pprevdist != NULL) ? *pprevdist : malloc(sizeof(prevDist_t) * numRelations);
	if (prevdist == NULL)
	{
		return false;
	}

	// Initsialiseerib Fibonacci kuhja/hunniku
	fibHeap_t pq;
	pq_init(&pq);

	// prevdist inistialiseeritakse, kõikidesse punktidesse on alguspunktis teepikkus esialgu lõpmata suur
	for (size_t i = 0; i < numRelations; ++i)
	{
		if (i != start->idx)
		{
			prevdist[i] = (prevDist_t){
				.dist = INFINITY,
				.prev = NULL
			};
		}
		else
		{
			prevdist[i] = (prevDist_t){
				.dist = 0.0f,
				.prev = NULL
			};
		}

		// Kui kuhja lisamine ebaõnnestub, siis "hävitab" kuhja ning tagastab 0
		if (!pq_pushWithPriority(&pq, i, prevdist[i].dist))
		{
			pq_destroy(&pq);
			free(prevdist);
			return false;
		}
	}

	// Teeb senikaua kuni puu ei ole tühi
	// Pseudokood: https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm#Using_a_priority_queue
	while (!pq_empty(&pq))
	{
		// Eemaldab lühima teepikkuse
		size_t uIdx = pq_extractMin(&pq);
		// Kontrollib igaks juhuks kas õnnestus DEBUG režiimis
		assert(uIdx != SIZE_MAX);

		writeLogger("Extracted minimum: %s; %.3f", points[uIdx]->id.str, (double)prevdist[uIdx].dist);

		// Käib läbi kõik punkti naabrid
		for (size_t vIdx = 0; vIdx < pq.n_lut; ++vIdx)
		{
			// Check if point is neighbour
			if ((pq.lut[vIdx] != NULL) && pf_bGet(relations, pf_calcIdx(uIdx, vIdx, numRelations)))
			{
				const float dx = points[uIdx]->x - points[vIdx]->x;
				const float dy = points[uIdx]->y - points[vIdx]->y;
				const float alt = prevdist[uIdx].dist + sqrtf((dx * dx) + (dy * dy));
				// Check if new distance is smaller than previous best
				if (alt < prevdist[vIdx].dist)
				{
					writeLogger("New minimum for %s is %.3f, old: %.3f", points[vIdx]->id.str, (double)alt, (double)prevdist[vIdx].dist);

					// Re-initialize with new distance
					prevdist[vIdx] = (prevDist_t){
						.dist = alt,
						.prev = points[uIdx]
					};
					// Decrease priority in Fibonacci heap
					pq_decPriority(&pq, vIdx, alt);
				}
			}
		}
	}

	pq_destroy(&pq);
	*pprevdist = prevdist;

	return true;
}

bool pf_makeDistMatrix(
	const point_t * const restrict * restrict startpoints,
	float * restrict * restrict pmatrix,
	size_t numPoints,
	const line_t * const restrict * restrict teed,
	size_t numTeed
)
{
	assert(startpoints != NULL);
	assert(pmatrix != NULL);
	assert(numPoints >= 2);
	assert(teed != NULL);
	assert(numTeed > 0);

	// Teeb hõlpsaks leidmiseks räsitabeli peatuspunktidest
	hashMapCK_t pmap;
	if (!hashMapCK_init(&pmap, numPoints))
	{
		return false;
	}

	// Teeb algus-punktide räsitabeli
	for (size_t i = 0; i < numPoints; ++i)
	{
		if (!hashMapCK_insert(&pmap, startpoints[i]->id.str, (void *)&startpoints[i]))
		{
			hashMapCK_destroy(&pmap);
			return false;
		}
	}

	// Teeb 1D-allokeeritud 2D-maatriksi 
	float * matrix = calloc(numPoints * numPoints, sizeof(float));
	if (matrix == NULL)
	{
		hashMapCK_destroy(&pmap);
		return false;
	}

	// Teeb relatsioonide maatriksi Dijkstra algoritmi jaoks
	size_t numRelations;
	uint8_t * relations = NULL;
	const point_t ** points = NULL;
	bool result = pf_createRelations(
		&relations,
		&numRelations,
		&points,
		teed,
		numTeed
	);
	if (!result)
	{
		hashMapCK_destroy(&pmap);
		free(matrix);
		return false;
	}

	prevDist_t * distances = NULL;

	// Viimast punkti ei pea läbi käima, sest kõikide eelnevate punktidega saab maatriksi täidetud
	for (size_t i = 0, n_1 = numPoints - 1; i < n_1; ++i)
	{
		// Leiab lühimad teed kõikidesse punktidesse konkreetsest alguspunktist
		result = pf_dijkstraSearch(
			&distances,
			points,
			relations,
			numRelations,
			startpoints[i]
		);
		if (!result)
		{
			hashMapCK_destroy(&pmap);
			free(matrix);
			free(points);
			free(relations);
			return false;
		}
		
		#if LOGGING_ENABLE == 1

		// Log the dijkstra's path
		writeLogger("Starting point: %s", startpoints[i]->id.str);
		for (size_t j = 0; j < numRelations; ++j)
		{
			writeLogger("%s -> %s: %.3f", distances[j].prev == NULL ? startpoints[i]->id.str : distances[j].prev->id.str, points[j]->id.str, (double)distances[j].dist);
		}

		#endif

		// Täidab maatriksit lühimate teedega, arvestab ainult soovitud peatuspunkte
		for (size_t j = 0; j < numRelations; ++j)
		{
			// Kontrollib kas punkt on peatuspunkt või mitte
			hashNodeCK_t * node = hashMapCK_get(&pmap, points[j]->id.str);
			const point_t ** ppoint = (node != NULL) ? node->value : NULL;
			if ((ppoint != NULL) && (points[j] == *ppoint))
			{
				// Arvutab punkti indeksi peatuste seas
				const size_t idx = (size_t)(ppoint - startpoints);
				const float dist = distances[j].dist;
				matrix[i   * numPoints + idx] = dist;
				matrix[idx * numPoints + i  ] = dist;
			}
		}
	}

	hashMapCK_destroy(&pmap);
	free(points);
	free(distances);
	free(relations);

	*pmatrix = matrix;
	return true;
}

typedef struct pf_qnode_impl
{
	size_t val;

	struct pf_qnode_impl * prev, * next;
} pf_qnode_implS;

bool pf_qnode_push_impl(pf_qnode_implS ** restrict pq, size_t val)
{
	assert(pq != NULL);

	pf_qnode_implS * n;
	if (*pq == NULL)
	{
		// Kui järjekorda ei eksisteeri, siis tekitab selle
		*pq = malloc(sizeof(pf_qnode_implS));
		n = *pq;
		if (n == NULL)
		{
			return false;
		}

		n->prev = n;
		n->next = n;
	}
	else
	{
		// Lisab uue punkti järjekorra lõppu
		n = malloc(sizeof(pf_qnode_implS));
		if (n == NULL)
		{
			return false;
		}

		n->prev = (*pq)->prev;
		n->next = *pq;
		(*pq)->prev->next = n;
		(*pq)->prev = n;
	}

	// Kirjutab soovitud väärtuse järjekorra uude elementi
	n->val = val;

	return true;
}
void pf_qnode_free_impl(pf_qnode_implS * restrict q)
{
	assert(q != NULL);

	// "Kustutab" järjekorra elemente senikaua kuni ei ole uuesti algusesse jõudnud
	pf_qnode_implS * n = q;
	do
	{
		pf_qnode_implS * next = n->next;
		free(n);
		n = next;
	} while (n != q);
}

typedef struct
{
	size_t n;
	size_t * arr;
	pf_qnode_implS * q;

} pf_perm_implS;

typedef struct
{
	const float * mtx;
	float lowest;

	size_t n;
	size_t startIdx, stopIdx;
	size_t * arr;
	size_t * best;

	pf_perm_implS perm;

} pf_fomo_implS;


static inline float pf_fomo_dist_impl(pf_fomo_implS * restrict arg)
{
	// Leiab antud punktide järjestusega teekonna pikkuse
	float dist = 0.0f;

	for (size_t i = 0, n_1 = arg->n - 1; i < n_1; ++i)
	{
		dist += arg->mtx[pf_calcIdx(arg->arr[i], arg->arr[i + 1], arg->n)];
	}

	return dist;
}

static inline void pf_fomo_iter_impl(pf_fomo_implS * restrict arg, size_t sz)
{	
	// Kui üks "rida" on täidetud, siis kontrollib, äkki on leitud lühem punktide läbimisjärjekord
	if (sz == 0)
	{
		float dist = pf_fomo_dist_impl(arg);
		// Kui praegu leitud läbimisjärjekord on parem eelnevatest, siis uuendab hetke-parimat
		if (dist < arg->lowest)
		{
			arg->lowest = dist;
			memcpy(&arg->best[1], &arg->arr[1], sizeof(size_t) * arg->perm.n);
		}
		
		// Teeb rekursiooni "katki"
		return;
	}

	// "Keerutab" läbi kõikide permutatsioonide
	for (size_t i = 0; i < sz; ++i)
	{
		// Jätab praeguse järjekorra "vana" alguse meelde
		pf_qnode_implS * oldprev = arg->perm.q;
		// Eemaldab esimese elemendi järjekorrast
		arg->perm.arr[arg->perm.n - sz] = oldprev->val;
		arg->perm.q = oldprev->next;
		oldprev->prev->next = arg->perm.q;
		arg->perm.q->prev = oldprev->prev;

		// Proovib omakorda kõik permutatsioonid järgijäävate indeksitega läbi
		// Rekursioon siin mälu-probleeme ei tekita, sest suure keerukuse tõttu jõuab arvuti
		// max. 20-sügavust rekursiooni läbi teha
		pf_fomo_iter_impl(arg, sz - 1);

		// Lisab eelnevalt eemaldatud esimese elemendi järjekorra lõppu
		oldprev->prev->next = oldprev;
		arg->perm.q->prev = oldprev;
	}
}

bool pf_findOptimalMatrixOrder(
	const float * restrict matrix,
	size_t numPoints,
	size_t startIdx,
	size_t stopIdx,
	size_t ** restrict poutIndexes
)
{
	assert(matrix != NULL);
	assert(numPoints >= 2);
	assert(startIdx < numPoints);
	assert(stopIdx < numPoints);
	assert(poutIndexes != NULL);
	
	// Initsialiseerib andmestruktuuri permutatsioonide läbiproovimiseks
	pf_fomo_implS arg = {
		.mtx      = matrix,
		.lowest   = (float)INFINITY,
		.n        = numPoints,
		.startIdx = startIdx,
		.stopIdx  = stopIdx,
		.arr      = malloc(sizeof(size_t) * numPoints),
		.best     = malloc(sizeof(size_t) * numPoints),
		.perm     = {
			.n   = numPoints - 2,
			.arr = &arg.arr[1],
			.q   = NULL
		}
	};
	// Kontrollib mälu allokeerimise õnnestumist
	if ((arg.arr == NULL) || (arg.best == NULL))
	{
		if (arg.arr != NULL)
		{
			free(arg.arr);
		}
		else if (arg.best != NULL)
		{
			free(arg.best);
		}
		return false;
	}
	// Algus- ja lõpp-punkt pannakse paika
	arg.best[0]             = arg.arr[0]             = startIdx;
	arg.best[numPoints - 1] = arg.arr[numPoints - 1] = stopIdx;

	// Täidetakse järjekorra andmestruktuur järjest kõikide punktide indeksitega, mis
	// ei ole algus- ega lõpp-punkti omad, sest need jäävad alati paika
	for (size_t i = 0, j = 1; j < (numPoints - 1); ++i)
	{
		if ((i != stopIdx) && (i != startIdx))
		{
			++j;
			if (pf_qnode_push_impl(&arg.perm.q, i) == false)
			{
				pf_qnode_free_impl(arg.perm.q);
				return false;
			}
		}
	}

	// Proovib kõik permutatsioonid läbi, et leida lühim peatuste läbimise järjekord
	pf_fomo_iter_impl(&arg, arg.perm.n);

	// Ressursid vabastatakse
	free(arg.arr);
	pf_qnode_free_impl(arg.perm.q);

	// Parim järjekord tagastatakse
	*poutIndexes = arg.best;
	return true;
}
