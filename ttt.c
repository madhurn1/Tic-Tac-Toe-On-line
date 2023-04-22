#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <sys/select.h>
#include "protocol.c"
#include <errno.h>

#define GRIDSIZE 3
#define BUFLEN 256
char board[GRIDSIZE][GRIDSIZE];

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    // The arguments to ttt are the domain name and port number of the desired service.
    int bytes;
    char *domain = argv[1];
    int portNum = atoi(argv[2]);
    char portString[6];
    snprintf(portString, sizeof(portString), "%d", portNum);

    struct addrinfo hints, *info_list, *info;
    int sockFD, errorflag;

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

    char buffer[BUFLEN];
    char command[5];

    // for (int i = 0; i < GRIDSIZE; i++) {
    //     for (int j = 0; j < GRIDSIZE; j++) {
    //         board[i][j] = '-';
    //     }
    // }

    //**********************************************************************************

    while (1)
    {
        // Read from fd and print to stdoutx
        bytes = read(sockFD, buffer, BUFLEN);
        if (bytes == -1)
        {
            perror("read");
        }
        else if (bytes == 0)
            break;

        else
        {
            strncpy(command, buffer, 4);
            command[4] = '\0';
        }

        // Read from stdin and write to fd
        //reading the rest of the actual name
        bytes = read(STDIN_FILENO, buffer, BUFLEN);
        if (bytes == -1)
        {
            // Handle read error        
        }
        else if (bytes == 0)
            // stdin has been closed
            break;
        else
        {
            strncpy(command, buffer, 4);
            command[4] = '\0';
            if (strcmp(command, "PLAY") == 0)
            {
                play(sockFD, buffer);
                write(STDOUT_FILENO, buffer, bytes);
            }
        }
    }

    /*
        while ((bytes = read(STDIN_FILENO, buf, BUFLEN)) > 0)
        {

            strncpy(command, buf, 4);
            command[4] = '\0';
            {
                if (strcmp(command, "PLAY") == 0)
                {
                    PLAY player;
                    sscanf(buf, "%4s|%d|%[^|]|", command, &player.size, player.name);
                    play(sockFD, player.size, player.name);
                    return 0;
                }
            }
            // work needs to be done from here... message 3 confusion.
        }
    */
    close(sockFD);
    return 0;
}

void printGrid(char grid[GRIDSIZE][GRIDSIZE])
{
    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            printf(" %c ", grid[i][j]);
            if (j < GRIDSIZE - 1)
            {
                printf("|");
            }
        }
        printf("\n");
        if (i < GRIDSIZE - 1)
        {
            printf("---|---|---\n");
        }
    }
}

void check_game_end(){
    // check rows
    for (int i = 0; i < GRIDSIZE; i++)
    {
        if (board[i][0] != '-' && board[i][0] == board[i][1] && board[i][1] == board[i][2])
        {
            printf("Game over. Winner: %c\n", board[i][0]);
            exit(0);
        }
    }

    // check columns
    for (int j = 0; j < GRIDSIZE; j++)
    {
        if (board[0][j] != '-' && board[0][j] == board[1][j] && board[1][j] == board[2][j])
        {
            printf("Game over. Winner: %c\n", board[0][j]);
            exit(0);
        }
    }
    // check diagonals
    if (board[0][0] != '-' && board[0][0] == board[1][1] && board[1][1] == board[2][2])
    {
        printf("Game over. Winner: %c\n", board[0][0]);
        exit(0);
    }
    if (board[0][2] != '-' && board[0][2] == board[1][1] && board[1][1] == board[2][0])
    {
        printf("Game over. Winner: %c\n", board[0][2]);
        exit(0);
    }

    // check for tie
    int count = 0;

    for (int i = 0; i < GRIDSIZE; i++)
    {
        for (int j = 0; j < GRIDSIZE; j++)
        {
            if (board[i][j] != '-')
            {
                count++;
            }
        }
    }
    if (count == GRIDSIZE * GRIDSIZE)
    {
        printf("Game over. Tie.\n");
        exit(0);
    }
}           

// Update the game board based on a move
void update_board(char role, int row, int col)
{
    board[row][col] = role;
    printGrid(board);
}