#ifndef INI_FILE_H
#define INI_FILE_H

#include "hashmap.h"

#include <stdint.h>
#include <stdbool.h>


#define FORBIDDEN_CODEPOINT 0xFFFF


/**
 * @brief Data structure to remember string length associated with character array.
 * 
 */
typedef struct iniString
{
	char * str;
	size_t len;
	
} iniString_t;

/**
 * @brief Data structure for INI file key-value pair, also holds index to remember
 * place in key-value ordered array.
 * 
 */
typedef struct iniValue_t
{
	iniString_t key, value;

	size_t idx;

} iniValue_t;

/**
 * @brief Data structure for INI file section structure, also holds index to remember
 * place in sections' ordered array. Holds hashmap of value key's.
 * 
 */
typedef struct iniSection
{
	iniString_t section;

	iniValue_t ** values;
	size_t numValues, maxValues;

	hashMap_t valueMap;

	size_t idx;

} iniSection_t;

/**
 * @brief Data structure for INI file, holds memory for iniSection structure array.
 * Holds hashmap of section names.
 * 
 */
typedef struct ini
{
	iniSection_t ** sections;
	size_t numSections, maxSections;

	hashMap_t sectionMap;

} ini_t;

/**
 * @brief Enumerator for INI-file related error codes
 * 
 */
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

/* ***************** Üldised funktsioonid ***************** */
/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */

/**
 * @brief Appends character to the end of the string
 * 
 * @param pstr Address of pointer to character array that holds the string
 * @param psize Pointer to variable, that holds the array size (number of characters)
 * @param pcap Pointer to variable, that holds the array capacity
 * @param ch Character to append
 * @return true Appending succeeded
 * @return false Appending failed
 */
bool g_ini_strAppendCh(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, char ch);
/**
 * @brief Appends UTF-8 code point to the end of the string
 * 
 * @param pstr Address of pointer to character array that holds the string
 * @param psize Pointer to variable, that holds the array size (number of characters)
 * @param pcap Pointer to variable, that holds the array capacity
 * @param codePoint code point to append
 * @return true Appending succeeded
 * @return false Appending failed
 */
bool g_ini_strAppendCP(char ** restrict pstr, size_t * restrict psize, size_t * restrict pcap, uint16_t codePoint);

/**
 * @brief Converts string character to hexadecimal value
 * 
 * @param ch Character from string in range 0 ... 9 or A ... F
 * @return uint8_t Hexadecimal value in the range of 0 ... 15, 0xFF on failure
 */
uint8_t g_ini_strHexToNum(char ch);
/**
 * @brief Converts string set of 4 characters to Unicode code point
 * 
 * @param str Pointer to character array holding Unicode code point string/number representation
 * @return uint16_t Unicode code point value in the range of 0 ... 0xFFFE, 0xFFFF on failure
 */
uint16_t g_ini_strCPFromStr(const char * restrict str);

/**
 * @brief "Escapes" escape characters in given string originating from INI file
 * 
 * @param string Pointer to character array, input
 * @param length Length of string in characters (not including null-terminator), -1 if null-terminated and/or unspecified
 * @param psize Pointer of variable receiving allocated memory size in bytes (including null-terminator)
 * @return char* Freshly allocated "escaped" null-terminated character array, NULL on failure
 */
char * g_ini_escapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize);
/**
 * @brief "Unsafe" version of g_ini_escapeStr_s, "escapes" escape characters from INI string
 * 
 * @param string Pointer to character array, input
 * @param length Length of string in characters (not including null-terminator), -1 if null-terminated and/or unspecified
 * @return char* Freshly allocated "escaped" null-terminated character array, NULL on failure
 */
char * g_ini_escapeStr(const char * restrict string, intptr_t length);
/**
 * @brief "Un-escapes" to escape characters in given string, result suitable for INI file
 * 
 * @param string Pointer to character array with escaped string, input
 * @param length Length of string in characters (not including null-terminator), -1 if null-terminated and/or unspecified
 * @param psize Pointer of variable receiving allocated memory size in bytes (including null-terminator)
 * @return char* Freshly allocated "unescaped" null-terminated character array, NULL on failure
 */
