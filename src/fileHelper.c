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
	
	FILE * file = fopen(fileName, "r");
	if (file == NULL)
	{
		writeLogger("Error opening file!");
		return NULL;
	}

	writeLogger("File open");

	fseek(file, 0, SEEK_END);
	long ftell_res = ftell(file);
	if (ftell_res == -1L)
	{
		fclose(file);
		return NULL;
	}
	size_t length = (size_t)ftell_res;
	rewind(file);

	writeLogger("File rewund, size %zu characters", length);

	char * content = malloc(length + 1);
	if (content == NULL)
	{
		fclose(file);
		return NULL;
	}
	size_t readBytes = fread(content, 1, length, file);
	fclose(file);

	writeLogger("Read %zu bytes, file closed", readBytes);

	if (readBytes < length)
	{
		char * newmem = realloc(content, readBytes + 1);
		if (newmem != NULL)
		{
			content = newmem;
		}
	}
	
	writeLogger("Resized array to fit");

	content[readBytes] = '\0';
	
	writeLogger("Added null-terminator");
	
	*resultLength = readBytes + 1;
	return content;
}
void * fhelper_readBin(const char * restrict fileName, size_t * restrict resultLength)
{
	assert(fileName != NULL);
	assert(resultLength != NULL);

	intptr_t fileLen = fhelper_fileSize(fileName);
	if (fileLen == -1)
	{
		return NULL;
	}
	size_t sfileLen = (size_t)fileLen;

	FILE * file = fopen(fileName, "rb");
	if (file == NULL)
	{
		return NULL;
	}

	void * mem = malloc(sfileLen);
	if (mem == NULL)
	{
		fclose(file);
		return NULL;
	}

	*resultLength = fread(mem, sfileLen, 1, file);
	fclose(file);
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

	FILE * file = fopen(fileName, "w");
	if (file == NULL)
	{
		return -1;
	}

	intptr_t writtenBytes = (intptr_t)fwrite(string, strnlen_s(string, (stringLength < 0) ? SIZE_MAX : (size_t)stringLength), 1, file);
	fclose(file);

	return writtenBytes;
}
intptr_t fhelper_writeBin(const char * fileName, const void * data, size_t dataLength)
{
	assert(fileName != NULL);
	assert(data != NULL);
	assert(dataLength > 0);

	FILE * file = fopen(fileName, "wb");
	if (file == NULL)
	{
		return -1;
	}

	intptr_t writtenBytes = (intptr_t)fwrite(data, dataLength, 1, file);
	fclose(file);

	return writtenBytes;
}
