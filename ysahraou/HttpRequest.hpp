#pragma once

#include "sockets.hpp"


// e.g, GET /index.html?key=value&key=value HTTP/1.1

struct HttpRequest {
    std::string method;          // e.g., "GET"
    std::string path;            // e.g., "/index.html"  
    std::string http_version;    // e.g., "HTTP/1.1"
    bool is_keep_alive; // true if the connection should be kept alive
    std::map<std::string, std::string> headers;
    std::string body; // the body
};


int parse_req(std::string request_data, int socket_fd, HttpRequest &request);
void readHeaders(std::string &request_data, int new_socket);
void readBody(HttpRequest &request, std::string &str_body, int new_socket);