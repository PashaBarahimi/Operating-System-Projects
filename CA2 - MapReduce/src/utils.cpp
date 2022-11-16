#include "defs.hpp"

#include <sstream>

std::vector<std::string> split(const std::string &str, char delim)
{
    std::vector<std::string> elems;
    std::istringstream ss(str);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        item.erase(0, item.find_first_not_of(' '));
        item.erase(item.find_last_not_of(' ') + 1);
        if (!item.empty())
            elems.push_back(item);
    }
    return elems;
}

std::string fifoFileName(const std::string &genre, int part)
{
    return ".fifo/" + genre + "_" + std::to_string(part);
}
