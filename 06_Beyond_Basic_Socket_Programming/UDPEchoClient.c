#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

int
main(int argc, char *argv[])
{
  
    
    if(argc < 3 || argc > 4)
    {
        fprintf(stderr, "<Server Address/Name> <Echo Word> [<Server Port / Service>]\n");
         return 0;
    }

    char *server = argv[1];
    char *echoString = argv[2];

    size_t echoStringLen = strlen(echoString);
        
    char *servPort = (argc == 4) ? argv[3] : "echo";

    // Tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;

    // For the following fields, a zero value means "don't care"

    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    // Get address(es)
    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(server, servPort, &addrCriteria, &servAddr);

    if (rtnVal != 0)
    {
        fprintf(stderr, "getaddrinfo() failed %s", gai_strerror(rtnVal));
        exit(EXIT_FAILURE);
    }
    
    // Create a datagram/UDP socket
    int sock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
    if(sock < 0)
    {
        fprintf(stderr, "socket() failed\n");
        exit(EXIT_FAILURE);
    }

    // Send the string to the server
    // sendto() : UDP/IP
    ssize_t numBytes = sendto(sock, echoString, echoStringLen, 0,
            servAddr->ai_addr, servAddr->ai_addrlen);
    if(numBytes < 0)
    {
        fprintf(stderr, "sendto() failed\n");
        exit(EXIT_FAILURE);
    }else if(numBytes != echoStringLen)
    {
        fprintf(stderr, "sendto() error : sent unexpected number of bytes\n");
        exit(EXIT_FAILURE);
    }

    // Receive response
    struct sockaddr_storage fromAddr;   // Source address of server
    // Set length of from address structure (in-out parameter)
    socklen_t fromAddrLen = sizeof(fromAddr);
    char buffer[1024];

    numBytes = recvfrom(sock, buffer, 1023, 0,
            (struct sockaddr *)&fromAddr, &fromAddrLen);
    if(numBytes < 0)
    {
        fprintf(stderr, "recvfrom() failed\n");
        exit(EXIT_FAILURE);
    }else if(numBytes != echoStringLen)
    {
        fprintf(stderr, "recvfrom() error : received unexpected number of bytes");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servAddr);

    buffer[1023] = '\0';
    printf("Received : %s\n", buffer);

    close(sock);
    return 0;
}

