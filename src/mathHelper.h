#ifndef MATH_HELPER_H
#define MATH_HELPER_H

#include <stdint.h>

int8_t mh_i8min(int8_t a, int8_t b);
uint8_t mh_u8min(uint8_t a, uint8_t b);
int16_t mh_i16min(int16_t a, int16_t b);
uint16_t mh_u16min(uint16_t a, uint16_t b);
int32_t mh_i32min(int32_t a, int32_t b);
uint32_t mh_u32min(uint32_t a, uint32_t b);
int64_t mh_i64min(int64_t a, int64_t b);
uint64_t mh_u64min(uint64_t a, uint64_t b);
size_t mh_zmin(size_t a, size_t b);
intptr_t mh_imin(intptr_t a, intptr_t b);

int8_t mh_i8max(int8_t a, int8_t b);
uint8_t mh_u8max(uint8_t a, uint8_t b);
int16_t mh_i16max(int16_t a, int16_t b);
uint16_t mh_u16max(uint16_t a, uint16_t b);
int32_t mh_i32max(int32_t a, int32_t b);
uint32_t mh_u32max(uint32_t a, uint32_t b);
int64_t mh_i64max(int64_t a, int64_t b);
uint64_t mh_u64max(uint64_t a, uint64_t b);
size_t mh_zmax(size_t a, size_t b);
intptr_t mh_imax(intptr_t a, intptr_t b);


float mh_clampf(float value, float min, float max);
double mh_clamp(double value, double min, double max);
long double mh_clampl(long double value, long double min, long double max);

float mh_clampUnif(float value, float a, float b);
double mh_clampUni(double value, double a, double b);
long double mh_clampUnil(long double value, long double a, long double b);


#endif
