#include "ysahraou/sockets.hpp"
#include "ysahraou/HttpRequest.hpp"
#include "abel-baz/Router.hpp"
#include "abel-baz/Config.hpp"
#include "abel-baz/Parser.hpp"
#include "abel-baz/Tokenizer.hpp"
#include "ysahraou/HttpResponse.hpp"

void loop(std::map <int, ConnectionInfo> &connections, Config &config)
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
        int ready = poll(pollfds.data(), pollfds.size(), 1000); // 1 second timeout
        if (ready < 0) {
            perror("poll");
            break;
        }
        // for each socket that is ready, check if it is a listener or a connected socket
        for (size_t i = 0; i < pollfds.size(); ++i) {
            HttpRequest request;
            int client_fd;

            // Check for errors or disconnections
            if (pollfds[i].revents & (POLLHUP | POLLERR)) {
                std::cout << "Client disconnected or socket error: " << pollfds[i].fd << std::endl;
                close(pollfds[i].fd);
                connections.erase(pollfds[i].fd);
                pollfds.erase(pollfds.begin() + i);
                --i;
                continue;
            }
            // check if the fd client_fd timeout
            if (connections.count(pollfds[i].fd) &&
                connections[pollfds[i].fd].type == CONNECTED &&
                time(NULL) - connections[pollfds[i].fd].last_active > config.getKeepAliveTimeout("", connections[pollfds[i].fd].portToConnect)) {
                std::cout << "Client socket " << pollfds[i].fd << " timed out." << std::endl;
                close(pollfds[i].fd);
                connections.erase(pollfds[i].fd);
                pollfds.erase(pollfds.begin() + i);
                --i;
                continue;
            }
            // Check for readable sockets
            if (pollfds[i].revents & POLLIN) {
                std::cout << "\nSocket " << pollfds[i].fd << " is ready for reading..." << std::endl;
                if (connections[pollfds[i].fd].type == LISTENER) {
                    // accept a new connection
                    client_fd = accept(pollfds[i].fd, NULL, NULL);
                    if (client_fd < 0) {
                        perror("accept");
                        continue;
                    }
                    std::cout << "New connection accepted on socket: " << client_fd << std::endl;
                    fcntl(client_fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking mode
                    connections[client_fd] = ConnectionInfo(CONNECTED, true);
                    connections[client_fd].last_active = time(NULL);
                    connections[client_fd].portToConnect = connections[pollfds[i].fd].port;
                    connections[client_fd].hostToConnect = connections[pollfds[i].fd].host;
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
                    // read the body
                    std::string str_body;
                    readBody(request, str_body, client_fd);
                    std::cout << request.body << std::endl;
                    std::cout << "====>requestData read successfully" << std::endl;
                    if (parse_req(request_data, client_fd, request))
                        continue;
                    // handle keep-alive connections
                    if (request.is_keep_alive) {
                        std::cout << "Connection is keep-alive setting keep_alive to true" << std::endl;
                        connections[pollfds[i].fd].keep_alive = true;
                        connections[pollfds[i].fd].last_active = time(NULL);
                    } else {
                        connections[pollfds[i].fd].keep_alive = false;
                    }
                    
                    // response
                    std::cout << "====>request parsed successfully" << std::endl;
                    response(pollfds[i].fd, request, config);
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

        // init servers
        std::map<int, ConnectionInfo> connections;
        std::vector<int> listening_sockets = initListeningSockets(config, connections);
        if (listening_sockets.empty()) {
            std::cerr << "No listening sockets initialized." << std::endl;
            return 1;
        }
        //loop
        loop(connections, config);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}