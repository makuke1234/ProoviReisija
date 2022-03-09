#include "test.h"
#include "../src/dataModel.h"
#include "../src/logger.h"

int main(void)
{
	initLogger();

	setlib("dataModel");

	dataModel_t dm;

	test(dm_initDataFile(&dm, "test.ini") == dmeOK, "Data reading failed!");

	teststr(dm.beg.id.str, "p0");
	teststr(dm.end.id.str, "p1");
	test(dm.numMidPoints == 0, "%zu middle points exist!", dm.numMidPoints);

	dm_destroy(&dm);

	return 0;
}
