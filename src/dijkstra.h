#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "dataModel.h"
#include "priorityQ.h"

void dijkstra_bSet(uint8_t * bArray, size_t idx, bool value);
bool dijkstra_bGet(const uint8_t * bArray, size_t idx);
size_t dijkstra_bArrBytes(size_t numItems);
size_t dijkstra_calcIdx(size_t row, size_t col, size_t numCols);

uint8_t * dijkstra_createRelations(
	size_t * numRelations,
	const hashMapCK_t * map,
	const line_t ** teed,
	size_t numTeed
);

typedef struct prevdist
{
	float dist;
	const point_t * prev;

} prevdist_t;

bool dijkstra_poc(
	prevdist_t ** pprevdist,
	const line_t ** roads,
	size_t numRoads,
	const uint8_t * relations,
	size_t numRelations,
	const point_t * start,
	const point_t * stop
);

#endif
