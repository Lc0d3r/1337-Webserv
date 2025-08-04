#pragma once
#include <string>
#include <map>
#include <vector>
#include "HttpRequest.hpp"
#include "sockets.hpp"
#include "../abel-baz/Router.hpp"
#include "../ziel-hac/cgi.hpp"
#include "../ziel-hac/post.hpp"
#include <dirent.h>
#include <fstream>
#include <cstdlib>
#include <ctime>   




#define CHUNK_SIZE 1024 * 1024 // 1 MB
#define MAX_TO_SEND 1024 * 64 // 64 KB


struct HttpResponse {
    std::string httpVersion;
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::vector<char> body;   // For PDFs, images, etc.
    
    HttpResponse() : httpVersion("HTTP/1.1"), statusCode(200), statusMessage("OK") {}
    
    HttpResponse(int code, const std::string& message)
    : httpVersion("HTTP/1.1"), statusCode(code), statusMessage(message) {}
    
    void addHeader(const std::string& key, const std::string& value) ;
    
    void setTextBody(const std::string& content);

    std::string setSessionId();

    std::string toString() const;
};


bool response(int client_fd,HttpRequest &request, Config &config, ConnectionInfo &connections);
void splithostport(const std::string& host, std::string& hostname, int& port);
bool resumeSending(ConnectionInfo& connections, std::vector<char> &buffer, int client_fd);
bool get_error_page(HttpResponse &response, int error_code, const HttpRequest &request, std::string error_message);
