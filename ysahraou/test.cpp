#include "fistpart.hpp"

int main() {

    //here I create a socket
    int server_fd;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("cannot create socket"); 
        return 0; 
    }


    // Define the address struct 
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    // all IP addresses 
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // only 127.0.0.1
    // if(inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr)<=0)
    // {
    //     printf("\nInvalid address/ Address not supported \n");
    //     return -1;
    // }
    int addrlen = sizeof(serverAddress);


    if (bind(server_fd,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) 
    { 
        perror("bind failed"); 
        return 0; 
    }


    if (listen(server_fd, 5) < 0) 
    { 
        perror("In listen"); 
        exit(EXIT_FAILURE); 
    }

    int new_socket;
    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&serverAddress, (socklen_t*)&addrlen))<0)
        {
            perror("In accept");            
            exit(EXIT_FAILURE);        
        }
        std::cout << serverAddress.sin_addr.s_addr << std::endl ;
        std::cout << server_fd << std::endl ;


        char buffer[1024] = {0};int valread = read(new_socket , buffer, 1024); 
        printf("%s\n",buffer );
        if(valread < 0)
        { 
            printf("No bytes are there to read");
        }

        char *hello = "Hello from the server";//IMPORTANT! WE WILL GET TO IT
        write(new_socket , hello , strlen(hello));
    }
}