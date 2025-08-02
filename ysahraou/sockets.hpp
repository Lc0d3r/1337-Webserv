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
#include "HttpRequest.hpp"
#include <iostream>
#include <ctime>
#include <iomanip>

// Color macros
#define COLOR_RED    "\033[31m"
#define COLOR_GREEN  "\033[32m"
#define COLOR_RESET  "\033[0m"

enum Type {
    LISTENER,  // Listening socket
    CONNECTED  // Connected socket
};

struct ConnectionInfo {
    std::string server_ip;
    std::string server_port;
    Type type;
    bool keep_alive;
    time_t last_active;
    // if client
    int portToConnect;
    std::string hostToConnect;
    // if server
    int port;
    std::string host;
    std::streamsize pos;
    bool is_old;
    std::string file_path;


    // request info
    HttpRequest request;


    ConnectionInfo() ;
    ConnectionInfo(Type t, bool ka);
};


int init_Socket(int domain, int type, int protocol, char *port, char *interface);
std::string intToString(int value);
std::vector<int> initListeningSockets(const Config &config, std::map<int, ConnectionInfo> &connections);
void log_time();