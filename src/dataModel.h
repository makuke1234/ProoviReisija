#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "iniFile.h"

#include <stdbool.h>
#include <stdint.h>


typedef struct point
{
	iniString_t id;
	float x, y;

} point_t;

bool point_initStr(point_t * restrict p, const char * restrict idstr, const char * restrict valuestr);
void point_destroy(point_t * restrict p);


#define MAX_POINTS 10

typedef struct dataModel
{
	point_t beginning, end;
	point_t middlePoints[MAX_POINTS];
	size_t numMiddlePoints;


} dataModel_t;


bool dm_readData(dataModel_t * restrict dm, const char * restrict filename);


#endif
