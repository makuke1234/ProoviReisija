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

	hashMapCK_t valueMap;

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

	hashMapCK_t sectionMap;

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
 * @param length Length of string in characters (not including null-terminator), can be -1 if null-terminated
 * @param psize Pointer of variable receiving allocated memory size in bytes (including null-terminator)
 * @return char* Freshly allocated "escaped" null-terminated character array, NULL on failure
 */
char * g_ini_escapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize);
/**
 * @brief "Unsafe" version of g_ini_escapeStr_s, "escapes" escape characters from INI string
 * 
 * @param string Pointer to character array, input
 * @param length Length of string in characters (not including null-terminator), can be -1 if null-terminated
 * @return char* Freshly allocated "escaped" null-terminated character array, NULL on failure
 */
char * g_ini_escapeStr(const char * restrict string, intptr_t length);
/**
 * @brief "Un-escapes" to escape characters in given string, result suitable for INI file
 * 
 * @param string Pointer to character array with escaped string, input
 * @param length Length of string in characters (not including null-terminator), can be -1 if null-terminated
 * @param psize Pointer of variable receiving allocated memory size in bytes (including null-terminator)
 * @return char* Freshly allocated "unescaped" null-terminated character array, NULL on failure
 */
char * g_ini_unescapeStr_s(const char * restrict string, intptr_t length, size_t * restrict psize);
/**
 * @brief "Unsafe" version of g_ini_unescapeStr_s, "un-escapes" to escape characters
 * in given string, result suitable for INI file
 * 
 * @param string Pointer to character array with escaped string, input
 * @param length Length of string in characters (not including null-terminator), can be -1 if null-terminated
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
 * @param str Pointer to character array holding input string, can be NULL
 * @param length Length of string in characters (not including null-terminator), can be -1 if null-terminated
 * @return true Initialisation succeeded
 * @return false Failure
 */
bool iniString_init(iniString_t * restrict pstr, const char * restrict str, intptr_t length);
/**
 * @brief Allocates memory for iniString structure, initialises it, copies "str" contents to new memory
 * 
 * @param str Pointer to character array holding input, can be NULL
 * @param length Lnegth of string incharacter (not including null-terminator), can be -1 if null-terminated
 * @return iniString_t* Pointer to heap-allocated iniString structure, NULL on failure
 */
iniString_t * iniString_make(const char * restrict str, intptr_t length);
/**
 * @brief Initialises iniString structure, copies & escapes contents of "str" to newly allocated location
 * 
 * @param pstr Pointer to iniString structure
 * @param str Pointer to character array holding input string, can be NULL
 * @param length Length of string in characters (not including null-terminator), can be -1 if null-terminated
 * @return true Initialisation succeeded
 * @return false Failure
 */
bool iniString_initEscape(iniString_t * restrict pstr, const char * restrict str, intptr_t length);
/**
 * @brief Allocates memory & initialises iniString structure,
 * copies & escapes contents of "str" to newly allocated location
 * 
 * @param str Pointer to character array holding input string, can be NULL
 * @param length Length of string in characters (not including null-terminator), can be -1 if null-terminated
 * @return iniString_t* Pointer to heap-allocated iniString structure, NULL on failure
 */
iniString_t * iniString_makeEscape(const char * restrict str, intptr_t length);
/**
 * @brief Initialises iniString structure, copies, escapes & converts "str" contents
 * to lowercase. Contents will be copied to newly allocateed location, leaving original
 * string untampered.
 * 
 * @param pstr Pointer to iniString structure, can be NULL
 * @param str Pointer to character array holding input string
 * @param length Length of string in characters (not including null-terminator), can be -1 if null-terminated
 * @return true Success
 * @return false Failure
 */
bool iniString_initEscapeLower(iniString_t * restrict pstr, const char * restrict str, intptr_t length);
/**
 * @brief Allocates memory for iniString strucutre, initialises it with escaped,
 * lower-case converted copied string of "str"
 * 
 * @param str Pointer to character array holding input string, can be NULL
 * @param length Length of string in characters (not including null-terminator), can be -1 if null-terminated
 * @return iniString_t* Pointer to heap-allocated iniString structure, NULL on failure
 */
