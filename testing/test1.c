#include "test.h"
#include "../src/fileHelper.h"

#include <string.h>

#define TEST_STRING "this is CRLF test\n\n\nFourth line\nFIfth & the last."

int main(void)
{
	setlib("fileHelper");

	fhelper_write("test.txt", TEST_STRING, -1);
	fhelper_writeBin("test.bin.txt", TEST_STRING, strlen(TEST_STRING));

	size_t txtSuurus;
	char * txt = fhelper_read("test.txt", &txtSuurus);
	test(txt != NULL && strcmp(TEST_STRING, txt) == 0, "Tekstifaili kirjutamise viga! Tekst: %s", txt);	
	free(txt);

	size_t txtBinSuurus;
	char * txtBin = fhelper_readBin("test.bin.txt", &txtBinSuurus);
	test(txtBin != NULL && strncmp(TEST_STRING, txtBin, txtBinSuurus) == 0, "Binaarse faili kirjutamise viga!");
	free(txtBin);

	return 0;
}
