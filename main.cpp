#include "ysahraou/sockets.hpp"
#include "ysahraou/HttpRequest.hpp"
#include "abel-baz/Router.hpp"
#include "abel-baz/Config.hpp"
#include "abel-baz/Parser.hpp"
#include "abel-baz/Tokenizer.hpp"
#include "ysahraou/HttpResponse.hpp"

void response(int client_fd)
{
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
        write(client_fd , response.toString().c_str() , strlen(response.toString().c_str()));
}


void loop(std::map <int, ConnectionInfo> &connections)
{
    
    // create a pollfd victor to monitor the listening sockets
    std::vector<struct pollfd> pollfds;
    std::map<int, ConnectionInfo>::iterator it;
    for (it = connections.begin(); it != connections.end(); ++it) {
        struct pollfd pfd;
        pfd.fd = it->first; // listening socket
        pfd.events = POLLIN; // events to monitor
        pollfds.push_back(pfd);
    }
    // loop to accept connections
    while (1)
    {
        HttpRequest request;
        int ready = poll(pollfds.data(), pollfds.size(), -1);
        if (ready < 0) {
            perror("poll");
            break;
        }
        for (size_t i = 0; i < pollfds.size(); ++i) {
            int client_fd;
            if (pollfds[i].revents & POLLIN) {
                if (connections[pollfds[i].fd].type == LISTENER) {
                        // accept a new connection
                    client_fd = accept(pollfds[i].fd, NULL, NULL);
                    if (client_fd < 0) {
                        perror("accept");
                        continue;
                    }
                    std::cout << "New connection accepted on socket: " << client_fd << std::endl;
                    fcntl(client_fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking mode
                    connections[client_fd] = ConnectionInfo(CONNECTED);
                    struct pollfd pfd;
                    pfd.fd = client_fd; // connected socket
                    pfd.events = POLLIN; // events to monitor
                    pollfds.push_back(pfd);
                }
                else if (connections[pollfds[i].fd].type == CONNECTED) {
                    client_fd = pollfds[i].fd;
                    std::cout << "Data available on socket: " << client_fd << std::endl;
                    // read data from the client
                    // read the headers
                    std::string request_data;
                    readHeaders(request_data, client_fd);
                    if (parse_req(request_data, client_fd, request))
                        continue;
                    // read the body
                    std::string str_body;
                    readBody(request, str_body, client_fd);
                    std::cout << request.body << std::endl;
                    
                    // response
                    response(pollfds[i].fd);
                    // close the connection
                    close(pollfds[i].fd);
                    std::cout << "Connection closed on socket: " << pollfds[i].fd << std::endl;
                    // remove the socket from the pollfds vector
                    pollfds.erase(pollfds.begin() + i);
                    // remove the connection from the connections map
                    connections.erase(pollfds[i].fd);
                    std::cout << "Connection removed from map" << std::endl;

                } else {
                    std::cerr << "Unknown connection type" << std::endl;
                    continue;
                }
            }
        }
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
        std::map<int, ConnectionInfo> connections;
        for (int i = 0; i < (int)listening_sockets.size(); ++i) {
            connections.insert(std::make_pair(listening_sockets[i], ConnectionInfo(LISTENER)));
        }
        //loop
        loop(connections);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } 
}