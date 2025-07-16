#include "sockets.hpp"
#include "HttpRequest.hpp"

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

        std::string request_data;
        char buffer[20] = {0};

        while (request_data.find("\r\n\r\n") == std::string::npos) {
            int bytes = read(new_socket, buffer, sizeof(buffer));
            if (bytes <= 0) {
                // client disconnected or error
                break;
            }
            request_data.append(buffer, bytes);
        }
        parse_req(request_data);


        printf("sending...=== ===== == \n");
        const char* response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 30\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<h1>Hello from the server</h1>";
        write(new_socket , response , strlen(response));
    }
}