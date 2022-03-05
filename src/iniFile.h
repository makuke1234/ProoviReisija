#ifndef INI_FILE_H
#define INI_FILE_H

#include "hashmap.h"

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

typedef struct IniData
{
	IniSection_t * sections;
	size_t numSections, maxSections;

	hashMap_t sectionMap;

} IniData_t;

typedef enum IniE
{
	IniE_OK,
	IniE_MEM

} IniE_t;


/* ********************* Funktsioonid ********************* */

char * ini_escapeStr(const char * restrict string, intptr_t length);
char * ini_unescapeStr(const char * restrict string, intptr_t length);


IniE_t ini_checkData(const char * restrict string, intptr_t length);
IniE_t ini_checkFile(const char * restrict fileName, intptr_t fnameLength);

IniE_t ini_parseData(const char * restrict string, intptr_t length, IniData_t * restrict pini);
IniE_t ini_parseFile(const char * restrict fileName, intptr_t fnameLength, IniData_t * restrict pini);



#endif
