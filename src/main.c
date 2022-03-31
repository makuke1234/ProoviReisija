#include "iniFile.h"
#include "dataModel.h"
#include "logger.h"
#include "dijkstra.h"
#include "mathHelper.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv)
{
	initLogger();

	if (argc < 2)
	{
		fprintf(stderr, "Kasutus: %s [info fail.ini]\n", argv[0]);
		return 1;
	}

	dataModel_t dm;
	dmErr_t code = dm_initDataFile(&dm, argv[1]);

	if (code != dmeOK)
	{
		fprintf(stderr, "Viga andmete laadimisel!\n");
		return 1;
	}

	// Väljastab vahepunktid
	for (size_t i = 0; i < dm.numTeed; ++i)
	{
		if (dm.teed[i] != NULL)
		{
			line_t * tee = dm.teed[i];
			printf(
				"Tee %s: %s, %zu: (%f, %f) -> %s, %zu: (%f, %f)\n",
				tee->id.str,
				tee->src->id.str, tee->src->idx, (double)tee->src->x, (double)tee->src->y,
				tee->dst->id.str, tee->dst->idx, (double)tee->dst->x, (double)tee->dst->y
			);
		}
	}

	// Koostab maatriksi lühimatest kaugustest punktide vahel
	size_t numRelations;
	uint8_t * relations = NULL;
	const point_t ** points = NULL;
	bool result = dijkstra_createRelations(
		&relations,
		&numRelations,
		&points,
		(const line_t * const *)dm.teed,
		dm.numTeed
	);
	if (!result)
	{
		fprintf(stderr, "Viga graafi relatsioonide tegemisel!\n");
		return 1;
	}

	size_t maxplen = 0;
	for (size_t i = 0; i < numRelations; ++i)
	{
		maxplen = mh_zmax(maxplen, points[i]->id.len);
	}

	printf("Relatsioonide maatriks:\n");
	printf("%*c ", maxplen, ' ');
	for (size_t i = 0; i < numRelations; ++i)
	{
		printf("%*s", maxplen, points[i]->id.str);
		if ((i + 1) < numRelations)
		{
			putchar(' ');
		}
	}
	putchar('\n');
	for (size_t i = 0; i < numRelations; ++i)
	{
		printf("%*s ", maxplen, points[i]->id.str);
		for (size_t j = 0; j < numRelations; ++j)
		{
			printf(
				"%*c%c",
				maxplen - 1,
				' ',
				dijkstra_bGet(relations, dijkstra_calcIdx(i, j, numRelations)) ? '1' : '0'
			);
			if ((j + 1) < numRelations)
			{
				putchar(' ');
			}
		}
		putchar('\n');
	}

	prevdist_t * distances = NULL;
	result = dijkstra_search_poc(
		&distances,
		points,
		relations,
		numRelations,
		dm.begp
	);
	if (!result)
	{
		fprintf(stderr, "Viga Dijkstra algoritmi t55s!\n");
		return 1;
	}


	printf("Kaugused punktide vahel: \n");
	for (size_t i = 0; i < numRelations; ++i)
	{
		prevdist_t * dist = &distances[i];
		printf("Eelmine punkt %s -> %s: %.3f\n", dist->prev != NULL ? dist->prev->id.str : "pole", points[i]->id.str, (double)dist->dist);
	}
	free(points);
	free(distances);
	free(relations);


	dm_destroy(&dm);

	return 0;
}
