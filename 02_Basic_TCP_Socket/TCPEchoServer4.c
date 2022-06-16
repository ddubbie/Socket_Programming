#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static const int MAXPENDING = 5;

inline static void HandleTCPClinet(int clntSocket)
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



int
main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Parameter(s) <Server Port>\n");
        return 0;
    }
    in_port_t servPort = atoi(argv[1]);

    // Create socket for incoming connections
    int servSock;
    if((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        fprintf(stderr, "sock() failed\n");
        exit(EXIT_FAILURE);
    }

    // Construct local addresss structure
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(servPort);

    // Bind to the local address
    if(bind(servSock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
    {
        fprintf(stderr, "bind() failed\n");
        exit(EXIT_FAILURE);
    }

    if (listen(servSock, MAXPENDING) < 0)
    {
        fprintf(stderr, "listen() failed\n");
        exit(EXIT_FAILURE);
    }

    for(;;)
    {
        struct sockaddr_in clntAddr;    // Client Address
        // Set length of clinet address structure (in-out parameter)
        socklen_t clntAddrLen = sizeof(clntAddr);

        // Wait for a client to connect
        int clntSock = accept(servSock, (struct sockaddr*)&clntAddr, &clntAddrLen);
        if (clntSock < 0)
        {
            fprintf(stderr, "accept() failed\n");
            exit(EXIT_FAILURE);
        }

        char clntName[256];

        if(inet_ntop(AF_INET, &clntAddr.sin_addr, clntName, // ip_addr ---> string
            sizeof(clntName)) != NULL)
        {
            printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
        }else{
            puts("Unable to get clinet address\n");
        }
        HandleTCPClinet(clntSock);
    }


}
