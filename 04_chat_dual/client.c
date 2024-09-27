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

/* Posix */
#include <pthread.h>

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

int clientSd = -1;

void signal_handler(int signal_number) {
    switch(signal_number) {
        case SIGINT:
            if(clientSd != -1) close(clientSd);
            exit(0);
            break;
    }
}

int setup() {
    struct hostent* client = gethostbyname(IP); 

    struct sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*client->h_addr_list));
    servAddr.sin_port = htons(PORT);
 
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);

    if(clientSd == -1) {
        return -1;
    }

    struct sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);

    int status = connect(clientSd,
                         (struct sockaddr*) &servAddr, sizeof(servAddr));
    
    if(status == -1) {
        return -1;
    }

    return clientSd;
}

void *receive_channel(void *args) {
    int clientSd = (int) args;
    char buffer[1024];

    while(1) {
        ssize_t bytesRead = recv(clientSd, buffer, sizeof(buffer), 0);

        if(bytesRead == -1) {
            perror(strerror(errno));
            break;
        }
        else if(bytesRead == 0) {
            printf("Server is down.\n");
            break;
        }

        buffer[bytesRead] = '\0';
        
        printf("%s\n", buffer);
    }
}

void *transmit_channel(void *args) {
    int clientSd = (int) args;
    char buffer[1024];

    while(1) {
        scanf("%[^\n]", buffer);
        while(getchar() != '\n');

        send(clientSd, buffer, strlen(buffer), 0);

        if(strcmp(buffer, "$exit") == 0) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    signal(SIGINT, &signal_handler);

    clientSd = setup();

    if(clientSd == -1) {
        perror(strerror(errno));
        exit(errno);
    }

    pthread_t receive_thread;
    pthread_t transmit_thread;

    pthread_create(&receive_thread, NULL, &receive_channel, (void *) clientSd);
    pthread_create(&transmit_thread, NULL, &transmit_channel, (void *) clientSd);

    pthread_join(transmit_thread, NULL);
    // pthread_join(&receive_thread, NULL);

    close(clientSd);
}