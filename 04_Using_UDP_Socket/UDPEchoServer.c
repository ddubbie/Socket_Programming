#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define MAXSTRINGLENGTH 4096

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
   if(argc !=2) // Test for orrect number of arguments
   {
      fprintf(stderr, "Parameters() <server Port/Service>");
      return 0;
   }

   char *service = argv[1]; 

   //Construct the server address structure
   struct addrinfo addCriteria;
   memset(&addCriteria, 0, sizeof(addCriteria));
   addCriteria.ai_family = AF_UNSPEC;
   addCriteria.ai_flags = AI_PASSIVE;
   addCriteria.ai_socktype = SOCK_DGRAM;
   addCriteria.ai_protocol = IPPROTO_UDP;

   struct addrinfo *servAddr;
   int rtnVal = getaddrinfo(NULL, service, &addCriteria, &servAddr);

   if(rtnVal != 0)
   {
      fprintf(stderr, "getaddrinfo() failed %s\n", gai_strerror(rtnVal));
      exit(EXIT_FAILURE);
   }
   int sock = socket(servAddr->ai_family, servAddr->ai_socktype,
                        servAddr->ai_protocol);
   if(sock < 0)
   {
      fprintf(stderr, "socket() failed\n");
      exit(EXIT_FAILURE);
   }
   // Bind to the local address
   if(bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
   {
      fprintf(stderr, "bind() failed\n");
      exit(EXIT_FAILURE);
   }

   // Free address list allocated by getaddrinfo()
   freeaddrinfo(servAddr);

   for(;;)  // Run forever
   {
      struct sockaddr_storage clntAddr;   // Client address
      // Set length of clinet address structure(in-out parameter)
      socklen_t clntAddrLen = sizeof(clntAddr);

      char buffer[1024];

      // Size of received message
      ssize_t numByteRcvd = recvfrom(sock ,buffer, 1024, 0,
                  (struct sockaddr *)&clntAddr, &clntAddrLen);

      if (numByteRcvd < 0)
      {
         fprintf(stderr, "recefrom() failed\n");
         exit(EXIT_FAILURE);
      }
      fputs("Handling client ", stdout);
      PrintSocketAddress((struct sockaddr *)&clntAddr, stdout);
      fputc('\n', stdout);

      // Send received datagram back to the client
      ssize_t numByteSent = sendto(sock, buffer, numByteRcvd, 0,
            (struct sockaddr *)&clntAddr, sizeof(clntAddr));

      if (numByteSent < 0)
      {
         fprintf(stderr, "sendto() failed\n");
         exit(EXIT_FAILURE);
      }else if(numByteSent != numByteRcvd)
      {
         fprintf(stderr, "sendto() : sent unexpected number of bytes");
         exit(EXIT_FAILURE);
      }
   }
}