#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <map>
#include <sstream>  // for std::istringstream
#include <string>
#include <cstdio>
#include <vector>
#include <netdb.h>
#include "../ziel-hac/cgi_utils.hpp"

extern std::map<std::string, std::string> cookies_map;

// e.g, GET /index.html?key=value&key=value HTTP/1.1

struct HttpRequest {
    std::string method;          // e.g., "GET"
    std::string path;            // e.g., "/index.html"
    std::string path_without_query; // e.g., "/index.html" without query string
    std::string http_version;    // e.g., "HTTP/1.1"
    bool is_keep_alive; // true if the connection should be kept alive
    std::map<std::string, std::string> headers;
    std::string body; // the body

    // error pages
    std::map<int, std::string> error_pages; // e.g., 404 => "/404.html"
    //////////
    bool in_progress;
    bool done;
    int byte_readed;
    int content_length;

    HttpRequest();
    //////////
    std::string getExtension() const;
    std::string getQueryString() const;
    std::string getContentType() const;
    std::string getContentLength() const;
    std::string getBoundary() const;
    std::string getTransferEncoding() const;
    std::string getCookie() const;
    std::string getSessionId() const;
};

void removeQueryString(HttpRequest &request);
int parse_req(std::string request_data, int socket_fd, HttpRequest &request);
void readHeaders(std::string &request_data, int new_socket);
bool readBody(HttpRequest &request, std::string &str_body, int new_socket);