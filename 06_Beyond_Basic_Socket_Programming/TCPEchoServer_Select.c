#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include "practical.h"

int
main(int argc, char *argv[])
{
    if (argc < 3)   // Test for correct number of arguments
    {
        fprintf(stderr, "Parameter(s), <Timeout (secs.)> <Port/Service1> ...");
        return 0;
    }
    long timeout = atoi(argv[1]);   // First arg : Timeout
    int noPorts = 2;                // Number of ports is argument count minus 2

    // Allocate list of sockets for incoming connections
    int servSock[noPorts];
    // Initilize maxDescriptor for use by select()
    int maxDescriptor = -1;

    // Create list of ports and sockets to handle ports
    for(int port = 0; port < noPorts; port++)
    {
        // Create port socket
        servSock[port] = SetupTCPServerSocket(argv[port+1]);

        if(servSock[port] > maxDescriptor)
            maxDescriptor = servSock[port];
    }
    puts("Starting server : Hit return to shutdown\n");
    bool running = true;    // true if server should continue running
    fd_set sockSet;

    while(running)
    {
        /* zero socket descriptor vector and set for server sockets
           This must be reset every time select() is called */
        FD_ZERO(&sockSet);
        // Add keyboard to descriptor vector
        FD_SET(STDIN_FILENO, &sockSet);

       for (int port = 0; port < noPorts; port++)
        {
            FD_SET(servSock[port], &sockSet);
        }
        struct timeval selTimout;        // Time out for select()
        selTimout.tv_sec = timeout;      // Set timeout (secs.)
        selTimout.tv_usec = 0;           // 0 micro seconds

        // Suspend program until descriptor is ready or timeout
        if (select(maxDescriptor +1, &sockSet, NULL, NULL, &selTimout) == 0)
        {
            printf("No echo request for %ld secs...Server still alive\n", timeout);
        }else{
            if(FD_ISSET(0, &sockSet)){  // If read event occurs in STDIN_FILENO...
                puts("Shutting down server");
                getchar();
                running = false;
            }
        }

        // Process connection requests
        for (int port = 0; port < noPorts; port++)
        {
            if(FD_ISSET(servSock[port], &sockSet))
            {
                printf("Request on port %d : \n", port);
                HandleTCPClient(AccepTCPConnection(servSock[port]));
            }
        }
    }
    
    // Close sockets
    for (int port = 0; port < noPorts; port++)
    {
        close(servSock[port]);
    }
    return 0;
}