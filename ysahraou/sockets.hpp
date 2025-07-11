#pragma once

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define PORT 8080


class SimpleSocket {
    private:
        int socket_fd;
        sockaddr_in serverAddress;
    public:
        SimpleSocket(int domain, int type, int protocol, int port, u_long interface);
        int get_socket_fd();
        sockaddr_in &get_serverAddress();
};