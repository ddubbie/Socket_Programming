TARGET = GetAddrInfo TCPEchoClient TCPEchoServer
CFLAGS = -Wall -g # -O3

all : $(TARGET)

GetAddrInfo : GetAddrInfo.c
	gcc $(CFLAGS) -o GetAddrInfo GetAddrInfo.c

TCPEchoClient : TCPEchoClient.c
	gcc $(CFLAGS) -o TCPEchoClient TCPEchoClient.c

TCPEchoServer : TCPEchoServer.c
	gcc $(CFLAGS) -o TCPEchoServer TCPEchoServer.c

clean :
	rm $(TARGET) -r

