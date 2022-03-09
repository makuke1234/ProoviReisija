#include "dataModel.h"

#include <stdlib.h>
#include <stdio.h>

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
point_t * point_makeStr(const char * restrict idstr, const char * restrict valuestr)
{
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

bool line_initStr(
	line_t * restrict l,
	const hashMap_t * restrict pointmap,
	const char * restrict idstr,
	const char * restrict valuestr
)
{
	assert(l != NULL);

	if (!iniString_init(&l->id, idstr, -1))
	{
		return false;
	}
	
	char id1[MAX_ID], id2[MAX_ID];
	if (sscanf_s(valuestr, "%s, %s", id1, MAX_ID, id2, MAX_ID) < 2)
	{
		iniString_destroy(&l->id);
		return false;
	}

	const hashNode_t * n1, * n2;
	n1 = hashMap_get(pointmap, id1);
	n2 = hashMap_get(pointmap, id2);
	if ((n1 == NULL) || (n2 == NULL))
	{
		iniString_destroy(&l->id);
		return false;
	}

	l->src = n1->value;
	l->dst = n2->value;

	return true;
}
void line_destroy(line_t * restrict l)
{
	assert(l != NULL);
	iniString_destroy(&l->id);
}

dmErr_t dm_initDataFile(dataModel_t * restrict dm, const char * restrict filename)
{
	assert(dm != NULL);
	assert(filename != NULL);

	// Räsitabeli koostamine punktidest
	if (!hashMap_init(&dm->ristmikud, 1))
	{
		return dmeMEM;
	}

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

	// Ristmikud
	for (size_t i = 0; i < ristmikud->numValues; ++i)
	{
		iniValue_t * val = ristmikud->values[i];
		if (val != NULL)
		{
			point_t * p = point_makeStr(val->key.str, val->value.str);
			// Punkti p id räsitabelisse lisamine
			if ((p == NULL) || !hashMap_insert(&dm->ristmikud, p->id.str, p))
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


	// Peatused

	size_t numStops = 0;
	for (size_t i = 0; i < peatused->numValues; ++i)
	{
		numStops += (peatused->values[i] != NULL);
	}
	// Peatuste lisamine
	if (numStops > (MAX_MID_POINTS + 2))
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

	for (size_t i = 0; i < dm->ristmikud.numNodes; ++i)
	{
		hashNode_t * node = dm->ristmikud.nodes[i];
		if (node != NULL)
		{
			point_free(node->value);
			node->value = NULL;
		}
	}
	hashMap_destroy(&dm->ristmikud);
}

