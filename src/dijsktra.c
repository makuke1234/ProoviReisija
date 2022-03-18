#include "dijkstra.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

void dijkstra_bSet(uint8_t * bArray, size_t idx, bool value)
{
	const uint8_t sop = idx % 8, shift1 = 0x01 << sop, shift2 = value << sop;
	const size_t idx8 = idx / 8;
	bArray[idx8] = (bArray[idx8] & ~shift1) | shift2;
}
bool dijkstra_bGet(const uint8_t * bArray, size_t idx)
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

uint8_t * dijkstra_createRelations(
	size_t * numRelations,
	const hashMapCK_t * map,
	const line_t ** teed,
	size_t numTeed
)
{
	assert(numRelations != NULL);
	assert(map != NULL);
	assert(teed != NULL);
	assert(numTeed > 0);

	*numRelations = 0;
	for (size_t i = 0; i < map->numItems; ++i)
	{
		const hashNodeCK_t * node = map->nodes[map->numItems - i - 1];
		if (node == NULL)
		{
			continue;
		}
		while (node->next != NULL)
		{
			node = node->next;
		}

		*numRelations = ((point_t *)node->value)->idx + 1;
		break;
	}
	assert(*numRelations > 0);

	size_t memSize = dijkstra_bArrBytes((*numRelations) * (*numRelations));

	uint8_t * relmem = calloc(memSize, 1);
	if (relmem == NULL)
	{
		return NULL;
	}

	// Populate matrix
	for (size_t i = 0; i < numTeed; ++i)
	{
		line_t * tee = teed[i];
		if (tee != NULL)
		{
			size_t i1 = tee->src->idx, i2 = tee->dst->idx;
			dijkstra_bSet(relmem, dijkstra_calcIdx(i1, i2, *numRelations), true);
			dijkstra_bSet(relmem, dijkstra_calcIdx(i2, i1, *numRelations), true);
		}
	}

	return relmem;
}


bool dijkstra_poc(
	prevdist_t ** pprevdist,
	const line_t ** roads,
	size_t numRoads,
	const uint8_t * relations,
	size_t numRelations,
	const point_t * start,
	const point_t * stop
)
{
	assert(roads != NULL);
	assert(numRoads > 0);
	assert(relations != NULL);
	assert(numRelations > 0);
	assert(start != NULL);
	assert(stop != NULL);

	// Priority Q variant:
	/*
		prevdist -> Array of vertices containing distance and previous vertex

		pq -> priority Q

	*/

	prevdist_t * prevdist = malloc(sizeof(prevdist_t) * numRelations);
	if (prevdist == NULL)
	{
		return false;
	}

	pQ_t pq;
	if (!pQ_init(&pq))
	{
		free(prevdist);
		return false;
	}

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

		if (!pQ_addPriority(&pq, i, prevdist[i].dist))
		{
			pQ_destroy(&pq);
			free(prevdist);
			return false;
		}
	}

	
	const point_t ** points = malloc(sizeof(const point_t *) * numRelations);
	if (points == NULL)
	{
		pQ_destroy(&pq);
		free(prevdist);
		return false;
	}

	// Add junctions
	for (size_t i = 0; i < numRoads; ++i)
	{
		if (roads[i] != NULL)
		{
			points[roads[i]->src->idx] = roads[i]->src;
			points[roads[i]->dst->idx] = roads[i]->dst;
		}
	}

	while (!pQ_empty(&pq))
	{
		size_t uIdx = pQ_extractMin(&pq);
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
					pQ_decPriority(&pq, i, alt);
				}
			}
		}
	}

	free(points);
	pQ_destroy(&pq);
	*pprevdist = prevdist;
	
	return true;
}
