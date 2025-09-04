#include "sockets.hpp"
#include "utils.hpp"

void log_time() {
    std::time_t now = std::time(0);             
    std::tm *ltm = std::localtime(&now);        

    std::cout << "[" << 1900 + ltm->tm_year << "-"
              << std::setw(2) << std::setfill('0') << 1 + ltm->tm_mon << "-"
              << std::setw(2) << std::setfill('0') << ltm->tm_mday << " "
              << std::setw(2) << std::setfill('0') << ltm->tm_hour << ":"
              << std::setw(2) << std::setfill('0') << ltm->tm_min << ":"
              << std::setw(2) << std::setfill('0') << ltm->tm_sec << "] ";
}


int init_Socket(int domain, int type, int protocol, char *port, char *interface) {
    int socket_fd;
    struct addrinfo hints, *res;

    // Create the socket
    socket_fd = socket(domain, type, protocol);
    if (socket_fd == -1) {
        perror("socket");
        return -1;
    }

    // Setup address hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = domain;
    hints.ai_socktype = type;

    // Get address info
    if (getaddrinfo(interface, port, &hints, &res) != 0) {
        perror("getaddrinfo");
        close(socket_fd);
        return -1;
    }

    int yes = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        close(socket_fd);
        freeaddrinfo(res);
        return -1;
    }
    // Bind to the address
    if (bind(socket_fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("bind");
        print_log("Failed to bind to " + std::string(interface) + ":" + std::string(port), DiSPLAY_LOG);
        close(socket_fd);
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return socket_fd;
}

std::string intToString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

bool check_listens(const std::vector<HostPort> &listens, const std::string &host, int port) {
    for (int i = 0; i < (int)listens.size(); i++) {
        if (listens[i].listen_host == host && listens[i].listen_port == port) {
            return true;
        }
    }
    return false;
}

std::vector<int> initListeningSockets(const Config &config, std::map<int, ConnectionInfo> &connections) {
        int socket_fd;
        std::vector<HostPort> listens;
        std::vector<int> listening_fds;
        for  (int i=0;i < (int)config.servers.size(); i++)
        {
            for (int j=0; j < (int)config.servers[i].listens.size(); j++)
            {
                if (check_listens(listens, config.servers[i].listens[j].listen_host, config.servers[i].listens[j].listen_port)) {
                    print_log("Warning: Duplicate listen directive for " + config.servers[i].listens[j].listen_host + ":" + 
                        intToString(config.servers[i].listens[j].listen_port), DiSPLAY_LOG);
                    continue; // Skip duplicate listens
                }
                socket_fd = init_Socket(AF_INET, SOCK_STREAM, 0, 
                    (char *)intToString(config.servers[i].listens[j].listen_port).c_str(),
                    (char *)config.servers[i].listens[j].listen_host.c_str());
                if (socket_fd < 0) {
                    print_log("Failed to initialize socket for " + config.servers[i].listens[j].listen_host + ":" + 
                        intToString(config.servers[i].listens[j].listen_port), DiSPLAY_LOG);
                    return std::vector<int>();
                }
                listens.push_back(config.servers[i].listens[j]);
                // Store the connection info
                connections[socket_fd] = ConnectionInfo(LISTENER, false);
                connections[socket_fd].port = config.servers[i].listens[j].listen_port;
                connections[socket_fd].host = config.servers[i].listens[j].listen_host;
                connections[socket_fd].server_ip = config.servers[i].listens[j].listen_host;
                connections[socket_fd].server_port = intToString(config.servers[i].listens[j].listen_port);

                // Set the socket to non-blocking mode
                int flags = fcntl(socket_fd, F_SETFL, O_NONBLOCK);
                if (flags < 0) {
                    perror("fcntl");
                    close(socket_fd);
                    return std::vector<int>();
                }

                if (listen(socket_fd, 5) < 0) 
                { 
                    perror("In listen"); 
                    exit(EXIT_FAILURE);
                    return std::vector<int>();
                }
                print_log("Listening on " + config.servers[i].listens[j].listen_host + ":" + intToString(config.servers[i].listens[j].listen_port), DiSPLAY_LOG);
                listening_fds.push_back(socket_fd);
            }
        }

        return listening_fds;
} 

ConnectionInfo::ConnectionInfo(Type t, bool ka) : type(t), keep_alive(ka), pos(0), is_old(false) {
    // Constructor implementation
}

ConnectionInfo::ConnectionInfo() : pos(0), is_old(false) {}

