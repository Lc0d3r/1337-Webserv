#include "sockets.hpp"


SimpleSocket::SimpleSocket(int domain, int type, int protocol, int port, u_long interface) {

    //here I create a socket
    this->socket_fd = socket(domain, type, protocol);

    // Define the address struct 
    sockaddr_in serverAddress;
    serverAddress.sin_family = domain;
    serverAddress.sin_port = htons(port);
    // all IP addresses 
    serverAddress.sin_addr.s_addr = interface;

    // only 127.0.0.1
    // if(inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr)<=0)
    // {
    //     printf("\nInvalid address/ Address not supported \n");
    //     return -1;
    // }

    bind(socket_fd,(struct sockaddr *)&serverAddress, sizeof(serverAddress));

}

sockaddr_in& SimpleSocket::get_serverAddress() {
    return serverAddress;
}

int SimpleSocket::get_socket_fd() {
    return socket_fd;
}