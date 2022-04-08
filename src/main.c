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
	float * costs = NULL;
	const point_t ** points = NULL;
	bool result = pf_createRelationsCosts(
		&relations,
		&costs,
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
		printf("%*s ", maxplen, points[i]->id.str);
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

	printf("\"Hindade\" maatriks:\n");
	printf("%*c ", 4, ' ');
	for (size_t i = 0; i < numRelations; ++i)
	{
		printf("%*s ", 4, points[i]->id.str);
	}
	putchar('\n');
	for (size_t i = 0; i < numRelations; ++i)
	{
		printf("%*s ", 4, points[i]->id.str);
		for (size_t j = 0; j < numRelations; ++j)
		{
			printf("%4.0f ", (double)costs[pf_calcIdx(i, j, numRelations)]);
		}
		putchar('\n');
	}


	const size_t totalStops = dm.numMidPoints + 2;

	distActual_t * matrix = NULL;
	result = pf_makeDistMatrix(
		dm.pointsp,
		&matrix,
		totalStops,
		(const line_t * const *)dm.teed,
		dm.numTeed
	);
	if (!result)
	{
		fprintf(stderr, "Viga Dijkstra algoritmi t55s!\n");
		return 1;
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
		distActual_t * row = &matrix[i * totalStops];
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
		distActual_t * row = &matrix[i * totalStops];
		printf("%8s ", dm.points[i].id.str);
		for (size_t j = 0; j < totalStops; ++j)
		{
			printf("%8.3f ", (double)row[j].actual);
		}
		putchar('\n');
	}

	printf("Permutatsioonide l2biproovimine...\n");

	size_t * bestIndexes = NULL;
	result = pf_findOptimalMatrixOrder(
		matrix,
		totalStops,
		0,
		1,
		&bestIndexes
	);

	if (!result)
	{
		fprintf(stderr, "Viga optimaalse j2rjekorra leidmisel!\n");
		return 1;
	}

	printf("Parim peatuste l2bimise j2rjekord:\n");
	for (size_t i = 0; i < totalStops; ++i)
	{
		printf("%s", dm.points[bestIndexes[i]].id.str);
		if (i < (totalStops - 1))
		{
			printf(" -> ");
		}
	}
	putchar('\n');
	float total = 0.0f;
	for (size_t i = 0, n_1 = totalStops - 1; i < n_1; ++i)
	{
		total += matrix[pf_calcIdx(bestIndexes[i], bestIndexes[i + 1], totalStops)].actual;
	}
	printf("Teekond kokku: %.3f km\n", (double)total / 1000.0);

	if (argc > 2)
	{
		// SVG failinimi on antud
		FILE * fsvg = fopen(argv[2], "w");
		if (fsvg == NULL)
		{
			fprintf(stderr, "SVG faili avamine eba6nnestus!\n");
		}
		else
		{
			printf("SVG faili kirjutamine...\n");
			float maxw = -INFINITY, minw = INFINITY, maxh = -INFINITY, minh = INFINITY;
			for (size_t i = 0; i < numRelations; ++i)
			{
				const float px = points[i]->x, py = points[i]->y;
				maxw = mh_fmaxf(maxw, px);
				minw = mh_fminf(minw, px);
				maxh = mh_fmaxf(maxh, py);
				minh = mh_fminf(minh, py);
			}

			svg_init();
			svg_setFont("Hermit");
			svg_header(fsvg, (int64_t)minw, (int64_t)maxh, (size_t)(maxw - minw), (size_t)(maxh - minh), svg_rgba32(0xCCCCCCFF));

			svgRGB_t color = svg_rgba32(0x888888FF);

			for (size_t i = 0; i < dm.numTeed; ++i)
			{
				svg_line(fsvg, dm.teed[i], color);
			}

			// Joonistab lühima teekonna
			color = svg_rgba32(0xFF1111FF);
			size_t pathLen;
			const point_t ** path = NULL;
			result = pf_generateShortestPath(
				&path,
				&pathLen,
				bestIndexes,
				dm.pointsp,
				totalStops,
				points,
				relations,
				costs,
				numRelations
			);
			if (!result)
			{
				fprintf(stderr, "Lyhima teekonna genereerimine nurjus!\n");
				return 1;
			}
			
			printf("Teekond pikalt:\n");

			hashMapCK_t spmap;
			if (!hashMap_init(&spmap, totalStops))
			{
				fprintf(stderr, "R2sitabeli initsialiseerimine eba6nnestus!\n");
				return 1;
			}

			for (size_t i = 0; i < totalStops; ++i)
			{
				if (!hashMapCK_insert(&spmap, dm.pointsp[i]->id.str, NULL))
				{
					fprintf(stderr, "Elemendi lisamine r2sitabelisse nurjus!\n");
					return 1;
				}
			}

			for (size_t i = 0; i < (pathLen - 1); ++i)
			{
				line_t l = {
					.src = path[i],
					.dst = path[i + 1]
				};

				if (hashMapCK_get(&spmap, l.dst->id.str) == NULL)
				{
					svg_linePoint(fsvg, &l, color, false);
				}
				else
				{
					svg_line(fsvg, &l, color);
				}

				printf("%s -> ", path[i]->id.str);
			}
			printf("%s\n", path[pathLen - 1]->id.str);

			hashMapCK_destroy(&spmap);

			free(path);

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
				size_t idx = bestIndexes[i];
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
	}

	free(points);
	free(relations);
	free(costs);
	free(matrix);
	free(bestIndexes);


	dm_destroy(&dm);

	return 0;
}
