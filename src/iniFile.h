#ifndef INI_FILE_H
#define INI_FILE_H

#include "hashmap.h"

#include <stdint.h>
#include <stdbool.h>


#define FORBIDDEN_CODEPOINT 0xFFFF


typedef struct IniString
{
	char * str;
	size_t len;
	
} IniString_t;

typedef struct IniValue
{
	IniString_t key, value;

	size_t idx;

} IniValue_t;

typedef struct IniSection
{
	IniString_t section;

	IniValue_t ** values;
	size_t numValues, maxValues;

	hashMap_t valueMap;

	size_t idx;

} IniSection_t;

typedef struct ini
{
	IniSection_t ** sections;
	size_t numSections, maxSections;

	hashMap_t sectionMap;

} ini_t;

typedef enum IniE
{
	IniE_OK,
	IniE_MEM,
	IniE_SECTION,
	IniE_ESCAPE,
	IniE_VALUE

} IniE_t;


/* ********************* Funktsioonid ********************* */

bool ini_str_appendCh(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, char ch);
bool ini_str_appendCP(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, uint16_t codePoint);

uint8_t ini_str_hexToNum(char ch);
uint16_t ini_str_codePointFromStr(const char * restrict str);

char * ini_escapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize);
char * ini_escapeStr(const char * restrict string, intptr_t length);
char * ini_unescapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize);
char * ini_unescapeStr(const char * restrict string, intptr_t length);




bool IniString_init(IniString_t * restrict istr, const char * restrict str, intptr_t length);
IniString_t * IniString_make(const char * restrict str, intptr_t length);
bool IniString_initEscape(IniString_t * restrict istr, const char * restrict str, intptr_t length);
IniString_t * IniString_makeEscape(const char * restrict str, intptr_t length);

void IniString_destroy(IniString_t * restrict istr);
void IniString_free(IniString_t * restrict istr);


bool IniValue_init(
	IniValue_t * restrict ival,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);
IniValue_t * IniValue_make(
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);

void IniValue_destroy(IniValue_t * restrict ival);
void IniValue_free(IniValue_t * restrict ival);


bool IniSection_init(IniSection_t * restrict isect, const char * sectname, intptr_t sectnameLen);
IniSection_t * IniSection_make(const char * sectname, intptr_t sectnameLen);

bool IniSection_addValue(
	IniSection_t * restrict isect,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);
IniValue_t * IniSection_getValue(IniSection_t * restrict isect, const char * restrict keystr);
bool IniSection_removeValue(IniSection_t * restrict isect, const char * restrict keystr);

void IniSection_destroy(IniSection_t * restrict isect);
void IniSection_free(IniSection_t * restrict isect);


bool ini_init(ini_t * restrict idata);
ini_t * ini_make();

bool ini_addSection(ini_t * restrict idata, const char * restrict secstr, intptr_t seclen);
IniSection_t * ini_getSection(ini_t * restrict idata, const char * restrict secstr);
bool ini_removeSection(ini_t * restrict idata, const char * restrict secstr);

IniE_t ini_checkData(const char * restrict string, intptr_t length);
IniE_t ini_checkFile(const char * restrict fileName);

IniE_t ini_initData(const char * restrict string, intptr_t length, ini_t * restrict pini);
IniE_t ini_initFile(const char * restrict fileName, ini_t * restrict pini);

void ini_destroy(ini_t * restrict pini);
void ini_free(ini_t * restrict pini);


#endif
