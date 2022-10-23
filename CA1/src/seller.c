#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "include/log.h"
#include "include/user.h"

#define MIN_ADVERTS_COUNT 10
#define LOG_DIR "log"
#define LOG_EXT ".txt"

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
    advertisementStatus status;
    uint16_t port;
    int latestOffer;
    int sockFd;
} advertisement;

struct advertisements
{
    advertisement *ads;
    size_t capacity;
    size_t count;
} adverts = { NULL, 0, 0 };

fd_set masterSet;
int maxFd;


void freeResources()
{
    close(bcSockFd);
    if (adverts.ads != NULL)
        free(adverts.ads);
    for (int i = 0; i < adverts.count; i++)
        if (adverts.ads[i].sockFd != -1)
            close(adverts.ads[i].sockFd);
}

void interruptHandler(int sig)
{
    logWarn("Interrupted");
    freeResources();
    exit(EXIT_SUCCESS);
}

void insertAdvertisement(const char* name, uint16_t port, int sockFd)
{
    if (adverts.count == adverts.capacity)
    {
        adverts.capacity = adverts.capacity == 0 ? MIN_ADVERTS_COUNT : adverts.capacity * 2;
        adverts.ads = (advertisement*)realloc(adverts.ads, adverts.capacity * sizeof(advertisement));
    }
    strcpy(adverts.ads[adverts.count].name, name);
    adverts.ads[adverts.count].status = AVAILABLE;
    adverts.ads[adverts.count].port = port;
    adverts.ads[adverts.count].latestOffer = 0;
    adverts.ads[adverts.count].sockFd = sockFd;
    adverts.count++;
}

void broadcastAdvert(const advertisement* advert)
{
    memset(buf, 0, BUF_SIZE);
    char* status = advert->status == AVAILABLE ? "AVAILABLE" : (advert->status == NEGOTIATING ? "NEGOTIATING" : "SOLD");
    sprintf(buf, "%s%s%s%s%d%s%s", advert->name, MESSAGE_DELIMITER, name, MESSAGE_DELIMITER, advert->port, MESSAGE_DELIMITER, status);
    if (sendto(bcSockFd, buf, BUF_SIZE, 0, (struct sockaddr*)&bcAddr, sizeof(bcAddr)) < 0)
        logPError("sendto");
    logInfo("New advertisement broadcasted");
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
        logError("Invalid name");
        return -1;
    }
    logError("Invalid argument");
    return -1;
}

int findAdvertisementFromSocket(int sockFd)
{
    for (int i = 0; i < adverts.count; i++)
        if (adverts.ads[i].sockFd == sockFd)
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
        logPError("bind");
        close(sockFd);
        return -1;
    }
    if (listen(sockFd, 1) < 0)
    {
        logPError("listen");
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
        logPError("socket");
        return -1;
    }
    int opt = 1;
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        logPError("setsockopt");
        close(sockFd);
        return -1;
    }
    return sockFd;
}

void logSale(int index)
{
    char* fileName = malloc(strlen(LOG_DIR) + 1 + strlen(LOG_EXT));
    sprintf(fileName, "%s/%s%s", LOG_DIR, name, LOG_EXT);
    int fd = open(fileName, O_RDWR | O_APPEND | O_CREAT, 0644);
    free(fileName);
    if (fd < 0)
    {
        logPError("open");
        return;
    }
    memset(buf, 0, BUF_SIZE);
    sprintf(buf, "%s -> %d\n", adverts.ads[index].name, adverts.ads[index].latestOffer);
    if (write(fd, buf, strlen(buf)) < 0)
        logPError("write");
    close(fd);
}

void sell(int index)
{
    int sockFd = adverts.ads[index].sockFd;
    if (sockFd != -1)
    {
        FD_CLR(sockFd, &masterSet);
        close(sockFd);
        adverts.ads[index].sockFd = -1;
    }
    adverts.ads[index].status = SOLD;
    broadcastAdvert(&adverts.ads[index]);
    logSale(index);
    logInfo("Sold %s", adverts.ads[index].name);
}

