#include <fstream>
#include <vector>
#include <regex>
#include <string>
#include <filesystem>

#include "defs.hpp"
#include "log.hpp"

const std::string libRegex = "^part([0-9]+)\\.csv$";
const std::string genreFile = "genres.csv";

bool isLibrary(const std::string &path)
{
    std::regex regex(libRegex);
    return std::regex_match(path, regex);
}

bool checkLibPath(const std::string &path)
{
    if (!std::filesystem::exists(path))
    {
        log::error("Path '%s' does not exist", path.c_str());
        return false;
    }
    if (!std::filesystem::is_directory(path))
    {
        log::error("Path '%s' is not a directory", path.c_str());
        return false;
    }
    return true;
}

bool getLibraries(const std::string &path, std::vector<std::filesystem::directory_entry> &libraries)
{
    if (!checkLibPath(path))
        return false;

    bool genresExist = false;
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (!entry.is_regular_file())
            continue;
        if (entry.path().filename() == genreFile)
        {
            genresExist = true;
            log::info("Found genres file '%s'", entry.path().filename().c_str());
            continue;
        }
        if (isLibrary(entry.path().filename()))
        {
            libraries.push_back(entry);
            log::info("Found library file '%s'", entry.path().filename().c_str());
        }
    }
    if (!genresExist)
    {
        log::error("Genres file '%s' does not exist in library", genreFile.c_str());
        return false;
    }
    if (libraries.empty())
    {
        log::error("No library files found in library");
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    log::setLevel(LOG_LEVEL);

    if (argc < 2)
    {
        log::error("Usage: %s <library>", argv[0]);
        return 1;
    }

    std::vector<std::filesystem::directory_entry> libraries;
    if (!getLibraries(argv[1], libraries))
        return 1;

    return 0;
}
