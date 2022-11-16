#include "log.hpp"

#include <cerrno>
#include <iostream>
#include <cstring>

const std::string INFO  = "\033[32m[INFO]\033[0m";
const std::string WARN  = "\033[33m[WARN]\033[0m";
const std::string ERROR = "\033[31m[ERRO]\033[0m";

log::log(log::level l) : lLevel(l) { }

void log::setLevel(log::level l)
{
    if (l >= log::level::INFO && l <= log::level::NONE)
        lLevel = l;
}

void log::logMsg(const std::string &level, const std::string &fmt, va_list &args, const std::string &perr = "") const
{
    std::cout << level << " ";
    vprintf(fmt.c_str(), args);
    if (perr != "")
        std::cout << ": " << perr;
    std::cout << std::endl;
}

void log::logInfo(const std::string &fmt, ...) const
{
    if (lLevel <= log::level::INFO)
    {
        va_list args;
        va_start(args, fmt);
        logMsg(INFO, fmt, args);
        va_end(args);
    }
}

void log::logWarn(const std::string &fmt, ...) const
{
    if (lLevel <= log::level::WARN)
    {
        va_list args;
        va_start(args, fmt);
        logMsg(WARN, fmt, args);
        va_end(args);
    }
}

void log::logError(const std::string &fmt, ...) const
{
    if (lLevel <= log::level::ERROR)
    {
        va_list args;
        va_start(args, fmt);
        logMsg(ERROR, fmt, args);
        va_end(args);
    }
}

void log::logPError(const std::string &fmt, ...) const
{
    if (lLevel <= log::level::ERROR)
    {
        va_list args;
        va_start(args, fmt);
        logMsg(ERROR, fmt, args, strerror(errno));
        va_end(args);
    }
}
