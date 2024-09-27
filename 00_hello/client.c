#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>

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

    struct sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);

    int status = connect(clientSd,
                         (struct sockaddr*) &servAddr, sizeof(servAddr));
    
    return clientSd;
}

int main(int argc, char *argv[]) {
    int clientSd = setup();

    char rec_buffer[1024];
    char send_buffer[] = "Oiiiiii";

    send(clientSd, (char*)&send_buffer, strlen(send_buffer), 0);

    recv(clientSd, (char*)&rec_buffer, sizeof(rec_buffer), 0);

    printf("SERVER SAID: %s\n", rec_buffer);

    close(clientSd);

    return 0;    
}