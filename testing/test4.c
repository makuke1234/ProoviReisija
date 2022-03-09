#include "test.h"
#include "../src/dataModel.h"


int main(void)
{
	setlib("dataModel");

	dataModel_t dm;

	test(dm_initDataFile(&dm, "test.ini") == dmeOK, "Data reading failed!");

	teststr(dm.beginning.id.str, "p0");

	dm_destroy(&dm);

	return 0;
}