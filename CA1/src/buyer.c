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
    advertisement_status status;
    uint16_t port;
} advertisement;

struct
{
    advertisement* ads;
    size_t capacity;
    size_t count;
} adverts;


int alarm_fired = 0, negSockFd = -1;

void alarmHandler(int sig)
{
    alarm_fired = 1;
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
    log_warn("Interrupted");
    freeResources();
    exit(EXIT_SUCCESS);
}

void addAdvertisement(const char* name, const char* owner, uint16_t port, advertisement_status status)
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

void updateAdvertisement(const char* name, const char* owner, uint16_t port, advertisement_status status)
{
    for (int i = 0; i < adverts.count; ++i)
        if (strcmp(adverts.ads[i].name, name) == 0)
        {
            adverts.ads[i].status = status;
            log_info("Advertisement %s updated", name);
            return;
        }
    addAdvertisement(name, owner, port, status);
    log_info("Advertisement %s added", name);
}

int recieveBroadcast()
{
    memset(buf, 0, BUF_SIZE);
    int bytes = recvfrom(bcSockFd, buf, BUF_SIZE, 0, NULL, NULL);
    if (bytes < 0)
        log_error("Error while recieving broadcast");
    if (bytes == 0)
        log_error("Broadcast connection closed");
    char* name = strtok(buf, "|");
    char* owner = strtok(NULL, "|");
    char* portStr = strtok(NULL, "|");
    char* statusStr = strtok(NULL, " \n\0");
    int port;
    advertisement_status status;
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
            log_warn("Invalid advertisement status");
            return 1;
        }
        log_info("New broadcast recieved");
        updateAdvertisement(name, owner, port, status);
        return 1;
    }
    log_warn("Invalid broadcast");
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
        log_error("Error while recieving answer from advertiser");
        return 0;
    }
    if (bytes == 0)
    {
        log_warn("Connection closed by advertiser");
        return 0;
    }
    log_info("New answer recieved from adveriser");
    write(STDOUT_FILENO, "Advertiser: ", 12);
    write(STDOUT_FILENO, buf, strlen(buf));
    write(STDOUT_FILENO, "\n", 1);
    if (strcmp(buf, ACCEPT_OFFER) == 0)
    {
        log_info("Offer accepted by advertiser");
        return 0;
    }
    if (strcmp(buf, REJECT_OFFER) == 0)
    {
        log_warn("Offer rejected by advertiser");
        return 0;
    }
    return 1;
}

void startNegotiation(int* maxFd, fd_set* masterSet)
{
    log_info("Negotiation started");
    alarm_fired = 0;
    alarm(NEGOTIATION_TIMEOUT);
    if (negSockFd > *maxFd)
        *maxFd = negSockFd;
    FD_SET(negSockFd, masterSet);
}

