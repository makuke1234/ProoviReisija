#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct point
{
	float x, y;
} point_t;

#define MAX_POINTS 10

typedef struct dataModel
{
	point_t beginning, end;
	point_t middlePoints[MAX_POINTS];
	size_t numMiddlePoints;


} dataModel_t;

bool dm_readData(const char * restrict filename, dataModel_t * restrict dm);


#endif
