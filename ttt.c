#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define GRIDSIZE 3

int socket(
    int address_family, // what sort of network?
    int socket_type, // what sort of socket?
    int protocol // protocol disambiguation (usually 0)
);


int main(int argc, char *argv[]){
    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    //The arguments to ttt are the domain name and port number of the desired service. 
    // char *domain = argv[1];
    // int portNum = atoi(argv[2]);
//********************************************************************************************
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
