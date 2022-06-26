#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

static const unsigned int TIMEOUT_SECS = 2; // Seconds between retransmits
static const unsigned int MAXTRIES = 5;     // Tries before giving up
unsigned int tries = 0;

void CatchAlarm(int ingnores);

int
main(int argc, char *argv[])
{
    if((argc < 3 || argc > 4))
    {
        fprintf(stderr, "Parameter(s), <server Address/Name> <Echo word> [<Server Port/Service>]\n");
        return 0;
    }
    char *server = argv[1];
    char *echoString = argv[2];

    size_t echoStringLen = strlen(echoString);
    if(echoStringLen > 8192)
    {
        fprintf(stderr, "echo string is too long\n");
        exit(EXIT_FAILURE);
    }

    char *service = (argc == 4) ? argv[3] : "echo";

    // Tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_DGRAM;
    addrCriteria.ai_protocol = IPPROTO_UDP;

    // Get address(es)
    struct addrinfo *servAddr;
    int rtnVal = getaddrinfo(server, service, &addrCriteria, &servAddr);
    if(rtnVal != 0)
    {
        fprintf(stderr, "getaddrinfo() failed %s", gai_strerror(rtnVal));
        exit(EXIT_FAILURE);
    }

    // Create a reliable, stream socket using TCP
    int sock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);

    if(sock < 0)
    {
        fprintf(stderr, "socket() failed\n");
        exit(EXIT_FAILURE);
    }

    // Set signal handler for alarm signal
    struct sigaction handler;   // signal handler
    handler.sa_handler = CatchAlarm;
    if(sigfillset(&handler.sa_mask) < 0)    // Block everything in handler
    {
        fprintf(stderr, "sigfillset() failed\n");
        exit(EXIT_FAILURE);
    }
    handler.sa_flags = 0;

    if(sigaction(SIGALRM, &handler, 0) < 0)
    {
        fprintf(stderr, "sigaction() failed for SIGALARM");
        exit(EXIT_FAILURE);
    }

    // Send the string the server
    ssize_t numBytes = sendto(sock ,echoString, echoStringLen, 0,
                                servAddr->ai_addr, servAddr->ai_addrlen);
    if(numBytes < 0)
    {
        fprintf(stderr, "sendto() failed]\n");
        exit(EXIT_FAILURE);
    }
    else if(numBytes != echoStringLen)
    {
        fprintf(stderr, "sendto() error, sent unexpected numbers of bytes\n");
        exit(EXIT_FAILURE);
    }

    // Receive response
    struct sockaddr_storage fromAddr;
    // Set length of from address structure (in-out parameter)
    socklen_t fromAddrLen = sizeof(fromAddr);
    alarm(TIMEOUT_SECS);
    char buffer[8193];

    while ((numBytes = recvfrom(sock, buffer, 8192, 0,
                (struct sockaddr *)&fromAddr, &fromAddrLen)) < 0)
    {
        if(errno == EINTR)  // alarma wne off
        {
            if(tries < MAXTRIES)
            {
                numBytes = sendto(sock, echoString, echoStringLen, 0,
                                (struct sockaddr*)servAddr->ai_addr, servAddr->ai_addrlen);
                if(numBytes < 0)
                {
                    fprintf(stderr, "sendto() failed\n");
                    exit(EXIT_FAILURE);
                }else if(numBytes != echoStringLen)
                {
                    fprintf(stderr, "send unexpected number of bytes\n");
                }

            }else
            {
                fprintf(stderr, "No response, unable to communicate with server\n");
                exit(EXIT_FAILURE);
            }
        }else
        {
            fprintf(stderr, "recvfrom() failed\n");
            exit(EXIT_FAILURE);
        }
    }
    alarm(0);

    buffer[echoStringLen] = '\0';
    printf("Received : %s\n", buffer);

    close(sock);
    return 0;
}

void
 CatchAlarm(int ingnores)
{
    tries += 1;
}