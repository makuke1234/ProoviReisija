#ifndef INI_FILE_H
#define INI_FILE_H

#include "hashmap.h"

#include <stdio.h>
#include <stdint.h>

typedef char * char_ptr_t;

typedef struct IniString
{
	union
	{
		char_ptr_t;
		char * str;
	};
	size_t len;
	
} IniString_t;

typedef struct IniValue
{
	IniString_t key, value;

} IniValue_t;

typedef struct IniSection
{
	IniString_t section;

	IniValue_t * values;
	size_t numValues, maxValues;

	hashMap_t valueMap;

} IniSection_t;

typedef struct IniFile
{
	IniSection_t * sections;
	size_t numSections, maxSections;

	hashMap_t sectionMap;

} IniFile_t;

typedef enum IniE
{
	IniE_OK,
	IniE_MEM

} IniE_t;


/* ********************* Funktsioonid ********************* */

char * ini_escapeStr(const char * restrict string, intptr_t length);
char * ini_unescapeStr(const char * restrict string, intptr_t length);


IniE_t ini_check(const char * restrict string, intptr_t length);
IniE_t ini_checkFile(FILE * restrict iniFile);

IniE_t ini_parse(const char * restrict string, intptr_t length, IniFile_t * restrict pini);
IniE_t ini_parseFile(FILE * restrict iniFile, IniFile_t * restrict pini);



#endif
