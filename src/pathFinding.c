#include "pathFinding.h"
#include "mathHelper.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

void pf_bSet(uint8_t * restrict bArray, size_t idx, bool value)
{
	const uint8_t sop = idx % 8, shift1 = 0x01 << sop, shift2 = value << sop;
	const size_t idx8 = idx / 8;
	bArray[idx8] = (uint8_t)((bArray[idx8] & ~shift1) | shift2);
}
bool pf_bGet(const uint8_t * restrict bArray, size_t idx)
{
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
	assert(*numRelations > 0);

	size_t memSize = pf_bArrBytes((*numRelations) * (*numRelations));

	uint8_t * relmem = calloc(memSize, 1);
	if (relmem == NULL)
	{
		return false;
	}
	const point_t ** points = malloc(sizeof(const point_t *) * (*numRelations));
	if (points == NULL)
	{
		free(relmem);
		return false;
	}

	// Populate matrix
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
	assert(numRelations > 0);
	assert(start != NULL);

	prevDist_t * prevdist = (*pprevdist != NULL) ? *pprevdist : malloc(sizeof(prevDist_t) * numRelations);
	if (prevdist == NULL)
	{
		return false;
	}

	fibHeap_t pq;
	pq_init(&pq);

	// Initialise previous distance
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

		if (!pq_pushWithPriority(&pq, i, prevdist[i].dist))
		{
			pq_destroy(&pq);
			free(prevdist);
			return false;
		}
	}

	while (!pq_empty(&pq))
	{
		size_t uIdx = pq_extractMin(&pq);
		assert(uIdx != SIZE_MAX);
		for (size_t i = 0; i < numRelations; ++i)
		{
			// Check if point is neighbour
			if ((i != uIdx) && pf_bGet(relations, pf_calcIdx(uIdx, i, numRelations)))
			{
				//Calc uvDist
				float dx = points[uIdx]->x - points[i]->x;
				float dy = points[uIdx]->y - points[i]->y;
				float uvDist = sqrtf((dx * dx) + (dy * dy));
				float alt = prevdist[uIdx].dist + uvDist;
				if (alt < prevdist[i].dist)
				{
					prevdist[i] = (prevDist_t){
						.dist = alt,
						.prev = points[uIdx]
					};
					pq_decPriority(&pq, i, alt);
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

	float * matrix = calloc(numPoints * numPoints, sizeof(float));
	if (matrix == NULL)
	{
		hashMapCK_destroy(&pmap);
		return false;
	}

	// Teeb relatsioonide maatriksi kõigepealt
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
	for (size_t i = 0; i < (numPoints - 1); ++i)
	{
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

		// Täidab maatriksit
		for (size_t j = 0; j < numRelations; ++j)
		{
			hashNodeCK_t * node = hashMapCK_get(&pmap, points[j]->id.str);
			const point_t ** ppoint = (node != NULL) ? node->value : NULL;
			if ((ppoint != NULL) && (points[j] == *ppoint))
			{
				// Punkt on algus/lõpp-punkt
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
