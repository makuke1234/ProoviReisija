#ifndef LOGGER_H
#define LOGGER_H

#if LOGGING_ENABLE == 1


/**
 * @brief Initialise Logger, exits on failure
 * 
 */
void initLogger(void);
/**
 * @brief Write a Logger log message
 * 
 * @param function Function name of writer
 * @param format Standard printf message format
 * @param ... Variadic format arguments
 */
void writeLogger_inner(const char * restrict function, const char * restrict format, ...);

/**
 * @brief Start Logger timestamp
 * 
 */
void loggerStart(void);
/**
 * @brief Stop Logger timestamp
 * 
 * @param funcName Function name of timestamp writer
 */
void loggerEnd_inner(const char * funcName);


#if __STDC_VERSION__ >= 199901L
	#define writeLogger(...) writeLogger_inner(__func__, ##__VA_ARGS__)
	#define loggerEnd() loggerEnd_inner(__func__)
#else
	#error "Compiler doesn't support __func__!"
#endif

#else

#define initLogger()
#define writeLogger(...)
#define loggerStart()
#define loggerEnd()

typedef int make_iso_compilers_happy;

#endif


#endif
