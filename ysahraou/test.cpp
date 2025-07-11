#include "sockets.hpp"

int main() {

    SimpleSocket my_socket(AF_INET, SOCK_STREAM, 0, PORT, INADDR_ANY);


    int addrlen = sizeof(my_socket.get_serverAddress());


    if (listen(my_socket.get_socket_fd(), 5) < 0) 
    { 
        perror("In listen"); 
        exit(EXIT_FAILURE); 
    }

    int new_socket;
    while (1)
    {
        if ((new_socket = accept(my_socket.get_socket_fd(), (struct sockaddr *)&my_socket.get_serverAddress(), (socklen_t*)&addrlen))<0)
        {
            perror("In accept");            
            exit(EXIT_FAILURE);        
        }
        std::cout << my_socket.get_serverAddress().sin_addr.s_addr << std::endl ;
        std::cout << my_socket.get_socket_fd() << std::endl ;


        char buffer[1024] = {0};
        int valread = read(new_socket , buffer, 1024); 
        printf("%s\n",buffer );
        if(valread < 0)
        { 
            printf("No bytes are there to read");
        }

        char *hello = "Hello from the server";//IMPORTANT! WE WILL GET TO IT
        write(new_socket , hello , strlen(hello));
    }
}