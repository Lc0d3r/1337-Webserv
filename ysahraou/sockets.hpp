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

    ConnectionInfo() {};
    ConnectionInfo(Type t);
};


int init_Socket(int domain, int type, int protocol, char *port, char *interface);
std::string intToString(int value);
std::vector<int> initListeningSockets(const Config &config);