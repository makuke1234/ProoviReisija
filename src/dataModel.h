#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "iniFile.h"
#include "hashmap.h"

#include <stdbool.h>
#include <stdint.h>

#define MAX_ID 256


typedef struct point
{
	iniString_t id;
	float x, y;

} point_t;

bool point_initStr(point_t * restrict p, const char * restrict idstr, const char * restrict valuestr);
point_t * point_makeStr(const char * restrict idstr, const char * restrict valuestr);
void point_destroy(point_t * restrict p);
void point_free(point_t * restrict p);


typedef struct line
{
	iniString_t id;

	const point_t * src, * dst;

} line_t;

bool line_initStr(
	line_t * restrict l,
	const hashMap_t * restrict pointmap,
	const char * restrict idstr,
	const char * restrict valuestr
);
void line_destroy(line_t * restrict l);


#define MAX_MID_POINTS 10
#define TOTAL_POINTS 12

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
	size_t numMidPoints;

	hashMap_t ristmikud;


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
