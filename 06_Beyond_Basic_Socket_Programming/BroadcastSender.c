#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "practical.h"

#define MAXSTRINGLENGTH 4096

int
main(int argc, char *argv[])
{
    if (argc != 4)  // Test for correct number of arguments
    {
        fprintf(stderr, "Parameter(s), [4|6] <Port> <String to Send>\n");
        return 0;
    }

    in_port_t port = htons((in_port_t) atoi(argv[2]));

    struct sockaddr_storage destStorage;
    memset(&destStorage, 0, sizeof(destStorage));

    size_t addrSize = 0;

    struct sockaddr_in *destAddr4 = (struct sockaddr_in *)&destStorage;
    destAddr4->sin_family = AF_INET;
    destAddr4->sin_port = port;
    destAddr4->sin_addr.s_addr = INADDR_BROADCAST;
    addrSize = sizeof(struct sockaddr_in);

    struct sockaddr *destAddress = (struct sockaddr *)&destStorage;

    size_t msgLen = strlen(argv[3]);

    if (msgLen > MAXSTRINGLENGTH)
    {
        fprintf(stderr, "String too long\n");
        exit(EXIT_FAILURE);
    }

    // Create socket for sending/receiving datagrams
    int sock = socket(destAddress->sa_family, SOCK_DGRAM, IPPROTO_UDP);
    if(sock < 0)
    {
        fprintf(stderr, "socket() failed\n");
        exit(EXIT_FAILURE);
    }

    // Set socket to allow broadcast
    int broadcastPerm = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastPerm, sizeof(broadcastPerm)) < 0)
    {
        fprintf(stderr, "setsockopt() failed\n");
        exit(EXIT_FAILURE);
    }

    for(;;)
    {
        // Broadcast msgString in datagram to client every 3 seconds
        ssize_t numBytes = sendto(sock, argv[3], msgLen, 0, destAddress, addrSize);

        if (numBytes < 0)
        {
            fprintf(stderr, "sendto() failed\n");
            exit(EXIT_FAILURE);
        }else if(numBytes != msgLen)
        {
            fprintf(stderr, "sendto(), sent unexpected number of bytes\n");
            exit(EXIT_FAILURE);
        }
        sleep(3);
    }
    // NOT REACHED
}