#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "practical.h"

void ProcessMain(int servsock);

int
main(int argc, char *argv[])
{
    if (argc != 3)  // Test for correct number of arguments
    {
        fprintf(stderr, "Parameter(s) <Server Port/Service> <Process Count>\n");
        return 0;
    }
    char *service = argv[1];
    unsigned int processLimit = atoi(argv[2]);

    // Server socket
    int servSock = SetupTCPServerSocket(service);

    // Fork limit-1 child process
    for (int processCt = 0; processCt < processLimit - 1; processCt++)
    {
        // Fork child process and report any errors
        pid_t processID = fork();
        if(processID < 0)
        {
            fprintf(stderr, "fork() is failed\n");
            exit(EXIT_FAILURE);
        }
        else if(processID == 0)
        {
            ProcessMain(servSock);
        }
    }
    ProcessMain(servSock);
}

void ProcessMain(int servSock)
{
    for(;;)
    {
        int clntSock = AccepTCPConnection(servSock);
        printf("with child rpcoess : %d\n", getpid());
        HandleTCPClient(clntSock);
    }
}