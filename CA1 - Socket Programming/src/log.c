#include "include/log.h"

char buffer[BUF_SIZE];

void setLogLevel(logLevel level)
{
    if (level < 0 || level > NO_LOG) return;
    lLevel = level;
}

void logMsg(const char* level, const char* fmt, va_list args, const char* perr)
{
    memset(buffer, 0, BUF_SIZE);
    int len = snprintf(buffer, BUF_SIZE, "%s", level);
    len += vsnprintf(buffer + len, BUF_SIZE - len, fmt, args);
    if (perr != NULL)
    {
        strcpy(buffer + len, ": ");
        strcpy(buffer + len + 2, perr);
    }
    strcat(buffer, "\n");
    write(STDOUT_FILENO, buffer, strlen(buffer));
}

void logInfo(const char* fmt, ...)
{
    if (lLevel > LOG_INFO) return;
    va_list args;
    va_start(args, fmt);
    logMsg(INFO, fmt, args, NULL);
    va_end(args);
}

void logWarn(const char* fmt, ...)
{
    if (lLevel > LOG_WARN) return;
    va_list args;
    va_start(args, fmt);
    logMsg(WARN, fmt, args, NULL);
    va_end(args);
}

void logError(const char* fmt, ...)
{
    if (lLevel > LOG_ERROR) return;
    va_list args;
    va_start(args, fmt);
    logMsg(ERROR, fmt, args, NULL);
    va_end(args);
}

void logPError(const char* fmt, ...)
{
    if (lLevel > LOG_ERROR) return;
    char* err = strerror(errno);
    va_list args;
    va_start(args, fmt);
    logMsg(ERROR, fmt, args, err);
    va_end(args);
}
