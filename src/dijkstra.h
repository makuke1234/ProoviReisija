#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "dataModel.h"
#include "priorityQ.h"

void dijkstra_bSet(uint8_t * restrict bArray, size_t idx, bool value);
bool dijkstra_bGet(const uint8_t * restrict bArray, size_t idx);
size_t dijkstra_bArrBytes(size_t numItems);
size_t dijkstra_calcIdx(size_t row, size_t col, size_t numCols);

bool dijkstra_createRelations(
	uint8_t ** restrict prelations,
	size_t * restrict numRelations,
	const point_t *** restrict ppoints,
	const line_t * const restrict * restrict teed,
	size_t numTeed
);

typedef struct prevdist
{
	float dist;
	const point_t * prev;

} prevdist_t;

bool dijkstra_search_poc(
	prevdist_t * restrict * restrict pprevdist,
	const point_t ** restrict points,
	const uint8_t * restrict relations,
	size_t numRelations,
	const point_t * restrict start
);

#endif
