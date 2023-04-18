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
#define INVL "INVALID_COMMAND"
#define WAIT "Wait"
#define PLAY "name goes here"
#define BEGN "RANDOMLY ASSIGNED HERE"

// global mutex to ensure thread-safe access to the list of player names
pthread_mutex_t player_list_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct PlayerNode
{
    char pName[NAME_SIZE];
    struct PlayerNode *next;
    char type;
} PlayerNode;

volatile int active = 1;

void handler(int signum)
{
    active = 0;
}

struct connection_data
{
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int fd;
};

PlayerNode *listOfPlayers = NULL;

void *clientHandle(void *arg);
void send_msg(int sock, char *type, char *msg);
int checkName(char *name);
void addPlayer(char *name);
void install_handlers(sigset_t *mask);

int main(int argc, char *argv[])
{
    // The argument to ttts is the port number it will use for connection requests.
    sigset_t mask;

    if (argc != 2)
    {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    int portNum = atoi(argv[1]);
    printf("%d\n",portNum);

    install_handlers(&mask);
    //**********************************************************************************************
    struct addrinfo hint, *info_list, *info;
    int error, sock;
    struct connection_data *con;

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
        exit(1);
    }
    // When a client connects, accept the connection and create a new thread to handle the client.

<<<<<<< HEAD
    while(active){

        //holds the address information of the client that connects to the server.
=======
    while (active)
    {
        // holds the address information of the client that connects to the server.
>>>>>>> 809cf60670b8c67cf7422c8467ae4de6c296fd4e
        con = (struct connection_data *)malloc(sizeof(struct connection_data));
        // struct sockaddr_storage client_addr;
        con->addr_len = sizeof(struct sockaddr_storage);

        // determine the size of the buffer where the client's address information will be stored.
        //  socklen_t client_addrlen = sizeof(client_addr);

        /*is a system call that waits for a connection request from a client and accepts
        it, returning a new socket file descriptor that is used to communicate with the client.
        */
        // int client_sock = accept(sock, (struct sockaddr*)&client_addr, &client_addrlen);
<<<<<<< HEAD
        con->fd = accept(sock,(struct sockaddr *)&con->addr, &con->addr_len);
        
        if (con->fd == -1) {
=======
        con->fd = accept(sock, (struct sockaddr *)&con->addr, &con->addr_len);

        if (con->fd == -1)
        {
>>>>>>> 809cf60670b8c67cf7422c8467ae4de6c296fd4e
            perror("accept");
            free(con);
            continue;
        }
        error = pthread_sigmask(SIG_BLOCK, &mask, NULL);
        if (error != 0)
        {
            fprintf(stderr, "sigmask: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }

        pthread_t tid;
        error = pthread_create(&tid, NULL, clientHandle, &con->fd);
        if (error != 0)
        {
            fprintf(stderr, "pthread_create: %s\n", strerror(error));
            close(con->fd);
            free(con);
            continue;
        }
    
        // automatically clean up child threads once they terminate
        pthread_detach(tid);

        // unblock handled signals
        error = pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
        if (error != 0)
        {
            fprintf(stderr, "sigmask: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }
    // shutting down
    close(sock);
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
        send_msg(client_sock, INVL, "Name currently in use");
        close(client_sock);
        return NULL;
    }
    // adding player to linked list structure.
    addPlayer(playerName);

    send_msg(client_sock, WAIT, NULL); // not sure for currect spot

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
<<<<<<< HEAD
    PlayerNode* p1 = listOfPlayers;
    PlayerNode* p2 = listOfPlayers->next;
   
=======
    PlayerNode *p1 = listOfPlayers;
    PlayerNode *p2 = listOfPlayers->next;
    int p = rand() % 2;
    if (p == 0)
        p1->type = 'o';
    else
    {
        p2->type = 'x';
    }
>>>>>>> 809cf60670b8c67cf7422c8467ae4de6c296fd4e
    strncpy(Player1, p1->pName, NAME_SIZE);
    strncpy(Player2, p2->pName, NAME_SIZE);
    listOfPlayers = listOfPlayers->next->next;
    free(p1);
    free(p2);
    pthread_mutex_unlock(&player_list_lock);
<<<<<<< HEAD
    
    //random role assignment
    int p = rand() % 2; 
    if(p==0)
    p1->type='o';
    else{
    p2->type='x';
    }
    //game can now be started
=======

    // game can now be started
    // randomly need to assign each player either an O and X
>>>>>>> 809cf60670b8c67cf7422c8467ae4de6c296fd4e

    
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
    char buf[1024];
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
void install_handlers(sigset_t *mask)
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigemptyset(mask);
    sigaddset(mask, SIGINT);
    sigaddset(mask, SIGTERM);
}