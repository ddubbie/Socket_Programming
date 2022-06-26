#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
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





int
main(int argc, char *argv[])
{
    if(argc != 3)
    {
        fprintf(stderr, "parameter(s) <address/Name> <Port/Service>");
    }
    char *addrString = argv[1];     // Server address/nam
    char *portString = argv[2];     // Server port/service
    
    // Tell the system what kind(s) of address info we want
    struct addrinfo addrCriteria;
    memset(&addrCriteria, 0, sizeof(addrCriteria));
    addrCriteria.ai_family = AF_UNSPEC;
    addrCriteria.ai_socktype = SOCK_STREAM;
    addrCriteria.ai_protocol = IPPROTO_TCP;

    // Get address(es) asociated with the specified name/service
    struct addrinfo *addrList;
    // modify servAddr contents to reference linked list of address
    int rtnVal = getaddrinfo(addrString, portString, &addrCriteria, &addrList);

    if (rtnVal != 0)
    {
        fprintf(stderr, "getaddrinfo() failed, %s\n", gai_strerror(rtnVal));
        exit(EXIT_FAILURE);
    }

    for (struct addrinfo *addr = addrList; addr != NULL; addr = addr->ai_next) {
        PrintSocketAddress(addr->ai_addr, stdout);
        fputc('\n', stdout);
    }

    freeaddrinfo(addrList);
}