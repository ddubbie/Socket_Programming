# Naming Service
- Mapping IP address to URL
- *Name servie* in socket implmentation
- Domain Namne System(DNS) for mapping IP address to URL
- *Name service* is hidden behind API.

## Accessing the Name Service

### API
```C
int getaddrinfo (const char *hostStr, const char *serviceStr,
                    const struct addrinfo *hints, struct addrinfo **results)
```

It returns all the viable combinations for a given hostname, service pair

Arguments
- `hostStr` : host name(URL) or address(IP)
- `serviceStr` : service name(URL) or address(IP)
- `hint` : type requred to return(IPv4, IPv6 )
- `result` : 

Return
- 0 : success
- non zero : fail
  

```C
void freeaddrinfo(struct addrinfo *addrList)
```
Dellocation of `addrList`

Arguments 
- `addrList` : a pointer, which has the name information

```
const char *gai_strerror(int errorCode)
```

### Structures for metadata
```C
struct addrinfo {
    int ai_flags; // Flags to control info resolution
    int ai_family; // Family: AF_INET, AF_INET6, AF_UNSPEC
    int ai_socktype; // Socket type: SOCK_STREAM, SOCK_DGRAM
    int ai_protocol; // Protocol: 0 (default) or IPPROTO_XXX
    socklen_t ai_addrlen; // Length of socket address ai_addr
    struct sockaddr *ai_addr; // Socket address for socket
    char *ai_canonname; // Canonical name
    struct addrinfo *ai_next; // Next addrinfo in linked list
};
```
