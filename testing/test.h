#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

/**
 * @brief Static pointer that holds current testing scenario's string identifier
 * 
 */
static const char * lib = "";

/**
 * @brief Function to set new library / testing scenario
 * 
 * @param newlib Pointer to null-terminated character array of scenario's string identifier,
 * has to stay "alive" until next setlib call or until the end of the program
 */
static inline void setlib(const char * newlib)
{
	lib = newlib;
}

/**
 * @brief Variadic version of test, tests a condition, outputs error message on failure
 * 
 * @param cond Boolean condition to test
 * @param errmsg Error message format, same as printf's
 * @param ap Argument pointer, same as given to vprintf/vfprintf
 */
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
/**
 * @brief Tests a condition, outputs error message on failure
 * 
 * @param cond Boolean condition to test
 * @param errmsg Error message format, same as printf's
 * @param ... Printf's variadic arguments
 */
static inline void test(bool cond, const char * errmsg, ...)
{
	va_list ap;
	va_start(ap, errmsg);

	vtest(cond, errmsg, ap);

	va_end(ap);	
}

/**
 * @brief Tests, if 2 strings are equal, outputs both if not equal
 * 
 * @param str1 Pointer to character array holding first null-terminated string operand
 * @param str2 Pointer to character array holding second null-terminated string operand
 */
static inline void teststr(const char * str1, const char * str2)
{
	test(strcmp(str1, str2) == 0, "\"%s\" != \"%s\"", str1, str2);
}

/**
 * @brief Sets new testing condition newlib, then test a condition, outputs error
 * message on mismatch
 * 
 * @param newlib New testing condition's name
 * @param cond Condition to test
 * @param errmsg Pointer to null-terminated character array holding error message
 */
static inline void testn(const char * newlib, bool cond, const char * errmsg)
{
	setlib(newlib);
	test(cond, errmsg);
}

/**
 * @brief Outputs message under current testing condition's name
 * 
 * @param msg Pointer to null-terminated character array holding the message
 */
static inline void testmsg(const char * msg)
{
	fprintf(stderr, "[%s] %s\n", lib, msg);
}

/**
 * @brief Ends a testing phase with a grateful message displaying the count
 * of current phase endings. This message is just informational.
 * 
 */
static inline void endphase(void)
{
	static int phasenum = 1;

	fprintf(stderr, "[%s] Phase %d complete!\n", lib, phasenum);

	++phasenum;
}

#endif
