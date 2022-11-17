#include <string>

#include <unistd.h>

namespace Color
{
static bool enabled = isatty(fileno(stdout));

inline std::string Reset  = enabled ? "\033[0m"  : "";
inline std::string Red    = enabled ? "\033[31m" : "";
inline std::string Green  = enabled ? "\033[32m" : "";
inline std::string Yellow = enabled ? "\033[33m" : "";
}
