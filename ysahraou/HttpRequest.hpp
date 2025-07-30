#pragma once

#include "sockets.hpp"
#include "../ziel-hac/cgi_utils.hpp"


// e.g, GET /index.html?key=value&key=value HTTP/1.1

struct HttpRequest {
    std::string method;          // e.g., "GET"
    std::string path;            // e.g., "/index.html"
    std::string path_without_query; // e.g., "/index.html" without query string
    std::string http_version;    // e.g., "HTTP/1.1"
    bool is_keep_alive; // true if the connection should be kept alive
    std::map<std::string, std::string> headers;
    std::string body; // the body

    std::string getExtension() const;
    std::string getQueryString() const;
    std::string getContentType() const;
    std::string getContentLength() const;
    std::string getBoundary() const;
    std::string getTransferEncoding() const;
};

void removeQueryString(HttpRequest &request);
int parse_req(std::string request_data, int socket_fd, HttpRequest &request);
void readHeaders(std::string &request_data, int new_socket);
void readBody(HttpRequest &request, std::string &str_body, int new_socket);