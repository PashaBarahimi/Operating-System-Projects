#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "include/utils.h"
#include "include/log.h"

char buf[BUF_SIZE];

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

int bindSockToPort(int sockFd, const char* portStr)
{
    long port = strtol(portStr, NULL, 10);
    if (port < 1024 || port > 65535)
    {
        log_error("invalid port number: %s", portStr);
        return -1;
    }
    struct sockaddr_in bc_address;
    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(port);
    bc_address.sin_addr.s_addr = inet_addr(BROADCAST_ADDR);
    if (bind(sockFd, (struct sockaddr*)&bc_address, sizeof(bc_address)) < 0)
    {
        log_perror("bind");
        return -1;
    }
    return 0;
}
