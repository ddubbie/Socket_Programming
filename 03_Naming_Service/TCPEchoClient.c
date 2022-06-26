#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int
SetupTCPClientSocket(const char *host, const char *service)
{
    // Tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    // Get address(es)
    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr);

    if(rtnVal != 0)
    {
        fprintf(stderr, "getaddrinfo() failed %s\n",  gai_strerror(rtnVal));
        exit(EXIT_FAILURE);
    }

    int sock = -1;

    for (struct addrinfo *addr = servAddr; addr != NULL; addr=addr->ai_next)
    {
        sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(sock < 0)    continue;

        if(connect(sock, addr->ai_addr, addr->ai_addrlen) == 0) break;
        
        close(sock);
        sock = -1;

    }

    freeaddrinfo(servAddr);
    return sock;
}

int 
main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, "Parameter(s) <Server Address/Name> <Echo Word> [<Server Port/ Service>]\n");
        return 0;
    }

    char *server = argv[1];
    char *echoString = argv[2];
    char *service = (argc == 4) ? argv[3] : "echo";

    // Create a connect TCP Socket
    int sock = SetupTCPClientSocket(server, service);

    if(sock < 0)
    {
        fprintf(stderr, "SetupTCPClientSocket() failed, unable to connect\n");
        exit(EXIT_FAILURE);
    }

    size_t echoStringLen = strlen(echoString);

    // Send the string to the server
    ssize_t numBytes = send(sock, echoString, echoStringLen, 0);
    if(numBytes < 0)
    {
        fprintf(stderr, "send() fail\n");
        exit(EXIT_FAILURE);
    }else if(numBytes != echoStringLen){
        fprintf(stderr, "send() send unexpected number of bytes\n");
        exit(EXIT_FAILURE);
    }

    // Receive the same sting back from the server
    unsigned int totalByteRcvd = 0;
    fputs("Received : ", stdout);
    while(totalByteRcvd < echoStringLen)
    {
        char buffer[1024];
        // Receive up to the buffer size
        numBytes = recv(sock, buffer, 1024, 0);

        if (numBytes < 0) 
        {
            fprintf(stderr, "recv() failed\n");
            exit(EXIT_FAILURE);
        }else if(numBytes == 0)
        {
            fprintf(stderr, "recv() : connection closed prematurely\n");
            exit(EXIT_FAILURE);
        }
        totalByteRcvd += numBytes;
        buffer[numBytes] = '\0';
        fputs(buffer,stdout);
    }
    fputc('\n', stdout);
}

