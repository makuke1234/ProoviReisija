#include "dataModel.h"
#include "logger.h"
#include "mathHelper.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void point_zero(point_t * restrict p)
{
	assert(p != NULL);
	iniString_zero(&p->id);
}
bool point_initStr(point_t * restrict p, const char * restrict idstr, const char * restrict valuestr)
{
	assert(p        != NULL);
	assert(valuestr != NULL);

	if (!iniString_init(&p->id, idstr, -1))
	{
		return false;
	}

	char * next = NULL;
	p->x = strtof(valuestr, &next);
	if (*next == ',')
	{
		++next;
	}
	p->y = strtof(next, NULL);

	return true;
}
point_t * point_makeStr(const char * restrict idstr, const char * restrict valuestr)
{
	assert(valuestr != NULL);

	point_t * p = malloc(sizeof(point_t));
	if (p == NULL)
	{
		return NULL;
	}
	else if (!point_initStr(p, idstr, valuestr))
	{
		free(p);
		return NULL;
	}

	return p;
}

void point_destroy(point_t * restrict p)
{
	assert(p != NULL);
	iniString_destroy(&p->id);
}
void point_free(point_t * restrict p)
{
	assert(p != NULL);
	point_destroy(p);
	free(p);
}

void line_zero(line_t * restrict l)
{
	assert(l != NULL);

	iniString_zero(&l->id);
	l->src = NULL;
	l->dst = NULL;

	l->dx     = 0.0f;
	l->dy     = 0.0f;
	l->length = 0.0f;
	l->cost   = 1.0f;
}
bool line_initStr(
	line_t * restrict l,
	const hashMapCK_t * restrict pointmap,
	const char * restrict idstr,
	const char * restrict valuestr
)
{
	assert(l        != NULL);
	assert(pointmap != NULL);
	assert(valuestr != NULL);

	if (!iniString_init(&l->id, idstr, -1))
	{
		return false;
	}
	
	char id1[MAX_ID], id2[MAX_ID], costStr[MAX_ID];
	char * strs[] = { id1, id2, costStr };
	bool found = false;
	size_t prevStop = 0, j = 0, len = strlen(valuestr);
	for (size_t i = 0; (i < len) && (j < 3); ++i)
	{
		if (valuestr[i] == ',')
		{
			found = true;
			memcpy(strs[j], &valuestr[prevStop], sizeof(char) * (i - prevStop));
			strs[j][i - prevStop] = '\0';
			++j;
			++i;
			while ((i < len) && ((valuestr[i] == ' ') || (valuestr[i] == '\t')))
			{
				++i;
			}
			prevStop = i;
		}
	}
	if (!found)
	{
		iniString_destroy(&l->id);
		return false;
	}
	if (j < 3)
	{
		memcpy(strs[j], &valuestr[prevStop], sizeof(char) * (len - prevStop));
		strs[j][len - prevStop] = '\0';
	}
	l->cost = 1.0f;
	if (j == 2)
	{
		l->cost = (float)atof(costStr);
		if (l->cost == 0.0f)
		{
			l->cost = 1.0f;
		}
	}

	writeLogger("prevStop: %zu, id1: \"%s\", id2: \"%s\", cost: %f", prevStop, id1, id2, (double)l->cost);

	const hashNodeCK_t * n1, * n2;
	n1 = hashMapCK_get(pointmap, id1);
	n2 = hashMapCK_get(pointmap, id2);
	if ((n1 == NULL) || (n2 == NULL))
	{
		iniString_destroy(&l->id);
		return false;
	}

	l->src = n1->value;
	l->dst = n2->value;

	line_calc(l);

	return true;
}
line_t * line_makeStr(
	const hashMapCK_t * restrict pointmap,
	const char * restrict idstr,
	const char * restrict valuestr
)
{
	assert(pointmap != NULL);
	assert(valuestr != NULL);

	line_t * mem = malloc(sizeof(line_t));
	if (mem == NULL)
	{
		return false;
	}
	else if (!line_initStr(mem, pointmap, idstr, valuestr))
	{
		free(mem);
		return false;
	}

	return mem;
}
bool line_init(
	line_t * restrict l,
	const char * restrict idstr,
	const point_t * restrict src,
	const point_t * restrict dst,
	float cost
)
{
	assert(l    != NULL);
	assert(src  != NULL);
	assert(dst  != NULL);
	assert(src  != dst);
	assert(cost != 0.0f);

	if (!iniString_init(&l->id, idstr, -1))
	{
		return false;
	}
	l->src  = src;
	l->dst  = dst;
	l->cost = cost;
	line_calc(l);

	return true;
}
line_t * line_make(
	const char * restrict idstr,
	const point_t * restrict src,
	const point_t * restrict dst,
	float cost
)
{
	assert(src != NULL);
	assert(dst != NULL);
	assert(src != dst);

	line_t * mem = malloc(sizeof(line_t));
	if (mem == NULL)
	{
		return false;
	}
	else if (!line_init(mem, idstr, src, dst, cost))
	{
		free(mem);
		return false;
	}

	return mem;
}

