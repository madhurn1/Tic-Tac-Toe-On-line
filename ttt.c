#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define GRIDSIZE 3
#define NAME_SIZE 256


int main(int argc, char *argv[]){
    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    //The arguments to ttt are the domain name and port number of the desired service. 
    char *domain = argv[1];
    int portNum = atoi(argv[2]);
    char portString[6];
    snprintf(portString, sizeof(portString), "%d", portNum);


    struct addrinfo hints, *info_list, *info;
    int sockFD, errorflag;

    memset(&hints, 0, sizeof(hints)); // Initialize hints to zero
    hints.ai_family = AF_UNSPEC; // In practice, this means give us IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Indicate we want a streaming socket

    errorflag = getaddrinfo(domain, portString, &hints, &info_list); // Look up the address info for the host and service
    if (errorflag) {
    fprintf(stderr, "error looking up %s:%s: %s\n", domain, portString, gai_strerror(errorflag));    
    exit(1);
    }

    for (info = info_list; info != NULL; info = info->ai_next) {
    sockFD = socket(info->ai_family, info->ai_socktype, info->ai_protocol); // Create socket
    if (sockFD < 0) continue; // If socket creation fails, move on to the next address

    errorflag = connect(sockFD, info->ai_addr, info->ai_addrlen); // Try connecting to the remote host
    if (errorflag) { // If connection fails, close the socket and move on to the next address
      close(sockFD);
      continue;
    }
    break; // If connection succeeds, break out of the loop
  }
  freeaddrinfo(info_list);
  if (info == NULL) { // If all addresses failed, print an error message and return -1
    fprintf(stderr, "Unable to connect to %s:%s\n", domain, portString);
    exit(1);
  }
  if (sockFD < 0) 
    exit(1);
//**********************************************************************************
    // char playerName[NAME_SIZE];
    //work needs to be done from here... message 3 confusion. 



    return 1;
}




void printGrid(char grid[GRIDSIZE][GRIDSIZE]) {
    for (int i = 0; i < GRIDSIZE; i++) {
        for (int j = 0; j < GRIDSIZE; j++) {
            printf(" %c ", grid[i][j]);
            if (j < GRIDSIZE - 1) {
                printf("|");
            }
        }
        printf("\n");
        if (i < GRIDSIZE - 1) {
            printf("---|---|---\n");
        }
    }
}
