#pragma once
#include <string>
#include <map>
#include <vector>
#include "HttpRequest.hpp"
#include "sockets.hpp"
#include "../abel-baz/Router.hpp"

#define CHUNK_SIZE 1024 * 1024 // 1 MB
#define MAX_TO_SEND 1024 * 64
struct HttpResponse {
    std::string httpVersion;
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string text_body;            // For text/html, JSON, etc.
    std::vector<char> binary_body;   // For PDFs, images, etc.
    bool is_binary;                  // Flag to know which one to send

    HttpResponse() : httpVersion("HTTP/1.1"), statusCode(200), statusMessage("OK") {}

    HttpResponse(int code, const std::string& message)
        : httpVersion("HTTP/1.1"), statusCode(code), statusMessage(message) {}

    void addHeader(const std::string& key, const std::string& value) ;

    void setTextBody(const std::string& content);

    std::string toString() const;
};


void response(int client_fd,HttpRequest &request, Config &config, ConnectionInfo &connections);
void splithostport(const std::string& host, std::string& hostname, int& port);
bool resumeSending(ConnectionInfo& connections, std::vector<char> &buffer, int client_fd);