#ifndef _USER_H_
#define _USER_H_

#define BUF_SIZE 1024
#define NET_ADDR "127.0.0.1"
#define BROADCAST_ADDR "192.168.1.255"

extern char buf[BUF_SIZE];

int getBroadcastSockFd(void);
int bindSockToPort(int sockFd, const char* portStr);

#endif
