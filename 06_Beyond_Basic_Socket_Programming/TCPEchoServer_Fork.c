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
#include <sys/wait.h>

#define MAX_BUF_SIZE 8192
#define MAXPENDING 5

static inline
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


inline static void
HandleTCPClient(int clntSocket)
{
    char buffer[MAX_BUF_SIZE];

    ssize_t numBytesRcvd = recv(clntSocket, buffer, MAX_BUF_SIZE, 0);
    if(numBytesRcvd < 0)
    {
        fprintf(stderr, "recv() failed\n");
        exit(EXIT_FAILURE);
    }
    while (numBytesRcvd > 0)
    {
        ssize_t numByteSent = send(clntSocket, buffer, numBytesRcvd, 0);
        if(numByteSent < 0)
        {
            fprintf(stderr, "send() failed\n");
            exit(EXIT_FAILURE);
        }else if(numByteSent != numBytesRcvd)
        {
            fprintf(stderr, "send unexpected number of bytes\n");
        }
        numBytesRcvd = recv(clntSocket, buffer, MAX_BUF_SIZE, 0);
        if(numBytesRcvd < 0)
        {
            fprintf(stderr, "recv() failed\n");
            exit(EXIT_FAILURE);
        }
    }
    fprintf(stdout, "Finish handling this client\n");
    close(clntSocket);
}

inline static int
SetupTCPServerSocket(const char *service)
{
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if (rtnVal != 0)
    {
        fprintf(stderr, "getaddrinfo() failed %s\n", gai_strerror(rtnVal));
        exit(EXIT_FAILURE);
    }
    int servSock = -1;
    for (struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next)
    {
        // Create a TCP socket
        servSock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
        if(servSock < 0)    continue;

        if(bind(servSock, servAddr->ai_addr, servAddr->ai_addrlen) == 0
                && (listen(servSock, MAXPENDING) == 0))
        {
            struct sockaddr_storage localAddr;
            socklen_t addrSize = sizeof(localAddr);

            // To check binded IP address and port number
            if(getsockname(servSock, (struct sockaddr *)&localAddr, &addrSize) < 0)
            {
                fprintf(stderr, "getsockname() failed\n");
                exit(EXIT_FAILURE);
            }
            fputs("Binding to ", stdout);
            PrintSocketAddress((struct sockaddr *)&localAddr, stdout);
            fputc('\n', stdout);
            break;
        }
        close(servSock);
        servSock = -1;
    }
    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);

    return servSock;
}

int
AccepTCPConnection(int servSock)
{
    struct sockaddr_storage clntAddr;
    // Set length of client address structure(in-out parameters)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    if(clntSock < 0)
    {
        fprintf(stderr, "accpet() failed\n");
        exit(EXIT_FAILURE);
    }
    /* ---------------------------- Blocking --------------------------------------------*/
    // clntSock is connected to a client!
    fputs("Handling client ", stdout);
    PrintSocketAddress((struct sockaddr *)&clntAddr, stdout);
    fputc('\n', stdout);
    return clntSock;
}


int
main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Parameter(s) <Server Port/Service\n");
        return 0;
    }
    char *service = argv[1];
    int servSock = SetupTCPServerSocket(service);
    if(servSock < 0)
    {
        fprintf(stderr, "SetupTCPServerSocket() failed, unbale to establish\n");
        exit(EXIT_FAILURE);
    }
    unsigned int childProcCount = 0;        // Number of child processes

    for(;;)
    {   // Run forever
        // New connection creates a client socket
        int clntSock = AccepTCPConnection(servSock);
        // Fork child process and report any errors
        pid_t processID = fork();

        if(processID < 0)
        {
            fprintf(stderr, "fork() failed\n");
            exit(EXIT_FAILURE);
        }else if(processID == 0)    // If this is the child process
        {
            close(servSock);        // Parent process still maintains a socket.
            HandleTCPClient(clntSock);
            exit(EXIT_SUCCESS);     // Child process terminates
        }

        /* codes for parent process */
        printf("with child proces : %d\n", processID);
        close(clntSock);        // Parent closes child socket descriptor
        childProcCount++;

        while(childProcCount)
        {   // Clean up all zombies
            processID = waitpid((pid_t)-1, NULL, WNOHANG);
            if(processID < 0)   // waitpid() error?
            {
                fprintf(stderr, "waitpid() failed\n");
            }else if(processID == 0)
            {
                break;
            }else
            {
                childProcCount--;
            }
        }
    }
}