#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "include/log.h"
#include "include/user.h"

#define MIN_ADVERTS_COUNT 10

#define HELP_MSG "Commands:\n" \
                 "\thelp - show this message\n" \
                 "\tlist - show the list of advertisements\n" \
                 "\tadd <name> <port> - add an advertisement\n" \
                 "\trespond (--id <id>)|(--name <name>) <message> - respond for an offer(or message)\n" \
                 "\taccept (--id <id>)|(--name <name>) - accept an offer\n" \
                 "\treject (--id <id>)|(--name <name>) - reject an offer\n" \
                 "\texit - exit the program\n"

typedef struct
{
    char name[ADVERT_NAME_LEN];
    advertisement_status status;
    uint16_t port;
    int latestOffer;
    int sockFd;
} advertisement;

advertisement* adverts = NULL;

int advertsCount = 0, advertsCapacity = 0;

void freeResources()
{
    close(bcSockFd);
    if (adverts != NULL)
        free(adverts);
    for (int i = 0; i < advertsCount; i++)
        if (adverts[i].sockFd != -1)
            close(adverts[i].sockFd);
}

void interruptHandler(int sig)
{
    log_warn("Interrupted");
    freeResources();
    exit(EXIT_SUCCESS);
}

void insertAdvertisement(const char* name, uint16_t port, int sockFd)
{
    if (advertsCount == advertsCapacity)
    {
        if (advertsCapacity == 0)
        {
            advertsCapacity = MIN_ADVERTS_COUNT;
            adverts = malloc(advertsCapacity * sizeof(advertisement));
        }
        else
        {
            advertsCapacity *= 2;
            adverts = realloc(adverts, advertsCapacity * sizeof(advertisement));
        }
    }
    strcpy(adverts[advertsCount].name, name);
    adverts[advertsCount].status = AVAILABLE;
    adverts[advertsCount].port = port;
    adverts[advertsCount].latestOffer = 0;
    adverts[advertsCount].sockFd = sockFd;
    advertsCount++;
}

void broadcastNewAdvert(const advertisement* advert)
{
    memset(buf, 0, BUF_SIZE);
    sprintf(buf, "new|%s|%s|%d", advert->name, name, advert->port);
    if (sendto(bcSockFd, buf, BUF_SIZE, 0, (struct sockaddr*)&bcAddr, sizeof(bcAddr)) < 0)
        log_perror("sendto");
    log_info("New advertisement broadcasted");
}

void broadcastAdvertUpdate(const advertisement* advert)
{
    memset(buf, 0, BUF_SIZE);
    char* status = advert->status == AVAILABLE ? "AVAILABLE" : (advert->status == NEGOTIATING ? "NEGOTIATING" : "SOLD");
    sprintf(buf, "update|%s|%s", advert->name, status);
    if (sendto(bcSockFd, buf, BUF_SIZE, 0, (struct sockaddr*)&bcAddr, sizeof(bcAddr)) < 0)
        log_perror("sendto");
    log_info("Advertisement update broadcasted");
}

int findAdvertisementFromBuf(const char* endDelim)
{
    char* type = strtok(NULL, " ");
    if (type == NULL)
    {
        log_error("Missing argument");
        return -1;
    }
    char* token = strtok(NULL, endDelim);
    if (token == NULL)
    {
        log_error("Missing argument");
        return -1;
    }
    if (strcmp(type, "--id") == 0)
    {
        int id = atoi(token);
        if (id <= 0 || id > advertsCount)
        {
            log_error("Invalid id");
            return -1;
        }
        return id;
    }
    if (strcmp(type, "--name") == 0)
    {
        for (int i = 0; i < advertsCount; i++)
            if (strcmp(adverts[i].name, token) == 0)
                return i;
        log_error("Invalid name");
        return -1;
    }
    log_error("Invalid argument");
    return -1;
}

int findAdvertisementFromSocket(int sockFd)
{
    for (int i = 0; i < advertsCount; i++)
        if (adverts[i].sockFd == sockFd)
            return i;
    return -1;
}

int bindTcpSocket(int sockFd, uint16_t port)
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(sockFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        log_perror("bind");
        close(sockFd);
        return -1;
    }
    if (listen(sockFd, 1) < 0)
    {
        log_perror("listen");
        close(sockFd);
        return -1;
    }
    return 0;
}

int getTcpSockFd()
{
    int sockFd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockFd < 0)
    {
        log_perror("socket");
        return -1;
    }
    return sockFd;
}

void logSale(int index)
{
    char fileName[NAME_LEN + 4];
    sprintf(fileName, "%s.txt", name);
    int fd = open(fileName, O_WRONLY | O_CREAT | O_APPEND);
    if (fd < 0)
    {
        log_perror("open");
        return;
    }
    sprintf(buf, "%s -> %d", adverts[index].name, adverts[index].latestOffer);
    if (write(fd, buf, strlen(buf)) < 0)
        log_perror("write");
    close(fd);
}

