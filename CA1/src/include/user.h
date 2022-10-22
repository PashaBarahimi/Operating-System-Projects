#ifndef _USER_H_
#define _USER_H_

#define BUF_SIZE 1024
#define NAME_LEN 32
#define ADVERT_NAME_LEN 64
#define NEGOTIATION_TIMEOUT 60
#define NET_ADDR "127.0.0.1"
#define BROADCAST_ADDR "192.168.1.255"
#define ACCEPT_OFFER "accept"
#define REJECT_OFFER "reject"

#define ADVERT_COLOR "\033[1;36m"
#define RESET_COLOR "\033[0m"


typedef enum
{
    AVAILABLE,
    NEGOTIATING,
    SOLD,
} advertisement_status;

extern char buf[BUF_SIZE];
extern char name[NAME_LEN];
extern int bcSockFd;
extern struct sockaddr_in bcAddr;

extern void interruptHandler(int sig);

int getBroadcastSockFd(void);
int getPort(const char* portStr);
int bindSockToPort(const char* portStr, const char* add);
void getName();
void setupUser(int argc, const char* argv[]);

#endif
