#include "ysahraou/sockets.hpp"
#include "ysahraou/HttpRequest.hpp"
#include "abel-baz/Router.hpp"
#include "abel-baz/Config.hpp"
#include "abel-baz/Parser.hpp"
#include "abel-baz/Tokenizer.hpp"
#include "ysahraou/HttpResponse.hpp"
#include <signal.h>
#include "ysahraou/utils.hpp"
#include "ysahraou/utils.hpp"

std::map<std::string, std::string> cookies_map;

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
        int ready = poll(pollfds.data(), pollfds.size(), 50); // 50 ms timeout
        if (ready < 0) {
            perror("poll");
            break;
        }
        // for each socket that is ready, check if it is a listener or a connected socket
        for (size_t i = 0; i < pollfds.size(); ++i) {
            int client_fd;

            // Check for errors or disconnections
            if (pollfds[i].revents & (POLLHUP | POLLERR)) {
                print_log("Client disconnected or socket error", DiSPLAY_LOG);
                close(pollfds[i].fd);
                connections.erase(pollfds[i].fd);
                pollfds.erase(pollfds.begin() + i);
                --i;
                continue;
            }
            // check if the fd client_fd timeout
            if (connections.count(pollfds[i].fd) &&
                connections[pollfds[i].fd].type == CONNECTED &&
                time(NULL) - connections[pollfds[i].fd].last_active > config.getKeepAliveTimeout("", connections[pollfds[i].fd].portToConnect, connections[pollfds[i].fd].hostToConnect)) {
                print_log( "Client timed out, closing connection.", DiSPLAY_LOG);
                close(pollfds[i].fd);
                connections.erase(pollfds[i].fd);
                pollfds.erase(pollfds.begin() + i);
                --i;
                continue;
            }
            // Check for readable sockets
            if (pollfds[i].revents & POLLIN) {
                // std::cout << "\nSocket " << pollfds[i].fd << " is ready for reading..." << std::endl;
                if (connections[pollfds[i].fd].type == LISTENER) {
                    // accept a new connection
                    client_fd = accept(pollfds[i].fd, NULL, NULL);
                    if (client_fd < 0) {
                        perror("accept");
                        continue;
                    }
                    fcntl(client_fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking mode
                    connections[client_fd] = ConnectionInfo(CONNECTED, true);
                    connections[client_fd].last_active = time(NULL);
                    connections[client_fd].portToConnect = connections[pollfds[i].fd].port;
                    connections[client_fd].hostToConnect = connections[pollfds[i].fd].host;
                    connections[client_fd].server_ip = connections[pollfds[i].fd].server_ip;
                    connections[client_fd].server_port = connections[pollfds[i].fd].server_port;
                    struct pollfd pfd;
                    pfd.fd = client_fd; // connected socket
                    pfd.events = POLLIN; // events to monitor
                    pollfds.push_back(pfd);

                    // requst object to hold the request data
                    connections[client_fd].request = HttpRequest();
                    print_log( "Accepted new connection comming to server: " + connections[client_fd].server_ip + ":" + connections[client_fd].server_port , DiSPLAY_LOG);
                }
                else if (connections[pollfds[i].fd].type == CONNECTED) {
                    client_fd = pollfds[i].fd;
                    connections[client_fd].request.error_pages = config.getErrorPages("", connections[client_fd].portToConnect, connections[client_fd].hostToConnect);
                    print_log( "Reading request comming to server: " + connections[client_fd].server_ip + ":" + connections[client_fd].server_port , DiSPLAY_LOG);

                    // read data from the client
                    // read the headers
                    if (!connections[pollfds[i].fd].request.in_progress && !connections[pollfds[i].fd].request.done) {
                        std::string request_data;
                        readHeaders(request_data, client_fd);
                        if (parse_req(request_data, client_fd, connections[pollfds[i].fd].request) )
                        {
                            close(client_fd);
                            print_log( "Error parsing request, closing connection." , DiSPLAY_LOG);
                            connections.erase(client_fd);
                            pollfds.erase(pollfds.begin() + i);
                            --i;
                            continue;
                        }
                    }
                    std::string str = decodePath(connections[pollfds[i].fd].request.path);
                    if (str.empty()) {
                        print_log( "Invalid path in request: " + connections[pollfds[i].fd].request.path , DiSPLAY_LOG);
                        close(client_fd);
                        connections.erase(client_fd);
                        pollfds.erase(pollfds.begin() + i);
                        --i;
                        continue;
                    }
                    connections[pollfds[i].fd].request.path = str;
                    removeQueryString(connections[pollfds[i].fd].request);
                    print_log( "Request with method: " + connections[pollfds[i].fd].request.method + " and path: " + connections[pollfds[i].fd].request.path_without_query + " received." , DiSPLAY_LOG);
                    // read the body
                    std::string str_body;
                    if (!readBody(connections[pollfds[i].fd].request, str_body, client_fd))
                    {
                        print_log( "No body to read or no conent length specified or no transfer encoding specified." , DiSPLAY_LOG);
                        HttpResponse response;
                        if (!get_error_page(response, 400, connections[pollfds[i].fd].request, "Bad Request")) {
                            response.statusCode = 400; // Bad Request
                            response.httpVersion = "HTTP/1.1";
                            response.statusMessage = "Bad Request";
                            response.addHeader("Content-Type", "text/html");
                            response.addHeader("Connection", "close");
                            response.addHeader("Content-Length", "57");
                            response.setTextBody("<html><body><h1>400 Bad Request</h1></body></html>");
                        }
                        print_log( "Sending 400 Bad Request response." , DiSPLAY_LOG);
                        write(client_fd, response.toString().c_str(), response.toString().size());
                        write (client_fd, response.body.data(), response.body.size());
                        close(pollfds[i].fd);
                        connections.erase(pollfds[i].fd);
                        pollfds.erase(pollfds.begin() + i);
                        --i;
                        continue; // no body to read or body size exceeds limit
                    }
                    // print headers 
                    if (!connections[pollfds[i].fd].request.body.empty() && connections[pollfds[i].fd].request.body.size() >= config.getMaxBodySize("", connections[pollfds[i].fd].portToConnect, connections[pollfds[i].fd].hostToConnect)) {
                        HttpResponse response;
                        if (!get_error_page(response, 413, connections[pollfds[i].fd].request, "Payload Too Large")) {
                            response.statusCode = 413; // Payload Too Large
                            response.httpVersion = "HTTP/1.1";
                            response.statusMessage = "Payload Too Large";
                            response.addHeader("Content-Type", "text/html");
                            response.addHeader("Connection", "close");
                            response.addHeader("Content-Length", "57");
                            response.setTextBody("<html><body><h1>413 Payload Too Large</h1></body></html>");
                        }
                        print_log( "Request body size exceeds limit, sending 413 response." , DiSPLAY_LOG);
                        write(client_fd, response.toString().c_str(), response.toString().size());
                        write (client_fd, response.body.data(), response.body.size());
                        continue; // no body to read or body size exceeds limit
                    }
                    if (connections[pollfds[i].fd].request.is_keep_alive) {
                        connections[pollfds[i].fd].keep_alive = true;
                        connections[pollfds[i].fd].last_active = time(NULL);
                    } else {
                        connections[pollfds[i].fd].keep_alive = false;
                    }
                    if (connections[pollfds[i].fd].request.in_progress) {
                        print_log( "Request is in progress, waiting for more data..." , DiSPLAY_LOG);
                        continue;
                    }
                    print_log( "Request is complete, preparing response..." , DiSPLAY_LOG);
                    if (!response(pollfds[i].fd, connections[pollfds[i].fd].request, config, connections[pollfds[i].fd]))
                    {
                        print_log( "Error in response, closing connection." , DiSPLAY_LOG);
                        close(client_fd);
                        connections.erase(client_fd);
                        pollfds.erase(pollfds.begin() + i);
                        --i;
                        continue;
                    }
                    else 
                        connections[pollfds[i].fd].request = HttpRequest(); // reset request
                }
            }
            else if (connections[pollfds[i].fd].type == CONNECTED && connections[pollfds[i].fd].is_old) {
                print_log("Resuming sending file from server : " + connections[pollfds[i].fd].server_ip + ":" + connections[pollfds[i].fd].server_port, DiSPLAY_LOG);
                // buffer to hold the data to be sent
                std::vector<char> buffer(CHUNK_SIZE);
                if (resumeSending(connections[pollfds[i].fd], buffer, pollfds[i].fd))
                    connections[pollfds[i].fd].last_active = time(NULL);
                if (!connections[pollfds[i].fd].is_old) {
                    print_log( "File sending completed from server : "  + connections[pollfds[i].fd].server_ip + ":" + connections[pollfds[i].fd].server_port, DiSPLAY_LOG);
                    close(pollfds[i].fd);
                    connections.erase(pollfds[i].fd);
                    pollfds.erase(pollfds.begin() + i);
                    --i;
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
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
            print_log("No listening sockets initialized. Exiting.", DiSPLAY_LOG);
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