void sell(int index, fd_set* masterSet)
{
    int sockFd = adverts[index].sockFd;
    if (sockFd != -1)
    {
        FD_CLR(sockFd, masterSet);
        close(sockFd);
        adverts[index].sockFd = -1;
    }
    adverts[index].status = SOLD;
    broadcastAdvertUpdate(&adverts[index]);
    logSale(index);
    log_info("Sold %s", adverts[index].name);
}

int startNegotiation(int index, fd_set* masterSet, int* maxFd)
{
    int sockFd = adverts[index].sockFd;
    if (adverts[index].status == SOLD)
    {
        log_error("Advertisement is already sold");
        return -1;
    }
    if (adverts[index].status == NEGOTIATING)
    {
        log_error("Advertisement is already being negotiated");
        return -1;
    }
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int newSockFd = accept(sockFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (newSockFd < 0)
    {
        log_perror("accept");
        return -1;
    }
    FD_SET(newSockFd, masterSet);
    if (newSockFd > *maxFd)
        *maxFd = newSockFd;
    FD_CLR(sockFd, masterSet);
    close(sockFd);
    adverts[index].sockFd = newSockFd;
    adverts[index].status = NEGOTIATING;
    broadcastAdvertUpdate(&adverts[index]);
    log_info("Negotiation started for %s", adverts[index].name);
    return newSockFd;
}

int endNegotiation(int index, fd_set* masterSet, int* maxFd)
{
    int sockFd = adverts[index].sockFd;
    if (adverts[index].status == SOLD)
    {
        log_error("Advertisement is already sold");
        return -1;
    }
    if (adverts[index].status == AVAILABLE)
    {
        log_error("Advertisement is not being negotiated");
        return -1;
    }
    if (sockFd != -1)
    {
        FD_CLR(sockFd, masterSet);
        close(sockFd);
    }
    sockFd = getTcpSockFd();
    if (sockFd < 0)
        return -1;
    if (bindTcpSocket(sockFd, adverts[index].port) < 0)
        return -1;
    if (listen(sockFd, 1) < 0)
    {
        log_perror("listen");
        close(sockFd);
        return -1;
    }
    FD_SET(sockFd, masterSet);
    if (sockFd > *maxFd)
        *maxFd = sockFd;
    adverts[index].sockFd = sockFd;
    adverts[index].status = AVAILABLE;
    broadcastAdvertUpdate(&adverts[index]);
    log_info("Negotiation ended for %s", adverts[index].name);
    return 0;
}

void setLatestOffer(int index, int offer)
{
    adverts[index].latestOffer = offer;
    log_info("Offer for %s set to %d", adverts[index].name, offer);
}

void listAdvertisements()
{
    write(STDOUT_FILENO, "\nAdvertisements:\n", 17);
    write(STDOUT_FILENO, ADVERT_COLOR, strlen(ADVERT_COLOR));
    for (int i = 0; i < advertsCount; i++)
    {
        char* status = adverts[i].status == AVAILABLE ? "AVAILABLE" : (adverts[i].status == NEGOTIATING ? "NEGOTIATING" : "SOLD");
        sprintf(buf, "%d -> %s, Port: %d, Status: %s, Latest Offer: %d\n", i + 1, adverts[i].name, adverts[i].port, status, adverts[i].latestOffer);
        write(STDOUT_FILENO, buf, strlen(buf));
    }
    write(STDOUT_FILENO, RESET_COLOR, strlen(RESET_COLOR));
}

void getClientOffer(int index)
{
    char* offerStr = strtok(NULL, " \n\0");
    int offer;
    if (offerStr == NULL || (offer = atoi(offerStr)) <= 0)
    {
        log_warn("Invalid offer received from client");
        return;
    }
    log_info("New offer received for %s", adverts[index].name);
    setLatestOffer(index, offer);
    listAdvertisements();
}

void getClientMessage(int index)
{
    char* message = strtok(NULL, "\n\0");
    if (message == NULL)
    {
        log_warn("Invalid message received from client");
        return;
    }
    log_info("New message received for %s", adverts[index].name);
    sprintf(buf, "Client (%s): %s", adverts[index].name, message);
    write(STDOUT_FILENO, buf, strlen(buf));
}

void getClientResponse(int sockFd, int index, fd_set* masterSet, int* maxFd)
{
    int bytes = recv(sockFd, buf, BUF_SIZE, 0);
    if (bytes < 0)
    {
        log_perror("recv");
        return;
    }
    if (bytes == 0)
    {
        log_info("Client disconnected from %s", adverts[index].name);
        endNegotiation(index, masterSet, maxFd);
        return;
    }
    char* type = strtok(buf, "|");
    if (type != NULL && strcmp(type, "offer"))
        getClientOffer(index);
    else if (type != NULL && strcmp(type, "message"))
        getClientMessage(index);
    else
        log_warn("Invalid message received from client");
}

void addAdvertisement()
{
    int port;
    char* name = strtok(NULL, " ");
    char* portStr = strtok(NULL, " \n\0");
    if (name == NULL || portStr == NULL || (port = getPort(portStr)) < 0)
    {
        log_warn("Invalid advertisement");
        return;
    }
    int sockFd = getTcpSockFd();
    if (sockFd < 0)
        return;
    if (bindTcpSocket(sockFd, port) < 0)
        return;
    if (listen(sockFd, 1) < 0)
    {
        log_perror("listen");
        close(sockFd);
        return;
    }
    insertAdvertisement(name, port, sockFd);
    log_info("Advertisement added: %s (Port %d)", name, port);
    broadcastNewAdvert(&adverts[advertsCount - 1]);
    listAdvertisements();
}

void respondClient(fd_set* masterSet, int* maxFd)
{
    int index = findAdvertisementFromBuf(" ");
    if (index < 0)
        return;
    if (adverts[index].status != NEGOTIATING)
    {
        log_error("Advertisement is not being negotiated");
        return;
    }
    char* msg = strtok(NULL, "\n\0");
    if (msg == NULL)
    {
        log_error("Invalid message");
        return;
    }
    if (strcmp(msg, ACCEPT_OFFER) == 0 || strcmp(msg, REJECT_OFFER) == 0)
    {
        log_error("Message cannot be 'accept' or 'reject', use commands instead");
        return;
    }
    int sockFd = adverts[index].sockFd;
    if (send(sockFd, msg, strlen(msg), 0) < 0)
    {
        log_perror("send");
        endNegotiation(index, masterSet, maxFd);
        return;
    }
    log_info("Message sent to client");
}

void acceptOffer(fd_set* masterSet, int* maxFd)
{
    int index = findAdvertisementFromBuf(" \n\0");
    if (index < 0)
        return;
    if (adverts[index].status != NEGOTIATING)
    {
        log_error("Advertisement is not being negotiated");
        return;
    }
    int sockFd = adverts[index].sockFd;
    if (send(sockFd, ACCEPT_OFFER, strlen(ACCEPT_OFFER), 0) < 0)
    {
        log_perror("send");
        endNegotiation(index, masterSet, maxFd);
        return;
    }
    log_info("Offer accepted");
    sell(index, masterSet);
}

void rejectOffer(fd_set* masterSet, int* maxFd)
{
    int index = findAdvertisementFromBuf(" \n\0");
    if (index < 0)
        return;
    if (adverts[index].status != NEGOTIATING)
    {
        log_error("Advertisement is not being negotiated");
        return;
    }
    int sockFd = adverts[index].sockFd;
    if (send(sockFd, REJECT_OFFER, strlen(REJECT_OFFER), 0) < 0)
        log_perror("send");
    else
        log_info("Offer rejected");
    endNegotiation(index, masterSet, maxFd);
}

int readCommand(fd_set* masterSet, int* maxFd)
{
    read(STDIN_FILENO, buf, BUF_SIZE);
    char* command = strtok(buf, " \n\0");
    if (command == NULL)
        return 0;
    if (strcmp(command, "add") == 0)
        addAdvertisement();
    else if (strcmp(command, "help") == 0)
        write(STDOUT_FILENO, HELP_MSG, strlen(HELP_MSG));
    else if (strcmp(command, "list") == 0)
        listAdvertisements();
    else if (strcmp(command, "respond") == 0)
        respondClient(masterSet, maxFd);
    else if (strcmp(command, "accept") == 0)
        acceptOffer(masterSet, maxFd);
    else if (strcmp(command, "reject") == 0)
        rejectOffer(masterSet, maxFd);
    else if (strcmp(command, "exit") == 0)
        return 1;
    else
        log_error("Invalid command");
    return 0;
}

void handleSocketEvent(int sockFd, fd_set* masterSet, int* maxFd)
{
    int index = findAdvertisementFromSocket(sockFd);
    if (index < 0)
        return;
    if (adverts[index].status == NEGOTIATING)
        getClientResponse(sockFd, index, masterSet, maxFd);
    else if (adverts[index].status == AVAILABLE)
        startNegotiation(index, masterSet, maxFd);
    else
    {
        log_warn("Event from invalid socket received, closing socket...");
        close(sockFd);
    }
}

void runClient()
{
    fd_set masterSet, workingSet;
    FD_ZERO(&masterSet);
    FD_ZERO(&workingSet);
    FD_SET(STDIN_FILENO, &masterSet);
    int maxFd = STDIN_FILENO;
    log_info("Server started...");
    while (1)
    {
        workingSet = masterSet;
        if (select(maxFd + 1, &workingSet, NULL, NULL, NULL) < 0)
        {
            log_perror("select");
            return;
        }
        if (FD_ISSET(STDIN_FILENO, &workingSet))
        {
            int exit = readCommand(&masterSet, &maxFd);
            if (exit)
                break;
        }
        else
            for (int i = 1; i <= maxFd; ++i)
                if (FD_ISSET(i, &workingSet))
                    handleSocketEvent(i, &masterSet, &maxFd);
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
