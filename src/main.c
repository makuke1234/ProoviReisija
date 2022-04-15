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

	// Logger initsialiseeritakse, avatakse logifail, see toimub ainult debug-režiimis kompileerides
	initLogger();

	if (argc < 2)
	{
		fprintf(stderr, "Kasutus: %s [info fail.ini] ([v2ljund-pilt.svg])\n", argv[0]);
		return 1;
	}

	// Andmed loetakse failist sisse
	dataModel_t dm;
	dmErr_t code = dm_initDataFile(&dm, argv[1]);
	const size_t totalStops = dm.numMidPoints + 2;

	if (code != dmeOK)
	{
		fprintf(stderr, "Viga andmete laadimisel!\n");
		return 1;
	}

	// Tehakse naabrusmaatriks, teede "hindade" maatriks ning ristmike/punktide massiiv
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

	// Leitakse maksimaalne teekonnapunkti nimetuse pikkus tähemärkides
	size_t maxStrlen = 0;
	for (size_t i = 0; i < totalStops; ++i)
	{
		maxStrlen = mh_zmax(maxStrlen, dm.points[i].id.len);
	}
	// Kuvatakse teekond
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
	// Leitakse jälle maks. pikema teekonna teekonnapunkti nimetuse pikkus tähemärkides
	for (size_t i = 0; i < dm.shortestPathLen; ++i)
	{
		maxStrlen = mh_zmax(maxStrlen, dm.shortestPath[i]->id.len);
	}
	// Koguteekond nullitakse uuesti ära
	total = 0.0;
	for (size_t i = 0, n_1 = dm.shortestPathLen - 1; i < n_1; ++i)
	{
		const point_t * p1 = dm.shortestPath[i], * p2 = dm.shortestPath[i + 1];
		// Arvutatakse reaalne kauguselisa kahe teekonnapunkti vahel kasutades
		// selleks ristkoordinaatide muutudele rakendatud Pythagorase teoreemi
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
		// SVG failinimi on antud, avatakse tekstikirjutamise režiimis, sest SVG on sarnane
		// teistele XML-perekonna keeltele nagu näiteks HTML
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

	// Andmemudel vabastatakse
	dm_destroy(&dm);


	// Kuvatakse programmi tööle kulunud aega sekundites sajandiksekundi täpsusega
	clock_t stopTime = clock();
	clock_t elapsed = stopTime - startTime;

	printf("Ajakulu: %.2f sekundit.\n", (double)elapsed / (double)CLOCKS_PER_SEC);

	return 0;
}