void line_calc(line_t * restrict l)
{
	assert(l      != NULL);
	assert(l->src != NULL);
	assert(l->dst != NULL);

	l->dx = l->dst->x - l->src->x;
	l->dy = l->dst->y - l->src->y;
	l->length = sqrtf((l->dx * l->dx) + (l->dy * l->dy));
}
void line_intersect(
	point_t * restrict ci,
	const point_t * restrict startp,
	const line_t * restrict line
)
{
	assert(ci     != NULL);
	assert(startp != NULL);
	assert(line   != NULL);

	if (line->dx == 0)
	{
		ci->x = line->src->x;
		ci->y = mh_clampUnif(startp->y, line->src->y, line->dst->y);
	}
	else if (line->dy == 0)
	{
		ci->x = mh_clampUnif(startp->x, line->src->x, line->dst->x);
		ci->y = line->src->y;
	}
	else
	{
		float k1 = line->dy / line->dx;
		float k2 = -1.0f / k1;

		float c1 = line->src->y - k1 * line->src->x;
		float c2 = startp->y    - k2 * startp->x;

		ci->x = mh_clampUnif((c2 - c1) / (k1 - k2), line->src->x, line->dst->x);
		ci->y = k1 * ci->x + c1;
	}

	writeLogger(
		"line %s: dx: %.3g, dy: %.3g, %s (%.3g; %.3g) -> %s (%.3g; %.3g), startpoint: (%.3g; %.3g) -> new point: (%.3g; %.3g)",
		line->id.str, (double)line->dx, (double)line->dy,
		line->src->id.str, (double)line->src->x, (double)line->src->y,
		line->dst->id.str, (double)line->dst->x, (double)line->dst->y,
		(double)startp->x, (double)startp->y,
		(double)ci->x, (double)ci->y
	);
}
void line_setSrc(line_t * restrict l, const point_t * restrict src)
{
	assert(l   != NULL);
	assert(src != NULL);

	l->src = src;
	
	line_calc(l);
}
void line_setDest(line_t * restrict l, const point_t * restrict dst)
{
	assert(l   != NULL);
	assert(dst != NULL);
	
	l->dst = dst;
	
	line_calc(l);
}

void line_destroy(line_t * restrict l)
{
	assert(l != NULL);
	iniString_destroy(&l->id);
}
void line_free(line_t * restrict l)
{
	assert(l != NULL);
	line_destroy(l);
	free(l);
}


