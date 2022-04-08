#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include "iniFile.h"
#include "hashmap.h"

#include <stdbool.h>
#include <stdint.h>

#define MAX_ID 256

/**
 * @brief Data structure that holds junction point's identifier string and it's coordinates.
 * Also holds identifying index to handling by Dijkstra's search algorithm.
 * 
 */
typedef struct point
{
	iniString_t id;
	float x, y;

	size_t idx;

} point_t;

/**
 * @brief Zeroes the memory of point structure.
 * 
 * @param p Pointer to point structure
 */
void point_zero(point_t * restrict p);
/**
 * @brief Initialises pointr structure with corresponding identifier string and values
 * from value string.
 * 
 * @param p Pointer to point structure
 * @param idstr Identifier string
 * @param valuestr Value string with equivalent scanf format of "%f,%f"
 * @return true Success in initialization
 * @return false Failure
 */
bool point_initStr(point_t * restrict p, const char * restrict idstr, const char * restrict valuestr);
/**
 * @brief Equivalent of point_initStr, except the memory is allocated on the heap, initialises
 * point structure with string identifier and values from value string.
 * 
 * @param idstr Identifier string
 * @param valuestr Value string with equivalent scanf format of "%f,%f"
 * @return point_t* Pointer to newly allocated & initialized point structure, NULL on failure
 */
point_t * point_makeStr(const char * restrict idstr, const char * restrict valuestr);

/**
 * @brief Destroys/frees resources of user-allocated point structure.
 * 
 * @param p Pointer to point structure
 */
void point_destroy(point_t * restrict p);
/**
 * @brief Frees all memory of point structure.
 * 
 * @param p Pointer to heap-allocated point structure
 */
void point_free(point_t * restrict p);


/**
 * @brief Data structure to represent roads/lines using 2 endpoints/junctions.
 * Also holds differential xy-coordinates and Euclidean distance between the points.
 * 
 */
typedef struct line
{
	iniString_t id;
	const point_t * src, * dst;

	float dx, dy, length, cost;

} line_t;

/**
 * @brief Zeroes the memory of line structure.
 * 
 * @param l Pointer to line structure
 */
void line_zero(line_t * restrict l);
/**
 * @brief Initialises the line structure with road identifier string and point identifiers.
 * 
 * @param l Pointer to line structure
 * @param pointmap Pointer to constant-keyed hashmap of points/junctions
 * @param idstr Identifier string
 * @param valuestr Value string, denotes 2 endpoint/junction identifiers with a pseudo-format of "%s,%s".
 * Can also include a cost of the road, then the pseudo-format is "%s,%s,%f".
 * @return true Success
 * @return false Failure
 */
bool line_initStr(
	line_t * restrict l,
	const hashMapCK_t * restrict pointmap,
	const char * restrict idstr,
	const char * restrict valuestr
);
/**
 * @brief Initialises and heap-allocates memory for line structure with road identifier string
 * and point identifiers.
 * 
 * @param pointmap Pointer to constant-keyed hashmap of points/junctions
 * @param idstr Identifier string
 * @param valuestr Value string, denotes 2 endpoint/junction identifiers with a pseudo-format of "%s,%s"
 * @return line_t* Pointer to heap-allocated line structure, NULL on failure
 */
line_t * line_makeStr(
	const hashMapCK_t * restrict pointmap,
	const char * restrict idstr,
	const char * restrict valuestr
);
/**
 * @brief Initialises line structure with corresponding road identifier string and endpoints.
 * 
 * @param l Pointer to line structure
 * @param idstr Identifier string
 * @param src Source point
 * @param dst Destination point
 * @param cost The cost of that particular road
 * @return true Success
 * @return false Failure
 */
bool line_init(
	line_t * restrict l,
	const char * restrict idstr,
	const point_t * restrict src,
	const point_t * restrict dst,
	float cost
);
/**
 * @brief Initialises & heap-allocated memory for line structure with corresponding
 * road identifier string and endpoints.
 * 
 * @param idstr Identifier string
 * @param src Source point
 * @param dst Destination point
 * @param cost The cost of that particular road
 * @return line_t* Pointer to heap-allocated line structure, NULL on failure
 */
line_t * line_make(
	const char * restrict idstr,
	const point_t * restrict src,
	const point_t * restrict dst,
	float cost
);

/**
 * @brief Calculates/updates differential xy-coordinates and Euclidean distance for a line/road
 * 
 * @param l Pointer to line structure
 */
