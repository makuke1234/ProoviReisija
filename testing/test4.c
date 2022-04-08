#include "test.h"
#include "../src/dataModel.h"
#include "../src/logger.h"

int main(void)
{
	initLogger();

	setlib("dataModel");

	dataModel_t dm;

	dmErr_t code = dm_initDataFile(&dm, "test.ini");
	test(code == dmeOK, "Data reading failed with code %d!", code);

	teststr(dm.beg.id.str, "p0");
	teststr(dm.end.id.str, "p2");
	test(dm.numMidPoints == 1, "%zu middle points exist!", dm.numMidPoints);

	// Väljastab vahepunktid
	for (size_t i = 0; i < dm.numRoads; ++i)
	{
		if (dm.roads[i] != NULL)
		{
			line_t * tee = dm.roads[i];
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