iniString_t * iniString_makeEscapeLower(const char * restrict str, intptr_t length);
/**
 * @brief Copies string from another iniString structure
 * 
 * @param pstr Pointer to destintation iniString structure
 * @param src Pointer to source iniString structure
 * @return true Success copying
 * @return false Failure
 */
bool iniString_initCopy(iniString_t * restrict pstr, const iniString_t * restrict src);
/**
 * @brief Copies string from another iniString structure, heap-allocates memory
 * for destination structure
 * 
 * @param src Pointer to source iniString structure
 * @return iniString_t* Resulting heap-allocated copy of src, NULL on failure
 */
iniString_t * iniString_makeCopy(const iniString_t * restrict src);

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


/**
 * @brief Initialises iniValue structure with specific key-value pair
 * 
 * @param pval Pointer to destination iniValue structure
 * @param keystr Pointer to character array that holds value's "key", can be NULL
 * @param keylen Length of keystr in characters, can be -1 if null-terminated
 * @param valstr Pointer to character array which holds object's value, can be NULL
 * @param vallen Length of valstr in characters, can be -1 if null-terminated
 * @return true Success
 * @return false Failure
 */
bool iniValue_init(
	iniValue_t * restrict pval,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);
/**
 * @brief Allocates memory for iniValue structure, initialises it with specific key-value pair
 * 
 * @param keystr Pointer to character array that holds value's "key", can be NULL
 * @param keylen Length of keystr in characters, can be -1 if null-terminated
 * @param valstr Pointer to character array which holds object's value, can be NULL
 * @param vallen Length of valstr in characters, can be -1 if null-terminated
 * @return iniValue_t* Pointer to newly heap-allocated iniValue structure, NULL on failure
 */
iniValue_t * iniValue_make(
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);

/**
 * @brief Destroys the contents of iniValue structure
 * 
 * @param pval Pointer to iniValue structure
 */
void iniValue_destroy(iniValue_t * restrict pval);
/**
 * @brief Destroys the contents of iniValue structure, frees structure memory
 * 
 * @param pval Pointer to heap-allocated iniValue structure
 */
void iniValue_free(iniValue_t * restrict pval);


/**
 * @brief Initialises iniSection structure with specific section name
 * 
 * @param psect Pointer to iniSection structure
 * @param sectname Pointer to character array which holds section name, can be NULL
 * @param sectnameLen Length of sectname in character, can be -1 if null-terminated
 * @return true Success
 * @return false Failure
 */
bool iniSection_init(iniSection_t * restrict psect, const char * sectname, intptr_t sectnameLen);
/**
 * @brief Heap-allocated memory for iniSectioln structure, initialises structure
 * with specific section name
 * 
 * @param sectname Pointer to character array which holds section name, can be NULL
 * @param sectnameLen Length of sectname in character, can be -1 if null-terminated
 * @return iniSection_t* Pointer to newly heap-allocated iniSection structure, NULL on failure
 */
iniSection_t * iniSection_make(const char * sectname, intptr_t sectnameLen);

/**
 * @brief Inserts/adds a key-value pair to a specific section
 * 
 * @param psect Pointer to target iniSection structure
 * @param keystr Pointer to character array holding object's "key", can be NULL
 * @param keylen keystr length, can be -1 if null-terminated
 * @param valstr Pointer to character array holding object's value, can be NULL
 * @param vallen valstr length, can be -1 if null-terminated
 * @return true Success
 * @return false Failure
 */
bool iniSection_addValue(
	iniSection_t * restrict psect,
	const char * restrict keystr, intptr_t keylen,
	const char * restrict valstr, intptr_t vallen
);
/**
 * @brief Returns key-value pair from specific section by pair's "key"
 * 
 * @param psect Pointer to iniSection structure
 * @param keystr Pointer to character array holding target object's "key"
 * @return iniValue_t* Pointer to queried iniValue structure from section, NULL on failure
 */
