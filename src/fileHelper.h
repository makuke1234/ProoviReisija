#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <stdio.h>
#include <stdint.h>

char * fhelper_readFile(const char * restrict fileName, intptr_t fnameLength, size_t * restrict resultLength);
char * fhelper_readFileBinary(const char * restrict fileName, intptr_t fnameLength, size_t * restrict resultLength);

intptr_t fhelper_writeFile(const char * fileName, intptr_t fnameLength, const char * string, intptr_t stringLength);
intptr_t fhelper_writeFileBinary(const char * fileName, intptr_t fnameLength, const char * string, intptr_t stringLength);


#endif

