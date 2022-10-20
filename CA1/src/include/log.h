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
} log_level;

static log_level l_level = LOG_INFO;

void set_log_level(log_level level);

void log_info(const char* fmt, ...);
void log_warn(const char* fmt, ...);
void log_error(const char* fmt, ...);
void log_perror(const char* fmt, ...);

#endif
