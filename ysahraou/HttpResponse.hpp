#include <string>
#include <map>
#include <vector>
#include "HttpRequest.hpp"
#include "sockets.hpp"
#include "../abel-baz/Router.hpp"

struct HttpResponse {
    std::string httpVersion;
    int statusCode;
    std::string statusMessage;
    std::string connection;
    std::string keep_alive;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse() : httpVersion("HTTP/1.1"), statusCode(200), statusMessage("OK") {}

    HttpResponse(int code, const std::string& message)
        : httpVersion("HTTP/1.1"), statusCode(code), statusMessage(message) {}

    void addHeader(const std::string& key, const std::string& value) ;

    void setBody(const std::string& content);

    std::string toString() const;
};


void response(int client_fd,HttpRequest &request, Config &config);
void splithostport(const std::string& host, std::string& hostname, int& port);