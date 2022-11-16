#include <iostream>
#include <unordered_map>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

#include "defs.hpp"
#include "log.hpp"

constexpr int MAX_DIGIT_COUNT = 18;

typedef struct
{
    std::string genre;
    std::unordered_map<std::string, int> fifoFiles;
} WorkerData;

WorkerData getData()
{
    WorkerData data;
    std::cin >> data.genre;
    int n;
    std::cin >> n;
    for (int i = 0; i < n; i++)
    {
        std::string fifoFile;
        std::cin >> fifoFile;
        data.fifoFiles[fifoFile] = -1;
    }
    return data;
}

bool openFifoFiles(WorkerData &data)
{
    for (auto &fifoFile : data.fifoFiles)
    {
        fifoFile.second = open(fifoFile.first.c_str(), O_WRONLY);
        if (fifoFile.second == -1)
        {
            log::error("Could not open fifo file '%s'", fifoFile.first.c_str());
            return false;
        }
    }
    return true;
}

long long readCount(int fd)
{
    char buffer[MAX_DIGIT_COUNT + 1];
    int count = read(fd, buffer, MAX_DIGIT_COUNT);
    if (count == -1)
    {
        log::perror("read");
        return -1;
    }
    buffer[count] = '\0';
    return std::stoll(buffer);
}

long long getGenreCount(WorkerData &data)
{
    long long count = 0;
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    for (const auto &fifoFile : data.fifoFiles)
        FD_SET(fifoFile.second, &master_set);
    while (!data.fifoFiles.empty())
    {
        working_set = master_set;
        int ready = select(FD_SETSIZE, &working_set, NULL, NULL, NULL);
        if (ready == -1)
        {
            log::perror("select");
            return -1;
        }
        for (const auto &fifoFile : data.fifoFiles)
            if (FD_ISSET(fifoFile.second, &working_set))
            {
                long long partialCount = readCount(fifoFile.second);
                if (partialCount == -1)
                    return -1;
                count += partialCount;
                close(fifoFile.second);
                FD_CLR(fifoFile.second, &master_set);
                if (unlink(fifoFile.first.c_str()) == -1)
                    log::perror("unlink");
                data.fifoFiles.erase(fifoFile.first);
                log::info("Read %lld from '%s'", partialCount, fifoFile.first.c_str());
                if (--ready == 0)
                    break;
            }
    }
    return count;
}

int main()
{
    log::setLevel(LOG_LEVEL);

    WorkerData data = getData();
    log::info("Reducer '%s' started", data.genre.c_str());
    if (!openFifoFiles(data))
        return EXIT_FAILURE;
    long long count = getGenreCount(data);
    if (count == -1)
        return EXIT_FAILURE;
    std::cout << data.genre << ": " << count << std::endl;
    log::info("Reducer '%s' finished", data.genre.c_str());
    return EXIT_SUCCESS;
}
