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
// #include "protocol.c"

// game Server
#define QUEUE_SIZE 8
#define FIELDLEN 256
#define BOARD [3][3]

// global mutex to ensure thread-safe access to the list of player names
pthread_mutex_t player_list_lock = PTHREAD_MUTEX_INITIALIZER;

char board[3][3] = {
    {'.', '.', '.'},
    {'.', '.', '.'},
    {'.', '.', '.'}};

typedef struct PlayerNode
{
    char pName[FIELDLEN];
    char role;
    int sock;
    struct PlayerNode *next;
} PlayerNode;

typedef struct msg
{
    char buf[FIELDLEN];
    char code[5];
    char fields[5][FIELDLEN];
    int len;
    int num_fields;
} msg_t;

int p_recv(int sockFD, msg_t *msg);
int field_count(char *);
char *updateBoard(char role, int xCor, int yCor);
void switchsock(int *sock, int sock1, int sock2);

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
void addPlayer(char *name, int sock);
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

    while (active)
    {
        // holds the address information of the client that connects to the server.
        con = (struct connection_data *)malloc(sizeof(struct connection_data));
        // struct sockaddr_storage client_addr;
        con->addr_len = sizeof(struct sockaddr_storage);

        // determine the size of the buffer where the client's address information will be stored.
        //  socklen_t client_addrlen = sizeof(client_addr);

        /*is a system call that waits for a connection request from a client and accepts
        it, returning a new socket file descriptor that is used to communicate with the client.
        */
        // int client_sock = accept(sock, (struct sockaddr*)&client_addr, &client_addrlen);
        con->fd = accept(sock, (struct sockaddr *)&con->addr, &con->addr_len);

        if (con->fd == -1)
        {
            perror("accept");
            free(con);
            continue;
        }
        printf("Connected\n");
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
    // char playerName[FIELDLEN];
    char player1[FIELDLEN];
    char player2[FIELDLEN];
    int sock1;
    int sock2;
    int role1;
    int role2;
    int playerflag = 0;

    // wait for "PLAY" message from client
    struct msg pass;
    pass.len = 0;
    pass.num_fields = 0;

    if (p_recv(client_sock, &pass) <= 0)
        close(client_sock);

    // checking to seee if player name is inuse

    if (checkName(pass.fields[0]) == 1)
        write(client_sock, "INVL|12|Name in use|", 19);
    else
        write(client_sock, "WAIT|0|", 7);

    // adding player to linked list structure.
    addPlayer(pass.fields[0], client_sock);

    while (1)
    {
        pthread_mutex_lock(&player_list_lock);
        if (listOfPlayers != NULL && listOfPlayers->next != NULL)
            playerflag = 2;

        pthread_mutex_unlock(&player_list_lock);
        if (playerflag == 2)
            // both players ready
            break;

        sleep(1);
    }

    pthread_mutex_lock(&player_list_lock);
    PlayerNode *p1 = listOfPlayers;
    PlayerNode *p2 = listOfPlayers->next;
    p1->role = 'X';
    p2->role = 'O';
    strncpy(player1, p1->pName, FIELDLEN);
    strncpy(player2, p2->pName, FIELDLEN);
    sock1 = p1->sock;
    sock2 = p2->sock;
    role1 = p1->role;
    role2 = p2->role;
    listOfPlayers = listOfPlayers->next->next;
    free(p1);
    free(p2);
    pthread_mutex_unlock(&player_list_lock);
    // game can now be started
    char buf[FIELDLEN];
    int bytes;
    bytes = snprintf(buf, FIELDLEN, "BEGN|%ld|%c|%s|", strlen(player2) + 3, role1, player2);
    write(sock1, buf, bytes);
    bytes = snprintf(buf, FIELDLEN, "BEGN|%ld|%c|%s|", strlen(player1) + 3, role2, player1);
    write(sock2, buf, bytes);
    int cursock = sock1;
    while (1)
    {
        if (p_recv(cursock, &pass) > 0)
        {
            if (strcmp(pass.code, "MOVE") == 0)
            {
                char *newboard = updateBoard(pass.fields[0][0], pass.fields[1][0] - '0' - 1, pass.fields[1][2] - '0' - 1);
                if (strcmp(newboard, "INVL") == 0)
                {
                    write(client_sock, "INVL|14|Invalid move|", 21);
                    continue;
                }

                snprintf(buf, FIELDLEN, "MOVD|12|%c|%s|", pass.fields[0][0], newboard);
                write(sock1, buf, 20);
                write(sock2, buf, 20);
                switchsock(&cursock, sock1, sock2);
            }

            else if (strcmp(pass.code, "RSGN") == 0)
            {
                bytes = snprintf(buf, FIELDLEN, "OVER|%ld|W|%s has resigned|", strlen(p1->pName) + 17, p1->pName);
                write(client_sock, buf, bytes);
                bytes = snprintf(buf, FIELDLEN, "OVER|%ld|L|%s has resigned|", strlen(p1->pName) + 17, p1->pName);
                write(client_sock, buf, bytes);
                break;
            }

            else if (strcmp(pass.code, "DRAW") == 0)
            {
                if (pass.fields[0][0] == 'S')
                    write(client_sock, "DRAW|2|S|", 9);
                if (pass.fields[0][0] == 'R')
                    write(client_sock, "DRAW|2|R|", 9);
                if (pass.fields[0][0] == 'A')
                {
                    write(client_sock, "OVER|37|D|Both players have decided to draw|", 44);
                    write(client_sock, "OVER|37|D|Both players have decided to draw|", 44);
                    break;
                }
            }
        }
    }
    close(client_sock);
    pthread_exit(NULL);
}

