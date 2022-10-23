#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "include/log.h"
#include "include/user.h"

#define MIN_ADVERTS_COUNT 20

#define HELP_MSG "Commands:\n" \
                 "\thelp - show this message\n" \
                 "\tlist - show the list of advertisements\n" \
                 "\tnegotiate (--id <id>)|(--name <name>) - start negotiation with advertiser\n" \
                 "\tsend <message> - send message to asdvertiser\n" \
                 "\toffer <price> - send offer to advertiser\n" \
                 "\tend - end negotiation\n" \
                 "\texit - exit the program\n"

typedef struct
{
    char name[ADVERT_NAME_LEN];
    char owner[NAME_LEN];
    advertisementStatus status;
    uint16_t port;
} advertisement;

struct
{
    advertisement* ads;
    size_t capacity;
    size_t count;
} adverts;

fd_set masterSet;
int maxFd;
int alarmFired = 0, negSockFd = -1;


void alarmHandler(int sig)
{
    alarmFired = 1;
}

void freeResources()
{
    if (negSockFd != -1)
        close(negSockFd);
    close(bcSockFd);
    if (adverts.ads != NULL)
        free(adverts.ads);
}

void interruptHandler(int sig)
{
    removeLine();
    logWarn("Interrupted");
    freeResources();
    exit(EXIT_SUCCESS);
}

void addAdvertisement(const char* name, const char* owner, uint16_t port, advertisementStatus status)
{
    if (adverts.count == adverts.capacity)
    {
        adverts.capacity = adverts.capacity == 0 ? MIN_ADVERTS_COUNT : adverts.capacity * 2;
        adverts.ads = (advertisement*)realloc(adverts.ads, adverts.capacity * sizeof(advertisement));
    }
    strcpy(adverts.ads[adverts.count].name, name);
    strcpy(adverts.ads[adverts.count].owner, owner);
    adverts.ads[adverts.count].port = port;
    adverts.ads[adverts.count].status = status;
    ++adverts.count;
}

void updateAdvertisement(const char* name, const char* owner, uint16_t port, advertisementStatus status)
{
    for (int i = 0; i < adverts.count; ++i)
        if (strcmp(adverts.ads[i].name, name) == 0)
        {
            adverts.ads[i].status = status;
            logInfo("Advertisement %s updated", name);
            return;
        }
    addAdvertisement(name, owner, port, status);
    logInfo("Advertisement %s added", name);
}

int recieveBroadcast()
{
    memset(buf, 0, BUF_SIZE);
    int bytes = recvfrom(bcSockFd, buf, BUF_SIZE, 0, NULL, NULL);
    if (bytes < 0)
        logError("Error while recieving broadcast");
    if (bytes == 0)
        logError("Broadcast connection closed");
    char* name = strtok(buf, MESSAGE_DELIMITER);
    char* owner = strtok(NULL, MESSAGE_DELIMITER);
    char* portStr = strtok(NULL, MESSAGE_DELIMITER);
    char* statusStr = strtok(NULL, " \n\0");
    int port;
    advertisementStatus status;
    if (name != NULL && owner != NULL && portStr != NULL && (port = getPort(portStr)) != -1 && statusStr != NULL)
    {
        if (strcmp(statusStr, "SOLD") == 0)
            status = SOLD;
        else if (strcmp(statusStr, "AVAILABLE") == 0)
            status = AVAILABLE;
        else if (strcmp(statusStr, "NEGOTIATING") == 0)
            status = NEGOTIATING;
        else
        {
            logWarn("Invalid advertisement status");
            return 1;
        }
        logInfo("New broadcast recieved");
        updateAdvertisement(name, owner, port, status);
        return 1;
    }
    logWarn("Invalid broadcast");
    return 0;
}

