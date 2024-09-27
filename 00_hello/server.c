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

int setup_server() {
    struct hostent* server = gethostbyname(IP); 

    struct sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*server->h_addr_list));
    servAddr.sin_port = htons(PORT);
 
    int serverSd = socket(AF_INET, SOCK_STREAM, 0);
    
    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr, sizeof(servAddr));

    listen(serverSd, 1);

    return serverSd;
}

int setup_client(int serverSd) {
    struct sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    
    int newSd = accept(serverSd, (struct sockaddr *) &newSockAddr, &newSockAddrSize);
    
    return newSd;
}

int main(int argc, char *argv[]) {
    int serverSd = setup_server();
    int newSd = setup_client(serverSd);

    char rec_buffer[1024];
    char send_buffer[] = "Fodase";

    recv(newSd, (char*)&rec_buffer, sizeof(rec_buffer), 0);
        
    send(newSd, (char*)&send_buffer, strlen(send_buffer), 0);

    printf("CLIENT SAID: %s\n", rec_buffer);
            
    close(newSd);
    close(serverSd);

    return 0;   
}