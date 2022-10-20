#ifndef _USER_H_
#define _USER_H_

#define BUF_SIZE 1024
#define NAME_LEN 32
#define NET_ADDR INADDR_ANY
#define BROADCAST_ADDR "192.168.1.255"

extern char buf[BUF_SIZE];
extern char name[NAME_LEN];

int getBroadcastSockFd(void);
int bindSockToPort(int sockFd, const char* portStr, const char* add);
void getName();

#endif
