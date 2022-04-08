#include "iniFile.h"
#include "dataModel.h"
#include "logger.h"
#include "pathFinding.h"
#include "mathHelper.h"
#include "svgWriter.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main(int argc, char ** argv)
{
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
	for (size_t i = 0; i < dm.numRoads; ++i)
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
	}

	if (!dm_createMatrices(&dm))
	{
		fprintf(stderr, "Maatriksite genereerimine nurjus!\n");
		return 1;
	}

	// Koostab maatriksi lühimatest kaugustest punktide vahel

	size_t maxplen = 0;
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
	}

	printf("Optimaalse teekonna leidmine...\n");

	if (!dm_findShortestPath(&dm))
	{
		fprintf(stderr, "Viga optimaalse teekonna leidmisel!\n");
		return 1;
	}

	printf("Parim peatuste l2bimise j2rjekord:\n");
	for (size_t i = 0; i < totalStops; ++i)
	{
		printf("%s", dm.points[dm.bestStopsIndices[i]].id.str);
		if (i < (totalStops - 1))
		{
			printf(" -> ");
		}
	}
	putchar('\n');
	float total = 0.0f;
	for (size_t i = 0, n_1 = totalStops - 1; i < n_1; ++i)
	{
		total += dm.stopsDistMatrix[pf_calcIdx(dm.bestStopsIndices[i], dm.bestStopsIndices[i + 1], totalStops)].actual;
	}
	printf("Teekond kokku: %.3f km\n", (double)total / 1000.0);

	printf("Teekond pikalt:\n");

	for (size_t i = 0, n_1 = dm.shortestPathLen - 1; i < n_1; ++i)
	{
		printf("%s ", dm.shortestPath[i]->id.str);
	}
	printf("%s\n", dm.shortestPath[dm.shortestPathLen - 1]->id.str);


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
		
		float maxw = -INFINITY, minw = INFINITY, maxh = -INFINITY, minh = INFINITY;
		for (size_t i = 0; i < dm.numJunctions; ++i)
		{
			const float px = dm.juncPoints[i]->x, py = dm.juncPoints[i]->y;
			maxw = mh_fmaxf(maxw, px);
			minw = mh_fminf(minw, px);
			maxh = mh_fmaxf(maxh, py);
			minh = mh_fminf(minh, py);
		}

		svg_init();
		svg_setFont("Hermit");
		svg_header(fsvg, (int64_t)minw, (int64_t)maxh, (size_t)(maxw - minw), (size_t)(maxh - minh), svg_rgba32(0xCCCCCCFF));

		svgRGB_t color = svg_rgba32(0x888888FF);

		for (size_t i = 0; i < dm.numRoads; ++i)
		{
			svg_line(fsvg, dm.roads[i], color);
		}

		// Joonistab lühima teekonna
		color = svg_rgba32(0xFF1111FF);
		
		for (size_t i = 0, n_1 = dm.shortestPathLen - 1; i < n_1; ++i)
		{
			line_t l = {
				.src = dm.shortestPath[i],
				.dst = dm.shortestPath[i + 1]
			};

			if (hashMapCK_get(&dm.stopsMap, l.dst->id.str) == NULL)
			{
				svg_linePoint(fsvg, &l, color, false);
			}
			else
			{
				svg_line(fsvg, &l, color);
			}

		}

		svgRGB_t blue = svg_rgba32(0x00A2E8FF);

		line_t line = {
			.src = &dm.beg,
			.dst = dm.begp
		};
		svg_line(fsvg, &line, blue);
		line = (line_t){
			.src = dm.endp,
			.dst = &dm.end
		};
		svg_line(fsvg, &line, blue);

		point_t p = dm.beg;
		p.id.str = "Start";
		svg_point(fsvg, &p, blue);
		p = dm.end;
		p.id.str = "Finiš";
		svg_point(fsvg, &p, blue);

		for (size_t i = 0; i < totalStops; ++i)
		{
			size_t idx = dm.bestStopsIndices[i];
			if ((idx != 0) && (idx != 1))
			{
				line = (line_t){
					.src = dm.pointsp[idx],
					.dst = &dm.points[idx]
				};
				svg_line(fsvg, &line, blue);
				svg_point(fsvg, &dm.points[idx], blue);
			}

			char temp[10];
			_ultoa((unsigned long)i + 1, temp, 10);
			strcpy(&temp[strlen(temp)], ".");
			svg_text(fsvg, dm.points[idx].x, dm.points[idx].y, temp, svgBase_central, svgAlign_middle);
		}


		svg_footer(fsvg);
		fclose(fsvg);

		printf("Valmis: %s\n", argv[2]);
	}


	dm_destroy(&dm);

	return 0;
}
