#ifndef FILEHELPER_H
#define FILEHELPER_H

#include <stdint.h>

intptr_t fhelper_fileSize(const char * restrict fileName);

char * fhelper_read(const char * restrict fileName, size_t * restrict resultLength);
void * fhelper_readBin(const char * restrict fileName, size_t * restrict resultLength);

intptr_t fhelper_write(const char * fileName, const char * string, intptr_t stringLength);
intptr_t fhelper_writeBin(const char * fileName, const void * string, size_t dataLength);


#endif

