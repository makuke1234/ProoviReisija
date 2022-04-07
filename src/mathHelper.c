#include "mathHelper.h"

int8_t mh_i8min(int8_t a, int8_t b)
{
	return (a < b) ? a : b;
}
uint8_t mh_u8min(uint8_t a, uint8_t b)
{
	return (a < b) ? a : b;
}
int16_t mh_i16min(int16_t a, int16_t b)
{
	return (a < b) ? a : b;
}
uint16_t mh_u16min(uint16_t a, uint16_t b)
{
	return (a < b) ? a : b;
}
int32_t mh_i32min(int32_t a, int32_t b)
{
	return (a < b) ? a : b;
}
uint32_t mh_u32min(uint32_t a, uint32_t b)
{
	return (a < b) ? a : b;
}
int64_t mh_i64min(int64_t a, int64_t b)
{
	return (a < b) ? a : b;
}
uint64_t mh_u64min(uint64_t a, uint64_t b)
{
	return (a < b) ? a : b;
}
size_t mh_zmin(size_t a, size_t b)
{
	return (a < b) ? a : b;
}
intptr_t mh_imin(intptr_t a, intptr_t b)
{
	return (a < b) ? a : b;
}

int8_t mh_i8max(int8_t a, int8_t b)
{
	return (a < b) ? b : a;
}
uint8_t mh_u8max(uint8_t a, uint8_t b)
{
	return (a < b) ? b : a;
}
int16_t mh_i16max(int16_t a, int16_t b)
{
	return (a < b) ? b : a;
}
uint16_t mh_u16max(uint16_t a, uint16_t b)
{
	return (a < b) ? b : a;
}
int32_t mh_i32max(int32_t a, int32_t b)
{
	return (a < b) ? b : a;
}
uint32_t mh_u32max(uint32_t a, uint32_t b)
{
	return (a < b) ? b : a;
}
int64_t mh_i64max(int64_t a, int64_t b)
{
	return (a < b) ? b : a;
}
uint64_t mh_u64max(uint64_t a, uint64_t b)
{
	return (a < b) ? b : a;
}
size_t mh_zmax(size_t a, size_t b)
{
	return (a < b) ? b : a;
}
intptr_t mh_imax(intptr_t a, intptr_t b)
{
	return (a < b) ? b : a;
}


float mh_fminf(float a, float b)
{
	return (a < b) ? a : b;
}
double mh_fmin(double a, double b)
{
	return (a < b) ? a : b;
}
long double mh_fminl(long double a, long double b)
{
	return (a < b) ? a : b;
}

float mh_fmaxf(float a, float b)
{
	return (a < b) ? b : a;
}
double mh_fmax(double a, double b)
{
	return (a < b) ? b : a;
}
long double mh_fmaxl(long double a, long double b)
{
	return (a < b) ? b : a;
}



float mh_clampf(float value, float min, float max)
{
	return (value < min) ? min : (max < value) ? max : value;
}
double mh_clamp(double value, double min, double max)
{
	return (value < min) ? min : (max < value) ? max : value;
}
long double mh_clampl(long double value, long double min, long double max)
{
	return (value < min) ? min : (max < value) ? max : value;
}

float mh_clampUnif(float value, float a, float b)
{
	return (a < b) ? mh_clampf(value, a, b) : mh_clampf(value, b, a);
}
double mh_clampUni(double value, double a, double b)
{
	return (a < b) ? mh_clamp(value, a, b) : mh_clamp(value, b, a);
}
long double mh_clampUnil(long double value, long double a, long double b)
{
	return (a < b) ? mh_clampl(value, a, b) : mh_clampl(value, b, a);
}
