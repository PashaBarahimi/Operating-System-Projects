#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <unistd.h>
#include <fcntl.h>

#include "defs.hpp"
#include "log.hpp"

typedef struct
{
    int part;
    std::string filePath;
    std::unordered_map<std::string, int> genres;
} WorkerData;

WorkerData getData()
{
    WorkerData data;
    std::cin >> data.part >> data.filePath;
    int n;
    std::cin >> n;
    for (int i = 0; i < n; i++)
    {
        std::string genre;
        std::cin >> genre;
        data.genres[genre] = 0;
    }
    return data;
}

bool processFile(WorkerData &data)
{
    std::ifstream file(data.filePath);
    if (!file.is_open())
    {
        log::error("Could not open file '%s'", data.filePath.c_str());
        return false;
    }
    std::string line;
    while (std::getline(file, line))
    {
        std::vector<std::string> fields = split(line, ',');
        if (fields.size() < 2)
        {
            log::warn("Invalid line '%s' in file '%s'", line.c_str(), data.filePath.c_str());
            continue;
        }
        std::string genre = fields[1];
        if (data.genres.find(genre) != data.genres.end())
            ++data.genres[genre];
        else
            log::warn("Invalid genre '%s' in file '%s'", genre.c_str(), data.filePath.c_str());
    }
    return true;
}

bool sendDataToReducer(WorkerData &data)
{
    for (const auto &genre : data.genres)
    {
        std::string fifoName = fifoFileName(genre.first, data.part);
        int fd = open(fifoName.c_str(), O_WRONLY);
        if (fd == -1)
        {
            log::perror("open");
            return false;
        }
        std::string count = std::to_string(genre.second) + '\n';
        if (write(fd, count.c_str(), count.size()) == -1)
        {
            log::perror("write");
            return false;
        }
        close(fd);
        log::info("Sent %d to %s", genre.second, fifoName.c_str());
    }
    return true;
}

int main(int argc, char *argv[])
{
    log::setLevel(LOG_LEVEL);

    WorkerData data = getData();
    log::info("Map worker %d started", data.part);
    if (!processFile(data))
        return EXIT_FAILURE;
    log::info("File %s processed", data.filePath.c_str());
    if (!sendDataToReducer(data))
        return EXIT_FAILURE;
    log::info("Map worker %d finished", data.part);
    return EXIT_SUCCESS;
}
