#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define DEBUG

static const int MAXPENDING = 5;

inline static
void HandleTCPClient(int clntSocket)
{
    char buffer[8192];

    // Receive message from client
    ssize_t numBytesRcvd = recv(clntSocket, buffer, 8192, 0);
    if (numBytesRcvd < 0)
    {
        fprintf(stderr, "recv() failed\n");
        exit(EXIT_FAILURE);
    }

    // Send received string and receive agin until end of stream
    while (numBytesRcvd > 0)
    {
        ssize_t numBytesSent = send(clntSocket, buffer, numBytesRcvd, 0);
        if(numBytesSent < 0)
        {
            fprintf(stderr, "send() failed\n");
            exit(EXIT_FAILURE);
        }else if (numBytesSent != numBytesRcvd)
        {
            fprintf(stderr, "sent unexpected number of bytes\n");
            exit(EXIT_FAILURE);
        }
        numBytesRcvd = recv(clntSocket, buffer, 8192, 0);
        if(numBytesRcvd < 0)
        {
            fprintf(stderr, "recv() failed\n");
            exit(EXIT_FAILURE);
        }
    }
   printf("Finish handling this client!\n");
   close(clntSocket);
}




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
    if (inet_ntop(address->sa_family, numericAddress, addrBuffer,
    sizeof(addrBuffer)) == NULL)
    fputs("[invalid address]", stream); // Unable to convert
    else {
    fprintf(stream, "%s", addrBuffer);
    if (port != 0) // Zero not valid in any socket addr
    fprintf(stream, "-%u", port);
    }
}

static int
AcceptTCPConnection(int servSock)   // servSock : fd
{
    // Client address
    struct sockaddr_storage clntAddr;               // ipv4
    // Set length of clinet address structure (in0out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    if(clntSock < 0)
    {
        fprintf(stderr, "accept() failed\n");
        exit(EXIT_FAILURE);
    }

    // clntSock is connected to a clinet!

    fputs("Handling clinet ", stdout);
    PrintSocketAddress((struct sockaddr *)&clntAddr, stdout);
    fputc('\n', stdout);

    return clntSock;
}



static int
SetupTCPServerSocket(const char *service)
{
    // Construct the server address structure
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_flags = AI_PASSIVE;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    struct addrinfo *servAddr;
    // Find url - port - ip set
    int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
    if(rtnVal != 0)
    {
        fprintf(stderr, "getaddrinfo() failed");
        exit(EXIT_FAILURE);
    }
    
    int servSock = -1;
    
    for(struct addrinfo *addr = servAddr; addr != NULL; addr = addr->ai_next)
    {
        // Create a TCP socket
        servSock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
        if (servSock < 0)   continue;

        // Bind to the local address and set socket to list
        if(bind(servSock, servAddr->ai_addr, servAddr->ai_addrlen) < 0){
            fprintf(stderr, "bind() error\n");
            servSock = -1;
            continue;

        }
        if(listen(servSock, MAXPENDING) < 0)
        {
            fprintf(stderr, "listen() error\n");
            servSock = -1;
            continue;
        }
           // Print local address of socket, servSock ---> socket fd
        struct sockaddr_storage localAddr;
        socklen_t addrSize = sizeof(localAddr);

            // Confirm and print wheter socket is binded or not.
        if(getsockname(servSock, (struct sockaddr *)&localAddr, &addrSize) != 0)
        {// localAddr gets information of socket
            fprintf(stderr, "getsockname() failed\n");
            servSock = -1;
            continue;
        }

        fputs("Binding to ", stdout);
        PrintSocketAddress((struct  sockaddr*)&localAddr, stdout);
        fputc('\n', stdout);
        break;
        
        close(servSock);
        servSock = -1;
    }
    // Free address list allocated by getaddrinfo()
    freeaddrinfo(servAddr);

    return servSock;
    
}

int
main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Parameter(s) <Server Port / Service>\n");
        return 0;
    }
    char *service = argv[1];    // First arg : local port
    int servSock = SetupTCPServerSocket(service);
    if(servSock < 0)
    {
        fprintf(stderr, "SetupTCPServerSocket() failed service\n");
        exit(EXIT_FAILURE);
    }
    for(;;)
    {
        int clntSock = AcceptTCPConnection(servSock);

        HandleTCPClient(clntSock);
        close(clntSock);
    }
}