char * g_ini_unescapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize);
/**
 * @brief "Unsafe" version of g_ini_unescapeStr_s, "un-escapes" to escape characters
 * in given string, result suitable for INI file
 * 
 * @param string Pointer to character array with escaped string, input
 * @param length Length of string in characters (not including null-terminator), -1 if null-terminated and/or unspecified
 * @return char* Freshly allocated "unescaped" null-terminated character array, NULL on failure
 */
char * g_ini_unescapeStr(const char * restrict string, intptr_t length);

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
/* ***************** Üldised funktsioonid ***************** */





/* ***************** Andmestruktuuride funktsioonid ***************** */
/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


/**
 * @brief Zeros the memory of iniString structure
 * 
 * @param pstr Pointer to pre-allocated iniString structure
 */
void iniString_zero(iniString_t * restrict pstr);
/**
 * @brief Initialises iniString structure, copies contents of "str" to newly allocated location
 * 
 * @param pstr Pointer to iniString structure
 * @param str Pointer to character array holding input string
 * @param length Length of string in characters (not including null-terminator), -1 if null-terminated and/or unspecified
 * @return true Initialisation succeeded
 * @return false Failure
 */
bool iniString_init(iniString_t * restrict pstr, const char * restrict str, intptr_t length);
/**
 * @brief Allocated memory for iniString structure, initialises it, copies "str" contents to new memory
 * 
 * @param str Pointer to character array holding input
 * @param length Lnegth of string incharacter (not including null-terminator), -1 if null-terminated and/or unspecified
 * @return iniString_t* Pointer to heap-allocated iniString structure, NULL on failure
 */
iniString_t * iniString_make(const char * restrict str, intptr_t length);
bool iniString_initEscape(iniString_t * restrict pstr, const char * restrict str, intptr_t length);
iniString_t * iniString_makeEscape(const char * restrict str, intptr_t length);
bool iniString_initEscapeLower(iniString_t * restrict pstr, const char * restrict str, intptr_t length);
iniString_t * iniString_makeEscapeLower(const char * restrict str, intptr_t length);

bool iniString_copy(iniString_t * restrict pstr, const iniString_t * restrict src);
iniString_t * iniString_copymake(const iniString_t * restrict src);

/**
 * @brief Destroys iniString object, frees memory allocated by string
 * 
 * @param pstr Pointer to iniString structure
 */
void iniString_destroy(iniString_t * restrict pstr);
/**
 * @brief Destroys iniString object, frees memory allocated for iniString structure itself
 * 
 * @param pstr Pointer to heap-allocated iniString structure
 */
void iniString_free(iniString_t * restrict pstr);


bool iniValue_init(
	iniValue_t * restrict pval,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);
iniValue_t * iniValue_make(
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);

void iniValue_destroy(iniValue_t * restrict pval);
void iniValue_free(iniValue_t * restrict pval);


bool iniSection_init(iniSection_t * restrict psect, const char * sectname, intptr_t sectnameLen);
iniSection_t * iniSection_make(const char * sectname, intptr_t sectnameLen);

bool iniSection_addValue(
	iniSection_t * restrict psect,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);
iniValue_t * iniSection_getValue(iniSection_t * restrict psect, const char * restrict keystr);
bool iniSection_removeValue(iniSection_t * restrict psect, const char * restrict keystr);

void iniSection_destroy(iniSection_t * restrict psect);
void iniSection_free(iniSection_t * restrict psect);


bool ini_init(ini_t * restrict pini);
ini_t * ini_make(void);

bool ini_addSection(ini_t * restrict pini, const char * restrict secstr, intptr_t seclen);
iniSection_t * ini_getSection(ini_t * restrict pini, const char * restrict secstr);
bool ini_removeSection(ini_t * restrict pini, const char * restrict secstr);

iniErr_t ini_checkData(const char * restrict string, intptr_t length);
iniErr_t ini_checkFile(const char * restrict fileName);

iniErr_t ini_initData(const char * restrict string, intptr_t length, ini_t * restrict pini);
iniErr_t ini_initFile(const char * restrict fileName, ini_t * restrict pini);

void ini_destroy(ini_t * restrict pini);
void ini_free(ini_t * restrict pini);

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
/* ***************** Andmestruktuuride funktsioonid ***************** */



#endif
