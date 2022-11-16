#ifndef _DEFS_HPP_
#define _DEFS_HPP_

#include <vector>

#include "log.hpp"

constexpr log::level LOG_LEVEL = log::level::info;

std::vector<std::string> split(const std::string &str, char delim = ',');

#endif
