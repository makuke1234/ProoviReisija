#ifndef PATH_FINDING_H
#define PATH_FINDING_H

#include "dataModel.h"
#include "priorityQ.h"

void pf_bSet(uint8_t * restrict bArray, size_t idx, bool value);
bool pf_bGet(const uint8_t * restrict bArray, size_t idx);
size_t pf_bArrBytes(size_t numItems);
size_t pf_calcIdx(size_t row, size_t col, size_t numCols);

bool pf_createRelations(
	uint8_t ** restrict prelations,
	size_t * restrict numRelations,
	const point_t *** restrict ppoints,
	const line_t * const restrict * restrict teed,
	size_t numTeed
);

typedef struct prevDist
{
	float dist;
	const point_t * prev;

} prevDist_t;

bool pf_dijkstraSearch(
	prevDist_t * restrict * restrict pprevdist,
	const point_t ** restrict points,
	const uint8_t * restrict relations,
	size_t numRelations,
	const point_t * restrict start
);

bool pf_makeDistMatrix(
	const point_t * const restrict * restrict startpoints,
	float * restrict * restrict pmatrix,
	size_t numPoints,
	const line_t * const restrict * restrict teed,
	size_t numTeed
);

bool pf_findOptimalMatrixOrder(
	const float * restrict matrix,
	size_t numPoints,
	size_t startIdx,
	size_t stopIdx,
	size_t ** restrict poutIndexes
);

#endif
