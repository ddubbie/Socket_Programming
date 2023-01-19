#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int
main(int argc, char *argv[])
{
    if( argc < 3 || argc > 4)
    {
        fprintf(stderr, "Parameters(s), <Server Address>, <echo World? [<Server Port>]\n");
        return 0;
    }

    char *servIP = argv[1];
    char *echoString = argv[2];
    in_port_t servPort = (argc == 4) ? atoi(argv[3]) : 7;  // in_port_T at <netinet/in.h>

    /* Create a reliable, stream socket using TCP */
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        fprintf(stderr, "socket() failed\n");
        exit(EXIT_FAILURE);
    }
    // Construct the servewr address structure
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;

    // Convert address to binary
    int rtnVal = inet_pton(AF_INET, servIP, &servAddr.sin_addr.s_addr);
    if (rtnVal == 0)
    {
        fprintf(stderr, "inet_pton() failed, invalid address string\n");
        return 0;
    }else if (rtnVal < 0){
        fprintf(stderr, "inet_pton() failed!, invalid address string\n");
        return 0;
    }
    servAddr.sin_port = htons(servPort);    // htons : <netinet/in.h>

    // Establish the connection to the echo server
    if(connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        fprintf(stderr, "connect() failed\n");
        return 0;
    }

    size_t echoStringLen = strlen(echoString);

    // Send the string to the server
    ssize_t numBytes = send(sock, echoString, echoStringLen, 0);
    if(numBytes < 0)
    {
        fprintf(stderr, "send() failed");
        return 0;
    }else if(numBytes != echoStringLen)
    {
        fprintf(stderr, "send(), sent unexpected number of bytes\n");
    }
    // Receive the same string back from server
    unsigned int totalBytesRcvd = 0;

    while (totalBytesRcvd < echoStringLen)
    {
        char buffer[8192];
        numBytes = recv(sock, buffer, BUFSIZ-1, 0);
        if(numBytes < 0)
        {
            fprintf(stderr, "recv() failed\n");
        }else if(numBytes == 0)
        {
            fprintf(stderr, "connection closed prematurely\n");
        }
        totalBytesRcvd += numBytes;
        buffer[numBytes] = '\0';
        fputs(buffer, stdout);
    }
    fputc('\n', stdout);

    close(sock);
    return 0;

}