int startNegotiation(int index)
{
    int sockFd = adverts.ads[index].sockFd;
    if (adverts.ads[index].status == SOLD)
    {
        logError("Advertisement is already sold");
        return -1;
    }
    if (adverts.ads[index].status == NEGOTIATING)
    {
        logError("Advertisement is already being negotiated");
        return -1;
    }
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int newSockFd = accept(sockFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (newSockFd < 0)
    {
        logPError("accept");
        return -1;
    }
    FD_SET(newSockFd, &masterSet);
    if (newSockFd > maxFd)
        maxFd = newSockFd;
    FD_CLR(sockFd, &masterSet);
    close(sockFd);
    adverts.ads[index].sockFd = newSockFd;
    adverts.ads[index].status = NEGOTIATING;
    broadcastAdvert(&adverts.ads[index]);
    logInfo("Negotiation started for %s", adverts.ads[index].name);
    return newSockFd;
}

int endNegotiation(int index)
{
    int sockFd = adverts.ads[index].sockFd;
    if (adverts.ads[index].status == SOLD)
    {
        logError("Advertisement is already sold");
        return -1;
    }
    if (adverts.ads[index].status == AVAILABLE)
    {
        logError("Advertisement is not being negotiated");
        return -1;
    }
    if (sockFd != -1)
    {
        FD_CLR(sockFd, &masterSet);
        close(sockFd);
    }
    sockFd = getTcpSockFd();
    if (sockFd < 0)
        return -1;
    if (bindTcpSocket(sockFd, adverts.ads[index].port) < 0)
        return -1;
    if (listen(sockFd, 1) < 0)
    {
        logPError("listen");
        close(sockFd);
        return -1;
    }
    FD_SET(sockFd, &masterSet);
    if (sockFd > maxFd)
        maxFd = sockFd;
    adverts.ads[index].sockFd = sockFd;
    adverts.ads[index].status = AVAILABLE;
    broadcastAdvert(&adverts.ads[index]);
    logInfo("Negotiation ended for %s", adverts.ads[index].name);
    return 0;
}

void setLatestOffer(int index, int offer)
{
    adverts.ads[index].latestOffer = offer;
    logInfo("Offer for %s set to %d", adverts.ads[index].name, offer);
}

void listAdvertisements()
{
    write(STDOUT_FILENO, "\nAdvertisements:\n", 17);
    write(STDOUT_FILENO, ADVERT_COLOR, strlen(ADVERT_COLOR));
    for (int i = 0; i < adverts.count; i++)
    {
        memset(buf, 0, BUF_SIZE);
        char* status = adverts.ads[i].status == AVAILABLE ? "AVAILABLE" : (adverts.ads[i].status == NEGOTIATING ? "NEGOTIATING" : "SOLD");
        sprintf(buf, "%d -> %s, Port: %d, Status: %s, Latest Offer: %d\n", i + 1, adverts.ads[i].name, adverts.ads[i].port, status, adverts.ads[i].latestOffer);
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
        logWarn("Invalid offer received from client");
        return;
    }
    logInfo("New offer received for %s", adverts.ads[index].name);
    setLatestOffer(index, offer);
    listAdvertisements();
}

void getClientMessage(int index)
{
    char* message = strtok(NULL, "\n\0");
    if (message == NULL)
    {
        logWarn("Invalid message received from client");
        return;
    }
    logInfo("New message received for %s", adverts.ads[index].name);
    char* msg = malloc(strlen(message) + 1);
    strcpy(msg, message);
    memset(buf, 0, BUF_SIZE);
    sprintf(buf, "Client (%s): %s", adverts.ads[index].name, msg);
    free(msg);
    write(STDOUT_FILENO, buf, strlen(buf));
    write(STDOUT_FILENO, "\n", 1);
}

void getClientResponse(int sockFd, int index)
{
    memset(buf, 0, BUF_SIZE);
    int bytes = recv(sockFd, buf, BUF_SIZE, 0);
    if (bytes < 0)
    {
        logPError("recv");
        return;
    }
    if (bytes == 0)
    {
        logInfo("Client disconnected from %s", adverts.ads[index].name);
        endNegotiation(index);
        return;
    }
    char* type = strtok(buf, MESSAGE_DELIMITER);
    if (type != NULL && strcmp(type, "offer") == 0)
        getClientOffer(index);
    else if (type != NULL && strcmp(type, "msg") == 0)
        getClientMessage(index);
    else
        logWarn("Invalid message received from client");
}

void addAdvertisement()
{
    int port;
    char* name = strtok(NULL, " ");
    char* portStr = strtok(NULL, " \n\0");
    if (name == NULL || portStr == NULL || (port = getPort(portStr)) < 0)
    {
        logWarn("Invalid advertisement");
        return;
    }
    int sockFd = getTcpSockFd();
    if (sockFd < 0)
        return;
    if (bindTcpSocket(sockFd, port) < 0)
        return;
    if (listen(sockFd, 1) < 0)
    {
        logPError("listen");
        close(sockFd);
        return;
    }
    FD_SET(sockFd, &masterSet);
    if (sockFd > maxFd)
        maxFd = sockFd;
    insertAdvertisement(name, port, sockFd);
    logInfo("Advertisement added: %s (Port %d)", name, port);
    broadcastAdvert(&adverts.ads[adverts.count - 1]);
    listAdvertisements();
}

void respondClient()
{
    int index = findAdvertisementFromBuf(" ");
    if (index < 0)
        return;
    if (adverts.ads[index].status != NEGOTIATING)
    {
        logError("Advertisement is not being negotiated");
        return;
    }
    char* msg = strtok(NULL, "\n\0");
    if (msg == NULL)
    {
        logError("Invalid message");
        return;
    }
    if (strcmp(msg, ACCEPT_OFFER) == 0 || strcmp(msg, REJECT_OFFER) == 0)
    {
        logError("Message cannot be 'accept' or 'reject', use commands instead");
        return;
    }
    int sockFd = adverts.ads[index].sockFd;
    if (send(sockFd, msg, strlen(msg), 0) < 0)
    {
        logPError("send");
        endNegotiation(index);
        return;
    }
    logInfo("Message sent to client");
}

void acceptOffer()
{
    int index = findAdvertisementFromBuf(" \n\0");
    if (index < 0)
        return;
    if (adverts.ads[index].status != NEGOTIATING)
    {
        logError("Advertisement is not being negotiated");
        return;
    }
    int sockFd = adverts.ads[index].sockFd;
    if (send(sockFd, ACCEPT_OFFER, strlen(ACCEPT_OFFER), 0) < 0)
    {
        logPError("send");
        endNegotiation(index);
        return;
    }
    logInfo("Offer accepted");
    sell(index);
}

void rejectOffer()
{
    int index = findAdvertisementFromBuf(" \n\0");
    if (index < 0)
        return;
    if (adverts.ads[index].status != NEGOTIATING)
    {
        logError("Advertisement is not being negotiated");
        return;
    }
    int sockFd = adverts.ads[index].sockFd;
    if (send(sockFd, REJECT_OFFER, strlen(REJECT_OFFER), 0) < 0)
        logPError("send");
    else
        logInfo("Offer rejected");
    endNegotiation(index);
}

int readCommand()
{
    memset(buf, 0, BUF_SIZE);
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
        respondClient();
    else if (strcmp(command, "accept") == 0)
        acceptOffer();
    else if (strcmp(command, "reject") == 0)
        rejectOffer();
    else if (strcmp(command, "exit") == 0)
        return 1;
    else
        logError("Invalid command");
    return 0;
}

void handleSocketEvent(int sockFd)
{
    int index = findAdvertisementFromSocket(sockFd);
    if (index < 0)
        return;
    if (adverts.ads[index].status == NEGOTIATING)
        getClientResponse(sockFd, index);
    else if (adverts.ads[index].status == AVAILABLE)
        startNegotiation(index);
    else
    {
        logWarn("Event from invalid socket received, closing socket...");
        close(sockFd);
    }
}

void runClient()
{
    fd_set workingSet;
    FD_ZERO(&masterSet);
    FD_ZERO(&workingSet);
    FD_SET(STDIN_FILENO, &masterSet);
    maxFd = STDIN_FILENO;
    logInfo("Server started...");
    while (1)
    {
        workingSet = masterSet;
        if (select(maxFd + 1, &workingSet, NULL, NULL, NULL) < 0)
        {
            logPError("select");
            return;
        }
        if (FD_ISSET(STDIN_FILENO, &workingSet))
        {
            int exit = readCommand();
            if (exit)
                break;
        }
        else
            for (int i = 1; i <= maxFd; ++i)
                if (FD_ISSET(i, &workingSet))
                    handleSocketEvent(i);
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
