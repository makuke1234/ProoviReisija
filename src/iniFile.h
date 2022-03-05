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

bool ini_str_appendCh(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, char ch);
bool ini_str_appendCP(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, uint16_t codePoint);

uint8_t ini_str_hexToNum(char ch);
uint16_t ini_str_codePointFromStr(const char * restrict str);




char * ini_escapeStr(const char * restrict string, intptr_t length);
char * ini_unescapeStr(const char * restrict string, intptr_t length);


IniE_t ini_checkData(const char * restrict string, intptr_t length);
IniE_t ini_checkFile(const char * restrict fileName, intptr_t fnameLength);

IniE_t ini_parseData(const char * restrict string, intptr_t length, IniData_t * restrict pini);
IniE_t ini_parseFile(const char * restrict fileName, intptr_t fnameLength, IniData_t * restrict pini);



#endif
