#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "protocol.h"

int play(int, char *);
void begin(int, int, char, char *);
void moved(int, int, char, int, int, char *);
void invalid(int, int, char *);
void draw(int, int, char);
void over(int, int, char, char *);

int play(int sockfd, char *buffer)
{
    PLAY msg;
    char response[4];
    sscanf(buffer, "%*4s|%d|%[^|]|", &msg.size, msg.name);
    send(sockfd, &msg.name, sizeof(msg.name), 0);
    recv(sockfd, &response, 4, 0);
    // Check if WAIT was received
    if (strcmp(response, "WAIT") == 0)
        return 1;

    return 0;
}

void begin(int sockfd, int size, char role, char *name)
{
    BEGN msg;
    msg.size = size;
    msg.role = role;
    strcpy(msg.name, name);
    send(sockfd, &msg, sizeof(msg), 0);
}

void moved(int sockfd, int size, char role, int x, int y, char *board)
{
    MOVD msg;
    msg.size = size;
    msg.role = role;
    msg.x = x;
    msg.y = y;
    strcpy(msg.board, board);
    send(sockfd, &msg, sizeof(msg), 0);
}

void invalid(int sockfd, int size, char *reason)
{
    INVL msg;
    msg.size = size;
    strcpy(msg.reason, reason);
    send(sockfd, &msg, sizeof(msg), 0);
}

void draw(int sockfd, int size, char message)
{
    DRAW msg;
    msg.size = size;
    msg.message = message;
    send(sockfd, &msg, sizeof(msg), 0);
}

void over(int sockfd, int size, char outcome, char *reason)
{
    OVER msg;
    msg.size = size;
    msg.outcome = outcome;
    strcpy(msg.reason, reason);
    send(sockfd, &msg, sizeof(msg), 0);
}