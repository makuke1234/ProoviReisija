#ifndef DATA_MODEL_H
#define DATA_MODEL_H

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

void point_zero(point_t * restrict p);
bool point_initStr(point_t * restrict p, const char * restrict idstr, const char * restrict valuestr);
point_t * point_makeStr(const char * restrict idstr, const char * restrict valuestr);

void point_destroy(point_t * restrict p);
void point_free(point_t * restrict p);


typedef struct line
{
	iniString_t id;
	const point_t * src, * dst;

	float dx, dy, length2;

} line_t;

void line_zero(line_t * restrict l);
bool line_initStr(
	line_t * restrict l,
	const hashMap_t * restrict pointmap,
	const char * restrict idstr,
	const char * restrict valuestr
);
line_t * line_makeStr(
	const hashMap_t * restrict pointmap,
	const char * restrict idstr,
	const char * restrict valuestr
);
bool line_init(
	line_t * restrict l,
	const char * restrict idstr,
	const point_t * restrict src,
	const point_t * restrict dst
);
line_t * line_make(
	const char * restrict idstr,
	const point_t * restrict src,
	const point_t * restrict dst
);

void line_intersect(
	point_t * restrict ci,
	const point_t * restrict startp,
	const line_t * restrict line
);

void line_destroy(line_t * restrict l);
void line_free(line_t * restrict l);


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
	
	line_t ** teed;
	size_t numTeed, maxTeed;

} dataModel_t;

typedef enum dmErr
{
	dmeOK,
	dmeMEM,
	dmeSECTIONS,
	dmeSTOPS_LIMIT

} dmErr_t;


dmErr_t dm_initDataFile(dataModel_t * restrict dm, const char * restrict filename);
bool dm_addStops(dataModel_t * restrict dm);
bool dm_addLine(dataModel_t * restrict dm, line_t * restrict pline);

void dm_destroy(dataModel_t * restrict dm);


#endif
