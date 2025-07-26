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
bool generateAutoIndex(const std::string& dirPath, const std::string& requestPath, std::string& body) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        std::cerr << "Failed to open directory: " << dirPath << std::endl;
        return false;
    }
    struct dirent* entry;
    body = "<!DOCTYPE html>\n"
           "<html>\n"
           "<head>\n"
           "<meta charset=\"UTF-8\">\n"
           "<title>Index of " + requestPath + "</title>\n"
           "<style>\n"
           "body { background: #f8f9fa; font-family: Arial, sans-serif; }\n"
           ".container { max-width: 600px; margin: 60px auto; background: #fff; border-radius: 10px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); padding: 30px; text-align: center; }\n"
           "h1 { color: #007bff; margin-bottom: 30px; }\n"
           "ul { list-style: none; padding: 0; }\n"
           "li { margin: 12px 0; }\n"
           "a { text-decoration: none; color: #333; font-size: 18px; transition: color 0.2s; }\n"
           "a:hover { color: #007bff; }\n"
           "</style>\n"
           "</head>\n"
           "<body>\n"
           "<div class=\"container\">\n"
           "<h1>Index of " + requestPath + "</h1>\n"
           "<ul>\n";
    while ((entry = readdir(dir)) != NULL) {
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == "..") continue;
        std::string href = requestPath;
        if (href.empty() || href[href.size() - 1] != '/')
            href += "/";
        href += entryName;
        body += "<li><a href=\"" + href + "\">" + entryName + "</a></li>\n";
    }
    body += "</ul>\n</div>\n</body>\n</html>\n";
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
        if (request.is_keep_alive) {
            response.addHeader("Connection", "keep-alive");
        } else {
            response.addHeader("Connection", "close");
        }
        return;
    }
    if (result.use_autoindex) {
        std::cout << "Autoindex enabled for: " << result.file_path << std::endl;
        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        std::string body;
        std::cout << "result.file_path = " << result.file_path << std::endl;
        if (generateAutoIndex(result.file_path, request.path, body)) {
            response.setBody(body);
            response.addHeader("Content-Type", "text/html");
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
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
    std::cout << "file path: " << result.file_path << std::endl;
}

void response(int client_fd, HttpRequest &request, Config &config)
{
    (void)request;
    (void)config;
    printf("sending... === ===== === \n");
    HttpResponse response(200, "OK");
    handleGETRequest(response, request, config);

    // sending the response
    std::cout << "strlen(response) = " << strlen(response.toString().c_str()) << std::endl;
    write(client_fd , response.toString().c_str() , strlen(response.toString().c_str()));
}
