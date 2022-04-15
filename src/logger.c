#include "logger.h"

#if LOGGING_ENABLE == 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

static FILE * loggingFile = NULL;

void initLogger(void)
{
	// Avab logifaili
	loggingFile = fopen("logger.txt", "a+");
	if (loggingFile == NULL)
	{
		fputs("Error opening logging file!\n", stderr);
		exit(1);
	}
	fputs("\n", loggingFile);
	writeLogger("Started application...");
}
void writeLogger_inner(const char * restrict function, const char * restrict format, ...)
{
	assert(function != NULL);
	assert(format   != NULL);

	if (loggingFile == NULL)
	{
		fputs("Logging file not open!\n", stderr);
		return;
	}

	// Kirjutab praeguse kellaaja faili
	time_t rawtime;
	time(&rawtime);
	struct tm * ti = localtime(&rawtime);
	fprintf(
		loggingFile,
		"[%.2d.%.2d.%d @%.2d:%.2d:%.2d] @%s<",
		ti->tm_mday, ti->tm_mon + 1, ti->tm_year + 1900,
		ti->tm_hour, ti->tm_min,     ti->tm_sec,
		function
	);
	// Kirjutab kasutajapoolse sõnumi
	va_list ap;
	va_start(ap, format);

	vfprintf(loggingFile, format, ap);

	va_end(ap);

	fprintf(loggingFile, ">\n");
	// Puhver tühjendatakse igaks juhuks logifaili, juhul kui programm peaks "kokku jooksma"
	fflush(loggingFile);
}

#define LOGGER_STACK_SIZE 256

static clock_t loggerStack[LOGGER_STACK_SIZE];
static uint16_t curStackLen = 0;

void loggerStart(void)
{
	assert(curStackLen < LOGGER_STACK_SIZE);

	// Stackile lisatakse praegune aeg
	loggerStack[curStackLen] = clock();
	++curStackLen;
}
void loggerEnd_inner(const char * funcName)
{
	assert(curStackLen > 0);

	// Stackilt eemaldatakse viimatine aeg (loodetavasti loggetStart funktsiooni poolt sinna lisatud)
	--curStackLen;
	// Ajakulu kirjutatakse logifaili
	writeLogger_inner(funcName, "Elapsed %.3f s", (double)(clock() - loggerStack[curStackLen]) / (double)CLOCKS_PER_SEC);
}

#endif
