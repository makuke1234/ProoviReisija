#include "test.h"
#include "../src/iniFile.h"

void testData(const char * data, bool pass)
{
	IniE_t code = ini_checkData(data, -1);
	test(pass ? (code == IniE_OK) : (code != IniE_OK), "Input: %s, return code: %d", data, code);
}

int main(void)
{
	setlib("iniFile_check");

	testData("key=value", true);
	testData("key value", false);	

	return 0;
}
