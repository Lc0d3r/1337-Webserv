#pragma once

#include "sockets.hpp"


// e.g, GET /index.html?key=value&key=value HTTP/1.1

struct HttpRequest {
    std::string method;          // e.g., "GET"
    std::string path;            // e.g., "/index.html"  
    std::string http_version;    // e.g., "HTTP/1.1"
    std::map<std::string, std::string> headers;
    std::string body; // the body
};


int parse_req(std::string request_data, int socket_fd, HttpRequest &request);