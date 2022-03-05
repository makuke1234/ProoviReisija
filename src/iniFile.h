#ifndef INI_FILE_H
#define INI_FILE_H

#include <stdint.h>

typedef struct IniString
{
	union
	{
		char *;
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
} IniE_t;

#endif
