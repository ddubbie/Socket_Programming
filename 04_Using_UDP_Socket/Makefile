TARGET =  UDPEchoServer UDPEchoClient
CC	   = gcc
CFLAGS = -Wall -O3
HIDE=@
MSG=@echo

all : $(TARGET)

UDPEchoServer : UDPEchoServer.c
	$(MSG) " CC $<"
	$(HIDE) $(CC) $(CFLAGS) $< -o $@	

UDPEchoClient : UDPEchoClient.c
	$(MSG) " CC $<"
	$(HIDE) $(CC) $(CFLAGS) $< -o $@

clean :
	rm $(TARGET) -r
	
