#include "test.h"
#include "../src/iniFile.h"

void testData(const char * data, bool pass)
{
	IniE_t code = ini_checkData(data, -1);
	test(pass ? (code == IniE_OK) : (code != IniE_OK), "Input: %s, return code: %d", data, code);
}

int main(void)
{
	setlib("ini_check");

	testData("key=value", true);
	testData("key value", false);
	testData("[section]\nkey1 = a\nkey2 = b\n; comment text\n", true);
	testData("[section]\ndomain = wikipedia.org\n\n[section.subsection]\nfoo = bar", true);
	testData("var = a\t; This is an inline comment\nfoo = bar\t# This is another inline comment", true);
	testData("[section]key = domain", true);
	testData("[section key = domain", false);

	return 0;
}
