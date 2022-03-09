#include "test.h"
#include "../src/iniFile.h"

#include <string.h>

#define FILE_DATA \
	"; last modified 1 April 2001 by John Doe\n" \
	"[owner]\n" \
	"name = John Doe\n" \
	"organization = Acme Widgets Inc.\n" \
	"\n" \
	"[database]\n" \
	"; use IP address in case network name resolution is not working\n" \
	"server = 192.0.2.62     \n" \
	"port = 143\n" \
	"file = \"payroll.dat\""


void testkv(iniSection_t * sect, const char * key, const char * value, bool outcome)
{
	iniValue_t * v = iniSection_getValue(sect, key);
	test(outcome ? (v != NULL) : (v == NULL), outcome ? "No key %s found!" : "Key %s found in wrong place!", key);
	if (!outcome)
	{
		return;
	}
	
	test(v->key.str != NULL && strncmp(v->key.str, key, v->key.len) == 0, "Key doesn't match! Data: %s", key, v->key.str);
	test(v->key.len == strlen(key), "Key length doesn't match! Data: %s", v->key.str);

	test(v->value.str != NULL && strncmp(v->value.str, value, v->value.len) == 0, "Value doesn't match \"%s\"! Data: %s", value, v->value.str);
	test(v->value.len == strlen(value), "Value length doesn't match! Data: %s", v->value.str);
}

void testData(const char * lib_, const char * data, bool pass, void (*testfunc)(ini_t * ptr))
{
	setlib(lib_);

	iniErr_t code = ini_checkData(data, -1);
	test(pass ? (code == inieOK) : (code != inieOK), "Input: %s, return code: %d", data, code);
	if (pass)
	{
		ini_t ini;
		test(ini_initData(data, -1, &ini) == inieOK, "INI structure initialization failed!");

		// More tests
		testfunc(&ini);

		// Free resources
		ini_destroy(&ini);
	}

	endphase();
}

void t1(ini_t * ptr)
{
	iniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	testkv(sect, "key", "value", true);
}
void t2(ini_t * ptr)
{
	iniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	sect = ini_getSection(ptr, "section");
	test(sect != NULL, "No section [section] found!");

	testkv(sect, "key1", "a", true);
	testkv(sect, "key2", "b", true);
}
void t3(ini_t * ptr)
{
	iniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	sect = ini_getSection(ptr, "section");
	test(sect != NULL, "No section [section] found!");

	testkv(sect, "domain", "wikipedia.org", true);
	testkv(sect, "foo", "bar", false);

	sect = ini_getSection(ptr, "section.subsection");
	test(sect != NULL, "No section [section.subsection] found!");

	testkv(sect, "foo", "bar", true);
	testkv(sect, "domain", "wikipedia.org", false);
}
void t4(ini_t * ptr)
{
	iniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	testkv(sect, "var", "abc", true);
	testkv(sect, "foo", "bar", true);
}
void t5(ini_t * ptr)
{
	iniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");

	testkv(sect, "key", "domain", false);

	sect = ini_getSection(ptr, "section");
	test(sect != NULL, "No section \"section\" found!");

	testkv(sect, "key", "domain", true);
}
void t6(ini_t * ptr)
{
	iniSection_t * sect = ini_getSection(ptr, "");
	test(sect != NULL, "No global section found!");
	
	testkv(sect, "name", "John Doe", false);

	sect = ini_getSection(ptr, "owner");
	test(sect != NULL, "No \"owner\" section!");

	testkv(sect, "name", "John Doe", true);
	testkv(sect, "organization", "Acme Widgets Inc.", true);
	testkv(sect, "server", "192.0.2.62", false);

	sect = ini_getSection(ptr, "database");
	test(sect != NULL, "No \"database\" section!");

	testkv(sect, "name", "John Doe", false);
	testkv(sect, "server", "192.0.2.62", true);
	testkv(sect, "port", "143", true);
	testkv(sect, "file", "payroll.dat", true);
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
	testData("iniFile t6", FILE_DATA, true, &t6);

	return 0;
}
