#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <sys/select.h>
// #include "protocol.c"
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#define FIELDLEN 256
int sockFD;

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
void printGrid(char *board);
void turn(int sock, char role);
void signal_handler(int sig);

void signal_handler(int sig)
{
    if (sig == SIGINT)
    {
        printf("Exiting...\n");
        close(sockFD);
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    // The arguments to ttt are the domain name and port number of the desired service.
    // int bytes;
    char *domain = argv[1];
    int portNum = atoi(argv[2]);
    char portString[6];
    snprintf(portString, sizeof(portString), "%d", portNum);

    struct addrinfo hints, *info_list, *info;
    int errorflag;

    memset(&hints, 0, sizeof(hints)); // Initialize hints to zero
    hints.ai_family = AF_UNSPEC;      // In practice, this means give us IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // Indicate we want a streaming socket

    errorflag = getaddrinfo(domain, portString, &hints, &info_list); // Look up the address info for the host and service
    if (errorflag)
    {
        fprintf(stderr, "error looking up %s:%s: %s\n", domain, portString, gai_strerror(errorflag));
        exit(1);
    }

    for (info = info_list; info != NULL; info = info->ai_next)
    {
        sockFD = socket(info->ai_family, info->ai_socktype, info->ai_protocol); // Create socket
        if (sockFD < 0)
            continue; // If socket creation fails, move on to the next address

        errorflag = connect(sockFD, info->ai_addr, info->ai_addrlen); // Try connecting to the remote host
        if (errorflag)
        { // If connection fails, close the socket and move on to the next address
            close(sockFD);
            continue;
        }
        break; // If connection succeeds, break out of the loop
    }

    //**********************************************************************************
    // char playerName[NAME_SIZE];
    // work needs to be done from here... message 3 confusion.

    freeaddrinfo(info_list);
    if (info == NULL)
    { // If all addresses failed, print an error message and return -1
        fprintf(stderr, "Unable to connect to %s:%s\n", domain, portString);
        exit(1);
    }

    if (sockFD < 0)
        exit(1);

    signal(SIGINT, signal_handler);

    char name[FIELDLEN];
    char buf[FIELDLEN];
    int bytes;
    char role;

    printf("First, what is your name?\n");
    bytes = read(STDIN_FILENO, name, FIELDLEN);
    name[bytes - 1] = '\0';
    bytes = snprintf(buf, FIELDLEN, "PLAY|%d|%s|", bytes, name);
    write(sockFD, buf, bytes);

    struct msg pass;
    pass.len = 0;
    pass.num_fields = 0;
    while (p_recv(sockFD, &pass) > 0 && strcmp(pass.code, "INVL") == 0)
        if (strcmp(pass.code, "INVL") == 0)
        {
            printf("%s Enter a new name:\n", pass.fields[0]);
            bytes = read(STDIN_FILENO, name, FIELDLEN);
            name[bytes - 1] = '\0';
            bytes = snprintf(buf, FIELDLEN, "PLAY|%d|%s|", bytes, name);
            write(sockFD, buf, bytes);
        }

    if (strcmp(pass.code, "WAIT") == 0)
    {
        printf("Wait for your opponent to connect...\n");
        if (p_recv(sockFD, &pass) > 0 && strcmp(pass.code, "BEGN") == 0)
        {
            role = pass.fields[0][0];
            printf("The game has begun. You will be facing %s.\n", pass.fields[1]);
            if (role == 'X')
                turn(sockFD, role);

            while (p_recv(sockFD, &pass) > 0)
            {
                if (strcmp(pass.code, "INVL") == 0)
                {
                    printf("%s\n", pass.fields[0]);
                    if (pass.fields[0][0] != '!')
                        turn(sockFD, role);
                }

                else if (strcmp(pass.code, "MOVD") == 0)
                {
                    printf("New board:\n");
                    printGrid(pass.fields[1]);
                    if (role != pass.fields[0][0])
                        turn(sockFD, role);
                }

                else if (strcmp(pass.code, "DRAW") == 0)
                {
                    if (pass.fields[0][0] == 'R')
                    {
                        printf("Your opponent has declined to draw.\n");
                        turn(sockFD, role);
                    }
                    if (pass.fields[0][0] == 'S')
                    {
                        printf("Your opponent wishes to draw. Enter 'A' to accept or 'R' to reject.\n");
                        char choice[FIELDLEN];
                        while (read(STDIN_FILENO, choice, FIELDLEN) != 2 || !(choice[0] == 'R' || choice[0] == 'A'))
                            printf("Invalid response, please try again.\n");
                        choice[1] = '\0';
                        snprintf(buf, FIELDLEN, "DRAW|2|%c|", choice[0]);
                        write(sockFD, buf, 9);
                    }
                }

                else if (strcmp(pass.code, "OVER") == 0)
                {
                    if (pass.fields[0][0] == 'W')
                        printf("%s You won!\n", pass.fields[1]);

                    if (pass.fields[0][0] == 'L')
                        printf("%s You lost :(\n", pass.fields[1]);

                    if (pass.fields[0][0] == 'D')
                        printf("%s The game ended in a tie.\n", pass.fields[1]);
                    break;
                }
            }
        }
    }

    close(sockFD);
    return 0;
}

int p_recv(int sockFD, msg_t *msg)
{
    int msgend = 0;
    int leftover_length = 0;
    int fieldcount;
    int size;
    int bytes_read = read(sockFD, msg->buf + msg->len, FIELDLEN - msg->len);
    if (bytes_read == -1)
    {
        perror("read");
        return -1;
    }

    else if (bytes_read == 0 && msg->len == 0)
        return 0;

    msg->len += bytes_read;
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
            }
            else
            {
                char field[FIELDLEN];
                strncpy(field, msg->buf + msgend, i - msgend);
                field[i - msgend] = '\0';
                strcpy(msg->fields[msg->num_fields - 2], field);
            }
            msgend = i + 1;
            msg->num_fields++;
        }

        if (fieldcount != 0 && msg->num_fields == fieldcount)
            break;
    }

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
    if (strcmp(type, "WAIT") == 0)
        return 2;
    if (strcmp(type, "INVL") == 0 || strcmp(type, "DRAW") == 0)
        return 3;
    if (strcmp(type, "BEGN") == 0 || strcmp(type, "MOVD") == 0 || strcmp(type, "OVER") == 0)
        return 4;

    return 0;
}

