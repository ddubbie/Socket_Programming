#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

void UseIdleTime();                     // Execution during idle time
void SIGIOHandler(int signalType);      // Handle SIGIO

int servSock;   //  Socket -- GLOBAL for signal handler


void PrintSocketAddress(const struct sockaddr *address, FILE *stream) {
    // Test for address and stream
    if (address == NULL || stream == NULL)
    return;

    void *numericAddress; // Pointer to binary address
    // Buffer to contain result (IPv6 sufficient to hold IPv4)
    char addrBuffer[INET6_ADDRSTRLEN];
    in_port_t port; // Port to print
    // Set pointer to address based on address family
    switch (address->sa_family) {
        case AF_INET:
            numericAddress = &((struct sockaddr_in *) address)->sin_addr;
            port = ntohs(((struct sockaddr_in *) address)->sin_port);
            break;
        case AF_INET6:
            numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
            port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
            break;
        default:
        fputs("[unknown type]", stream); // Unhandled type
        return;
    }
    // Convert binary to printable address
    if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL)
    {
        fputs("[invalid address]", stream); // Unable to convert
    }
    else {
        fprintf(stream, "%s", addrBuffer);
         if (port != 0) // Zero not valid in any socket addr
            fprintf(stream, "-%u", port);
    }
}



int
main(int argc, char *argv[])
{
    if(argc != 2)   //  Test for correct number of arguments
    {
        fprintf(stderr, "Parametre(s) <Server Port/Service>\n");
        return 0;
    }
    char *service = argv[1];    // local port

    // Construct the server address structure
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if(rtnVal != 0)
    {
        fprintf(stderr, "getaddrinfo() failed\n");
        exit(EXIT_FAILURE);
    }

    // Create socket for incoming connections
    servSock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
    if(servSock < 0)
    {
        fprintf(stderr, "socket() failed");
        exit(EXIT_FAILURE);
    }

    // Bind to the local address
    if(bind(servSock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
    {
        fprintf(stderr, "bind() failed");
        exit(EXIT_FAILURE);
    }

    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);

    struct sigaction handler;
    handler.sa_handler = SIGIOHandler;

    // Create mask that mask all signals
    if(sigfillset(&handler.sa_mask) < 0)
    {
        fprintf(stderr, "sigfillset() failed\n");
        exit(EXIT_FAILURE);
    }
    handler.sa_flags = 0;   // No flags

    if (sigaction(SIGIO, &handler, 0) < 0)
    {
        fprintf(stderr, "sigaction() failed for SIGIO\n");
        exit(EXIT_FAILURE);
    }

    // We must own the socket to receive the SIGIO message
    if(fcntl(servSock, F_SETOWN, getpid()) < 0)
    {
        fprintf(stderr, "Unable to set process owner");
        exit(EXIT_FAILURE);
    }


    // Arrange for nonblocking I/O and SIGIO delivery
    if(fcntl(servSock, F_SETFL, O_NONBLOCK | O_ASYNC) < 0)
    {
        fprintf(stderr, "Unable to put client sock into non-blocking/async mode\n");
        exit(EXIT_FAILURE);
    }
     // Go off and do real work; echoing happens in the background

     for(;;)
     {
        UseIdleTime();
        // NOT REACHED
     }
}

void
UseIdleTime()
{
    puts(".");
    sleep(3);
}

void 
SIGIOHandler(int signalType)
{
    ssize_t numBytesRcvd;
    do{// As long as there is input
        struct sockaddr_storage clntAddr;       // Address of datagram source
        socklen_t clntlen = sizeof(clntAddr);      // Address length in-out parameter
        char buffer[8192];

        numBytesRcvd = recvfrom(servSock, buffer, 8192, 0,
            (struct sockaddr *)&clntAddr, &clntlen);
        
        if (numBytesRcvd < 0)
        {   // Only acceptable error : recvfrom() would have blocked
            if(errno != EWOULDBLOCK)
            {
                fprintf(stderr, "recvfrom() failed\n");
                exit(EXIT_FAILURE);
            }
        }
        else{
            fprintf(stdout, "Handling client\n");
            PrintSocketAddress((struct sockaddr *)&clntAddr, stdout);
            fputc('\n', stdout);

            ssize_t numBytesSent = sendto(servSock, buffer, numBytesRcvd, 0,
                           (struct sockaddr *)&clntAddr, sizeof(clntAddr));

            if (numBytesSent < 0)
            {
                fprintf(stderr, "sendto() failed\n");
                exit(EXIT_FAILURE);
            }else if(numBytesSent != numBytesRcvd)
            {
                fprintf(stderr, "sendto(), sent unexpected number of bytes\n");
            }
         }
    }while(numBytesRcvd >= 0);
    // Nothing left to recive
}