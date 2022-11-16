#ifndef _LOG_HPP_
#define _LOG_HPP_

#include <cstdarg>
#include <string>

class log
{
public:
    enum class level
    {
        INFO,
        WARN,
        ERROR,
        NONE
    };

    log(level l = level::INFO);

    void setLevel(level l);
    void logInfo(const std::string &fmt, ...) const;
    void logWarn(const std::string &fmt, ...) const;
    void logError(const std::string &fmt, ...) const;
    void logPError(const std::string &fmt, ...) const;
private:
    void logMsg(const std::string &level, const std::string &fmt, va_list &args, const std::string &perr) const;
    level lLevel;
};

#endif
