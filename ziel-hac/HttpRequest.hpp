#pragma once

#include "sockets.hpp"
#include "cgi_utils.hpp"


// e.g, GET /index.html/profile/name?key=value&key=value HTTP/1.1

struct HttpRequest {
    std::string method;          // e.g., "GET"
    std::string path;            // e.g., "/index.html"  
    std::string http_version;    // e.g., "HTTP/1.1"
    std::map<std::string, std::string> headers;
    std::string body; // the body

    const std::string& getExtension() const;
    const std::string& getQueryString() const;
    const std::string& getContentType() const;
    const std::string& getContentLength() const;
    const std::string& getBoundary() const;
    const std::string& getTransferEncoding() const;
    
};


int parse_req(std::string request_data, int socket_fd, HttpRequest &request);
void readHeaders(std::string &request_data, int new_socket);
void readBody(HttpRequest &request, std::string &str_body, int new_socket);