void switchsock(int *sock, int sock1, int sock2)
{
    if (*sock == sock1)
        *sock = sock2;
    else
        *sock = sock1;
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

void addPlayer(char *name, int sock)
{
    // linked list
    PlayerNode *node_p = (PlayerNode *)malloc(sizeof(PlayerNode)); // this needs to be freed som
    strncpy(node_p->pName, name, FIELDLEN);
    node_p->sock = sock;
    node_p->next = NULL;
    pthread_mutex_lock(&player_list_lock);
    if (listOfPlayers == NULL)
        listOfPlayers = node_p;

    else
    {
        PlayerNode *temp = listOfPlayers;
        while (temp->next != NULL)
            temp = temp->next;

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

int p_recv(int sockFD, msg_t *msg)
{
    int msgend = 0;
    int leftover_length = 0;
    int fieldcount;
    int size;
    int bytes_read = read(sockFD, msg->buf + msg->len, FIELDLEN - msg->len);
    printf("BUF: %s\n", msg->buf);

    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    else if (bytes_read == 0 && msg->len == 0)
        // connection closed
        return 0;

    msg->len += bytes_read;

    printf("%d %d\n", msg->len, bytes_read);
    // check for complete message
    for (int i = msgend; i < msg->len; i++)
    {
        if (msg->buf[i] == '|')
        {
            if (msg->num_fields == 0)
            {
                // first field is the code
                if (i - msgend != 4)
                {
                    fprintf(stderr, "error: invalid code length\n");
                    return -1;
                }
                strncpy(msg->code, msg->buf + msgend, 4);
                msg->code[4] = '\0';
                fieldcount = field_count(msg->code);
                printf("CODE: %s, FIELDS: %d\n", msg->code, fieldcount);
            }
            else if (msg->num_fields == 1)
            {
                // second field is the message length
                // if (read_field(buf, msgend, i, size_str) == -1)                        return -1;
                char size_str[FIELDLEN];
                strncpy(size_str, msg->buf + msgend, i - msgend);
                size_str[i - msgend] = '\0';
                size = atoi(size_str);

                if (size < 0 || size > FIELDLEN)
                {
                    fprintf(stderr, "error: invalid message size\n");
                    return -1;
                }

                printf("SIZE: %d\n", size);
            }
            else
            {
                // subsequent fields are variable-length strings
                // if (read_field(buf, msgend, i, field) == -1)                        return -1;
                char field[FIELDLEN];
                strncpy(field, msg->buf + msgend, i - msgend);
                field[i - msgend] = '\0';
                strcpy(msg->fields[msg->num_fields - 2], field);
                printf("FIELD: %s\n", field);
            }
            msgend = i + 1;
            msg->num_fields++;
        }

        if (fieldcount != 0 && msg->num_fields == fieldcount)
            break;
    }

    printf("LEN: %d\n", msg->len);
    printf("FIELDS: %d\n", msg->num_fields);

    // check for improperly formatted message
    if (msg->buf[msg->len - 1] != '|')
    {
        fprintf(stderr, "error: message not terminated with '|'\n");
        return -1;
    }

    // there's more data to come, so move leftover data to the front of the buffer

    leftover_length = msg->len - msgend;
    memmove(msg->buf, msg->buf + msgend, leftover_length);
    msg->buf[leftover_length] = '\0';
    msg->len = leftover_length;
    msg->num_fields = 0;
    return 1;
}

int field_count(char *type)
{
    if (strcmp(type, "RSGN") == 0)
        return 2;
    if (strcmp(type, "PLAY") == 0 || strcmp(type, "DRAW") == 0)
        return 3;
    if (strcmp(type, "MOVE") == 0)
        return 4;

    return 0;
}

char *updateBoard(char role, int xCor, int yCor)
{
    printf("%d,%d\n", xCor, yCor);
    if (xCor > 2 || xCor < 0 || yCor > 2 || yCor < 0)
        return "INVL";

    if (board[xCor][yCor] == '.')
        board[xCor][yCor] = role;
    else
        return NULL;

    static char stringBoard[10];
    int count = 0;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            stringBoard[count++] = board[i][j];

    printf("%s\n", stringBoard);
    return stringBoard;
}