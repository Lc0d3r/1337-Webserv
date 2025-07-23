#include <string>
#include <map>
#include <vector> // Included for completeness, though not strictly used in current body implementation

struct HttpResponse {
    std::string httpVersion;
    int statusCode;
    std::string statusMessage;
    std::map<std::string, std::string> headers;
    std::string body;

    HttpResponse() : httpVersion("HTTP/1.1"), statusCode(200), statusMessage("OK") {}

    HttpResponse(int code, const std::string& message)
        : httpVersion("HTTP/1.1"), statusCode(code), statusMessage(message) {}

    void addHeader(const std::string& key, const std::string& value) ;

    void setBody(const std::string& content);

    std::string toString() const;
};