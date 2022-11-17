#include <fstream>
#include <vector>
#include <regex>
#include <string>
#include <filesystem>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "defs.hpp"
#include "log.hpp"

const std::string LIB_REGEX = "^part([0-9]+)\\.csv$";
const std::string GENRE_FILE = "genres.csv";

const char MAP_WORKER[] = "./bin/map_worker";
const char RED_WORKER[] = "./bin/red_worker";

bool isLibrary(const std::string &path)
{
    std::regex regex(LIB_REGEX);
    return std::regex_match(path, regex);
}

int extractNumber(const std::string &path)
{
    std::regex regex(LIB_REGEX);
    std::smatch match;
    if (!std::regex_search(path, match, regex))
        return -1;
    return std::stoi(match[1]);
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
        else if (entry.path().filename() != GENRE_FILE)
            log::warn("Ignoring file '%s'", entry.path().filename().c_str());
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

pid_t runWorker(const char *worker, int *pipefd)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        log::perror("fork");
        return -1;
    }
    if (pid == 0)
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        execl(worker, worker, nullptr);
        log::perror("execl");
        exit(EXIT_FAILURE);
    }
    close(pipefd[0]);
    return pid;
}

bool sendDataToMapWorker(const std::filesystem::directory_entry &lib, const std::vector<std::string> &genres, int fd)
{
    std::ostringstream ss;
    ss << extractNumber(lib.path().filename()) << " " << lib.path().string() << std::endl;
    ss << genres.size() << std::endl;
    for (const auto &genre : genres)
        ss << genre << std::endl;
    std::string data = ss.str();
    if (write(fd, data.c_str(), data.size()) == -1)
    {
        log::perror("write");
        return false;
    }
    return true;
}

bool runMapWorkers(const std::vector<std::filesystem::directory_entry> &libraries, const std::vector<std::string> &genres, std::vector<pid_t> &pids)
{
    for (const auto &lib : libraries)
    {
        int pipefd[2];
        if (pipe(pipefd) == -1)
        {
            log::perror("pipe");
            return false;
        }
        pid_t pid = runWorker(MAP_WORKER, pipefd);
        if (pid == -1)
            return false;
        pids.push_back(pid);
        if (!sendDataToMapWorker(lib, genres, pipefd[1]))
            return false;
        if (close(pipefd[1]) == -1)
            log::perror("close");
        log::info("Started map worker for library '%s' with pid %d", lib.path().filename().c_str(), pid);
    }
    return true;
}

bool sendDataToRedWorker(const std::vector<std::string> &fifoFiles, const std::string &genre, int fd)
{
    std::ostringstream ss;
    ss << genre << std::endl;
    ss << fifoFiles.size() << std::endl;
    for (const auto &fifo : fifoFiles)
        ss << fifo << std::endl;
    std::string data = ss.str();
    if (write(fd, data.c_str(), data.size()) == -1)
    {
        log::perror("write");
        return false;
    }
    return true;
}

bool runRedWorkers(const std::vector<std::vector<std::string>> &fifoFiles, const std::vector<std::string> &genres, std::vector<pid_t> &pids)
{
    for (size_t i = 0; i < genres.size(); i++)
    {
        int pipefd[2];
        if (pipe(pipefd) == -1)
        {
            log::perror("pipe");
            return false;
        }
        pid_t pid = runWorker(RED_WORKER, pipefd);
        if (pid == -1)
            return false;
        pids.push_back(pid);
        if (!sendDataToRedWorker(fifoFiles[i], genres[i], pipefd[1]))
            return false;
        if (close(pipefd[1]) == -1)
            log::perror("close");
        log::info("Started reduce worker for genre '%s' with pid %d", genres[i].c_str(), pid);
    }
    return true;
}

bool makeFifoFiles(const std::vector<std::string> &genres, const std::vector<std::filesystem::directory_entry> &libraries, std::vector<std::vector<std::string>> &fifoFiles)
{
    for (const auto &genre : genres)
    {
        std::vector<std::string> fifoFilesForGenre;
        for (const auto &lib : libraries)
        {
            std::string fifoFile = fifoFileName(genre, extractNumber(lib.path().filename()));
            if (mkfifo(fifoFile.c_str(), 0666) == -1)
            {
                log::perror("mkfifo");
                return false;
            }
            fifoFilesForGenre.push_back(fifoFile);
        }
        fifoFiles.push_back(fifoFilesForGenre);
    }
    return true;
}

void waitForWorkers(const std::vector<pid_t> &pids)
{
    for (const auto &pid : pids)
    {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            if (WEXITSTATUS(status) != EXIT_SUCCESS)
                log::error("Worker with pid %d exited with status %d", pid, WEXITSTATUS(status));
            else
                log::info("Worker with pid %d exited with status %d", pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
            log::error("Worker with pid %d killed by signal %d", pid, WTERMSIG(status));
    }
}

void removeFifoFiles(const std::vector<std::vector<std::string>> &fifoFiles)
{
    for (const auto &fifoFilesForGenre : fifoFiles)
        for (const auto &fifoFile : fifoFilesForGenre)
            if (unlink(fifoFile.c_str()) == -1)
                log::perror("unlink");
}

bool runWorkers(const std::vector<std::filesystem::directory_entry> &libraries, const std::vector<std::string> &genres)
{
    std::vector<pid_t> pids;
    std::vector<std::vector<std::string>> fifoFiles;
    if (!makeFifoFiles(genres, libraries, fifoFiles))
        return false;
    if (!runMapWorkers(libraries, genres, pids))
        return false;
    if (!runRedWorkers(fifoFiles, genres, pids))
        return false;
    waitForWorkers(pids);
    removeFifoFiles(fifoFiles);
    return true;
}

int main(int argc, char *argv[])
{
    log::setLevel(LOG_LEVEL);

    if (argc < 2)
    {
        log::error("Usage: %s <library>", argv[0]);
        return EXIT_FAILURE;
    }

    std::vector<std::filesystem::directory_entry> libraries;
    std::vector<std::string> genres;
    if (!getLibraries(argv[1], libraries))
        return EXIT_FAILURE;
    if (!getGenres(argv[1], genres))
        return EXIT_FAILURE;

    if (!runWorkers(libraries, genres))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
