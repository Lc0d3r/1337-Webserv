#pragma once

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <map>
#include <sstream>  // for std::istringstream
#include <string>
#include <cstdio>
#include <vector>
#include <netdb.h>
#include <poll.h>
#include <fstream>
#include <fcntl.h>
#include <sys/types.h>
#include "../abel-baz/Config.hpp"

enum Type {
    LISTENER,  // Listening socket
    CONNECTED  // Connected socket
};

struct ConnectionInfo {
    Type type;
    bool keep_alive;
    time_t last_active;
    // if client
    int portToConnect;
    std::string hostToConnect;
    // if server
    int port;
    std::string host;
    ConnectionInfo() {};
    ConnectionInfo(Type t, bool ka);
};


int init_Socket(int domain, int type, int protocol, char *port, char *interface);
std::string intToString(int value);
std::vector<int> initListeningSockets(const Config &config, std::map<int, ConnectionInfo> &connections);