#include "iniFile.h"
#include "dataModel.h"
#include "logger.h"
#include "pathFinding.h"
#include "mathHelper.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

int main(int argc, char ** argv)
{
	// Mõõdab algusaja
	clock_t startTime = clock();

	initLogger();

	if (argc < 2)
	{
		fprintf(stderr, "Kasutus: %s [info fail.ini] ([v2ljund-pilt.svg])\n", argv[0]);
		return 1;
	}

	dataModel_t dm;
	dmErr_t code = dm_initDataFile(&dm, argv[1]);
	const size_t totalStops = dm.numMidPoints + 2;

	if (code != dmeOK)
	{
		fprintf(stderr, "Viga andmete laadimisel!\n");
		return 1;
	}

	// Väljastab vahepunktid
	/*for (size_t i = 0; i < dm.numRoads; ++i)
	{
		if (dm.roads[i] != NULL)
		{
			line_t * tee = dm.roads[i];
			printf(
				"Tee %s: %s, %zu: (%f, %f) -> %s, %zu: (%f, %f)\n",
				tee->id.str,
				tee->src->id.str, tee->src->idx, (double)tee->src->x, (double)tee->src->y,
				tee->dst->id.str, tee->dst->idx, (double)tee->dst->x, (double)tee->dst->y
			);
		}
	}*/

	if (!dm_createMatrices(&dm))
	{
		fprintf(stderr, "Maatriksite genereerimine nurjus!\n");
		return 1;
	}


	/*size_t maxplen = 0;
	for (size_t i = 0; i < dm.numJunctions; ++i)
	{
		maxplen = mh_zmax(maxplen, dm.juncPoints[i]->id.len);
	}

	printf("Relatsioonide maatriks:\n");
	printf("%*c ", maxplen, ' ');
	for (size_t i = 0; i < dm.numJunctions; ++i)
	{
		printf("%*s ", maxplen, dm.juncPoints[i]->id.str);
	}
	putchar('\n');
	for (size_t i = 0; i < dm.numJunctions; ++i)
	{
		printf("%*s ", maxplen, dm.juncPoints[i]->id.str);
		for (size_t j = 0; j < dm.numJunctions; ++j)
		{
			printf(
				"%*c%c",
				maxplen - 1,
				' ',
				pf_bGet(dm.relations, pf_calcIdx(i, j, dm.numJunctions)) ? '1' : '0'
			);
			if ((j + 1) < dm.numJunctions)
			{
				putchar(' ');
			}
		}
		putchar('\n');
	}

	printf("\"Hindade\" maatriks:\n");
	printf("%*c ", 4, ' ');
	for (size_t i = 0; i < dm.numJunctions; ++i)
	{
		printf("%*s ", 4, dm.juncPoints[i]->id.str);
	}
	putchar('\n');
	for (size_t i = 0; i < dm.numJunctions; ++i)
	{
		printf("%*s ", 4, dm.juncPoints[i]->id.str);
		for (size_t j = 0; j < dm.numJunctions; ++j)
		{
			printf("%4.0f ", (double)dm.costs[pf_calcIdx(i, j, dm.numJunctions)]);
		}
		putchar('\n');
	}



	printf("Peatuste vaheliste \"hindade\" maatriks:\n");
	printf("%8c ", ' ');
	for (size_t i = 0; i < totalStops; ++i)
	{
		printf("%8s ", dm.points[i].id.str);
	}
	putchar('\n');
	for (size_t i = 0; i < totalStops; ++i)
	{
		distActual_t * row = &dm.stopsDistMatrix[i * totalStops];
		printf("%8s ", dm.points[i].id.str);
		for (size_t j = 0; j < totalStops; ++j)
		{
			printf("%8.3f ", (double)row[j].dist);
		}
		putchar('\n');
	}

	printf("Peatuste vaheliste tegelike kauguste maatriks:\n");
	printf("%8c ", ' ');
	for (size_t i = 0; i < totalStops; ++i)
	{
		printf("%8s ", dm.points[i].id.str);
	}
	putchar('\n');
	for (size_t i = 0; i < totalStops; ++i)
	{
		distActual_t * row = &dm.stopsDistMatrix[i * totalStops];
		printf("%8s ", dm.points[i].id.str);
		for (size_t j = 0; j < totalStops; ++j)
		{
			printf("%8.3f ", (double)row[j].actual);
		}
		putchar('\n');
	}*/

	printf("Optimaalse teekonna leidmine...\n");

	if (!dm_findShortestPath(&dm))
	{
		fprintf(stderr, "Viga optimaalse teekonna leidmisel!\n");
		return 1;
	}

	printf("Parim peatuste l2bimise j2rjekord:\n");

	double total = 0.0;
	size_t maxslen = 0;
	for (size_t i = 0; i < totalStops; ++i)
	{
		maxslen = mh_zmax(maxslen, dm.points[i].id.len);
	}
	for (size_t i = 0, n_1 = totalStops - 1; i < n_1; ++i)
	{
		const double dist = (double)dm.stopsDistMatrix[pf_calcIdx(dm.bestStopsIndices[i], dm.bestStopsIndices[i + 1], totalStops)].actual;
		total += dist;
		printf(
			"%*s -> %*s: +%.3f km, hetkel kokku: %.3f km\n",
			maxslen, dm.points[dm.bestStopsIndices[i]].id.str,
			maxslen, dm.points[dm.bestStopsIndices[i + 1]].id.str,
			dist / 1000.0, total / 1000.0
		);
	}
	printf("Teekond kokku: %.3f km\n", total / 1000.0);


	printf("Teekond pikalt:\n");
	for (size_t i = 0; i < dm.shortestPathLen; ++i)
	{
		maxslen = mh_zmax(maxslen, dm.shortestPath[i]->id.len);
	}
	total = 0.0;
	for (size_t i = 0, n_1 = dm.shortestPathLen - 1; i < n_1; ++i)
	{
		const point_t * p1 = dm.shortestPath[i], * p2 = dm.shortestPath[i + 1];
		const double dx = (double)(p1->x - p2->x), dy = (double)(p1->y - p2->y);
		const double dist = sqrt((dx * dx) + (dy * dy));
		total += dist;
		printf(
			"%*s -> %*s: +%.3f km, hetkel kokku: %.3f km\n",
			maxslen, p1->id.str,
			maxslen, p2->id.str,
			dist / 1000.0, total / 1000.0
		);
	}
	putchar('\n');


	if (argc > 2)
	{
		// SVG failinimi on antud
		FILE * fsvg = fopen(argv[2], "w");
		if (fsvg == NULL)
		{
			fprintf(stderr, "SVG faili avamine eba6nnestus!\n");
			return 1;
		}
		printf("SVG faili kirjutamine...\n");
		
		if (!dm_writeSvg(&dm, fsvg))
		{
			fprintf(stderr, "SVG faili kirjutamine nurjus!\n");
			return 1;
		}

		fclose(fsvg);

		printf("Valmis: %s\n", argv[2]);
	}


	dm_destroy(&dm);

	clock_t stopTime = clock();
	clock_t elapsed = stopTime - startTime;

	printf("Ajakulu: %.2f sekundit.\n", (double)elapsed / (double)CLOCKS_PER_SEC);

	return 0;
}
