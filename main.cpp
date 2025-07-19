#include "ysahraou/sockets.hpp"
#include "ysahraou/HttpRequest.hpp"
#include "abel-baz/Router.hpp"
#include "abel-baz/Config.hpp"
#include "abel-baz/Parser.hpp"
#include "abel-baz/Tokenizer.hpp"

void loop(SimpleSocket &my_socket)
{
    int addrlen = sizeof(my_socket.get_serverAddress());
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
        readHeaders(request_data, new_socket);
        if (parse_req(request_data, new_socket, request))
            continue;
        // read the body
        std::string str_body;
        readBody(request, str_body, new_socket);
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

int main(int argc, char **argv) {


    // config part
    if (argc != 2) {
        std::cerr << "Usage: ./webserv <config_file>" << std::endl;
        return 1;
    }

    try {
        Tokenizer tokenizer(argv[1]);
        Parser parser(tokenizer.tokenize());
        Config config = parser.parse();

        RoutingResult result = routingResult(config, "localhost", 8080, "/docs/index.html", "DELETE");

        std::cout << "Server count: " << result.server_count << std::endl;
        if (result.is_redirect)
            std::cout << "Redirect to: " << result.redirect_url << std::endl;
        else if (result.use_autoindex)
            std::cout << "Autoindex enabled for: " << result.file_path << std::endl;
        else
            std::cout << "Serve file: " << result.file_path << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } 
    //////////////
    SimpleSocket my_socket(AF_INET, SOCK_STREAM, 0, PORT, INADDR_ANY);
    
    if (listen(my_socket.get_socket_fd(), 5) < 0) 
    { 
        perror("In listen"); 
        exit(EXIT_FAILURE); 
    }

    //loop
    loop(my_socket);
}