iniValue_t * iniSection_getValue(iniSection_t * restrict psect, const char * restrict keystr);
/**
 * @brief Removes key-value pair from specific section by pair's "key"
 * 
 * @param psect Pointer to iniSection structure
 * @param keystr Pointer to character array holding target object's "key"
 * @return true Success removing
 * @return false Failure
 */
bool iniSection_removeValue(iniSection_t * restrict psect, const char * restrict keystr);

/**
 * @brief Destroys contents of iniSection structure
 * 
 * @param psect Pointer to iniSection structure
 */
void iniSection_destroy(iniSection_t * restrict psect);
/**
 * @brief Destroys contents of structure, frees memory
 * 
 * @param psect Pointer to heap-allocated iniSection structure
 */
void iniSection_free(iniSection_t * restrict psect);


/**
 * @brief Initialises ini structure
 * 
 * @param pini Pointer to ini structure
 * @return true Success
 * @return false Failure
 */
bool ini_init(ini_t * restrict pini);
/**
 * @brief Heap-allocates memory for ini structure, initialises it
 * 
 * @return ini_t* Pointer to heap-allocated ini structure, NULL on failure
 */
ini_t * ini_make(void);

/**
 * @brief Adds a section to ini structure with specific name
 * 
 * @param pini 
 * @param secstr 
 * @param seclen 
 * @return true 
 * @return false 
 */
bool ini_addSection(ini_t * restrict pini, const char * restrict secstr, intptr_t seclen);
/**
 * @brief Returns a section from ini structure by section's name
 * 
 * @param pini 
 * @param secstr 
 * @return iniSection_t* 
 */
iniSection_t * ini_getSection(ini_t * restrict pini, const char * restrict secstr);
/**
 * @brief Removes a section from ini structure by section's name
 * 
 * @param pini 
 * @param secstr 
 * @return true 
 * @return false 
 */
bool ini_removeSection(ini_t * restrict pini, const char * restrict secstr);

/**
 * @brief Checks the validity of INI file data string
 * 
 * @param string Pointer to character array of INI file data string to be checked
 * @param length Length of string, can be -1, if null-terminated
 * @return iniErr_t Error code from checker, inieOK if INI string is valid
 */
iniErr_t ini_checkData(const char * restrict string, intptr_t length);
/**
 * @brief Checks the validity of INI file
 * 
 * @param fileName Pointer to null-terminated character array of INI file's name to be checked
 * @return iniErr_t Error code from checker, inieOK if INI file is valid
 */
iniErr_t ini_checkFile(const char * restrict fileName);

/**
 * @brief Checks the validity of INI file data string, parses it to ini structure if valid
 * 
 * @param pini Pointer to ini structure to be filled
 * @param string Pointer to character array of INI file data string to be checked
 * @param length Length of string, can be -1, if null-terminated
 * @return iniErr_t Error code from parser, inieOK if INI file is valid
 */
iniErr_t ini_initData(ini_t * restrict pini, const char * restrict string, intptr_t length);
/**
 * @brief Checks the validity of INI file, parses it to ini structure if valid
 * 
 * @param pini Pointer to ini structure to be filled
 * @param fileName Pointer to null-terminated character array of INI file's name to be checked
 * @return iniErr_t Error code from parser, inieOK if INI file is valid
 */
iniErr_t ini_initFile(ini_t * restrict pini, const char * restrict fileName);

/**
 * @brief Destroys contents of ini structure
 * 
 * @param pini Pointer to ini structure
 */
void ini_destroy(ini_t * restrict pini);
/**
 * @brief Destroys contents of ini structure, frees heap-allocated memory
 * 
 * @param pini Pointer to heap-allocated ini structure
 */
void ini_free(ini_t * restrict pini);

/* ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ */
/* ***************** Andmestruktuuride funktsioonid ***************** */



#endif
