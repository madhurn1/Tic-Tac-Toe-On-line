#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>
#include <pthread.h>

// game Server
#define QUEUE_SIZE 8
#define NAME_SIZE 256

// global mutex to ensure thread-safe access to the list of player names
pthread_mutex_t player_list_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct PlayerNode
{
    char pName[NAME_SIZE];
    struct PlayerNode *next;
    char type;
    struct PlayerNode *next;
} PlayerNode;

PlayerNode *listOfPlayers = NULL;

void *clientHandle(void *arg);
void send_msg(int sock, char *type, char *msg);
int checkName(char *name);
void addPlayer(char *name);

int main(int argc, char *argv[])
{
    // The argument to ttts is the port number it will use for connection requests.
    if (argc != 2)
    {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    int portNum = atoi(argv[1]);

    // int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //**********************************************************************************************
    struct addrinfo hint, *info_list, *info;
    int error, sock;

    // initialize hints
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    char portString[6];
    snprintf(portString, sizeof(portString), "%d", portNum);

    // obtain information for listening socket
    error = getaddrinfo(NULL, portString, &hint, &info_list); // check portNum
    if (error)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
        exit(1);
    }
    // attempt to create socket
    for (info = info_list; info != NULL; info = info->ai_next)
    {
        sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (sock == -1)
            continue; // try the next method if we could not create the socket
        error = bind(sock, info->ai_addr, info->ai_addrlen);
        if (error)
        {
            close(sock);
            continue;
        }
        // enable listening for incoming connection requests
        error = listen(sock, QUEUE_SIZE);
        if (error)
        {
            close(sock);
            continue;
        }
        // if we got this far, we have opened the socket
        break;
    }
    freeaddrinfo(info_list);
    if (info == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        if (sock != -1)
            close(sock);
        exit(1);
    }
    // When a client connects, accept the connection and create a new thread to handle the client.
    while (1)
    {
        // holds the address information of the client that connects to the server.
        struct sockaddr_storage client_addr;
        // determine the size of the buffer where the client's address information will be stored.
        socklen_t client_addrlen = sizeof(client_addr);
        /*is a system call that waits for a connection request from a client and accepts
        it, returning a new socket file descriptor that is used to communicate with the client.
        */
        int client_sock = accept(sock, (struct sockaddr *)&client_addr, &client_addrlen);

        if (client_sock == -1)
        {
            perror("accept");
            continue;
        }

        else
        {
            printf("Connected\n");
        }
        pthread_t tid;
        pthread_create(&tid, NULL, clientHandle, &client_sock);
    }

    return 1;
}
// pointer to the integer value that represents the client socket file descriptor.
void *clientHandle(void *arg)
{
    int client_sock = *(int *)arg;
    char playerName[NAME_SIZE];
    char Player1[NAME_SIZE];
    char Player2[NAME_SIZE];
    int playerflag = 0;

    // wait for "PLAY" message from client
    ssize_t val = recv(client_sock, playerName, sizeof(playerName), 0);
    if (val <= 0)
    {
        close(client_sock);
        return NULL;
    }
    // checking to seee if player name is inuse
    if (checkName(playerName) == 1)
    {
        send_msg(client_sock, "INVL", "Name currently in use");
        close(client_sock);
        return NULL;
    }
    // adding player to linked list structure.
    addPlayer(playerName);
    send(client_sock, "WAIT", sizeof("WAIT"), 0);

    while (1)
    {
        pthread_mutex_lock(&player_list_lock);
        if (listOfPlayers != NULL && listOfPlayers->next != NULL)
        {
            playerflag = 2;
        }
        pthread_mutex_unlock(&player_list_lock);
        if (playerflag == 2)
        {
            // both players ready
            break;
        }
        sleep(1);
    }

    pthread_mutex_lock(&player_list_lock);
    PlayerNode *p1 = listOfPlayers;
    PlayerNode *p2 = listOfPlayers->next;
    int p = rand() % 2;
    if (p == 0)
        p1->type = 'o';
    else
    {
        p2->type = 'x';
    }
    PlayerNode *p1 = listOfPlayers;
    PlayerNode *p2 = listOfPlayers->next;
    strncpy(Player1, p1->pName, NAME_SIZE);
    strncpy(Player2, p2->pName, NAME_SIZE);
    listOfPlayers = listOfPlayers->next->next;
    free(p1);
    free(p2);
    pthread_mutex_unlock(&player_list_lock);

    // game can now be started
    // randomly need to assign each player either an O and X

    close(client_sock);
    pthread_exit(NULL);
}

int checkName(char *name)
{
    int flag = 0;
    pthread_mutex_lock(&player_list_lock);
    PlayerNode *temp = listOfPlayers;
    while (temp != NULL)
    {
        if (strcmp(temp->pName, name) == 0)
        {
            pthread_mutex_unlock(&player_list_lock);
            flag = 1;
            return flag;
        }
        temp = temp->next;
    }
    pthread_mutex_unlock(&player_list_lock);
    return flag;
}

void send_msg(int sock, char *type, char *msg)
{
    char buf[NAME_SIZE];
    sprintf(buf, "%s %s\n", type, msg);
    send(sock, buf, strlen(buf), 0);
}

void addPlayer(char *name)
{
    // linked list
    PlayerNode *node_p = (PlayerNode *)malloc(sizeof(PlayerNode)); // this needs to be freed som
    strncpy(node_p->pName, name, NAME_SIZE);
    node_p->next = NULL;
    pthread_mutex_lock(&player_list_lock);
    if (listOfPlayers == NULL)
    {
        listOfPlayers = node_p;
    }
    else
    {
        PlayerNode *temp = listOfPlayers;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        temp->next = node_p;
    }
    pthread_mutex_unlock(&player_list_lock);
}