void line_calc(line_t * restrict l);
/**
 * @brief Finds optimal intersection point between a starting point/junction & a line/road
 * 
 * @param ci Pointer to resulting optimal intersection point on the line/road
 * @param startp Pointer to starting point structure
 * @param line Pointer to line structure to intersect with
 */
void line_intersect(
	point_t * restrict ci,
	const point_t * restrict startp,
	const line_t * restrict line
);
/**
 * @brief Sets a new source for a line, calculates new info automatically
 * 
 * @param l Pointer to line structure
 * @param src Pointer to new source point structure
 */
void line_setSrc(line_t * restrict l, const point_t * restrict src);
/**
 * @brief Sets a new destination for a line, calculates new info automatically
 * 
 * @param l Pointer to line structure
 * @param dst Pointer to new desination point structure
 */
void line_setDest(line_t * restrict l, const point_t * restrict dst);

/**
 * @brief Destroys user-allocated line structure
 * 
 * @param l Pointer to line structure
 */
void line_destroy(line_t * restrict l);
/**
 * @brief Destroys & frees heap-allocated line structure memory
 * 
 * @param l Pointer to heap-allocated line structure
 */
void line_free(line_t * restrict l);


/**
 * @brief Data structure to hold distance with costs involved and real distance
 * 
 */
typedef struct distActual
{
	float dist, actual;

} distActual_t;

#define MAX_MID_POINTS 10
#define TOTAL_POINTS   12
#define START_IDX      0
#define STOP_IDX       1

/**
 * @brief The data model, representing all crucial structured data of the whole
 * application.
 * 
 */
typedef struct dataModel
{
	union
	{
		struct
		{
			point_t beg, end;
			point_t mid[MAX_MID_POINTS];
		};
		point_t points[TOTAL_POINTS];
	};
	union
	{
		struct
		{
			const point_t * begp, * endp;
			const point_t * midp[MAX_MID_POINTS];
		};
		const point_t * pointsp[TOTAL_POINTS];
	};
	size_t numMidPoints;

	hashMapCK_t junctionMap, stopsMap;
	
	line_t ** roads;
	size_t numRoads, maxRoads;

	uint8_t * relations;
	float * costs;
	const point_t ** juncPoints;
	size_t numJunctions;

	distActual_t * stopsDistMatrix;

	size_t * bestStopsIndices;

	const point_t ** shortestPath;
	size_t shortestPathLen;

} dataModel_t;

/**
 * @brief Error code enumerator, useful to "decipher" error codes returned by
 * various dm_* functions
 * 
 */
typedef enum dmErr
{
	dmeOK,
	dmeMEM,
	dmeSECTIONS,
	dmeSTOPS_LIMIT

} dmErr_t;


/**
 * @brief Initialises dataModel structure with a data file
 * 
 * @param dm Pointer to dataModel structure
 * @param filename File name string of the data file
 * @return dmErr_t Error code, dmeOK on success
 */
dmErr_t dm_initDataFile(dataModel_t * restrict dm, const char * restrict filename);
/**
 * @brief Adds all stopping points as the nearest intersecting points with existing
 * roads to the the data model
 * 
 * @param dm Pointer to dataModel structure
 * @return true Success
 * @return false Failure
 */
bool dm_addStops(dataModel_t * restrict dm);
/**
 * @brief Adds a new line/road to the data model
 * 
 * @param dm Pointer to dataModel structure
 * @param pline Pointer to heap-allocated line to add to the data model
 * @return true Success
 * @return false Failure
 */
bool dm_addLine(dataModel_t * restrict dm, line_t * restrict pline);
/**
 * @brief Updates junction indexes accordingly for the Dijsktra's algorithm
 * 
 * @param dm Pointer to dataModel structure
 */
void dm_updateJunctionIndexes(dataModel_t * restrict dm);

/**
 * @brief Creates relations matrix, costs matrix and points array
 * 
 * @param dm Pointer to dataModel structure
 * @return true Success
 * @return false Failure
 */
bool dm_createMatrices(dataModel_t * restrict dm);
/**
 * @brief Finds a sequence of stops that results in the shortest path,
 * also finds the shortest path
 * 
 * @param dm Pointer to dataModel structure
 * @return true Success
 * @return false Failure
 */
bool dm_findShortestPath(dataModel_t * restrict dm);

/**
 * @brief Destroys the data model's allocated resources
 * 
 * @param dm Pointer to user-allocated data model structure
 */
void dm_destroy(dataModel_t * restrict dm);


#endif
