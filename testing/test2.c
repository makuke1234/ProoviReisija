#include "test.h"
#include "../src/iniFile.h"

#include <string.h>
#include <stdlib.h>

void testEsc(const char * str1, const char * str2, char * (*escapeFunc)(const char * restrict str1, intptr_t length))
{
	char * str = escapeFunc(str1, -1);
	test(str != NULL && strcmp(str, str2) == 0, "Sisend: %s; V2ljund: %s; Soovitud v2ljund: %s", str1, str, str2);
	free(str);
}

int main(void)
{
	setlib("ini_escape");

	testEsc("asdasdffsgjfgjDSDDFDFaksdj\u2191", "asdasdffsgjfgjDSDDFDFaksdj\u2191", &ini_escapeStr);
	testEsc("asdasd", "asdasd", &ini_unescapeStr);

	testEsc("Hello world\\n!", "Hello world\n!", &ini_escapeStr);
	testEsc("Hello world\n!", "Hello world\\n!", &ini_unescapeStr);
	testEsc("Hello world\\x0020\\n!", "Hello world \n!", &ini_escapeStr);
	testEsc("Hello world\\x2140\\n!", "Hello world\u2140\n!", &ini_escapeStr);
	testEsc("Hello world\\x2141\\n!", "Hello world\u2141\n!", &ini_escapeStr);

	uint16_t value;
	value = ini_str_codePointFromStr("2191");
	test(value == 0x2191, "Expected 0x2191, got 0x%X!", value);

	testEsc("Hello world\\x2191\\n!", "Hello world\u2191\n!", &ini_escapeStr);
	testEsc("Hello world\u2191\n!", "Hello world\u2191\\n!", &ini_unescapeStr);

	return 0;
}
