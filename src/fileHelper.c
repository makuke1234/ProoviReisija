#include "fileHelper.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

intptr_t fhelper_fileSize(const char * restrict fileName)
{
	struct stat fileStatus;
	if (stat(fileName, &fileStatus) < 0)
	{
		return -1;
	}
	return (intptr_t)fileStatus.st_size;
}

char * fhelper_read(const char * restrict fileName, size_t * restrict resultLength)
{
	assert(fileName != NULL);
	assert(resultLength != NULL);
	
	// Avatakse soovitud fail tekstilugemise režiimis, kontrollitakse õnnestumist
	FILE * file = fopen(fileName, "r");
	if (file == NULL)
	{
		writeLogger("Error opening file!");
		return NULL;
	}

	writeLogger("File open");

	// Saadakse teada faili suurus, tekstirežiimis. Vahe seisneb selles, et
	// tekstirežiimis emuleeritakse LF-realõppe isegi Windowsis CRLF-realõppudega
	fseek(file, 0, SEEK_END);
	long ftell_res = ftell(file);
	if (ftell_res == -1L)
	{
		fclose(file);
		return NULL;
	}
	size_t length = (size_t)ftell_res;
	rewind(file);

	writeLogger("File rewound, size: %zu characters", length);

	// Allokeerib mälu teksti ning lisanduva null-terminaatori jaoks, kontrollib õnnestumist
	char * content = malloc(length + 1);
	if (content == NULL)
	{
		// Kui ei õnnestunud mälu allokeerida, siis fail suletakse
		writeLogger("Error allocating memory!");
		fclose(file);
		return NULL;
	}
	// Sälitatakse failist loetud baitide arv
	size_t readBytes = fread(content, 1, length, file);
	fclose(file);

	writeLogger("Read %zu bytes, file closed", readBytes);

	// Kui suudeti mingil põhjusel oodatust vähem baite lugeda, siis proovitakse allokeeritud mälu hulka vähendada minimaalseks
	if (readBytes < length)
	{
		// Allokeeritud mälu hulka muudetakse võrdseks loetud info mahuga, arvestatakse ka null-terminaatori jaoks vajamineva ruumiga
		char * newmem = realloc(content, readBytes + 1);
		if (newmem != NULL)
		{
			content = newmem;
			writeLogger("Resized array to fit");
		}
	}

	// Loetud failisisule pannakse lõppu ka null-terminaator
	content[readBytes] = '\0';
	
	writeLogger("Added null-terminator");
	
	*resultLength = readBytes + 1;
	return content;
}
void * fhelper_readBin(const char * restrict fileName, size_t * restrict resultLength)
{
	assert(fileName != NULL);
	assert(resultLength != NULL);

	// Esmalt saadakse teada binaarfaili suurus, kontrollitakse operatsiooni õnnestumist
	intptr_t fileLen = fhelper_fileSize(fileName);
	if (fileLen == -1)
	{
		return NULL;
	}
	size_t sfileLen = (size_t)fileLen;

	// Avatakse fail binaarlugemise režiimis, kontrollitakse õnnestumist
	FILE * file = fopen(fileName, "rb");
	if (file == NULL)
	{
		return NULL;
	}

	// Faili sisu jaoks allokeeritakse mälu
	void * mem = malloc(sfileLen);
	if (mem == NULL)
	{
		fclose(file);
		return NULL;
	}

	// Binaarsisu loetakse failist mällu, fail suletakse
	*resultLength = fread(mem, sfileLen, 1, file);
	fclose(file);
	// Kui mingil põhjusel tervet faili ei õnnesutnud lugeda, siis optimeeritakse mälukasutust
	if (*resultLength < sfileLen)
	{
		void * newmem = realloc(mem, *resultLength);
		if (newmem != NULL)
		{
			mem = newmem;
		}
	}

	return mem;
}

intptr_t fhelper_write(const char * fileName, const char * string, intptr_t stringLength)
{
	assert(fileName != NULL);
	assert(string != NULL);

	// Avatakse fail tekstikirjutamise režiimis
	FILE * file = fopen(fileName, "w");
	if (file == NULL)
	{
		return -1;
	}

	// Jäetakse meelde kirjutada õnnestunud baitide arv, see tagastatakse kasutajale
	intptr_t writtenBytes = (intptr_t)fwrite(string, strnlen_s(string, (stringLength < 0) ? SIZE_MAX : (size_t)stringLength), 1, file);
	fclose(file);

	return writtenBytes;
}
intptr_t fhelper_writeBin(const char * fileName, const void * data, size_t dataLength)
{
	assert(fileName != NULL);
	assert(data != NULL);
	assert(dataLength > 0);

	// Soovitud fail avatakse binaarkirjutamise režiimis, kontrollitakse õnnestumist
	FILE * file = fopen(fileName, "wb");
	if (file == NULL)
	{
		return -1;
	}

	// Kirjutada õnnestunud baitide arv jäetakse meelde, see tagastatakse kasutajale
	intptr_t writtenBytes = (intptr_t)fwrite(data, dataLength, 1, file);
	fclose(file);

	return writtenBytes;
}
