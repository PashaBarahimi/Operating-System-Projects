#include <stdlib.h>

#include "include/log.h"
#include "include/user.h"

int main(int argc, const char* argv[])
{
    set_log_level(LOG_INFO);
    int sockFd, port;

    if (argc != 2)
    {
        log_error("Usage: ./seller <port>");
        exit(EXIT_FAILURE);
    }

    if ((sockFd = getBroadcastSockFd()) < 0) exit(EXIT_FAILURE);
    if ((port = bindSockToPort(sockFd, argv[1], BROADCAST_ADDR)) < 0) exit(EXIT_FAILURE);
    log_info("Socket bound to port %d", port);

    getName();
    log_info("Name: %s", name);
}
