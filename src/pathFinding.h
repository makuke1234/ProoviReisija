#ifndef PATH_FINDING_H
#define PATH_FINDING_H

#include "dataModel.h"
#include "priorityQ.h"

/**
 * @brief Sets one bit value in a tightly packed boolean array
 * 
 * @param bArray Pointer to tightly packed boolean array
 * @param idx Bit index to change value
 * @param value User-selectable value, either 0/1, false/true
 */
void pf_bSet(uint8_t * restrict bArray, size_t idx, bool value);
/**
 * @brief Retrieves one bit value from a tightly packed boolean array
 * 
 * @param bArray Pointer to tightly packed boolean array
 * @param idx Bit index to extract
 * @return true Bit was set to 1
 * @return false Bit was set to 0
 */
bool pf_bGet(const uint8_t * restrict bArray, size_t idx);
/**
 * @brief Calculates tightly packed boolean array size in bytes from given
 * number of values
 * 
 * @param numItems Desired number of boolean values in array
 * @return size_t Size of tightly packed boolean array in bytes
 */
size_t pf_bArrBytes(size_t numItems);
/**
 * @brief Calculates 1-dimensional array index from 2-dimensional index
 * 
 * @param row Desired row index
 * @param col Desired column index
 * @param numCols Number of columns, the width of the 2-dimensional index space
 * @return size_t 1-dimensional equivalent array index
 */
size_t pf_calcIdx(size_t row, size_t col, size_t numCols);

/**
 * @brief Creates relational matrix & unique junctions array from roads
 * 
 * @param prelations Pointer to receiving 1D array for the relational matrix
 * @param pcosts Pointer to receiving 1D array for the costs matrix
 * @param numRelations Pointer to receiving the number of unique junctions
 * @param ppoints Pointer to receiving the array of unique junction pointers, data
 * in there corresponds directly to the rows/columns of the relational matrix
 * @param teed Roads array
 * @param numTeed Number of roads
 * @return true Success
 * @return false Failure
 */
bool pf_createRelationsCosts(
	uint8_t ** restrict prelations,
	float ** restrict pcosts,
	size_t * restrict numRelations,
	const point_t *** restrict ppoints,
	const line_t * const restrict * restrict teed,
	size_t numTeed
);

/**
 * @brief Data structure for the Dijkstra algorithm, holds current best distance and
 * pointer to previous point via which the point was reached
 * 
 */
typedef struct prevDist
{
	float dist, actual;
	const point_t * prev;

} prevDist_t;

/**
 * @brief Data structure to hold distance with costs involved and real distance
 * 
 */
typedef struct distActual
{
	float dist, actual;

} distActual_t;

/**
 * @brief Performs the Dijkstra optimal path search algorithm starting from given point.
 * Returns shortest paths to all junctions, uses junction relational matrix.
 * 
 * @param pprevdist Pointer to receiving prevDist structure array
 * @param points Array of unique junction pointers
 * @param relations Relational matrix
 * @param costs Costs matrix corresponding to the relational matrix
 * @param numRelations Number of unique junctions
 * @param start Starting point pointer
 * @return true Success
 * @return false Failure
 */
bool pf_dijkstraSearch(
	prevDist_t * restrict * restrict pprevdist,
	const point_t ** restrict points,
	const uint8_t * restrict relations,
	const float * restrict costs,
	size_t numRelations,
	const point_t * restrict start
);

/**
 * @brief Creates 1D-allocated 2D matrix of shortest distances between any
 * two desired points.
 * 
 * @param startpoints Array of starting point pointers 
 * @param pmatrix Pointer to receiving 1D matrix of shortest distances
 * @param numPoints Number of (stopping) points
 * @param teed Roads array
 * @param numTeed Number of roads
 * @return true Success
 * @return false Failure
 */
bool pf_makeDistMatrix(
	const point_t * const restrict * restrict startpoints,
	distActual_t * restrict * restrict pmatrix,
	size_t numPoints,
	const line_t * const restrict * restrict teed,
	size_t numTeed
);

/**
 * @brief Finds optimal sequence of stops to take given the shortest distances
 * between any two points and the starting & stopping points. Uses Dijkstra search
 * algorithm to find shortest distances between any two points.
 * 
 * @param matrix 1D-allocated shortest distances matrix
 * @param numPoints Number of (stopping) points
 * @param startIdx Starting points index (usually 0)
 * @param stopIdx Stopping point index (usually 1)
 * @param poutIndexes Pointer to receiving shortest index sequence array
 * @return true Success
 * @return false Failure
 */
bool pf_findOptimalMatrixOrder(
	const distActual_t * restrict matrix,
	size_t numPoints,
	size_t startIdx,
	size_t stopIdx,
	size_t ** restrict poutIndexes
);

bool pf_generateShortestPath(
	const point_t *** restrict ppath,
	size_t * restrict ppathLen,
	const size_t * restrict bestIndexes,
	const point_t ** restrict startpoints,
	size_t numPoints,
	const point_t ** restrict points,
	const uint8_t * restrict relations,
	const float * restrict costs,
	size_t numRelations
);

#endif
