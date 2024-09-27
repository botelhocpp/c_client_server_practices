/* Standard C */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Posix */
#include <pthread.h>

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

/* Others */
#include "list.h"

/* Global Constants */

#define SERVER_PORT    5500
#define SERVER_IP      "127.0.0.1"

/* Client Type */

typedef struct client_t {
    int clientSd;
    char clientName[256];
} client_t;

typedef struct message_t {
    char clientName[256];
    char clientMessage[1024];
} message_t;

client_t *client_new(int clientSd, const char *clientName) {
    client_t *client = malloc(sizeof(client_t));
    client->clientSd = clientSd;
    strcpy(client->clientName, clientName);
    return client;
}

void client_delete(client_t *client) {
    free(client);
}

/* Global Handles */

pthread_mutex_t listLock; 

List clientsList;

int serverSd = -1;

/* Function Callbacks */

void client_deleter(void *client) {
    client_t *client_obj = (client_t *) client;

    if(client_obj->clientSd >= 0) {
        close(client_obj->clientSd);
    }

    client_delete(client_obj);
}

bool find_client(void *element, void *args) {
    client_t *client_obj = (client_t *) element;
    client_t *client_wanted = (client_t *) args;

    return client_obj->clientSd == client_wanted->clientSd;
}

void broadcast_clients(void *element, void *args) {
    client_t *client_obj = (client_t *) element;
    message_t* message = (message_t*) args;

    char buffer[2048];

    snprintf(buffer, 2048, "[%s]: %s", message->clientName, message->clientMessage);

    ssize_t bytesSend = send(client_obj->clientSd, buffer, strlen(buffer), 0);

    if(bytesSend == -1) {
        if(errno != ECONNRESET) {
            perror(strerror(errno));
        }
    } 
}

void signal_handler(int signal_number) {
    switch(signal_number) {
        case SIGINT:
            if(serverSd != -1) {
                close(serverSd);
            }
            if(clientsList) {
                list_delete(clientsList, client_deleter);
                pthread_mutex_destroy(&listLock); 
            }
            exit(0);
            break;
    }
}

void *client_callback(void *args) {
    const char welcome_msg[] = "Welcome! What's your name?";
    int clientSd = (int) args;
    
    message_t message;

    send(clientSd, welcome_msg, sizeof(welcome_msg) - 1, 0);
    
    ssize_t bytesRead = recv(clientSd, message.clientName, sizeof(message.clientName), 0);
    message.clientName[bytesRead] = '\0';

    client_t *client = client_new(clientSd, message.clientName);

    strcpy(message.clientMessage, "joined!");

    printf("[%s:%d]: joined.\n", message.clientName, client->clientSd);

    pthread_mutex_lock(&listLock); 
    list_foreach(clientsList, &broadcast_clients, &message);
    list_push_back(clientsList, client);
    pthread_mutex_unlock(&listLock);

    while(1) {
        ssize_t bytesRead = recv(client->clientSd, message.clientMessage, sizeof(message.clientMessage), 0);

        if(bytesRead == -1) {
            if(errno != ECONNRESET) {
                perror(strerror(errno));
            }
            break;
        }
        else if(bytesRead == 0) {
            printf("[%s:%d] disconnected.\n", client->clientName, client->clientSd);
            break;
        }

        message.clientMessage[bytesRead] = '\0';

        if(strcmp(message.clientMessage, "$exit") == 0) {
            printf("[%s:%d] exited.\n", client->clientName, client->clientSd);
            break;
        }

        printf("[%s:%d] {%s}\n", client->clientName, client->clientSd, message.clientMessage);

        pthread_mutex_lock(&listLock); 
        list_foreach(clientsList, &broadcast_clients, &message);
        pthread_mutex_unlock(&listLock);
    }

    pthread_mutex_lock(&listLock); 
    list_search_remove(clientsList, &find_client, client, &client_deleter);
    pthread_mutex_unlock(&listLock);

    pthread_exit(NULL);
}

/* Global Functions */

int setup_server() {
    struct hostent* server = gethostbyname(SERVER_IP); 
    struct sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*server->h_addr_list));
    servAddr.sin_port = htons(SERVER_PORT);
 
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

    if((clientsList = list_new()) == NULL) {
        perror(strerror(errno));
        return 1;
    }
    
    if (pthread_mutex_init(&listLock, NULL) != 0) { 
        perror(strerror(errno)); 
        return 1; 
    } 

    static char buffer[1024];

    while(1) {
        int clientSd = setup_client(serverSd);
        if(clientSd == -1) {
            perror(strerror(errno));
            return 1;
        }

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, &client_callback, (void *) clientSd);
    }  
}
