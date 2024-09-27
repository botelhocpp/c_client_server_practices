/* Standard C */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Unix */
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

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

int main(int argc, char *argv[]) {
    int clientSd = setup();

    if(clientSd == -1) {
        perror(strerror(errno));
    }

    static char buffer[1024];

    scanf("%s", buffer);

    send(clientSd, buffer, strlen(buffer), 0);

    recv(clientSd, buffer, sizeof(buffer), 0);
    
    printf("SERVER SAID: %s\n", buffer);
    
    close(clientSd);

    return 0;    
}