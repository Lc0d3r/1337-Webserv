#include "HttpResponse.hpp"
#include "sockets.hpp"
#include <dirent.h>
#include <fstream>
#include "../ziel-hac/post.hpp"

std::string HttpResponse::toString() const {
        std::string responseString = httpVersion + " " + intToString(statusCode) + " " + statusMessage + "\r\n";
        std::map<std::string, std::string> tempHeaders = headers;

        // loop through headers and append them to the response string in c++ 98 style
        for (int i = 0; i < (int)tempHeaders.size(); ++i) {
            responseString += tempHeaders.begin()->first + ": " + tempHeaders.begin()->second + "\r\n";
            tempHeaders.erase(tempHeaders.begin());
        }

        responseString += "\r\n";

        return responseString;
};

void HttpResponse::setTextBody(const std::string& content) {
    text_body = content;
    is_binary = false; // Set the flag to indicate this is a text response
}

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

std::string check_file_format(const std::string& file_path) {
    size_t pos = file_path.find_last_of('.');
    if (pos == std::string::npos) {
        return ""; // No extension found
    }
    return file_path.substr(pos + 1);
}

int readbinaryortext(const std::string& format) {
    if (format == "mp4" || format == "gpg" || format == "png" || format == "jpeg" || format == "jpg" || format == "gif" || format == "ico") {
        return 1; // Binary
    } else if (format == "html" || format == "txt" || format == "css" || format == "js" || format == "json") {
        return 0; // Text
    }
    return -1; // Unsupported format
}

std::streamsize Check_file_size(const std::string& file_path) {
    std::ifstream file(file_path.c_str(), std::ios::binary );
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return 0;
    }
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size <= 0) {
        std::cerr << "File is empty or error reading size: " << file_path << std::endl;
        return 0;
    }
    std::cout << "File size: " << size * 1e-6 << " MB" << std::endl;
    return size;
}

bool read_file(const std::string& file_path, HttpResponse& response) {
    Check_file_size(file_path);
    if (readbinaryortext(check_file_format(file_path)) == 0) {
        std::ifstream file(file_path.c_str());
        if (!file.is_open()) {
            std::cerr << "Failed to open text file: " << file_path << std::endl;
            return false;
        }
        std::cout << "Reading text file: " << file_path << std::endl;
        std::stringstream buffer;
        buffer << file.rdbuf();
        response.setTextBody(buffer.str());
        response.is_binary = false; // Mark the response as text
        return true;
    }
    else if (readbinaryortext(check_file_format(file_path)) == 1 || readbinaryortext(check_file_format(file_path)) == -1) {
        std::ifstream file(file_path.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open binary file: " << file_path << std::endl;
            return false;
        }
        std::cout << "Reading binary file: " << file_path << std::endl;
        // Go to the end to get the size
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size <= 0) {
            std::cerr << "binary file is empty or error reading size: " << file_path << std::endl;
            return false;
        }

        response.binary_body.resize(size); // Allocate space
        if (!file.read(response.binary_body.data(), size)) {
            std::cerr << "Error reading binary file: " << file_path << std::endl;
            return false;
        }

        response.is_binary = true; // Mark the response as binary
        std::cout << "binary file binary successfully: " << file_path << " (" << size * 1e-6 << " MB)" << std::endl;
        if (readbinaryortext(check_file_format(file_path)) == -1) {
            std::cout << "Adding Content-Disposition header for file download: " << file_path << std::endl;
            response.addHeader("Content-disposition", "attachment; filename=\"" + file_path.substr(file_path.find_last_of('/') + 1) + "\"");
        }
        return true;
    }
    else {
        std::cerr << "Unsupported file format: " << file_path << std::endl;
        return false;
    }
}

bool resumeSending(ConnectionInfo connections, std::vector<char> &buffer) {
    if (readbinaryortext(check_file_format(connections.file_path)) == 1 || readbinaryortext(check_file_format(connections.file_path)) == -1) {
        std::cout << "Resuming binary file sending: " << connections.file_path << std::endl;
        if (connections.pos < Check_file_size(connections.file_path)) {
            std::ifstream file(connections.file_path.c_str(), std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "Failed to open binary file: " << connections.file_path << std::endl;
                return false;
            }
            file.seekg(connections.pos);
            // Implement binary file sending logic here
            file.read(buffer.data(), buffer.size());
            connections.pos += file.gcount();
        }
    } else {
        std::cout << "Resuming text file sending: " << connections.file_path << std::endl;
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
            response.setTextBody(body);
            response.addHeader("Content-Type", "text/html");
            response.addHeader("Content-Length", intToString(body.length()));
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        } else {
            response.statusCode = 500; // Internal Server Error
            response.statusMessage = "Internal Server Error";
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
            response.setTextBody("<h1>500 Internal Server Error</h1>");
        }
        return;
    }
    else {
        std::cout << "Not using autoindex for: " << result.file_path << std::endl;
        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        std::string body;

        // Check the file format and read the file accordingly
        if (read_file(result.file_path, response)) {
            if (check_file_format(result.file_path) == "html" || 
                check_file_format(result.file_path) == "txt" || 
                check_file_format(result.file_path) == "css") { 
                response.addHeader("Content-Type", "application/" + check_file_format(result.file_path));
            } else if (check_file_format(result.file_path) == "js" || 
                       check_file_format(result.file_path) == "json") {
                response.addHeader("Content-Type", "application/" + check_file_format(result.file_path));
            } else if (check_file_format(result.file_path) == "gpg" || 
                       check_file_format(result.file_path) == "png" || 
                       check_file_format(result.file_path) == "jpeg" || 
                       check_file_format(result.file_path) == "jpg" || 
                       check_file_format(result.file_path) == "gif") {
                response.addHeader("Content-Type", "image/" + check_file_format(result.file_path));
            } else if (check_file_format(result.file_path) == "mp4") {
                response.addHeader("Content-Type", "video/mp4");
                if (response.is_binary)
                    response.addHeader("Content-Length", intToString(response.binary_body.size()));
                else
                    response.addHeader("Content-Length", intToString(response.text_body.length()));
            } 
        }
        else {
                response.statusCode = 404; // Not Found
                response.statusMessage = "Not Found";
                response.addHeader("Content-Type", "text/html");
                response.setTextBody("<h1>404 Not Found</h1>");
                response.addHeader("Content-Length", intToString(response.text_body.length()));
            }
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
            response.addHeader("Connection", "close");
        }
    // std::cout << "file path: " << result.file_path << std::endl;
    }
}

void response(int client_fd, HttpRequest &request, Config &config)
{
    (void)request;
    (void)config;
    printf("===========\[sending...]===========\n");
    HttpResponse response(200, "OK");
    if (request.method == "GET") {
        handleGETRequest(response, request, config);
    } else if (request.method == "POST") {
        errorType error = NO_ERROR;
        int port;
        std::string hostname;
        splithostport(request.headers.at("Host"), hostname, port);
        RoutingResult routing_result = routingResult(config, hostname, port, request.path, request.method, error);
        if (error == NO_ERROR) {
            posthandler(&request, &routing_result, response);
        }
    }

    // sending the response
    std::cout << "strlen(response) = " << strlen(response.toString().c_str()) << std::endl;
    write(client_fd , response.toString().c_str() , strlen(response.toString().c_str()));
    if (response.is_binary) {
        write(client_fd, response.binary_body.data(), response.binary_body.size());
    }
    else {
        write(client_fd, response.text_body.c_str(), response.text_body.length());
    }
    std::cout << "===========\[response sent]===========\n";
}
