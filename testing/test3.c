#include "test.h"
#include "../src/iniFile.h"

#include <string.h>

void testData(const char * data, bool pass, void (*testfunc)(ini_t * ptr))
{
	IniE_t code = ini_checkData(data, -1);
	test(pass ? (code == IniE_OK) : (code != IniE_OK), "Input: %s, return code: %d", data, code);
	if (pass)
	{
		ini_t ini;
		test(ini_initData(data, -1, &ini) == IniE_OK, "INI structure initialization failed!");

		// More tests
		testfunc(&ini);

		// Free resources
		ini_destroy(&ini);
	}
}

void t1(ini_t * ptr)
{
	IniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	IniValue_t * val = IniSection_getValue(sect, "key");
	test(val != NULL, "No expected key \"key\"!");

	test(strncmp(val->key.str, "key", val->key.len) == 0, "Key doesn't match real data!");
	test(val->key.len == 3, "Key length is incorrect!");

	test(strncmp(val->value.str, "value", val->value.len) == 0, "Value doesn't match real data!");
	test(val->value.len == 5, "Value length is incorrect!");
}
void t2(ini_t * ptr)
{

}
void t3(ini_t * ptr)
{

}
void t4(ini_t * ptr)
{

}
void t5(ini_t * ptr)
{

}

int main(void)
{
	setlib("ini_check");

	testData("key=value", true, &t1);
	testData("key value", false, NULL);
	testData("[section]\nkey1 = a\nkey2 = b\n; comment text\n", true, &t2);
	testData("[section]\ndomain = wikipedia.org\n\n[section.subsection]\nfoo = bar", true, &t3);
	testData("var = a\t; This is an inline comment\nfoo = bar\t# This is another inline comment", true, &t4);
	testData("[section]key = domain", true, &t5);
	testData("[section key = domain", false, NULL);

	return 0;
}
