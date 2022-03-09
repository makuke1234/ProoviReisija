#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

static const char * lib = "";

static inline void setlib(const char * newlib)
{
	lib = newlib;
}

static inline void vtest(bool cond, const char * errmsg, va_list ap)
{
	static int testnum = 1;
	
	if (cond)
	{
		fprintf(stderr, "[%s] Test #%d passed\n", lib, testnum);
	}
	else
	{
		fprintf(stderr, "[%s] Test #%d failed: ", lib, testnum);
		vfprintf(stderr, errmsg, ap);
		fputc('\n', stderr);
		exit(1);
	}
	++testnum;
}
static inline void test(bool cond, const char * errmsg, ...)
{
	va_list ap;
	va_start(ap, errmsg);

	vtest(cond, errmsg, ap);

	va_end(ap);	
}

static inline void teststr(const char * str1, const char * str2)
{
	test(strcmp(str1, str2) == 0, "\"%s\" != \"%s\"", str1, str2);
}

static inline void testn(const char * newlib, bool cond, const char * errmsg)
{
	setlib(newlib);
	test(cond, errmsg);
}

static inline void testmsg(const char * msg)
{
	fprintf(stderr, "[%s] %s\n", lib, msg);
}

static inline void endphase(void)
{
	static int phasenum = 1;

	fprintf(stderr, "[%s] Phase %d complete!\n", lib, phasenum);

	++phasenum;
}

#endif
