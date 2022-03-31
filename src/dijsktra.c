#include "dijkstra.h"
#include "mathHelper.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

void dijkstra_bSet(uint8_t * restrict bArray, size_t idx, bool value)
{
	const uint8_t sop = idx % 8, shift1 = 0x01 << sop, shift2 = value << sop;
	const size_t idx8 = idx / 8;
	bArray[idx8] = (uint8_t)((bArray[idx8] & ~shift1) | shift2);
}
bool dijkstra_bGet(const uint8_t * restrict bArray, size_t idx)
{
	return (bArray[idx / 8] & (0x01 << (idx % 8))) != 0;
}
size_t dijkstra_bArrBytes(size_t numItems)
{
	return (numItems + 7) / 8;
}
size_t dijkstra_calcIdx(size_t row, size_t col, size_t numCols)
{
	return row * numCols + col;
}

bool dijkstra_createRelations(
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

	size_t memSize = dijkstra_bArrBytes((*numRelations) * (*numRelations));

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
			dijkstra_bSet(relmem, dijkstra_calcIdx(i1, i2, *numRelations), true);
			dijkstra_bSet(relmem, dijkstra_calcIdx(i2, i1, *numRelations), true);
			
			points[i1] = tee->src;
			points[i2] = tee->dst;
		}
	}

	*prelations = relmem;
	*ppoints = points;
	return true;
}


bool dijkstra_search_poc(
	prevdist_t * restrict * restrict pprevdist,
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

	prevdist_t * prevdist = malloc(sizeof(prevdist_t) * numRelations);
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
			prevdist[i] = (prevdist_t){
				.dist = INFINITY,
				.prev = NULL
			};
		}
		else
		{
			prevdist[i] = (prevdist_t){
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
		for (size_t i = 0; i < numRelations; ++i)
		{
			// Check if point is neighbour
			if (i != uIdx && dijkstra_bGet(relations, dijkstra_calcIdx(uIdx, i, numRelations)))
			{
				//Calc uvDist
				float dx = points[uIdx]->x - points[i]->x;
				float dy = points[uIdx]->y - points[i]->y;
				float uvDist = (dx * dx) + (dy * dy);
				float alt = prevdist[uIdx].dist + uvDist;
				if (alt < prevdist[i].dist)
				{
					prevdist[i] = (prevdist_t){
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