void endNegotiation(fd_set* masterSet)
{
    log_info("Negotiation ended");
    alarm(0);
    FD_CLR(negSockFd, masterSet);
    close(negSockFd);
    negSockFd = -1;
    alarm_fired = 0;
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

void connectToAdvertiser(int index, int* maxFd, fd_set* masterSet)
{
    negSockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (negSockFd < 0)
    {
        log_perror("socket");
        return;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(adverts.ads[index].port);
    addr.sin_addr.s_addr = inet_addr(NET_ADDR);
    if (connect(negSockFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        log_perror("connect");
        close(negSockFd);
        negSockFd = -1;
        return;
    }
    log_info("Connected to advertiser");
    startNegotiation(maxFd, masterSet);
}

void negotiate(int* maxFd, fd_set* masterSet)
{
    if (negSockFd != -1)
    {
        log_error("Already negotiating");
        return;
    }
    int index;
    char* type = strtok(NULL, " \n");
    if (type != NULL && strcmp(type, "--name") == 0)
    {
        char* name = strtok(NULL, " \n");
        if ((index = findAdvertisement(name)) == -1)
        {
            log_error("Advertisement %s not found", name);
            return;
        }
    }
    else if (type != NULL && strcmp(type, "--id") == 0)
    {
        char* indexStr = strtok(NULL, " \n");
        if (indexStr == NULL || (index = atoi(indexStr)) <= 0 || index > adverts.count)
        {
            log_error("Invalid advertisement id");
            return;
        }
    }
    else
    {
        log_error("Invalid argument");
        return;
    }
    if (adverts.ads[index].status != AVAILABLE)
    {
        log_error("Advertisement %s is not available", adverts.ads[index].name);
        return;
    }
    connectToAdvertiser(index - 1, maxFd, masterSet);
}

void sendMsg(fd_set* masterSet)
{
    if (negSockFd == -1)
    {
        log_error("Not negotiating");
        return;
    }
    char* message = strtok(NULL, "\n");
    if (message == NULL)
    {
        log_error("Invalid message");
        return;
    }
    char* msg = malloc(strlen(message) + 1);
    strcpy(msg, message);
    memset(buf, '\0', BUF_SIZE);
    sprintf(buf, "msg|%s", msg);
    free(msg);
    if (send(negSockFd, buf, strlen(buf), 0) < 0)
    {
        log_perror("send");
        endNegotiation(masterSet);
    }
    alarm(NEGOTIATION_TIMEOUT);
    log_info("Message sent to advertiser");
}

void sendOffer(fd_set* masterSet)
{
    if (negSockFd == -1)
    {
        log_error("Not negotiating");
        return;
    }
    char* offer = strtok(NULL, " \n");
    int offerInt = atoi(offer);
    if (offer == NULL || offerInt < 1)
    {
        log_error("Invalid offer");
        return;
    }
    memset(buf, '\0', BUF_SIZE);
    sprintf(buf, "offer|%d", offerInt);
    if (send(negSockFd, buf, strlen(buf), 0) < 0)
    {
        log_perror("send");
        endNegotiation(masterSet);
    }
    alarm(NEGOTIATION_TIMEOUT);
    log_info("Offer sent to advertiser");
}

int readCommand(int* maxFd, fd_set* masterSet)
{
    memset(buf, 0, BUF_SIZE);
    read(STDIN_FILENO, buf, BUF_SIZE);
    char* command = strtok(buf, " \n");
    if (command == NULL)
        return 0;
    if (strcmp(command, "exit") == 0)
    {
        log_info("Exiting");
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
            log_warn("No negotiation in progress");
            return 0;
        }
        log_info("Negotiation ended by user");
        endNegotiation(masterSet);
    }
    else if (strcmp(command, "negotiate") == 0)
        negotiate(maxFd, masterSet);
    else if (strcmp(command, "offer") == 0)
        sendOffer(masterSet);
    else if (strcmp(command, "send") == 0)
        sendMsg(masterSet);
    else
        log_error("Invalid command");
    return 0;
}

int handleEvent(int* maxFd, fd_set* masterSet, fd_set* workingSet)
{
    if (FD_ISSET(STDIN_FILENO, workingSet))
    {
        int exit = readCommand(maxFd, masterSet);
        if (exit) return 0;
    }
    if (FD_ISSET(bcSockFd, workingSet))
        if (recieveBroadcast())
            listAdvertisements();
    if (negSockFd != -1 && FD_ISSET(negSockFd, workingSet))
    {
        int isNegotiating = getAdvertiserAnswer();
        if (isNegotiating)
            alarm(NEGOTIATION_TIMEOUT);
        else
            endNegotiation(masterSet);
    }
    return 1;
}

void runClient()
{
    fd_set masterSet, workingSet;
    FD_ZERO(&masterSet);
    FD_ZERO(&workingSet);
    FD_SET(STDIN_FILENO, &masterSet);
    FD_SET(bcSockFd, &masterSet);
    int maxFd = bcSockFd;
    sigaction(SIGALRM, &(struct sigaction){.sa_handler = alarmHandler, .sa_flags = SA_RESTART}, NULL);
    log_info("Looking for advertisements...");
    while (1)
    {
        workingSet = masterSet;
        if (select(maxFd + 1, &workingSet, NULL, NULL, NULL) < 0)
            log_perror("select");
        else if (!handleEvent(&maxFd, &masterSet, &workingSet))
            break;
        if (alarm_fired)
        {
            log_warn("Negotiation timed out");
            endNegotiation(&masterSet);
        }
    }
}

int main(int argc, const char* argv[])
{
    set_log_level(LOG_INFO);
    setupUser(argc, argv);
    runClient();
    freeResources();
    return 0;
}
