#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "practical.h"

void *ThreadMain(void *arg);    // Main program of a thread

// Structure of arguments to pass to client thread
struct ThreadArgs
{
    int clntSock;   // socket descriptor for client
};

int
main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Parameter(s) <Server Port / Service>\n");
        return 0;
    }
    char *service = argv[1];
    int servSock = SetupTCPServerSocket(service);

    if(servSock < 0)
    {
        fprintf(stderr, "SetupTCPServerSocket() failed, unable to establish\n");
        exit(EXIT_FAILURE);
    }
    for(;;)
    {
        int clntSock = AccepTCPConnection(servSock);

        // Create seperate memory for clinet argument
        struct ThreadArgs *threadArgs = (struct ThreadArgs*)malloc(
                sizeof(struct ThreadArgs));
        if(threadArgs == NULL)
        {
            fprintf(stderr, "malloc() failed\n");
            exit(EXIT_FAILURE);
        }
        threadArgs->clntSock = clntSock;

        // Create client thread
        pthread_t threadID;
        fputs("Accept!\n",stdout);
        int returnValue = pthread_create(&threadID, NULL,
                                             ThreadMain, threadArgs);
        if(returnValue != 0)
        {
            fprintf(stderr, "pthread_create() failed\n");
            exit(EXIT_FAILURE);
        }
    }
}

void *ThreadMain(void *ThreadArgs)
{
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());

    // Extract socket file descriptor from argument
    int clntSock = ((struct ThreadArgs *)ThreadArgs)->clntSock;
    free(ThreadArgs);   // Deallocate memory for argument
    HandleTCPClient(clntSock);

    return NULL;
}

