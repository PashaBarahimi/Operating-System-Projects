#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "include/user.h"
#include "include/log.h"

char buf[BUF_SIZE];
char name[NAME_LEN];
int bcSockFd;
struct sockaddr_in bcAddr;

int getBroadcastSockFd(void)
{
    int sockFd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0)
    {
        logPError("socket");
        return -1;
    }
    int broadcast = 1, opt = 1;
    if (setsockopt(sockFd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
    {
        logPError("setsockopt");
        return -1;
    }
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        logPError("setsockopt");
        return -1;
    }
    return sockFd;
}

int getPort(const char* portStr)
{
    char* endPtr;
    long port = strtol(portStr, &endPtr, 10);
    if (port < 1024 || port > 65535 || *endPtr != '\0')
    {
        logError("Invalid port number: %s", portStr);
        return -1;
    }
    return (int)port;
}

int bindSockToPort(const char* portStr, const char* addr)
{
    int port = getPort(portStr);
    bcAddr.sin_family = AF_INET;
    bcAddr.sin_port = htons((uint16_t)port);
    bcAddr.sin_addr.s_addr = inet_addr(addr);
    if (bind(bcSockFd, (struct sockaddr*)&bcAddr, sizeof(bcAddr)) < 0)
    {
        logPError("bind");
        return -1;
    }
    return port;
}

void getName()
{
    memset(name, 0, NAME_LEN);
    write(STDOUT_FILENO, "Enter your name: ", 17);
    int n = read(STDIN_FILENO, name, NAME_LEN);
    if (n <= 0)
    {
        logPError("read");
        exit(EXIT_FAILURE);
    }
    else if (n == NAME_LEN)
    {
        logError("Name too long (max %d characters)", NAME_LEN - 1);
        exit(EXIT_FAILURE);
    }
    name[n - 1] = '\0';
}

void setupUser(int argc, const char* argv[])
{
    int port;

    if (argc != 2)
    {
        logError("Usage: %s <port>", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((bcSockFd = getBroadcastSockFd()) < 0) exit(EXIT_FAILURE);
    if ((port = bindSockToPort(argv[1], BROADCAST_ADDR)) < 0)
    {
        close(bcSockFd);
        exit(EXIT_FAILURE);
    }
    logInfo("Socket bound to port %d", port);

    signal(SIGINT, interruptHandler);

    getName();
    logInfo("Logged in as: %s", name);
    write(STDOUT_FILENO, "Type 'help' for help\n", 21);
}

void removeLine()
{
    write(STDOUT_FILENO, "\r", 1);
    for (int i = 0; i < BUF_SIZE && buf[i] != '\0'; ++i)
        write(STDOUT_FILENO, " ", 1);
    write(STDOUT_FILENO, "\r", 1);
}