void turn(int sock, char role)
{
    char choice[FIELDLEN];
    char buf[FIELDLEN];
    int bytes;
    printf("Enter 'MOVE' to make a move, 'RSGN' to resign, or 'DRAW' to draw.\n");
    bytes = read(STDIN_FILENO, choice, FIELDLEN);
    choice[bytes - 1] = '\0';
    if (strcmp(choice, "MOVE") == 0)
    {
        char pos[FIELDLEN];
        printf("Enter the position of your move in this format 'x,y'.\n");
        while (read(STDIN_FILENO, pos, FIELDLEN) != 4 || !(isdigit(pos[0]) && pos[1] == ',' && isdigit(pos[2])))
            printf("Invalid input, please try again.\n");
        pos[3] = '\0';
        bytes = snprintf(buf, FIELDLEN, "MOVE|6|%c|%.3s|", role, pos);
        printf("%s\n", buf);
        write(sock, buf, bytes);
    }
    else if (strcmp(choice, "RSGN") == 0)
        write(sock, "RSGN|0|", 7);

    else if (strcmp(choice, "DRAW") == 0)
        write(sock, "DRAW|2|S|", 9);

    else
    {
        printf("Invalid input, please try again.\n");
        turn(sock, role);
    }
}

void printGrid(char *stringBoard)
{
    int k = 0;
    // Print the grid
    for (int x = 0; x < 3; x++)
    {
        for (int y = 0; y < 3; y++)
            printf("%c ", stringBoard[k++]);
        printf("\n");
    }
}