#include "log.hpp"

#include <cerrno>
#include <iostream>
#include <cstring>

#include "colors.hpp"

const std::string INFO  = Color::Green  + "[INFO]"  + Color::Reset;
const std::string WARN  = Color::Yellow + "[WARN]"  + Color::Reset;
const std::string ERROR = Color::Red    + "[ERRO]"  + Color::Reset;

void log::setLevel(log::level l)
{
    if (l >= log::level::info && l <= log::level::none)
        lLevel = l;
}

void log::logMsg(const std::string &level, const std::string &fmt, va_list &args, const std::string &perr = "")
{
    std::cout << level << " ";
    vprintf(fmt.c_str(), args);
    if (perr != "")
        std::cout << ": " << perr;
    std::cout << std::endl;
}

void log::info(const std::string &fmt, ...)
{
    if (lLevel <= log::level::info)
    {
        va_list args;
        va_start(args, fmt);
        logMsg(INFO, fmt, args);
        va_end(args);
    }
}

void log::warn(const std::string &fmt, ...)
{
    if (lLevel <= log::level::warn)
    {
        va_list args;
        va_start(args, fmt);
        logMsg(WARN, fmt, args);
        va_end(args);
    }
}

void log::error(const std::string &fmt, ...)
{
    if (lLevel <= log::level::error)
    {
        va_list args;
        va_start(args, fmt);
        logMsg(ERROR, fmt, args);
        va_end(args);
    }
}

void log::perror(const std::string &fmt, ...)
{
    if (lLevel <= log::level::error)
    {
        va_list args;
        va_start(args, fmt);
        logMsg(ERROR, fmt, args, strerror(errno));
        va_end(args);
    }
}
