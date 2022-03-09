#include "dataModel.h"

#include <stdlib.h>

bool point_initStr(point_t * restrict p, const char * restrict idstr, const char * restrict valuestr)
{
	assert(p != NULL);

	if (!iniString_init(&p->id, idstr, -1))
	{
		return false;
	}

	char * next = NULL;
	p->x = strtof(valuestr, &next);
	p->y = strtof(next, NULL);

	return true;
}
void point_destroy(point_t * restrict p)
{
	assert(p != NULL);
	iniString_destroy(&p->id);
}


dmErr_t dm_initDataFile(dataModel_t * restrict dm, const char * restrict filename)
{
	assert(dm != NULL);
	assert(filename != NULL);

	ini_t inifile;
	if (ini_initFile(filename, &inifile) != inieOK)
	{
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


	// Peatused

	size_t numStops = 0;
	for (size_t i = 0; i < peatused->numValues; ++i)
	{
		numStops += (peatused->values[i] != NULL);
	}
	// Peatuste lisamine
	if (numStops > (MAX_POINTS + 2))
	{
		ini_destroy(&inifile);
		return dmeSTOPS_LIMIT;
	}


	dm->numMidPoints = 0;

	for (size_t i = 0, realStops = 0; i < peatused->numValues; ++i)
	{
		iniValue_t * val = peatused->values[i];
		if (val != NULL)
		{
			point_t p;
			if (!point_initStr(&p, val->key.str, val->value.str))
			{
				ini_destroy(&inifile);
				return dmeMEM;
			}
			else if (realStops == 0)
			{
				dm->beg = p;
			}
			else if ((realStops + 1) == numStops)
			{
				dm->end = p;
			}
			else
			{
				dm->mid[dm->numMidPoints] = p;
				++dm->numMidPoints;
			}

			++realStops;
		}
	}


	// Mälu vabastamine, kui kõik on õnnestunud siiamaani
	ini_destroy(&inifile);

	return dmeOK;
}
void dm_destroy(dataModel_t * restrict dm)
{
	point_destroy(&dm->beg);
	point_destroy(&dm->end);

	for (size_t i = 0; i < dm->numMidPoints; ++i)
	{
		point_destroy(&dm->mid[i]);
	}
}

