#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "include/user.h"
#include "include/log.h"

char buf[BUF_SIZE];
char name[NAME_LEN];

int getBroadcastSockFd(void)
{
    int sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockFd < 0)
    {
        log_perror("socket");
        return -1;
    }
    int broadcast = 1, opt = 1;
    if (setsockopt(sockFd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
    {
        log_perror("setsockopt");
        return -1;
    }
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        log_perror("setsockopt");
        return -1;
    }
    return sockFd;
}

int bindSockToPort(int sockFd, const char* portStr, const char* addr)
{
    long port = strtol(portStr, NULL, 10);
    if (port < 1024 || port > 65535)
    {
        log_error("Invalid port number: %s", portStr);
        return -1;
    }
    struct sockaddr_in bc_address;
    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons((uint16_t)port);
    bc_address.sin_addr.s_addr = inet_addr(addr);
    if (bind(sockFd, (struct sockaddr*)&bc_address, sizeof(bc_address)) < 0)
    {
        log_perror("bind");
        return -1;
    }
    return (int)port;
}

void getName()
{
    memset(name, 0, NAME_LEN);
    write(STDOUT_FILENO, "Enter your name: ", 17);
    int n = read(STDIN_FILENO, name, NAME_LEN);
    if (n <= 0)
    {
        log_perror("read");
        exit(EXIT_FAILURE);
    }
    else if (n == NAME_LEN)
    {
        log_error("Name too long");
        exit(EXIT_FAILURE);
    }
    name[n - 1] = '\0';
}
