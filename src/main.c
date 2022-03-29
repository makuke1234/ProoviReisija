#include "iniFile.h"
#include "dataModel.h"
#include "logger.h"
#include "dijkstra.h"

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
	size_t numRel;
	uint8_t * relations = dijkstra_createRelations(&numRel, &dm.ristmikud, (const line_t * const *)dm.teed, dm.numTeed);

	if (relations == NULL)
	{
		fprintf(stderr, "Viga graafi relatsioonide tegemisel!\n");
		return 1;
	}

	prevdist_t * distances = NULL;
	const point_t ** points = NULL;
	if (!dijkstra_search_poc(
			&distances,
			&points,
			(const line_t * const *)dm.teed,
			dm.numTeed,
			relations,
			numRel,
			dm.begp
		)
	)
	{
		fprintf(stderr, "Viga Dijkstra algoritmi t55s!\n");
		return 1;
	}


	printf("Kaugused punktide vahel: \n");
	for (size_t i = 0; i < numRel; ++i)
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
