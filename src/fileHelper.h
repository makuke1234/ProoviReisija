#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <stdio.h>
#include <stdint.h>

char * fhelper_read(const char * restrict fileName, size_t * restrict resultLength);
char * fhelper_readBin(const char * restrict fileName, size_t * restrict resultLength);

intptr_t fhelper_write(const char * fileName, const char * string, intptr_t stringLength);
intptr_t fhelper_writeBin(const char * fileName, const char * string, intptr_t stringLength);


#endif

