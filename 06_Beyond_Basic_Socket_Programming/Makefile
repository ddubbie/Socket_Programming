TARGET = SigAction  UDPEchoServer_SIGIO UDPEchoClient \
			 UDPEchoClient_Timeout TCPEchoServer_Fork \
			 TCPEchoServer_Thread TCPEchoClient \
			 TCPEchoServer_ForkN TCPEchoServer_Select \
			 BroadcastSender
CC = gcc
CFLAGS = -Wall -O3
MSG = @echo
HIDE = @
LDFLAGS = -lpthread

all : $(TARGET)

SigAction : SigAction.c
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<

UDPEchoServer_SIGIO : UDPEchoServer_SIGIO.c 
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<

UDPEchoClient : UDPEchoClient.c 
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<

 UDPEchoClient_Timeout :  UDPEchoClient_Timeout.c 
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<

 TCPEchoServer_Fork :  TCPEchoServer_Fork.c 
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<

TCPEchoServer_Thread :  TCPEchoServer_Thread.c 
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

TCPEchoServer_Select : TCPEchoServer_Select.c
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<

 TCPEchoClient :  TCPEchoClient.c
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<

TCPEchoServer_ForkN : TCPEchoServer_ForkN.c
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<

TCPEchoServer_Select : TCPEchoServer_Select.c
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<

BroadcastSender : BroadcastSender.c 
	$(MSG) "CC $<"
	$(HIDE) $(CC) $(CFLAGS) -o $@ $<
clean :
	rm -f $(TARGET)