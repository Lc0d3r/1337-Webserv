// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <iostream>

#define PORT 8080

int main(int argc, char const *argv[])
{
    int sock = 0; long valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("====start sending ====\n");
    const char* chunked_request =
    "f /Balanced_Webserv_Split-1.pdf HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Connection: keep-alive\r\n"
    "\r\n"
    "fsdsdfsdfsdf";
    write(sock , chunked_request , strlen(chunked_request));
    printf("====end sending ====\n");
    printf("====start reseving====\n");
    read( sock , buffer, 1024);
    printf("%s\n",buffer );
    printf("====end reseving====\n");
    while (1)
    {}
    close(sock);
    return 0;
}