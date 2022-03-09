#include "dataModel.h"

#include <stdlib.h>

bool point_initStr(point_t * restrict p, const char * restrict idstr, const char * restrict valuestr)
{
	assert(p != NULL);

	if (!iniString_init(&p->id, idstr, -1))
	{
		return false;
	}

	char * next = NULL;
	p->x = strtof(valuestr, &next);
	p->y = strtof(next, NULL);

	return true;
}
void point_destroy(point_t * restrict p)
{
	assert(p != NULL);
	iniString_destroy(&p->id);
}


bool dm_readData(dataModel_t * restrict dm, const char * restrict filename)
{
	ini_t inifile;
	if (!ini_initFile(filename, &inifile))
	{
		return false;
	}




	ini_destroy(&inifile);

	return true;
}
