#include "include/log.h"

void set_log_level(enum log_level level)
{
    if (level < 0 || level > NO_LOG) return;
    l_level = level;
}

void log_msg(const char* level, const char* fmt, va_list args, const char* perr)
{
    memset(buf, 0, BUF_SIZE);
    int len = snprintf(buf, BUF_SIZE, "%s", level);
    len += vsnprintf(buf + len, BUF_SIZE - len, fmt, args);
    if (perr != NULL)
    {
        strcpy(buf + len, ": ");
        strcpy(buf + len + 2, perr);
    }
    strcat(buf, "\n");
    write(STDOUT_FILENO, buf, strlen(buf));
}

void log_info(const char* fmt, ...)
{
    if (l_level > LOG_INFO) return;
    va_list args;
    va_start(args, fmt);
    log_msg(INFO, fmt, args, NULL);
    va_end(args);
}

void log_warn(const char* fmt, ...)
{
    if (l_level > LOG_WARN) return;
    va_list args;
    va_start(args, fmt);
    log_msg(WARN, fmt, args, NULL);
    va_end(args);
}

void log_error(const char* fmt, ...)
{
    if (l_level > LOG_ERROR) return;
    va_list args;
    va_start(args, fmt);
    log_msg(ERROR, fmt, args, NULL);
    va_end(args);
}

void log_perror(const char* fmt, ...)
{
    if (l_level > LOG_ERROR) return;
    char* err = strerror(errno);
    va_list args;
    va_start(args, fmt);
    log_msg(ERROR, fmt, args, err);
    va_end(args);
}
