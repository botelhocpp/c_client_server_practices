/* Standard C */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Unix */
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

/* System */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>

/* Network */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT    5500
#define IP      "127.0.0.1"

int serverSd = -1;
int clientSd = -1;

void signal_handler(int signal_number) {
    switch(signal_number) {
        case SIGINT:
            if(serverSd != -1) close(serverSd);
            if(clientSd != -1) close(clientSd);
            exit(1);
            break;
    }
}

int setup_server() {
    struct hostent* server = gethostbyname(IP); 
    struct sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*server->h_addr_list));
    servAddr.sin_port = htons(PORT);
 
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(serverFd == -1) {
        return -1;
    }

    const int enable = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
        return -1;
    }

    int bindStatus = bind(serverFd, (struct sockaddr*) &servAddr, sizeof(servAddr));

    if(listen(serverFd, 1) == -1) {
        return -1;
    }

    return serverFd;
}

int setup_client(int serverSd) {
    struct sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    
    int clientSd = accept(serverSd, (struct sockaddr *) &newSockAddr, &newSockAddrSize);
    
    return clientSd;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, &signal_handler);

    if((serverSd = setup_server()) == -1) {
        perror(strerror(errno));
        return 1;
    } 

    static char buffer[1024];

    while(1) {
        if((clientSd = setup_client(serverSd)) == -1) {
            perror(strerror(errno));
            return 1;
        }

        ssize_t bytesRead = recv(clientSd, buffer, sizeof(buffer), 0);

        if(bytesRead == -1) {
            perror(strerror(errno));
        }

        buffer[bytesRead] = '\0';

        ssize_t bytesSend = send(clientSd, buffer, strlen(buffer), 0);

        if(bytesSend == -1) {
            perror(strerror(errno));
        }  

        close(clientSd);
    }

    return 0;   
}