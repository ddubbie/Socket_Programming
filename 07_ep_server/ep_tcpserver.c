#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define LISTEN_BACKLOG 15
#define MAX_EVENTS 1024

int
main(int argc, char *argv[])
{
    int error_check;
    int server_fd;
    int flags;
    int option;
    int epoll_fd;
    struct epoll_event epoll_events[MAX_EVENTS];
    int event_count;
    int timeout = -1;
    int client_fd;
    int client_len;
    struct sockaddr_in client_addr;
    char data[4096];
    int strlen;

    data[4095] = '\0';

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr, "socket() error\n");
        exit(EXIT_FAILURE);
    }
    // Set ferver fd as non blocking socket to use edge trigger
    flags = fcntl(server_fd, F_GETFL);
    flags |= O_NONBLOCK;

    if (fcntl(server_fd, F_SETFL, flags) < 0)
    {
        fprintf(stderr, "server_fd fcntl() error\n");
        exit(EXIT_FAILURE);
    }

    // set socket option
    option = true;
    if ((error_check = setsockopt(server_fd, SOL_SOCKET, 
                                    SO_REUSEADDR, &option, sizeof(option))) < 0)
    {
        fprintf(stderr, "setsockopt() error\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in mSockAddr;
    memset(&mSockAddr, 0 ,sizeof*(mSockAddr));
    mSockAddr.sin_family = AF_INET;
    mSockAddr.sin_port = htons(44444);
    mSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind socket
    if (error_check = bind(server_fd, (struct sockaddr*)&mSockAddr, sizeof(mSockAddr)) < 0)
    {
        fprintf(stderr, "bind() error\n");
        exit(EXIT_FAILURE);
    }
    // listen
    if (listen(server_fd, LISTEN_BACKLOG) < 0)
    {
        fprintf(stderr, "listen() error\n");
        close(server_fd);
    }

    // Create epoll fd
    if((epoll_fd = epoll_create(1024)) < 0)
    {
        fprintf(stderr, "epoll_create() error\n");
        exit(EXIT_FAILURE);
    }

    // Register server fd on epoll
    // Edge trigger : if state changes from 0 to 1, an event occur.
    struct epoll_event  events;
    events.events = (EPOLLIN | EPOLLET);
    events.data.fd = server_fd;

    // Set detected events server fd to epoll
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &events) < 0)
    {
        fprintf(stderr, "epoll_ctl() error\n");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        event_count = epoll_wait(epoll_fd, epoll_events, MAX_EVENTS, timeout);
        printf("event count[%d]\n", event_count);

        if (event_count < 0)
        {
            fprintf(stderr, "epoll_wait() error [%d]\n", event_count);
            exit(EXIT_FAILURE);
        }
        for (int i=0; i < event_count; i++)
        {
            if ( epoll_events[i].data.fd == server_fd)
            {
                printf("User Accept\n");
                client_len = sizeof(client_addr);
                if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr,
                                            (socklen_t*)&client_len)) < 0)
                {
                    fprintf(stderr, "accept() failed\n");
                    close(server_fd);
                    close(epoll_fd);
                    exit(EXIT_FAILURE);
                }
                flags = fcntl(client_fd, F_GETFL);
                flags |= O_NONBLOCK;

                if (fcntl(client_fd, F_SETFL, flags) < 0)
                {
                    fprintf(stderr, "fcntl() error\n");
                    close(server_fd);
                    close(epoll_fd);
                    exit(EXIT_FAILURE);
                }

                events.data.fd = client_fd;
                events.events = EPOLLIN | EPOLLET;

                if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &events) < 0)
                {
                    fprintf(stderr, "epoll_ctl() error\n");
                    close(client_fd);
                    continue;
                }

            }
            else
            {
                client_fd = epoll_events[i].data.fd;
                while((strlen = read(client_fd, data, sizeof(data) - 1)) > 0)
                {
                    printf("Recv data : %s\n", data);
                }
                if (strlen < 0)
                {
                    fprintf(stderr, "client socket error\n");
                    close(client_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("Connection terminates\n");
                    close(client_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                }
            }

        }
    }
}   
