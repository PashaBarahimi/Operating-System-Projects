#ifndef _LOG_H_
#define _LOG_H_

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#define INFO "\033[32m[INFO]\033[0m "
#define WARN "\033[33m[WARN]\033[0m "
#define ERROR "\033[31m[ERROR]\033[0m "

#define BUF_SIZE 1024

typedef enum
{
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    NO_LOG,
} logLevel;

static logLevel lLevel = LOG_INFO;

void setLogLevel(logLevel level);

void logInfo(const char* fmt, ...);
void logWarn(const char* fmt, ...);
void logError(const char* fmt, ...);
void logPError(const char* fmt, ...);

#endif
