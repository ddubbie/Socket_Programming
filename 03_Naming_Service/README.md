# GetAddrInfo
Get service IP address and port number

## Execution
```sh
./GetAddrInfo 169.1.1.100 time
```

## Result
```sh
169.1.1.100-37
```



# Generic Server-Client with GetAddrInfo

## TCPEchoClient

### SetupTCPClientSocket()

- Get a target server with URL and service name
- `getaddrinfo` returns linked list of *IP - Port* sets



## TCPEchoServer

### SetupTCPServerSocket

- Get a list of proper `IP-PORT` sets implemented as linked list
- When we enters first argument of `getaddrinfo` as `NULL`, the function find information in local.



## Execution

```sh
./TCPEchoServer 8888							# on server side
```

```sh
./TCPEchoClient localhost echo_this_word 8888	# on client side
```



## Result

```sh
# On server side
Binding to 0.0.0.0-8888
Handling clinet 127.0.0.1-33394 
Finish handling this client! 
```

```sh
# On client side
Received : echo_this_word
```