void listAdvertisements()
{
    write(STDOUT_FILENO, "\nAdvertisements:\n", 17);
    write(STDOUT_FILENO, ADVERT_COLOR, strlen(ADVERT_COLOR));
    for (int i = 0; i < adverts.count; ++i)
    {
        memset(buf, '\0', BUF_SIZE);
        char* status = adverts.ads[i].status == AVAILABLE ? "AVAILABLE" : (adverts.ads[i].status == NEGOTIATING ? "NEGOTIATING" : "SOLD");
        sprintf(buf, "%d -> Seller: %s, Advertisement: %s, Status: %s, Port: %d\n", i + 1, adverts.ads[i].owner, adverts.ads[i].name, status, adverts.ads[i].port);
        write(STDOUT_FILENO, buf, strlen(buf));
    }
    write(STDOUT_FILENO, RESET_COLOR, strlen(RESET_COLOR));
}

int getAdvertiserAnswer()
{
    memset(buf, 0, BUF_SIZE);
    int bytes = recv(negSockFd, buf, BUF_SIZE, 0);
    if (bytes < 0)
    {
        logError("Error while recieving answer from advertiser");
        return 0;
    }
    if (bytes == 0)
    {
        logWarn("Connection closed by advertiser");
        return 0;
    }
    logInfo("New answer recieved from adveriser");
    write(STDOUT_FILENO, "Advertiser: ", 12);
    write(STDOUT_FILENO, buf, strlen(buf));
    write(STDOUT_FILENO, "\n", 1);
    if (strcmp(buf, ACCEPT_OFFER) == 0)
    {
        logInfo("Offer accepted by advertiser");
        return 0;
    }
    if (strcmp(buf, REJECT_OFFER) == 0)
    {
        logWarn("Offer rejected by advertiser");
        return 0;
    }
    return 1;
}

void startNegotiation()
{
    logInfo("Negotiation started");
    alarmFired = 0;
    alarm(NEGOTIATION_TIMEOUT);
    if (negSockFd > maxFd)
        maxFd = negSockFd;
    FD_SET(negSockFd, &masterSet);
}

void endNegotiation()
{
    logInfo("Negotiation ended");
    alarm(0);
    FD_CLR(negSockFd, &masterSet);
    close(negSockFd);
    negSockFd = -1;
    alarmFired = 0;
}

int findAdvertisement(const char* name)
{
    if (name != NULL)
    {
        for (int i = 0; i < adverts.count; ++i)
            if (strcmp(adverts.ads[i].name, name) == 0)
                return i;
        return -1;
    }
    return -1;
}

