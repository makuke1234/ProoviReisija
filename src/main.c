#include "iniFile.h"
#include "dataModel.h"
#include "logger.h"
#include "pathFinding.h"
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
	bool result = pf_createRelations(
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
				pf_bGet(relations, pf_calcIdx(i, j, numRelations)) ? '1' : '0'
			);
			if ((j + 1) < numRelations)
			{
				putchar(' ');
			}
		}
		putchar('\n');
	}
	free(relations);
	free(points);



	float * matrix = NULL;
	result = pf_makeDistMatrix(
		dm.pointsp,
		&matrix,
		dm.numMidPoints + 2,
		(const line_t * const *)dm.teed,
		dm.numTeed
	);
	if (!result)
	{
		fprintf(stderr, "Viga Dijkstra algoritmi t55s!\n");
		return 1;
	}
	const size_t totalPoints = dm.numMidPoints + 2;


	printf("Peatuste vaheliste kauguste maatriks:\n");
	printf("%8c ", ' ');
	for (size_t i = 0; i < totalPoints; ++i)
	{
		printf("%8s ", dm.points[i].id.str);
	}
	putchar('\n');
	for (size_t i = 0; i < totalPoints; ++i)
	{
		float * row = &matrix[i * totalPoints];
		printf("%8s ", dm.points[i].id.str);
		for (size_t j = 0; j < totalPoints; ++j)
		{
			printf("%8.3f ", (double)row[j]);
		}
		putchar('\n');
	}

	printf("Permutatsioonide l2biproovimine...\n");

	size_t * bestIndexes = NULL;
	result = pf_findOptimalMatrixOrder(
		matrix,
		totalPoints,
		0,
		1,
		&bestIndexes
	);
	free(matrix);

	if (!result)
	{
		fprintf(stderr, "Viga optimaalse j2rjekorra leidmisel!\n");
		return 1;
	}

	printf("Parim peatuste l2bimise j2rjekord:\n");
	for (size_t i = 0; i < totalPoints; ++i)
	{
		printf("%s ", dm.points[bestIndexes[i]].id.str);
	}
	putchar('\n');

	free(bestIndexes);


	dm_destroy(&dm);

	return 0;
}