dmErr_t dm_initDataFile(dataModel_t * restrict dm, const char * restrict filename)
{
	assert(dm       != NULL);
	assert(filename != NULL);

	*dm = (dataModel_t){
		.numMidPoints = 0,
		.teed         = NULL,
		.numTeed      = 0,
		.maxTeed      = 0
	};

	for (size_t i = 0; i < TOTAL_POINTS; ++i)
	{
		point_zero(&dm->points[i]);
	}
	hashMapCK_zero(&dm->junctionMap);
	hashMapCK_zero(&dm->stopsMap);

	// Räsitabeli koostamine punktidest
	if (!hashMapCK_init(&dm->junctionMap, 1))
	{
		return dmeMEM;
	}
	if (!hashMapCK_init(&dm->stopsMap, TOTAL_POINTS))
	{
		return dmeMEM;
	}

	ini_t inifile;
	if (ini_initFile(&inifile, filename) != inieOK)
	{
		writeLogger("File error!");
		return dmeMEM;
	}

	const iniSection_t * ristmikud, * teed, * peatused;
	ristmikud = ini_getSection(&inifile, "ristmikud");
	teed      = ini_getSection(&inifile, "teed");
	peatused  = ini_getSection(&inifile, "peatused");

	if ((ristmikud == NULL) || (teed == NULL) || (peatused == NULL))
	{
		ini_destroy(&inifile);
		return dmeSECTIONS;
	}

	// Ristmikud
	for (size_t i = 0; i < ristmikud->numValues; ++i)
	{
		iniValue_t * val = ristmikud->values[i];
		if (val != NULL)
		{
			point_t * p = point_makeStr(val->key.str, val->value.str);
			// Punkti p id räsitabelisse lisamine
			if ((p == NULL) || !hashMapCK_insert(&dm->junctionMap, p->id.str, p))
			{
				if (p != NULL)
				{
					point_free(p);
				}
				ini_destroy(&inifile);
				dm_destroy(dm);
				return dmeMEM;
			}
		}
	}

	// Teed

	for (size_t i = 0; i < teed->numValues; ++i)
	{
		iniValue_t * val = teed->values[i];
		if (val != NULL)
		{
			line_t * line = line_makeStr(&dm->junctionMap, val->key.str, val->value.str);
			if ((line == NULL) || !dm_addLine(dm, line))
			{
				if (line != NULL)
				{
					free(line);
				}
				ini_destroy(&inifile);
				dm_destroy(dm);
				return dmeMEM;
			}
		}
	}

	// Peatused

	size_t numStops = 0;
	for (size_t i = 0; i < peatused->numValues; ++i)
	{
		numStops += (peatused->values[i] != NULL);
	}
	// Peatuste lisamine
	if (numStops > TOTAL_POINTS)
	{
		ini_destroy(&inifile);
		return dmeSTOPS_LIMIT;
	}


	for (size_t i = 0, realStops = 0; i < peatused->numValues; ++i)
	{
		iniValue_t * val = peatused->values[i];
		if (val != NULL)
		{
			point_t p;
			if (!point_initStr(&p, val->key.str, val->value.str))
			{
				ini_destroy(&inifile);
				dm_destroy(dm);
				return dmeMEM;
			}

			if ((realStops > 0) && ((realStops + 1) < numStops))
			{
				dm->mid[dm->numMidPoints] = p;
				++dm->numMidPoints;
			}
			else
			{
				dm->points[(realStops > 0)] = p;
			}

			++realStops;
		}
	}
	
	if (!dm_addStops(dm))
	{
		ini_destroy(&inifile);
		dm_destroy(dm);
		return dmeMEM;
	}

	dm_updateJunctionIndexes(dm);

	// Mälu vabastamine, kui kõik on õnnestunud siiamaani
	ini_destroy(&inifile);

	return dmeOK;
}
bool dm_addStops(dataModel_t * restrict dm)
{
	assert(dm != NULL);

	size_t totPoints = 2 + dm->numMidPoints;

	for (size_t i = 0; i < totPoints; ++i)
	{
		point_t * p = &dm->points[i];
		// Otsib lähima tee konkreetsele punktile
		float shortestLen2;
		point_t bestPoint;
		line_t * tee = NULL;
		bool pointSet = false;

		for (size_t j = 0; j < dm->numTeed; ++j)
		{
			point_t tempPoint;
			line_intersect(&tempPoint, p, dm->teed[j]);

			const float dx = tempPoint.x - p->x;
			const float dy = tempPoint.y - p->y;
			float len2 = (dx * dx) + (dy * dy);
			// Tee "hinda" võetakse ka arvesse, eelistatakse "kiiremaid" teid
			len2 *= dm->teed[j]->cost;

			if (!pointSet || (len2 < shortestLen2))
			{
				pointSet = true;

				shortestLen2 = len2;
				bestPoint = tempPoint;
				tee = dm->teed[j];
			}
		}

		if (!pointSet || !iniString_copy(&bestPoint.id, &p->id))
		{
			return false;
		}

		// Tee "poolitamine"
		
		// Saadud "ideaalne" punkt lisatakse uue ristmikuna, mille id = peatuse id
		point_t * pointmem = malloc(sizeof(point_t));
		if (pointmem == NULL)
		{
			point_destroy(&bestPoint);
			return false;
		}
		*pointmem = bestPoint;
		// Lisa punkti projektsiooni viit õigesse kohta
		dm->pointsp[i] = pointmem;
		if (!hashMapCK_insert(&dm->junctionMap, pointmem->id.str, pointmem))
		{
			point_free(pointmem);
			return false;
		}

		line_t * linemem = line_make(pointmem->id.str, pointmem, tee->dst, tee->cost);
		if (linemem == NULL)
		{
			return false;
		}

		// Lisatakse uus tee, mille id = peatuse id
		if (!dm_addLine(dm, linemem))
		{
			line_free(linemem);
			return false;
		}
		line_setDest(tee, pointmem);
	}

	// Add stops to hashmap
	for (size_t i = 0; i < totPoints; ++i)
	{
		if (!hashMapCK_insert(&dm->stopsMap, dm->pointsp[i]->id.str, &dm->pointsp[i]))
		{
			return false;
		}
	}

	return true;
}
bool dm_addLine(dataModel_t * restrict dm, line_t * restrict pline)
{
	assert(dm    != NULL);
	assert(pline != NULL);

	if (dm->numTeed >= dm->maxTeed)
	{
		size_t newcap = (dm->numTeed + 1) * 2;
		line_t ** newmem = realloc(dm->teed, sizeof(line_t *) * newcap);
		if (newmem == NULL)
		{
			return false;
		}

		for (size_t i = dm->maxTeed; i < newcap; ++i)
		{
			newmem[i] = NULL;
		}

		dm->teed    = newmem;
		dm->maxTeed = newcap;
	}

	dm->teed[dm->numTeed] = pline;
	++dm->numTeed;

	return true;
}
void dm_updateJunctionIndexes(dataModel_t * restrict dm)
{
	assert(dm != NULL);

	size_t idx = 0;
	for (size_t i = 0; i < dm->junctionMap.numNodes; ++i)
	{
		hashNodeCK_t * node = dm->junctionMap.nodes[i];
		while (node != NULL)
		{
			if (node->value != NULL)
			{
				((point_t *)node->value)->idx = idx;
				++idx;
			}

			node = node->next;
		}
	}
}

void dm_destroy(dataModel_t * restrict dm)
{
	assert(dm != NULL);

	point_destroy(&dm->beg);
	point_destroy(&dm->end);

	for (size_t i = 0; i < dm->numMidPoints; ++i)
	{
		point_destroy(&dm->mid[i]);
	}

	for (size_t i = 0; i < dm->junctionMap.numNodes; ++i)
	{
		hashNodeCK_t * node = dm->junctionMap.nodes[i];
		if (node != NULL)
		{
			point_free(node->value);
			node->value = NULL;
		}
	}
	hashMapCK_destroy(&dm->junctionMap);
	hashMapCK_destroy(&dm->stopsMap);

	if (dm->teed != NULL)
	{
		for (size_t i = 0; i < dm->numTeed; ++i)
		{
			if (dm->teed[i] != NULL)
			{
				line_free(dm->teed[i]);
			}
		}
		free(dm->teed);
		dm->teed = NULL;
	}
}

