#include "test.h"
#include "../src/iniFile.h"

#include <string.h>

void testData(const char * lib_, const char * data, bool pass, void (*testfunc)(ini_t * ptr))
{
	setlib(lib_);

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
	test(val->key.len == 3, "Key length is incorrect!, data: %s", val->key.str);

	test(strncmp(val->value.str, "value", val->value.len) == 0, "Value doesn't match real data!, data: %s", val->value.str);
	test(val->value.len == 5, "Value length is incorrect!, data: %s", val->value.str);
}
void t2(ini_t * ptr)
{
	IniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	sect = ini_getSection(ptr, "section");
	test(sect != NULL, "No section [section] found!");

	IniValue_t * val = IniSection_getValue(sect, "key1");
	test(val != NULL, "No expected key \"key1\"!");

	test(strncmp(val->key.str, "key1", val->key.len) == 0, "Key doesn't match real data!");
	test(val->key.len == 4, "Key length is incorrect!, data: %s", val->key.str);

	test(strncmp(val->value.str, "a", val->value.len) == 0, "Value doesn't match real data!, data: %s", val->value.str);
	test(val->value.len == 1, "Value length is incorrect!, data: %s", val->value.str);
}
void t3(ini_t * ptr)
{
	IniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	sect = ini_getSection(ptr, "section");
	test(sect != NULL, "No section [section] found!");

	IniValue_t * val = IniSection_getValue(sect, "domain");
	test(val != NULL, "No expected key \"domain\"!");

	test(strncmp(val->key.str, "domain", val->key.len) == 0, "Key doesn't match real data!");
	test(val->key.len == 6, "Key length is incorrect!, data: %s", val->key.str);

	test(strncmp(val->value.str, "wikipedia.org", val->value.len) == 0, "Value doesn't match real data!, data: %s", val->value.str);
	test(val->value.len == 13, "Value length is incorrect!, data: %s", val->value.str);

	sect = ini_getSection(ptr, "section.subsection");
	test(sect != NULL, "No section [section.subsection] found!");

	val = IniSection_getValue(sect, "foo");
	test(val != NULL, "No excpected key \"foo\"!");

	test(strncmp(val->key.str, "foo", val->key.len) == 0, "Key doesn't match real data!");
	test(val->key.len == 3, "Key length is incorrect!, data: %s", val->key.str);

	test(strncmp(val->value.str, "bar", val->value.len) == 0, "Value doesn't match real data!, data: %s", val->value.str);
	test(val->value.len == 3, "Value length is incorrect!, data: %s", val->value.str);
}
void t4(ini_t * ptr)
{
	IniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	IniValue_t * val = IniSection_getValue(sect, "var");
	test(val != NULL, "No expected key \"var\"!");

	test(strncmp(val->key.str, "var", val->key.len) == 0, "Key doesn't match real data!");
	test(val->key.len == 3, "Key length is incorrect!, data: %s", val->key.str);

	test(strncmp(val->value.str, "abc", val->value.len) == 0, "Value doesn't match real data!, data: %s", val->value.str);
	test(val->value.len == 3, "Value length is incorrect!, data: %s", val->value.str);

	val = IniSection_getValue(sect, "foo");
	
	test(strncmp(val->key.str, "foo", val->key.len) == 0, "Key doesn't match real data!");
	test(val->key.len == 3, "Key length is incorrect!, data: %s", val->key.str);

	test(strncmp(val->value.str, "bar", val->value.len) == 0, "Value doesn't match real data!, data: %s", val->value.str);
	test(val->value.len == 3, "Value length is incorrect!, data: %s", val->value.str);
}
void t5(ini_t * ptr)
{
	IniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	IniValue_t * val = IniSection_getValue(sect, "key");
	test(val == NULL, "Key \"key\" found in wrong section!");

	sect = ini_getSection(ptr, "section");
	test(sect != NULL, "No section \"section\" found!");

	val = IniSection_getValue(sect, "key");
	test(val != NULL, "No key \"key\" found!");


	test(strncmp(val->key.str, "key", val->key.len) == 0, "Key doesn't match!");
	test(val->key.len == 3, "Key length incorrect!, data: %s", val->key.str);

	test(strncmp(val->value.str, "domain", val->value.len) == 0, "Value doesn't match!, data: %s", val->value.str);
	test(val->value.len == 6, "Value length incorrect!, data: %s", val->value.str);
}

int main(void)
{
	setlib("iniFile");

	testData("iniFile t1", "key=value", true, &t1);
	testData("iniFile tx", "key value", false, NULL);
	testData("iniFile t2", "[section]\nkey1 = a\nkey2 = b\n; comment text\n", true, &t2);
	testData("iniFile t3", "[section]\ndomain = wikipedia.org\n\n[section.subsection]\nfoo = bar", true, &t3);
	testData("iniFile t4", "var = abc\t; This is an inline comment\nfoo = bar\t# This is another inline comment", true, &t4);
	testData("iniFile t5", "[section]key = domain", true, &t5);
	testData("iniFile tx", "[section key = domain", false, NULL);

	return 0;
}
