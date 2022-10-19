#include "include/log.h"

static void log_msg(const char* level, const char* msg, const char* perr)
{
    write(STDOUT_FILENO, level, strlen(level));
    write(STDOUT_FILENO, msg, strlen(msg));
    if (perr != NULL) {
        write(STDOUT_FILENO, ": ", 2);
        write(STDOUT_FILENO, perr, strlen(perr));
    }
    write(STDOUT_FILENO, "\n", 1);
}

void log_info(const char* msg)
{
    log_msg(INFO, msg, NULL);
}

void log_warn(const char* msg)
{
    log_msg(WARN, msg, NULL);
}

void log_error(const char* msg)
{
    log_msg(ERROR, msg, NULL);
}

void log_perror(const char* msg)
{
    char* err = strerror(errno);
    log_msg(ERROR, msg, err);
}
