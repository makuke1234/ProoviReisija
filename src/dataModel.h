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
	point_t beg, end;
	point_t mid[MAX_POINTS];
	size_t numMidPoints;


} dataModel_t;

typedef enum dmErr
{
	dmeOK,
	dmeMEM,
	dmeSECTIONS,
	dmeSTOPS_LIMIT
} dmErr_t;


dmErr_t dm_initDataFile(dataModel_t * restrict dm, const char * restrict filename);
void dm_destroy(dataModel_t * restrict dm);


#endif
