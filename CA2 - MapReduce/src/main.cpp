#include <fstream>
#include <vector>
#include <regex>
#include <string>
#include <filesystem>

#include "defs.hpp"
#include "log.hpp"

const std::string LIB_REGEX  = "^part([0-9]+)\\.csv$";
const std::string GENRE_FILE = "genres.csv";

bool isLibrary(const std::string &path)
{
    std::regex regex(LIB_REGEX);
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

    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (!entry.is_regular_file())
            continue;
        if (isLibrary(entry.path().filename()))
        {
            libraries.push_back(entry);
            log::info("Found library file '%s'", entry.path().filename().c_str());
        }
    }
    if (libraries.empty())
    {
        log::error("No library files found in library");
        return false;
    }
    return true;
}

bool getGenres(const std::string &path, std::vector<std::string> &genres)
{
    std::string genrePath = path + "/" + GENRE_FILE;
    std::ifstream file(genrePath);
    if (!file.is_open())
    {
        log::error("Failed to open genres file '%s'", genrePath.c_str());
        return false;
    }

    std::string line;
    std::getline(file, line);
    genres = split(line, ',');
    if (genres.empty())
    {
        log::error("No genres found in genres file '%s'", genrePath.c_str());
        return false;
    }
    log::info("Found %d genres", genres.size());
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
    std::vector<std::string> genres;
    if (!getLibraries(argv[1], libraries))
        return 1;
    if (!getGenres(argv[1], genres))
        return 1;

    return 0;
}
