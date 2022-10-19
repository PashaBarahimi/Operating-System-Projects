#include <string.h>
#include <unistd.h>
#include <errno.h>

#define INFO "\033[32m[INFO]\033[0m "
#define WARN "\033[33m[WARN]\033[0m "
#define ERROR "\033[31m[ERROR]\033[0m "

static void log_msg(const char* level, const char* msg, const char* perr);

void log_info(const char* msg);
void log_warn(const char* msg);
void log_error(const char* msg);
void log_perror(const char* msg);
