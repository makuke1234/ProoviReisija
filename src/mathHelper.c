#include "mathHelper.h"

float mh_clampf(float value, float min, float max)
{
	return (value < min) ? min : (max < value) ? max : value;
}
float mh_clampUnif(float value, float a, float b)
{
	return (a < b) ? mh_clampf(value, a, b) : mh_clampf(value, b, a);
}
float mh_maxf(float a, float b)
{
	return (a < b) ? b : a;
}
float mh_minf(float a, float b)
{
	return (a < b) ? a : b;
}
