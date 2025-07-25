#include "HttpResponse.hpp"
#include "sockets.hpp"
#include <dirent.h>
#include <fstream>

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

void splithostport(const std::string& host, std::string& hostname, int& port) {
    size_t colonPos = host.find(':');
    hostname = host.substr(0, colonPos);
    port = std::atoi(host.substr(colonPos + 1).c_str());
}

// generate autoindex HTML for a directory
bool generateAutoIndex(const std::string& dirPath, std::string& body) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        std::cerr << "Failed to open directory: " << dirPath << std::endl;
        return false;
    }
    struct dirent* entry;
    body = "<html><body><h1>Index of " + dirPath + "</h1><ul>";
    while ((entry = readdir(dir)) != NULL) {
        std::cout << "entry->d_name = " << entry->d_name << std::endl;
        // if (entry->d_name[0] == '.') continue; // skip hidden files
        std::string entryName = entry->d_name;
        std::string href = entryName;
        // If dirPath ends with '/', don't add another '/'
        if (dirPath[dirPath.size() - 1] == '/')
            href = dirPath + entryName;
        else
            href = dirPath + "/" + entryName;
        body += "<li><a href=\"" + href + "\">" + entryName + "</a></li>";
    }
    body += "</ul></body></html>";
    closedir(dir);
    return true;
}

bool readindexFile(const std::string& filePath, std::string& body) {
    std::ifstream file;
    file.open(filePath.c_str());
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    body = buffer.str();
    file.close();
    if (body.empty()) {
        std::cerr << "File is empty: " << filePath << std::endl;
        return false;
    }
    return true;
}

void handleGETRequest(HttpResponse& response, const HttpRequest& request, const Config& config) {
    (void)response;
    int port;
    std::string hostname;
    splithostport(request.headers.at("Host"), hostname, port);
    errorType error = NO_ERROR;
    RoutingResult result = routingResult(config, hostname, port, request.path, request.method, error);
    if (result.is_redirect) {
        response.statusCode = 301; // Moved Permanently
        response.statusMessage = "Moved Permanently";
        response.addHeader("Location", result.redirect_url);
        return;
    }
    if (result.use_autoindex) {
        std::cout << "Autoindex enabled for: " << result.file_path << std::endl;
        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        std::string body;
        std::cout << "result.file_path = " << result.file_path << std::endl;
        if (generateAutoIndex(result.file_path, body)) {
            response.setBody(body);
            response.addHeader("Content-Type", "text/html");
        } else {
            response.statusCode = 500; // Internal Server Error
            response.statusMessage = "Internal Server Error";
            response.setBody("<h1>500 Internal Server Error</h1>");
        }
        return;
    }
    else {
        std::cout << "Not using autoindex for: " << result.file_path << std::endl;
        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        std::string body;
        if (readindexFile(result.file_path, body)) {
            response.setBody(body);
            response.addHeader("Content-Type", "text/html");
        } else {
            response.statusCode = 404; // Not Found
            response.statusMessage = "Not Found";
            response.setBody("<h1>404 Not Found</h1>");
        }
    }
}

void response(int client_fd, HttpRequest &request, Config &config)
{
    (void)request;
    (void)config;
    printf("sending... === ===== === \n");
    HttpResponse response(200, "OK");
    handleGETRequest(response, request, config);
    // read file index.html put it in the body
    // std::fstream file("www/index.html");
    // std::string body;
    // if (file.is_open()) {
    //     std::string line;
    //     while (std::getline(file, line)) {
    //         body += line + "\n";
    //     }
    //     file.close();
    // } else {
    //     std::cerr << "Unable to open file" << std::endl;
    //     response.statusCode = 404;
    //     response.statusMessage = "Not Found";
    //     body = "<h1>404 Not Found</h1>";
    // }
    // response.setBody(body);
    // response.addHeader("Content-Type", "text/html");
    // response.addHeader("Connection", "close");
    std::cout << "strlen(response) = " << strlen(response.toString().c_str()) << std::endl;
    write(client_fd , response.toString().c_str() , strlen(response.toString().c_str()));
}