void connectToAdvertiser(int index)
{
    negSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (negSockFd < 0)
    {
        logPError("socket");
        return;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(adverts.ads[index].port);
    addr.sin_addr.s_addr = inet_addr(NET_ADDR);
    if (connect(negSockFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        logPError("connect");
        close(negSockFd);
        negSockFd = -1;
        return;
    }
    logInfo("Connected to advertiser");
    startNegotiation();
}

int findAdvertisementFromBuf(const char* endDelim)
{
    char* type = strtok(NULL, " ");
    if (type == NULL)
    {
        logError("Missing argument");
        return -1;
    }
    char* token = strtok(NULL, endDelim);
    if (token == NULL)
    {
        logError("Missing argument");
        return -1;
    }
    if (strcmp(type, "--id") == 0)
    {
        int id = atoi(token);
        if (id <= 0 || id > adverts.count)
        {
            logError("Invalid id");
            return -1;
        }
        return id - 1;
    }
    if (strcmp(type, "--name") == 0)
    {
        for (int i = 0; i < adverts.count; i++)
            if (strcmp(adverts.ads[i].name, token) == 0)
                return i;
        logError("Advertisement %s not found", token);
        return -1;
    }
    logError("Invalid argument");
    return -1;
}

void negotiate()
{
    if (negSockFd != -1)
    {
        logError("Already negotiating");
        return;
    }
    int index = findAdvertisementFromBuf(" \n");
    if (index == -1)
        return;
    if (adverts.ads[index].status != AVAILABLE)
    {
        logError("Advertisement %s is not available", adverts.ads[index].name);
        return;
    }
    connectToAdvertiser(index);
}

void sendMsg()
{
    if (negSockFd == -1)
    {
        logError("Not negotiating");
        return;
    }
    char* message = strtok(NULL, "\n");
    if (message == NULL)
    {
        logError("Invalid message");
        return;
    }
    char* msg = malloc(strlen(message) + 1);
    strcpy(msg, message);
    memset(buf, '\0', BUF_SIZE);
    sprintf(buf, "msg%s%s", MESSAGE_DELIMITER, msg);
    free(msg);
    if (send(negSockFd, buf, strlen(buf), 0) < 0)
    {
        logPError("send");
        endNegotiation();
    }
    alarm(NEGOTIATION_TIMEOUT);
    logInfo("Message sent to advertiser");
}

void sendOffer()
{
    if (negSockFd == -1)
    {
        logError("Not negotiating");
        return;
    }
    char* offer = strtok(NULL, " \n");
    int offerInt = atoi(offer);
    if (offer == NULL || offerInt < 1)
    {
        logError("Invalid offer");
        return;
    }
    memset(buf, '\0', BUF_SIZE);
    sprintf(buf, "offer%s%d", MESSAGE_DELIMITER, offerInt);
    if (send(negSockFd, buf, strlen(buf), 0) < 0)
    {
        logPError("send");
        endNegotiation();
    }
    alarm(NEGOTIATION_TIMEOUT);
    logInfo("Offer sent to advertiser");
}

int readCommand()
{
    memset(buf, 0, BUF_SIZE);
    read(STDIN_FILENO, buf, BUF_SIZE);
    char* command = strtok(buf, " \n");
    if (command == NULL)
        return 0;
    if (strcmp(command, "exit") == 0)
    {
        logInfo("Exiting");
        return 1;
    }
    if (strcmp(command, "help") == 0)
        write(STDOUT_FILENO, HELP_MSG, strlen(HELP_MSG));
    else if (strcmp(command, "list") == 0)
        listAdvertisements();
    else if (strcmp(command, "end") == 0)
    {
        if (negSockFd == -1)
        {
            logWarn("No negotiation in progress");
            return 0;
        }
        logInfo("Negotiation ended by user");
        endNegotiation();
    }
    else if (strcmp(command, "negotiate") == 0)
        negotiate();
    else if (strcmp(command, "offer") == 0)
        sendOffer();
    else if (strcmp(command, "send") == 0)
        sendMsg();
    else
        logError("Invalid command");
    return 0;
}

int handleEvent(fd_set* workingSet)
{
    if (FD_ISSET(STDIN_FILENO, workingSet))
    {
        int exit = readCommand();
        if (exit) return 0;
    }
    removeLine();
    if (FD_ISSET(bcSockFd, workingSet))
        if (recieveBroadcast())
            listAdvertisements();
    if (negSockFd != -1 && FD_ISSET(negSockFd, workingSet))
    {
        int isNegotiating = getAdvertiserAnswer();
        if (isNegotiating)
            alarm(NEGOTIATION_TIMEOUT);
        else
            endNegotiation();
    }
    return 1;
}

void runClient()
{
    fd_set workingSet;
    FD_ZERO(&masterSet);
    FD_ZERO(&workingSet);
    FD_SET(STDIN_FILENO, &masterSet);
    FD_SET(bcSockFd, &masterSet);
    maxFd = bcSockFd;
    sigaction(SIGALRM, &(struct sigaction){.sa_handler = alarmHandler, .sa_flags = SA_RESTART}, NULL);
    logInfo("Looking for advertisements...");
    while (1)
    {
        workingSet = masterSet;
        write(STDOUT_FILENO, PROMPT, strlen(PROMPT));
        if (select(maxFd + 1, &workingSet, NULL, NULL, NULL) < 0)
            logPError("select");
        else if (!handleEvent(&workingSet))
            break;
        if (alarmFired)
        {
            logWarn("Negotiation timed out");
            endNegotiation();
        }
    }
}

int main(int argc, const char* argv[])
{
    setLogLevel(LOG_INFO);
    setupUser(argc, argv);
    runClient();
    freeResources();
    return 0;
}
