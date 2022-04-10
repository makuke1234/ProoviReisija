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

	if (!dm_createMatrices(&dm))
	{
		fprintf(stderr, "Maatriksite genereerimine nurjus!\n");
		return 1;
	}


	printf("Optimaalse teekonna leidmine...\n");

	if (!dm_findShortestPath(&dm))
	{
		fprintf(stderr, "Viga optimaalse teekonna leidmisel!\n");
		return 1;
	}

	printf("Parim peatuste l2bimise j2rjekord:\n");

	double total = 0.0;
	size_t maxStrlen = 0;
	for (size_t i = 0; i < totalStops; ++i)
	{
		maxStrlen = mh_zmax(maxStrlen, dm.points[i].id.len);
	}
	for (size_t i = 0, n_1 = totalStops - 1; i < n_1; ++i)
	{
		const double dist = (double)dm.stopsDistMatrix[pf_calcIdx(dm.bestStopsIndices[i], dm.bestStopsIndices[i + 1], totalStops)].actual;
		total += dist;
		printf(
			"%*s -> %*s: +%.3f km, hetkel kokku: %.3f km\n",
			maxStrlen, dm.points[dm.bestStopsIndices[i]].id.str,
			maxStrlen, dm.points[dm.bestStopsIndices[i + 1]].id.str,
			dist / 1000.0, total / 1000.0
		);
	}
	printf("Teekond kokku: %.3f km\n", total / 1000.0);


	printf("Teekond pikalt:\n");
	for (size_t i = 0; i < dm.shortestPathLen; ++i)
	{
		maxStrlen = mh_zmax(maxStrlen, dm.shortestPath[i]->id.len);
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
			maxStrlen, p1->id.str,
			maxStrlen, p2->id.str,
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
