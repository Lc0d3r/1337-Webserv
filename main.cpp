#include "ysahraou/sockets.hpp"
#include "ysahraou/HttpRequest.hpp"

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
        HttpRequest request;
        if ((new_socket = accept(my_socket.get_socket_fd(), (struct sockaddr *)&my_socket.get_serverAddress(), (socklen_t*)&addrlen))<0)
        {
            perror("In accept");            
            exit(EXIT_FAILURE);        
        }

        // read the headers
        std::string request_data;
        char buffer[2] = {0};
        while (request_data.find("\r\n\r\n") == std::string::npos) {
            int bytes = read(new_socket, buffer, 1);
            if (bytes <= 0) {
                // client disconnected or error
                break;
            }
            request_data.append(buffer, bytes);
        }
        if (parse_req(request_data, new_socket, request))
            continue;
        // read the body
        int content_length = 0;
        std::string str_body;
        if (request.headers.count("Content-Length"))
        {
            content_length = std::atoi(request.headers["Content-Length"].c_str());
            std::cout << "true content lenght is there -> " << request.headers.at("Content-Length") << "\n";
            char buffer[2] = {0};
            while (str_body.size() < static_cast<size_t>(content_length))
            {
                int bytes = read(new_socket, buffer, 1);
                if (bytes <= 0) {
                    // client disconnected or error
                    break;
                }
                str_body.append(buffer, bytes);
            }
            request.body = str_body;
        }
        else 
        {
            std::cout << "false content lenght is not there\n";
            char buffer[2] = {0};
            while (str_body.find("\r\n\r\n") == std::string::npos)
            {
                int bytes = read(new_socket, buffer, 1);
                if (bytes <= 0) {
                    // client disconnected or error
                    break;
                }
                str_body.append(buffer, bytes);
            }
            request.body = str_body;
        }
        std::cout << request.body << std::endl;

        // respond
        printf("sending... === ===== === \n");
        const char* response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 883\r\n"
            "Connection: close\r\n"
            "\r\n\r\n"
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "  <title>Post Test</title>\n"
            "  <style>\n"
            "    body { font-family: Arial; background: #f0f2f5; display: flex; justify-content: center; align-items: center; height: 100vh; }\n"
            "    .container { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }\n"
            "    textarea { width: 100%; padding: 8px; border-radius: 4px; border: 1px solid #ccc; resize: none; }\n"
            "    button { margin-top: 10px; padding: 10px 20px; background: #007BFF; color: white; border: none; border-radius: 4px; cursor: pointer; }\n"
            "    button:hover { background: #0056b3; }\n"
            "  </style>\n"
            "</head>\n"
            "<body>\n"
            "  <div class=\"container\">\n"
            "    <h2>Send POST Request</h2>\n"
            "    <form action=\"/\" method=\"POST\">\n"
            "      <textarea name=\"message\" rows=\"5\" placeholder=\"Type your message here...\"></textarea><br>\n"
            "      <button type=\"submit\">Send</button>\n"
            "    </form>\n"
            "  </div>\n"
            "</body>\n"
            "</html>";
        std::cout << "strlen(response) = " << strlen(response) << std::endl;
        write(new_socket , response , strlen(response));
    }
}