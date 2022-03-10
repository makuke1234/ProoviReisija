#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include <stdint.h>

/**
 * @brief Calculates files size
 * 
 * @param fileName Pointer to character array of desired filename
 * @return intptr_t On failure -1, otherwise file size in bytes
 */
intptr_t fhelper_fileSize(const char * restrict fileName);

/**
 * @brief Reads file contents in text mode
 * 
 * @param fileName Pointer to character array of desired filename 
 * @param resultLength Pointer to variable that will hold the resuling character array size in bytes
 * (including null-terminator)
 * @return char* Pointer to freshly allocated character array conatining null-terminated file contents,
 * NULL on failure
 */
char * fhelper_read(const char * restrict fileName, size_t * restrict resultLength);
/**
 * @brief Reads file contents in binary mode
 * 
 * @param fileName Pointer to character array of desired filename
 * @param resultLength Pointer to variable that will hold the resulting byte array size in bytes
 * @return void* Pointer to allocated array of byte-data containing file contents, NULL on failure,
 * NB! Data is NOT null-terminated!
 */
void * fhelper_readBin(const char * restrict fileName, size_t * restrict resultLength);

/**
 * @brief Writes array contents to file in text mode
 * 
 * @param fileName Pointer to character array of desired filename
 * @param string Pointer to source character array to be written to file
 * @param stringLength Number of characters (bytes) to be written to file
 * @return intptr_t Number of characters written, -1 on failure
 */
intptr_t fhelper_write(const char * fileName, const char * string, intptr_t stringLength);
/**
 * @brief Writes array contents to file in binary mode
 * 
 * @param fileName Pointer to character array of desired filename
 * @param data Pointer to source byte array to be written to file
 * @param dataLength Number of bytes to be written to file
 * @return intptr_t Number of bytes written, -1 on failure
 */
intptr_t fhelper_writeBin(const char * fileName, const void * data, size_t dataLength);


#endif

