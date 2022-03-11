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
	teststr(dm.end.id.str, "p1");
	test(dm.numMidPoints == 0, "%zu middle points exist!", dm.numMidPoints);

	// Väljastab vahepunktid
	for (size_t i = 0; i < dm.numTeed; ++i)
	{
		if (dm.teed[i] != NULL)
		{
			line_t * tee = dm.teed[i];
			printf(
				"Tee %s: %s: (%f, %f) -> %s: (%f, %f)\n",
				tee->id.str,
				tee->src->id.str, tee->src->x, tee->src->y,
				tee->dst->id.str, tee->dst->x, tee->dst->y
			);
		}
	}

	dm_destroy(&dm);

	return 0;
}
