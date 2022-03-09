#ifndef INI_FILE_H
#define INI_FILE_H

#include "hashmap.h"

#include <stdint.h>
#include <stdbool.h>


#define FORBIDDEN_CODEPOINT 0xFFFF


typedef struct iniString
{
	char * str;
	size_t len;
	
} iniString_t;

typedef struct iniValue_t
{
	iniString_t key, value;

	size_t idx;

} iniValue_t;

typedef struct iniSection
{
	iniString_t section;

	iniValue_t ** values;
	size_t numValues, maxValues;

	hashMap_t valueMap;

	size_t idx;

} iniSection_t;

typedef struct ini
{
	iniSection_t ** sections;
	size_t numSections, maxSections;

	hashMap_t sectionMap;

} ini_t;

typedef enum iniErr
{
	inieOK,
	inieMEM,
	inieSECTION,
	inieESCAPE,
	inieVALUE,
	inieQUOTE

} iniErr_t;


/* ********************* Funktsioonid ********************* */

bool g_ini_strAppendCh(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, char ch);
bool g_ini_strAppendCP(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, uint16_t codePoint);

uint8_t g_ini_strHexToNum(char ch);
uint16_t g_ini_strCPFromStr(const char * restrict str);

char * g_ini_escapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize);
char * g_ini_escapeStr(const char * restrict string, intptr_t length);
char * g_ini_unescapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize);
char * g_ini_unescapeStr(const char * restrict string, intptr_t length);




bool iniString_init(iniString_t * restrict istr, const char * restrict str, intptr_t length);
iniString_t * iniString_make(const char * restrict str, intptr_t length);
bool iniString_initEscape(iniString_t * restrict istr, const char * restrict str, intptr_t length);
iniString_t * iniString_makeEscape(const char * restrict str, intptr_t length);

void iniString_destroy(iniString_t * restrict istr);
void iniString_free(iniString_t * restrict istr);


bool iniValue_init(
	iniValue_t * restrict ival,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);
iniValue_t * iniValue_make(
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);

void iniValue_destroy(iniValue_t * restrict ival);
void iniValue_free(iniValue_t * restrict ival);


bool iniSection_init(iniSection_t * restrict isect, const char * sectname, intptr_t sectnameLen);
iniSection_t * iniSection_make(const char * sectname, intptr_t sectnameLen);

bool iniSection_addValue(
	iniSection_t * restrict isect,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);
iniValue_t * iniSection_getValue(iniSection_t * restrict isect, const char * restrict keystr);
bool iniSection_removeValue(iniSection_t * restrict isect, const char * restrict keystr);

void iniSection_destroy(iniSection_t * restrict isect);
void iniSection_free(iniSection_t * restrict isect);


bool ini_init(ini_t * restrict idata);
ini_t * ini_make(void);

bool ini_addSection(ini_t * restrict idata, const char * restrict secstr, intptr_t seclen);
iniSection_t * ini_getSection(ini_t * restrict idata, const char * restrict secstr);
bool ini_removeSection(ini_t * restrict idata, const char * restrict secstr);

iniErr_t ini_checkData(const char * restrict string, intptr_t length);
iniErr_t ini_checkFile(const char * restrict fileName);

iniErr_t ini_initData(const char * restrict string, intptr_t length, ini_t * restrict pini);
iniErr_t ini_initFile(const char * restrict fileName, ini_t * restrict pini);

void ini_destroy(ini_t * restrict pini);
void ini_free(ini_t * restrict pini);


#endif
