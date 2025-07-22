#include "ysahraou/sockets.hpp"
#include "ysahraou/HttpRequest.hpp"
#include "abel-baz/Router.hpp"
#include "abel-baz/Config.hpp"
#include "abel-baz/Parser.hpp"
#include "abel-baz/Tokenizer.hpp"
#include "ysahraou/HttpResponse.hpp"

void loop(std::vector<int> &listening_sockets)
{
    int new_socket;

    while (1)
    {
        HttpRequest request;
        if ((new_socket = accept(listening_sockets[0], NULL, NULL)) < 0)
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
        HttpResponse response(200, "OK");
        // read file index.html put it in the body
        std::fstream file("www/index.html");
        std::string body;
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                body += line + "\n";
            }
            file.close();
        } else {
            std::cerr << "Unable to open file" << std::endl;
            response.statusCode = 404;
            response.statusMessage = "Not Found";
            body = "<h1>404 Not Found</h1>";
        }
        response.setBody(body);
        response.addHeader("Content-Type", "text/html");
        response.addHeader("Connection", "close");
        std::cout << "strlen(response) = " << strlen(response.toString().c_str()) << std::endl;
        write(new_socket , response.toString().c_str() , strlen(response.toString().c_str()));
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

        // RoutingResult result = routingResult(config, "localhost", 8080, "/docs/index.html", "DELETE");

        // init servers
        std::vector<int> listening_sockets = initListeningSockets(config);
        if (listening_sockets.empty()) {
            std::cerr << "No listening sockets initialized." << std::endl;
            return 1;
        }
        //loop
        loop(listening_sockets);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } 
}