#include "HttpResponse.hpp"
#include "sockets.hpp"

std::string HttpResponse::toString() const {
        std::string responseString = httpVersion + " " + intToString(statusCode) + " " + statusMessage + "\r\n";
        std::map<std::string, std::string> tempHeaders = headers;

        // loop through headers and append them to the response string in c++ 98 style
        for (int i = 0; i < (int)tempHeaders.size(); ++i) {
            responseString += tempHeaders.begin()->first + ": " + tempHeaders.begin()->second + "\r\n";
            tempHeaders.erase(tempHeaders.begin());
        }

        responseString += "\r\n";
        responseString += body;

        return responseString;
};


void HttpResponse::setBody(const std::string& content) {
        body = content;
        if (!body.empty() || statusCode == 200) {
            addHeader("Content-Length", intToString(body.length()));
        } else {
            headers.erase("Content-Length");
        }
};

void HttpResponse::addHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
}

void appandtostring(std::string& str, const std::string& content) {
    str += content;
}
