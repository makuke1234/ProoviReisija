#include "iniFile.h"
#include "dataModel.h"
#include "logger.h"

#include <stdio.h>

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
				"Tee %s: %s: (%f, %f) -> %s: (%f, %f)\n",
				tee->id.str,
				tee->src->id.str, (double)tee->src->x, (double)tee->src->y,
				tee->dst->id.str, (double)tee->dst->x, (double)tee->dst->y
			);
		}
	}

	dm_destroy(&dm);

	return 0;
}
